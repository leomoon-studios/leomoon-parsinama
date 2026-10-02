#include <QByteArrayView>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "data/SearchNormalizer.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <memory>

namespace {

[[noreturn]] void fail(const QString &message)
{
    throw std::runtime_error(message.toUtf8().toStdString());
}

struct JsonFile
{
    QJsonObject object;
    QByteArray bytes;
};

JsonFile readObject(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        fail(QStringLiteral("Cannot read %1: %2").arg(path, file.errorString()));
    }
    JsonFile result;
    result.bytes = file.readAll();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(result.bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        fail(QStringLiteral("Invalid JSON object in %1: %2").arg(path, parseError.errorString()));
    }
    result.object = document.object();
    return result;
}

QString textField(const QJsonObject &object, const char *field, const QString &path)
{
    const QJsonValue value = object.value(QLatin1String(field));
    if (!value.isString() || value.toString().isEmpty()) {
        fail(QStringLiteral("Missing or invalid %1 in %2").arg(QLatin1String(field), path));
    }
    return value.toString();
}

qint64 idField(const QJsonObject &object, const char *field, const QString &path)
{
    const QJsonValue value = object.value(QLatin1String(field));
    if (!value.isDouble() || value.toInteger(-1) <= 0) {
        fail(QStringLiteral("Missing or invalid %1 in %2").arg(QLatin1String(field), path));
    }
    return value.toInteger();
}

QJsonArray arrayField(const QJsonObject &object, const char *field, const QString &path)
{
    const QJsonValue value = object.value(QLatin1String(field));
    if (!value.isArray()) {
        fail(QStringLiteral("Missing or invalid %1 in %2").arg(QLatin1String(field), path));
    }
    return value.toArray();
}

void sql(QSqlDatabase &database, const QString &statement,
         const QVariantList &values = {})
{
    QSqlQuery query(database);
    if (values.isEmpty()) {
        if (!query.exec(statement)) {
            fail(QStringLiteral("SQLite error: %1; %2")
                     .arg(query.lastError().text(), statement));
        }
        return;
    }
    if (!query.prepare(statement)) {
        fail(QStringLiteral("SQLite prepare error: %1").arg(query.lastError().text()));
    }
    for (const QVariant &value : values) {
        query.addBindValue(value);
    }
    if (!query.exec()) {
        fail(QStringLiteral("SQLite insert error: %1").arg(query.lastError().text()));
    }
}

struct SourceDigest
{
    QByteArray sha256;
    qint64 files = 0;
    qint64 poets = 0;
    qint64 categories = 0;
    qint64 poems = 0;
};

class CatalogBuilder
{
public:
    CatalogBuilder(const QString &sourceRoot, const QString &outputPath)
        : m_root(QDir(sourceRoot).canonicalPath()),
          m_poetsRoot(QDir(m_root).filePath(QStringLiteral("poets"))),
          m_poetsCanonical(QDir(m_poetsRoot).canonicalPath())
    {
        if (m_root.isEmpty() || m_poetsCanonical.isEmpty()
            || !QFileInfo::exists(QDir(m_root).filePath(QStringLiteral("manifest.json")))) {
            fail(QStringLiteral("Invalid data root: %1").arg(sourceRoot));
        }
        m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), QStringLiteral("catalog_builder"));
        m_database.setDatabaseName(outputPath);
        try {
            if (!m_database.open()) {
                fail(QStringLiteral("Cannot create catalog: %1").arg(m_database.lastError().text()));
            }
            sql(m_database, QStringLiteral("PRAGMA foreign_keys = ON"));
            sql(m_database, QStringLiteral("PRAGMA journal_mode = OFF"));
            sql(m_database, QStringLiteral("PRAGMA synchronous = OFF"));
            createSchema();
            m_searchInsert = std::make_unique<QSqlQuery>(m_database);
            if (!m_searchInsert->prepare(QStringLiteral("INSERT INTO search_fts "
                    "(entry_type, full_url, poet_url, category_url, title, context, "
                    "original_text, normalized_text) VALUES (?, ?, ?, ?, ?, ?, ?, ?)"))) {
                fail(QStringLiteral("Cannot prepare search index: %1").arg(m_searchInsert->lastError().text()));
            }
        } catch (...) {
            m_searchInsert.reset();
            m_database.close();
            m_database = QSqlDatabase();
            QSqlDatabase::removeDatabase(QStringLiteral("catalog_builder"));
            throw;
        }
    }

    ~CatalogBuilder()
    {
        m_searchInsert.reset();
        m_database.close();
        m_database = QSqlDatabase();
        QSqlDatabase::removeDatabase(QStringLiteral("catalog_builder"));
    }

    void build();

