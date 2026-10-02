#pragma once

#include <QAbstractListModel>
#include <QString>

#include <atomic>
#include <memory>

struct SearchHit
{
    QString entryType;
    QString fullUrl;
    QString title;
    QString context;
    QString snippet;
};

struct SearchBatch
{
    QVector<SearchHit> hits;
    int totalCount = 0;
    bool hasMore = false;
    QString error;
    bool cancelled = false;
};

class SearchRepository final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY stateChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY stateChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY stateChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)

public:
    enum Role { EntryTypeRole = Qt::UserRole + 1, FullUrlRole, TitleRole,
                ContextRole, SnippetRole };
    explicit SearchRepository(QString catalogPath, QObject *parent = nullptr);
    ~SearchRepository() override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_hits.size(); }
    int totalCount() const { return m_totalCount; }
    bool loading() const { return m_loading; }
    bool hasMore() const { return m_hasMore; }
    QString error() const { return m_error; }

    Q_INVOKABLE void search(const QString &text, const QString &poetUrl = {},
                            const QString &categoryUrl = {}, const QString &type = {});
    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void cancel();

signals:
    void stateChanged();

private:
    void startBatch(int offset);
    static SearchBatch runBatch(const QString &path, const QString &expression,
                                const QString &poetUrl, const QString &categoryUrl,
                                const QString &type, int offset,
                                const std::shared_ptr<std::atomic_bool> &cancelled);

    QString m_path;
    QString m_expression;
    QString m_poetUrl;
    QString m_categoryUrl;
    QString m_type;
    QVector<SearchHit> m_hits;
    std::shared_ptr<std::atomic_bool> m_cancelled;
    quint64 m_generation = 0;
    int m_totalCount = 0;
    bool m_loading = false;
    bool m_hasMore = false;
    QString m_error;
};
