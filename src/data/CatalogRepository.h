#pragma once

#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QString>

#include <optional>

struct PoetRecord
{
    qint64 id = 0;
    int sortOrder = 0;
    QString slug;
    QString name;
    QString nickname;
    QString fullUrl;
    QString description;
};

struct CategoryRecord
{
    qint64 id = 0;
    qint64 poetId = 0;
    qint64 parentId = 0;
    QString title;
    QString fullUrl;
    QString description;
    QString bookName;
};

struct PoemRecord
{
    qint64 id = 0;
    qint64 poetId = 0;
    qint64 categoryId = 0;
    int sortOrder = 0;
    QString title;
    QString fullTitle;
    QString fullUrl;
};

class CatalogRepository final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY stateChanged)
    Q_PROPERTY(QString catalogPath READ catalogPath NOTIFY stateChanged)

public:
    explicit CatalogRepository(QObject *parent = nullptr);
    ~CatalogRepository() override;

    bool openCatalog(const QString &path);
    bool ready() const { return m_ready; }
    QString error() const { return m_error; }
    QString statusText() const;
    QString catalogPath() const { return m_catalogPath; }

    QList<PoetRecord> poets() const;
    std::optional<PoetRecord> poetByUrl(const QString &url) const;
    std::optional<CategoryRecord> categoryByUrl(const QString &url) const;
    QList<CategoryRecord> childCategories(qint64 parentId) const;
    QList<PoemRecord> categoryPoems(qint64 categoryId) const;
    std::optional<PoemRecord> poemByUrl(const QString &url) const;

signals:
    void stateChanged();

private:
    void closeCatalog();
    void failOpen(const QString &error);

    QString m_connectionName;
    QString m_catalogPath;
    QString m_error;
    QSqlDatabase m_database;
    bool m_ready = false;
};