private:
    void createSchema();
    QStringList urlSegments(const QString &fullUrl) const;
    QString contentPath(const QString &fullUrl, const QString &slug, bool category) const;
    void importPoet(const QJsonObject &reference, qsizetype position);
    qint64 importCategory(const QString &url, const QString &slug, qint64 poetId,
                          qint64 parentId, int depth);
    void importPoem(const QJsonObject &reference, const QString &slug,
                    qint64 poetId, qint64 categoryId, const QString &categoryUrl);
    SourceDigest hashSources() const;
    void metadata(const QString &key, const QString &value);
    void searchEntry(const QString &type, const QString &url, const QString &poetUrl,
                     const QString &categoryUrl, const QString &title,
                     const QString &context, const QString &original);

    QString m_root;
    QString m_poetsRoot;
    QString m_poetsCanonical;
    QSqlDatabase m_database;
    QSet<qint64> m_poetIds;
    QSet<qint64> m_categoryIds;
    QSet<qint64> m_poemIds;
    QString m_poetName;
    QString m_poetUrl;
    QString m_categoryTitle;
    std::unique_ptr<QSqlQuery> m_searchInsert;
};

void CatalogBuilder::createSchema()
{
    sql(m_database, QStringLiteral("CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)"));
    sql(m_database, QStringLiteral("CREATE TABLE poets ("
                                   "id INTEGER PRIMARY KEY, sort_order INTEGER NOT NULL UNIQUE, "
                                   "slug TEXT NOT NULL UNIQUE, name TEXT NOT NULL, nickname TEXT NOT NULL, "
                                   "full_url TEXT NOT NULL UNIQUE, description TEXT, data_json BLOB NOT NULL)"));
    sql(m_database, QStringLiteral("CREATE TABLE categories ("
                                   "id INTEGER PRIMARY KEY, poet_id INTEGER NOT NULL REFERENCES poets(id), "
                                   "parent_id INTEGER REFERENCES categories(id), title TEXT NOT NULL, "
                                   "full_url TEXT NOT NULL UNIQUE, description TEXT, book_name TEXT, "
                                   "data_json BLOB NOT NULL)"));
    sql(m_database, QStringLiteral("CREATE TABLE category_children ("
                                   "parent_id INTEGER NOT NULL REFERENCES categories(id), "
                                   "child_id INTEGER NOT NULL UNIQUE REFERENCES categories(id), "
                                   "sort_order INTEGER NOT NULL, PRIMARY KEY(parent_id, sort_order))"));
    sql(m_database, QStringLiteral("CREATE TABLE poems ("
                                   "id INTEGER PRIMARY KEY, poet_id INTEGER NOT NULL REFERENCES poets(id), "
                                   "cat_id INTEGER NOT NULL REFERENCES categories(id), title TEXT NOT NULL, "
                                   "full_title TEXT, full_url TEXT NOT NULL UNIQUE, "
                                   "data_json BLOB NOT NULL)"));
    sql(m_database, QStringLiteral("CREATE TABLE category_poems ("
                                   "category_id INTEGER NOT NULL REFERENCES categories(id), "
                                   "poem_id INTEGER NOT NULL UNIQUE REFERENCES poems(id), "
                                   "sort_order INTEGER NOT NULL, PRIMARY KEY(category_id, sort_order))"));
    sql(m_database, QStringLiteral("CREATE INDEX categories_by_poet ON categories(poet_id)"));
    sql(m_database, QStringLiteral("CREATE INDEX poems_by_category ON poems(cat_id)"));
    sql(m_database, QStringLiteral("CREATE VIRTUAL TABLE search_fts USING fts5("
                                   "entry_type UNINDEXED, full_url UNINDEXED, poet_url UNINDEXED, "
                                   "category_url UNINDEXED, title UNINDEXED, context UNINDEXED, "
                                   "original_text UNINDEXED, normalized_text, "
                                   "tokenize='unicode61 remove_diacritics 0')"));
}

