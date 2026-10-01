#pragma once

#include <QAbstractListModel>
#include <QVector>

class CatalogRepository;
class NavigationController;

struct BookmarkRecord
{
    QString type;
    QString url;
    QString title;
    QString context;
    bool available = false;
};

class BookmarkStore final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool canFavoriteCurrent READ canFavoriteCurrent NOTIFY currentChanged)
    Q_PROPERTY(bool currentFavorite READ currentFavorite NOTIFY currentChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)

public:
    enum Role { TypeRole = Qt::UserRole + 1, UrlRole, TitleRole, ContextRole, AvailableRole };

    BookmarkStore(CatalogRepository *repository, NavigationController *navigation,
                  QString configBasePath = {}, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_records.size(); }
    bool canFavoriteCurrent() const;
    bool currentFavorite() const;
    QString error() const { return m_error; }
    QString filePath() const;

    Q_INVOKABLE bool contains(const QString &type, const QString &url) const;
    Q_INVOKABLE bool toggleCurrent();
    Q_INVOKABLE bool add(const QString &type, const QString &url);
    Q_INVOKABLE bool remove(const QString &type, const QString &url);
    bool reload();

signals:
    void countChanged();
    void currentChanged();
    void errorChanged();

private:
    QString currentType() const;
    BookmarkRecord resolved(const BookmarkRecord &saved) const;
    bool save(const QVector<BookmarkRecord> &records);
    void replace(QVector<BookmarkRecord> records);
    void setError(const QString &error);

    CatalogRepository *m_repository;
    NavigationController *m_navigation;
    QString m_configBasePath;
    QVector<BookmarkRecord> m_records;
    QString m_error;
    bool m_writable = true;
};
