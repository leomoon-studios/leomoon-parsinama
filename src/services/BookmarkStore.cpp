#include "BookmarkStore.h"

#include "UserDataPaths.h"
#include "data/CatalogRepository.h"
#include "data/NavigationController.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

#include <utility>

namespace {
constexpr int bookmarksVersion = 1;

bool validType(const QString &type)
{
    return type == QLatin1String("poet") || type == QLatin1String("collection")
        || type == QLatin1String("poem");
}

QString keyFor(const QString &type, const QString &url)
{
    return type + QLatin1Char('\n') + url;
}
}

BookmarkStore::BookmarkStore(CatalogRepository *repository, NavigationController *navigation,
                             QString configBasePath, QObject *parent)
    : QAbstractListModel(parent), m_repository(repository), m_navigation(navigation),
      m_configBasePath(std::move(configBasePath))
{
    if (m_navigation) {
        connect(m_navigation, &NavigationController::stateChanged, this, &BookmarkStore::currentChanged);
    }
    if (m_repository) {
        connect(m_repository, &CatalogRepository::stateChanged, this, [this] {
            QVector<BookmarkRecord> updated;
            updated.reserve(m_records.size());
            for (const BookmarkRecord &record : std::as_const(m_records)) updated.append(resolved(record));
            replace(std::move(updated));
        });
    }
    reload();
}

int BookmarkStore::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_records.size();
}

QVariant BookmarkStore::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size()) return {};
    const BookmarkRecord &record = m_records.at(index.row());
    switch (role) {
    case TypeRole: return record.type;
    case UrlRole: return record.url;
    case TitleRole: return record.title;
    case ContextRole: return record.context;
    case AvailableRole: return record.available;
    default: return {};
    }
}

QHash<int, QByteArray> BookmarkStore::roleNames() const
{
    return {{TypeRole, "entryType"}, {UrlRole, "fullUrl"}, {TitleRole, "title"},
            {ContextRole, "context"}, {AvailableRole, "available"}};
}

QString BookmarkStore::filePath() const
{
    return UserDataPaths::bookmarksFile(m_configBasePath);
}

void BookmarkStore::setError(const QString &error)
{
    if (m_error == error) return;
    m_error = error;
    emit errorChanged();
}

void BookmarkStore::replace(QVector<BookmarkRecord> records)
{
    beginResetModel();
    m_records = std::move(records);
    endResetModel();
    emit countChanged();
    emit currentChanged();
}

BookmarkRecord BookmarkStore::resolved(const BookmarkRecord &saved) const
{
    BookmarkRecord record = saved;
    record.available = false;
    if (!m_repository || !m_repository->ready()) return record;

    QStringList context;
    qint64 categoryId = 0;
    bool firstCategory = true;
    bool foundRoot = false;
    if (record.type == QLatin1String("poet")) {
        const auto poet = m_repository->poetByUrl(record.url);
        if (!poet) return record;
        record.title = poet->name;
        record.context = QStringLiteral("شاعر");
        record.available = true;
        return record;
    }
    if (record.type == QLatin1String("collection")) {
        const auto category = m_repository->categoryByUrl(record.url);
        if (!category) return record;
        record.title = category->title;
        categoryId = category->id;
    } else if (record.type == QLatin1String("poem")) {
        const auto poem = m_repository->poemByUrl(record.url);
        if (!poem) return record;
        record.title = poem->title;
        categoryId = poem->categoryId;
    } else {
        return record;
    }

    for (int depth = 0; categoryId && depth < 64; ++depth) {
        const auto category = m_repository->categoryById(categoryId);
        if (!category) return record;
        if (category->parentId == 0) {
            const auto poet = m_repository->poetById(category->poetId);
            if (!poet) return record;
            context.prepend(poet->name);
            foundRoot = true;
            break;
        }
        if (record.type == QLatin1String("poem") || !firstCategory) {
            context.prepend(category->title);
        }
        firstCategory = false;
        categoryId = category->parentId;
    }
    if (foundRoot) {
        record.context = context.join(QStringLiteral(" » "));
        record.available = true;
    }
    return record;
}

bool BookmarkStore::contains(const QString &type, const QString &url) const
{
    for (const BookmarkRecord &record : m_records) {
        if (record.type == type && record.url == url) return true;
    }
    return false;
}