void CatalogBuilder::searchEntry(const QString &type, const QString &url,
                                 const QString &poetUrl, const QString &categoryUrl,
                                 const QString &title, const QString &context,
                                 const QString &original)
{
    const QString normalized = SearchNormalizer::normalize(original);
    if (normalized.isEmpty()) return;
    m_searchInsert->bindValue(0, type);
    m_searchInsert->bindValue(1, url);
    m_searchInsert->bindValue(2, poetUrl);
    m_searchInsert->bindValue(3, categoryUrl);
    m_searchInsert->bindValue(4, title);
    m_searchInsert->bindValue(5, context);
    m_searchInsert->bindValue(6, original);
    m_searchInsert->bindValue(7, normalized);
    if (!m_searchInsert->exec()) {
        fail(QStringLiteral("Cannot index search text: %1").arg(m_searchInsert->lastError().text()));
    }
}

QStringList CatalogBuilder::urlSegments(const QString &url) const
{
    if (!url.startsWith(QLatin1Char('/')) || url.size() < 2) {
        fail(QStringLiteral("Invalid FullUrl: %1").arg(url));
    }
    const QStringList segments = url.mid(1).split(QLatin1Char('/'));
    for (const QString &segment : segments) {
        if (segment.isEmpty() || segment == QLatin1String(".")
            || segment == QLatin1String("..") || segment.contains(QLatin1Char('\\'))
            || segment.contains(QLatin1Char('?')) || segment.contains(QLatin1Char('#'))) {
            fail(QStringLiteral("Unsafe FullUrl: %1").arg(url));
        }
        for (const QChar character : segment) {
            if (character.isNull() || character.isSpace() || character.isHighSurrogate()
                || character.isLowSurrogate()) {
                fail(QStringLiteral("Unsafe FullUrl: %1").arg(url));
            }
        }
    }
    return segments;
}

QString CatalogBuilder::contentPath(const QString &url, const QString &slug, bool category) const
{
    const QStringList segments = urlSegments(url);
    if (segments.constFirst() != slug) {
        fail(QStringLiteral("FullUrl belongs to another poet: %1").arg(url));
    }
    const QString relative = segments.join(QLatin1Char('/'))
        + (category ? QStringLiteral("/_cat.json") : QStringLiteral(".json"));
    const QString path = QDir(m_poetsRoot).filePath(relative);
    const QString canonical = QFileInfo(path).canonicalFilePath();
    if (canonical.isEmpty()) {
        fail(QStringLiteral("Missing referenced file: %1").arg(path));
    }
    if (!canonical.startsWith(m_poetsCanonical + QDir::separator())) {
        fail(QStringLiteral("Referenced file is outside data/poets: %1").arg(path));
    }
    return canonical;
}

void CatalogBuilder::importPoet(const QJsonObject &reference, qsizetype position)
{
    const QString source = QStringLiteral("manifest poet %1").arg(position);
    const qint64 id = idField(reference, "Id", source);
    const QString url = textField(reference, "FullUrl", source);
    const QStringList segments = urlSegments(url);
    if (segments.size() != 1 || m_poetIds.contains(id)) {
        fail(QStringLiteral("Duplicate or invalid poet: %1").arg(url));
    }
    const QString slug = segments.constFirst();
    const QString path = QDir(m_poetsRoot).filePath(slug + QStringLiteral("/poet.json"));
    const QString canonical = QFileInfo(path).canonicalFilePath();
    if (canonical.isEmpty() || !canonical.startsWith(m_poetsCanonical + QDir::separator())) {
        fail(QStringLiteral("Missing or unsafe poet file: %1").arg(path));
    }
    const JsonFile file = readObject(canonical);
    if (idField(file.object, "Id", canonical) != id
        || textField(file.object, "FullUrl", canonical) != url
        || textField(file.object, "Nickname", canonical) != textField(reference, "Nickname", source)) {
        fail(QStringLiteral("Poet metadata disagrees with manifest: %1").arg(path));
    }
    sql(m_database, QStringLiteral("INSERT INTO poets "
                                   "(id, sort_order, slug, name, nickname, full_url, description, data_json) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"),
        {id, position, slug, textField(file.object, "Name", canonical),
         textField(file.object, "Nickname", canonical), url,
         file.object.value(QStringLiteral("Description")).toString(), file.bytes});
    m_poetIds.insert(id);
    m_poetName = textField(file.object, "Name", canonical);
    m_poetUrl = url;
    searchEntry(QStringLiteral("poet"), url, url, url, m_poetName, QString{},
                m_poetName + QLatin1Char(' ') + textField(file.object, "Nickname", canonical));
    importCategory(url, slug, id, 0, 0);
}

