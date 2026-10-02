#include "SearchRepository.h"
#include "SearchNormalizer.h"

#include <QFileInfo>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <QtConcurrent>

namespace {
constexpr int pageSize = 40;
}

SearchRepository::SearchRepository(QString catalogPath, QObject *parent)
    : QAbstractListModel(parent), m_path(QFileInfo(catalogPath).absoluteFilePath())
{
}

SearchRepository::~SearchRepository() { cancel(); }

int SearchRepository::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_hits.size();
}

QVariant SearchRepository::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_hits.size()) return {};
    const SearchHit &hit = m_hits.at(index.row());
    switch (role) {
    case EntryTypeRole: return hit.entryType;
    case FullUrlRole: return hit.fullUrl;
    case TitleRole: return hit.title;
    case ContextRole: return hit.context;
    case SnippetRole: return hit.snippet;
    default: return {};
    }
}

QHash<int, QByteArray> SearchRepository::roleNames() const
{
    return {{EntryTypeRole, "entryType"}, {FullUrlRole, "fullUrl"},
            {TitleRole, "title"}, {ContextRole, "context"}, {SnippetRole, "snippet"}};
}

void SearchRepository::cancel()
{
    ++m_generation;
    if (m_cancelled) m_cancelled->store(true);
    m_cancelled.reset();
    m_loading = false;
    emit stateChanged();
}

void SearchRepository::search(const QString &text, const QString &poetUrl,
                              const QString &categoryUrl, const QString &type)
{
    cancel();
    beginResetModel();
    m_hits.clear();
    endResetModel();
    m_totalCount = 0;
    m_hasMore = false;
    m_error.clear();
    m_expression = SearchNormalizer::matchExpression(text);
    m_poetUrl = poetUrl;
    m_categoryUrl = categoryUrl;
    m_type = type;
    if (!m_expression.isEmpty()) startBatch(0);
    else emit stateChanged();
}

void SearchRepository::loadMore()
{
    if (!m_loading && m_hasMore) startBatch(m_hits.size());
}

void SearchRepository::startBatch(int offset)
{
    m_loading = true;
    m_cancelled = std::make_shared<std::atomic_bool>(false);
    const auto token = m_cancelled;
    const quint64 generation = m_generation;
    auto *watcher = new QFutureWatcher<SearchBatch>(this);
    connect(watcher, &QFutureWatcher<SearchBatch>::finished, this,
            [this, watcher, generation, offset]() {
        const SearchBatch batch = watcher->result();
        watcher->deleteLater();
        if (generation != m_generation || batch.cancelled) return;
        m_loading = false;
        m_error = batch.error;
        if (m_error.isEmpty()) {
            m_totalCount = batch.totalCount;
            if (!batch.hits.isEmpty()) {
                beginInsertRows({}, offset, offset + batch.hits.size() - 1);
                m_hits += batch.hits;
                endInsertRows();
            }
            m_hasMore = batch.hasMore;
        }
        emit stateChanged();
    });
    watcher->setFuture(QtConcurrent::run(&SearchRepository::runBatch, m_path, m_expression,
                                         m_poetUrl, m_categoryUrl, m_type, offset, token));
    emit stateChanged();
}

SearchBatch SearchRepository::runBatch(const QString &path, const QString &expression,
                                       const QString &poetUrl, const QString &categoryUrl,
                                       const QString &type, int offset,
                                       const std::shared_ptr<std::atomic_bool> &cancelled)
{
    SearchBatch batch;
    if (cancelled->load()) { batch.cancelled = true; return batch; }
    const QString connection = QStringLiteral("search-")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(path);
        database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        if (!database.open()) {
            batch.error = database.lastError().text();
        } else {
            QString condition = QStringLiteral("search_fts MATCH ?");
            QVariantList values { expression };
            if (!poetUrl.isEmpty()) {
                condition += QStringLiteral(" AND poet_url = ?");
                values.append(poetUrl);
            }
            if (!categoryUrl.isEmpty()) {
                condition += QStringLiteral(" AND (category_url = ? OR instr(category_url, ? || '/') = 1)");
                values.append(categoryUrl);
                values.append(categoryUrl);
            }
            if (type == QLatin1String("poet") || type == QLatin1String("collection")) {
                condition += QStringLiteral(" AND entry_type = ?");
                values.append(type);
            } else if (type == QLatin1String("poem")) {
                condition += QStringLiteral(" AND entry_type IN ('poem', 'verse')");
            }
            QSqlQuery count(database);
            count.prepare(QStringLiteral("SELECT count(*) FROM search_fts WHERE ") + condition);
            for (const QVariant &value : values) count.addBindValue(value);
            if (!count.exec() || !count.next()) {
                batch.error = count.lastError().text();
            } else {
                batch.totalCount = count.value(0).toInt();
                if (cancelled->load()) batch.cancelled = true;
                else {
                    QSqlQuery query(database);
                    query.prepare(QStringLiteral("SELECT entry_type, full_url, title, context, "
                                                 "original_text FROM search_fts WHERE ") + condition
                                  + QStringLiteral(" ORDER BY bm25(search_fts), rowid LIMIT ? OFFSET ?"));
                    for (const QVariant &value : values) query.addBindValue(value);
                    query.addBindValue(pageSize + 1);
                    query.addBindValue(offset);
                    if (!query.exec()) batch.error = query.lastError().text();
                    else {
                        while (query.next() && !cancelled->load()) {
                            if (batch.hits.size() == pageSize) { batch.hasMore = true; break; }
                            batch.hits.append({query.value(0).toString(), query.value(1).toString(),
                                               query.value(2).toString(), query.value(3).toString(),
                                               query.value(4).toString()});
                        }
                        batch.cancelled = cancelled->load();
                    }
                }
            }
            count.finish();
            database.close();
        }
    }
    QSqlDatabase::removeDatabase(connection);
    return batch;
}