QString BookmarkStore::currentType() const
{
    if (!m_navigation) return {};
    return m_navigation->page();
}

bool BookmarkStore::canFavoriteCurrent() const
{
    return validType(currentType()) && !m_navigation->url().isEmpty();
}

bool BookmarkStore::currentFavorite() const
{
    return canFavoriteCurrent() && contains(currentType(), m_navigation->url());
}

bool BookmarkStore::toggleCurrent()
{
    if (!canFavoriteCurrent()) return false;
    const QString type = currentType();
    const QString url = m_navigation->url();
    return contains(type, url) ? remove(type, url) : add(type, url);
}

bool BookmarkStore::save(const QVector<BookmarkRecord> &records)
{
    if (!UserDataPaths::ensureDirectory(m_configBasePath)) {
        setError(QStringLiteral("ساخت پوشهٔ نشانک‌ها ممکن نشد."));
        return false;
    }
    QJsonArray entries;
    for (const BookmarkRecord &record : records) {
        entries.append(QJsonObject{{QStringLiteral("type"), record.type},
                                   {QStringLiteral("url"), record.url},
                                   {QStringLiteral("title"), record.title},
                                   {QStringLiteral("context"), record.context}});
    }
    const QByteArray data = QJsonDocument(QJsonObject{
        {QStringLiteral("version"), bookmarksVersion},
        {QStringLiteral("bookmarks"), entries}
    }).toJson(QJsonDocument::Indented);
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
        setError(QStringLiteral("ذخیرهٔ نشانک‌ها ممکن نشد."));
        return false;
    }
    setError({});
    return true;
}

bool BookmarkStore::reload()
{
    QFile file(filePath());
    if (!file.exists()) {
        if (!save({})) return false;
        m_writable = true;
        replace({});
        return true;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        m_writable = false;
        setError(QStringLiteral("خواندن نشانک‌ها ممکن نشد."));
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()
        || document.object().value(QStringLiteral("version")).toInt() != bookmarksVersion
        || !document.object().value(QStringLiteral("bookmarks")).isArray()) {
        m_writable = false;
        setError(QStringLiteral("پروندهٔ نشانک‌ها معتبر نیست."));
        return false;
    }
    QVector<BookmarkRecord> records;
    QSet<QString> seen;
    for (const QJsonValue &value : document.object().value(QStringLiteral("bookmarks")).toArray()) {
        if (!value.isObject()) {
            m_writable = false;
            setError(QStringLiteral("پروندهٔ نشانک‌ها معتبر نیست."));
            return false;
        }
        const QJsonObject object = value.toObject();
        BookmarkRecord record{object.value(QStringLiteral("type")).toString(),
                              object.value(QStringLiteral("url")).toString(),
                              object.value(QStringLiteral("title")).toString(),
                              object.value(QStringLiteral("context")).toString(), false};
        if (!validType(record.type) || !record.url.startsWith(QLatin1Char('/'))
            || record.title.isEmpty()) {
            m_writable = false;
            setError(QStringLiteral("پروندهٔ نشانک‌ها معتبر نیست."));
            return false;
        }
        const QString key = keyFor(record.type, record.url);
        if (seen.contains(key)) continue;
        seen.insert(key);
        records.append(resolved(record));
    }
    m_writable = true;
    setError({});
    replace(std::move(records));
    return true;
}

bool BookmarkStore::add(const QString &type, const QString &url)
{
    if (!m_writable || !validType(type) || url.isEmpty() || contains(type, url)) return false;
    BookmarkRecord record = resolved({type, url, {}, {}, false});
    if (!record.available) return false;
    QVector<BookmarkRecord> next = m_records;
    next.prepend(std::move(record));
    if (!save(next)) return false;
    replace(std::move(next));
    return true;
}

bool BookmarkStore::remove(const QString &type, const QString &url)
{
    if (!m_writable || !contains(type, url)) return false;
    QVector<BookmarkRecord> next;
    next.reserve(m_records.size() - 1);
    for (const BookmarkRecord &record : std::as_const(m_records)) {
        if (record.type != type || record.url != url) next.append(record);
    }
    if (!save(next)) return false;
    replace(std::move(next));
    return true;
}