qint64 CatalogBuilder::importCategory(const QString &url, const QString &slug,
                                      qint64 poetId, qint64 parentId, int depth)
{
    if (depth > 128) {
        fail(QStringLiteral("Category nesting exceeds 128 levels: %1").arg(url));
    }
    const QString path = contentPath(url, slug, true);
    const JsonFile file = readObject(path);
    const qint64 id = idField(file.object, "Id", path);
    if (m_categoryIds.contains(id) || textField(file.object, "FullUrl", path) != url
        || idField(file.object, "PoetId", path) != poetId) {
        fail(QStringLiteral("Duplicate or inconsistent category: %1").arg(path));
    }
    if (parentId != 0 && idField(file.object, "ParentId", path) != parentId) {
        fail(QStringLiteral("Wrong ParentId in %1").arg(path));
    }
    if (parentId == 0 && file.object.contains(QStringLiteral("ParentId"))
        && !file.object.value(QStringLiteral("ParentId")).isNull()) {
        fail(QStringLiteral("Poet root has an unexpected ParentId: %1").arg(path));
    }
    sql(m_database, QStringLiteral("INSERT INTO categories "
                                   "(id, poet_id, parent_id, title, full_url, description, book_name, data_json) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"),
        {id, poetId, parentId ? QVariant(parentId) : QVariant(),
         textField(file.object, "Title", path), url,
         file.object.value(QStringLiteral("Description")).toString(),
         file.object.value(QStringLiteral("BookName")).toString(), file.bytes});
    m_categoryIds.insert(id);
    const QString categoryTitle = textField(file.object, "Title", path);
    if (parentId != 0) {
        searchEntry(QStringLiteral("collection"), url, m_poetUrl, url, categoryTitle,
                    m_poetName, categoryTitle);
    }
    const QString previousCategoryTitle = m_categoryTitle;
    m_categoryTitle = categoryTitle;

    const QJsonArray children = arrayField(file.object, "ChildCats", path);
    for (qsizetype order = 0; order < children.size(); ++order) {
        if (!children.at(order).isObject()) {
            fail(QStringLiteral("Invalid ChildCats entry in %1").arg(path));
        }
        const QJsonObject reference = children.at(order).toObject();
        const qint64 childId = idField(reference, "Id", path);
        const QString childUrl = textField(reference, "FullUrl", path);
        if (!childUrl.startsWith(url + QLatin1Char('/'))
            || urlSegments(childUrl).size() != urlSegments(url).size() + 1) {
            fail(QStringLiteral("Child category is outside its parent: %1").arg(childUrl));
        }
        if (importCategory(childUrl, slug, poetId, id, depth + 1) != childId) {
            fail(QStringLiteral("Child category ID mismatch in %1").arg(path));
        }
        sql(m_database, QStringLiteral("INSERT INTO category_children "
                                       "(parent_id, child_id, sort_order) VALUES (?, ?, ?)"),
            {id, childId, order});
    }

    const QJsonArray poems = arrayField(file.object, "Poems", path);
    for (qsizetype order = 0; order < poems.size(); ++order) {
        if (!poems.at(order).isObject()) {
            fail(QStringLiteral("Invalid Poems entry in %1").arg(path));
        }
        const QJsonObject reference = poems.at(order).toObject();
        const qint64 poemId = idField(reference, "Id", path);
        // A few Ganjoor poem URLs use a different segment from their category URL.
        // The category reference and the poem's CatId define the relationship.
        importPoem(reference, slug, poetId, id, url);
        sql(m_database, QStringLiteral("INSERT INTO category_poems "
                                       "(category_id, poem_id, sort_order) VALUES (?, ?, ?)"),
            {id, poemId, order});
    }
    m_categoryTitle = previousCategoryTitle;
    return id;
}

