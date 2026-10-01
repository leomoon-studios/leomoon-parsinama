#include "CatalogRepository.h"

#include <QFileInfo>
#include <QHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace {

void logQueryError(const QSqlQuery &query)
{
    qWarning().noquote() << "Catalog query failed:" << query.lastError().text();
}

PoetRecord readPoet(const QSqlQuery &query)
{
    return {
        query.value(0).toLongLong(), query.value(1).toInt(), query.value(2).toString(),
        query.value(3).toString(), query.value(4).toString(), query.value(5).toString(),
        query.value(6).toString()
    };
}

CategoryRecord readCategory(const QSqlQuery &query)
{
    return {
        query.value(0).toLongLong(), query.value(1).toLongLong(),
        query.value(2).toLongLong(), query.value(3).toString(), query.value(4).toString(),
        query.value(5).toString(), query.value(6).toString()
    };
}

PoemRecord readPoem(const QSqlQuery &query)
{
    return {
        query.value(0).toLongLong(), query.value(1).toLongLong(),
        query.value(2).toLongLong(), query.value(3).toInt(), query.value(4).toString(),
        query.value(5).toString(), query.value(6).toString()
    };
}

} // namespace

CatalogRepository::CatalogRepository(QObject *parent)
    : QObject(parent)
{
}

CatalogRepository::~CatalogRepository()
{
    closeCatalog();
}

void CatalogRepository::closeCatalog()
{
    if (m_connectionName.isEmpty()) {
        return;
    }
    m_database.close();
    m_database = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connectionName);
    m_connectionName.clear();
    m_ready = false;
}

void CatalogRepository::failOpen(const QString &error)
{
    closeCatalog();
    m_error = error;
    emit stateChanged();
}

bool CatalogRepository::openCatalog(const QString &path)
{
    closeCatalog();
    m_catalogPath = QFileInfo(path).absoluteFilePath();
    m_error.clear();

    const QFileInfo file(m_catalogPath);
    if (!file.isFile() || !file.isReadable()) {
        failOpen(QStringLiteral("پایگاه دادهٔ شعر پیدا نشد یا خواندنی نیست: %1").arg(m_catalogPath));
        return false;
    }

    m_connectionName = QStringLiteral("parsinama-reader-")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    m_database.setDatabaseName(m_catalogPath);
    m_database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
    if (!m_database.open()) {
        failOpen(QStringLiteral("پایگاه دادهٔ شعر باز نشد: %1").arg(m_database.lastError().text()));
        return false;
    }

    QHash<QString, QString> metadata;
    bool metadataReadable = false;
    {
        QSqlQuery query(m_database);
        metadataReadable = query.exec(QStringLiteral("SELECT key, value FROM metadata"));
        if (metadataReadable) {
            while (query.next()) {
                metadata.insert(query.value(0).toString(), query.value(1).toString());
            }
        }
    }
    if (!metadataReadable) {
        failOpen(QStringLiteral("ساختار پایگاه دادهٔ شعر معتبر نیست."));
        return false;
    }
    const QStringList tables = m_database.tables(QSql::Tables);
    const QStringList requiredTables {
        QStringLiteral("metadata"), QStringLiteral("poets"),
        QStringLiteral("categories"), QStringLiteral("category_children"),
        QStringLiteral("category_poems"), QStringLiteral("poems")
    };
    for (const QString &table : requiredTables) {
        if (!tables.contains(table)) {
            failOpen(QStringLiteral("ساختار پایگاه دادهٔ شعر کامل نیست."));
            return false;
        }
    }
    if (metadata.value(QStringLiteral("catalog_schema_version")) != QLatin1String("1")
        || metadata.value(QStringLiteral("source_schema_version")) != QLatin1String("1")
        || metadata.value(QStringLiteral("source_digest_sha256")).size() != 64
        || metadata.value(QStringLiteral("poets_count")).toLongLong() <= 0
        || metadata.value(QStringLiteral("poems_count")).toLongLong() <= 0) {
        failOpen(QStringLiteral("نسخهٔ پایگاه دادهٔ شعر با برنامه سازگار نیست."));
        return false;
    }

    m_ready = true;
    emit stateChanged();
    return true;
}

QString CatalogRepository::statusText() const
{
    return m_ready ? QStringLiteral("داده‌های شعر آماده‌اند.") : m_error;
}

QList<PoetRecord> CatalogRepository::poets() const
{
    QList<PoetRecord> result;
    if (!m_ready) {
        return result;
    }
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral("SELECT id, sort_order, slug, name, nickname, full_url, description "
                                   "FROM poets ORDER BY sort_order"))) {
        logQueryError(query);
        return result;
    }
    while (query.next()) {
        result.append(readPoet(query));
    }
    return result;
}

std::optional<PoetRecord> CatalogRepository::poetByUrl(const QString &url) const
{
    if (!m_ready) {
        return std::nullopt;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT id, sort_order, slug, name, nickname, full_url, description "
                                 "FROM poets WHERE full_url = ?"));
    query.addBindValue(url);
    if (!query.exec()) {
        logQueryError(query);
        return std::nullopt;
    }
    return query.next() ? std::optional<PoetRecord>(readPoet(query)) : std::nullopt;
}

std::optional<CategoryRecord> CatalogRepository::categoryByUrl(const QString &url) const
{
    if (!m_ready) {
        return std::nullopt;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT id, poet_id, parent_id, title, full_url, description, book_name "
                                 "FROM categories WHERE full_url = ?"));
    query.addBindValue(url);
    if (!query.exec()) {
        logQueryError(query);
        return std::nullopt;
    }
    return query.next() ? std::optional<CategoryRecord>(readCategory(query)) : std::nullopt;
}

QList<CategoryRecord> CatalogRepository::childCategories(qint64 parentId) const
{
    QList<CategoryRecord> result;
    if (!m_ready) {
        return result;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT c.id, c.poet_id, c.parent_id, c.title, c.full_url, "
                                 "c.description, c.book_name FROM category_children cc "
                                 "JOIN categories c ON c.id = cc.child_id "
                                 "WHERE cc.parent_id = ? ORDER BY cc.sort_order"));
    query.addBindValue(parentId);
    if (!query.exec()) {
        logQueryError(query);
        return result;
    }
    while (query.next()) {
        result.append(readCategory(query));
    }
    return result;
}

QList<PoemRecord> CatalogRepository::categoryPoems(qint64 categoryId) const
{
    QList<PoemRecord> result;
    if (!m_ready) {
        return result;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT p.id, p.poet_id, p.cat_id, cp.sort_order, p.title, "
                                 "p.full_title, p.full_url FROM category_poems cp "
                                 "JOIN poems p ON p.id = cp.poem_id "
                                 "WHERE cp.category_id = ? ORDER BY cp.sort_order"));
    query.addBindValue(categoryId);
    if (!query.exec()) {
        logQueryError(query);
        return result;
    }
    while (query.next()) {
        result.append(readPoem(query));
    }
    return result;
}

std::optional<PoemRecord> CatalogRepository::poemByUrl(const QString &url) const
{
    if (!m_ready) {
        return std::nullopt;
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT p.id, p.poet_id, p.cat_id, cp.sort_order, p.title, "
                                 "p.full_title, p.full_url FROM poems p "
                                 "JOIN category_poems cp ON cp.poem_id = p.id "
                                 "WHERE p.full_url = ?"));
    query.addBindValue(url);
    if (!query.exec()) {
        logQueryError(query);
        return std::nullopt;
    }
    return query.next() ? std::optional<PoemRecord>(readPoem(query)) : std::nullopt;
}