void CatalogBuilder::importPoem(const QJsonObject &reference, const QString &slug,
                                qint64 poetId, qint64 categoryId, const QString &categoryUrl)
{
    const qint64 id = idField(reference, "Id", QStringLiteral("poem reference"));
    const QString url = textField(reference, "FullUrl", QStringLiteral("poem reference"));
    const QString path = contentPath(url, slug, false);
    const JsonFile file = readObject(path);
    if (m_poemIds.contains(id) || idField(file.object, "Id", path) != id
        || idField(file.object, "CatId", path) != categoryId
        || textField(file.object, "FullUrl", path) != url
        || textField(file.object, "Title", path) != textField(reference, "Title", path)) {
        fail(QStringLiteral("Duplicate or inconsistent poem: %1").arg(path));
    }
    arrayField(file.object, "Sections", path);
    arrayField(file.object, "Verses", path);
    sql(m_database, QStringLiteral("INSERT INTO poems "
                                   "(id, poet_id, cat_id, title, full_title, full_url, data_json) "
                                   "VALUES (?, ?, ?, ?, ?, ?, ?)"),
        {id, poetId, categoryId, textField(file.object, "Title", path),
         file.object.value(QStringLiteral("FullTitle")).toString(), url, file.bytes});
    m_poemIds.insert(id);
    const QString title = textField(file.object, "Title", path);
    const QString context = m_poetName + QStringLiteral(" » ") + m_categoryTitle;
    searchEntry(QStringLiteral("poem"), url, m_poetUrl, categoryUrl, title, context, title);
    for (const QJsonValue &verseValue : arrayField(file.object, "Verses", path)) {
        if (!verseValue.isObject()) continue;
        const QString verse = verseValue.toObject().value(QStringLiteral("Text")).toString().trimmed();
        if (!verse.isEmpty()) {
            searchEntry(QStringLiteral("verse"), url, m_poetUrl, categoryUrl, title, context, verse);
        }
    }
}

SourceDigest CatalogBuilder::hashSources() const
{
    const QDir root(m_root);
    QStringList files;
    QDirIterator iterator(m_root, QDir::Files | QDir::NoDotAndDotDot,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        files.append(root.relativeFilePath(iterator.next()));
    }
    std::sort(files.begin(), files.end());

    SourceDigest result;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    for (const QString &relative : files) {
        const QString path = root.filePath(relative);
        const QString canonical = QFileInfo(path).canonicalFilePath();
        if (canonical.isEmpty() || !canonical.startsWith(m_root + QDir::separator())) {
            fail(QStringLiteral("Source file is outside data root: %1").arg(path));
        }
        QFile file(canonical);
        if (!file.open(QIODevice::ReadOnly)) {
            fail(QStringLiteral("Cannot hash %1: %2").arg(path, file.errorString()));
        }
        hash.addData(relative.toUtf8());
        hash.addData(QByteArrayView("\0", 1));
        while (!file.atEnd()) {
            const QByteArray block = file.read(256 * 1024);
            if (block.isEmpty() && file.error() != QFileDevice::NoError) {
                fail(QStringLiteral("Cannot hash %1: %2").arg(path, file.errorString()));
            }
            hash.addData(block);
        }
        hash.addData(QByteArrayView("\0", 1));
        ++result.files;
        if (relative.startsWith(QLatin1String("poets/"))) {
            if (relative.endsWith(QLatin1String("/poet.json"))) {
                ++result.poets;
            } else if (relative.endsWith(QLatin1String("/_cat.json"))) {
                ++result.categories;
            } else if (relative.endsWith(QLatin1String(".json"))) {
                ++result.poems;
            }
        }
    }
    result.sha256 = hash.result().toHex();
    return result;
}

void CatalogBuilder::metadata(const QString &key, const QString &value)
{
    sql(m_database, QStringLiteral("INSERT INTO metadata (key, value) VALUES (?, ?)"),
        {key, value});
}

void CatalogBuilder::build()
{
    const QString path = QDir(m_root).filePath(QStringLiteral("manifest.json"));
    const QJsonObject manifest = readObject(path).object;
    if (idField(manifest, "SchemaVersion", path) != 1) {
        fail(QStringLiteral("Unsupported manifest SchemaVersion in %1").arg(path));
    }
    const qint64 expectedPoets = idField(manifest, "PoetsCount", path);
    const qint64 expectedPoems = idField(manifest, "PoemsCount", path);
    const QString generatedAt = textField(manifest, "GeneratedAtUtc", path);
    const QJsonArray poets = arrayField(manifest, "Poets", path);
    if (poets.size() != expectedPoets) {
        fail(QStringLiteral("PoetsCount does not match the manifest poet list"));
    }

    for (qsizetype position = 0; position < poets.size(); ++position) {
        if (!poets.at(position).isObject() || !m_database.transaction()) {
            fail(QStringLiteral("Invalid poet entry or SQLite transaction at position %1").arg(position));
        }
        importPoet(poets.at(position).toObject(), position);
        if (!m_database.commit()) {
            fail(QStringLiteral("Cannot commit poet: %1").arg(m_database.lastError().text()));
        }
        if ((position + 1) % 20 == 0 || position + 1 == poets.size()) {
            qInfo("Imported %lld/%lld poets, %lld categories, %lld poems",
                  static_cast<long long>(position + 1), static_cast<long long>(expectedPoets),
                  static_cast<long long>(m_categoryIds.size()),
                  static_cast<long long>(m_poemIds.size()));
        }
    }
    if (m_poemIds.size() != expectedPoems) {
        fail(QStringLiteral("Poem count mismatch: imported %1, expected %2")
                 .arg(m_poemIds.size()).arg(expectedPoems));
    }
    const SourceDigest digest = hashSources();
    if (digest.poets != expectedPoets || digest.categories != m_categoryIds.size()
        || digest.poems != m_poemIds.size()) {
        fail(QStringLiteral("Unreferenced or missing poet/category/poem files: %1/%2/%3")
                 .arg(digest.poets).arg(digest.categories).arg(digest.poems));
    }

    if (!m_database.transaction()) {
        fail(QStringLiteral("Cannot start metadata transaction: %1").arg(m_database.lastError().text()));
    }
    metadata(QStringLiteral("catalog_schema_version"), QStringLiteral("2"));
    metadata(QStringLiteral("source_schema_version"), QStringLiteral("1"));
    metadata(QStringLiteral("source_generated_at_utc"), generatedAt);
    metadata(QStringLiteral("poets_count"), QString::number(expectedPoets));
    metadata(QStringLiteral("categories_count"), QString::number(m_categoryIds.size()));
    metadata(QStringLiteral("poems_count"), QString::number(expectedPoems));
    metadata(QStringLiteral("source_file_count"), QString::number(digest.files));
    metadata(QStringLiteral("source_digest_sha256"), QString::fromLatin1(digest.sha256));
    if (!m_database.commit()) {
        fail(QStringLiteral("Cannot commit metadata: %1").arg(m_database.lastError().text()));
    }
    sql(m_database, QStringLiteral("PRAGMA optimize"));
    qInfo().noquote() << "Catalog source SHA-256:" << digest.sha256;
}

QString optionValue(const QStringList &arguments, const QString &name)
{
    const qsizetype position = arguments.indexOf(name);
    if (position < 0 || position + 1 >= arguments.size()) {
        fail(QStringLiteral("Missing %1. Usage: parsinama-catalog-builder "
                            "--data-root DATA_DIR --output CATALOG.sqlite").arg(name));
    }
    return arguments.at(position + 1);
}

void publish(const QString &temporary, const QString &output)
{
    const QString backup = output + QStringLiteral(".previous.")
        + QString::number(QCoreApplication::applicationPid());
    const bool replacing = QFileInfo::exists(output);
    if (replacing && !QFile::rename(output, backup)) {
        fail(QStringLiteral("Cannot move previous catalog aside: %1").arg(output));
    }
    if (!QFile::rename(temporary, output)) {
        if (replacing) {
            QFile::rename(backup, output);
        }
        fail(QStringLiteral("Cannot publish catalog: %1").arg(output));
    }
    if (replacing && !QFile::remove(backup)) {
        qWarning().noquote() << "Could not remove previous catalog backup:" << backup;
    }
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QString temporary;
    bool ownsTemporary = false;
    try {
        const QString source = optionValue(application.arguments(), QStringLiteral("--data-root"));
        const QFileInfo outputInfo(optionValue(application.arguments(), QStringLiteral("--output")));
        const QString outputDir = outputInfo.absoluteDir().absolutePath();
        if (!QDir().mkpath(outputDir)) {
            fail(QStringLiteral("Cannot create output directory: %1").arg(outputDir));
        }
        const QString sourceCanonical = QDir(source).canonicalPath();
        const QString outputCanonical = QDir(outputDir).canonicalPath();
        if (sourceCanonical.isEmpty() || outputCanonical == sourceCanonical
            || outputCanonical.startsWith(sourceCanonical + QDir::separator())) {
            fail(QStringLiteral("Catalog output must be outside the data source directory"));
        }

        const QString output = outputInfo.absoluteFilePath();
        temporary = output + QStringLiteral(".building.")
            + QString::number(QCoreApplication::applicationPid());
        if (QFileInfo::exists(temporary)) {
            fail(QStringLiteral("Temporary catalog already exists: %1").arg(temporary));
        }
        ownsTemporary = true;
        {
            CatalogBuilder builder(source, temporary);
            builder.build();
        }
        publish(temporary, output);
        qInfo().noquote() << "Catalog ready:" << output;
        return EXIT_SUCCESS;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "Catalog build failed: %s\n", error.what());
        if (ownsTemporary) {
            QFile::remove(temporary);
        }
        return EXIT_FAILURE;
    }
}
