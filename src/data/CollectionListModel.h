#pragma once

#include "CatalogRepository.h"

#include <QAbstractListModel>

class CollectionListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY collectionChanged)
    Q_PROPERTY(QString title READ title NOTIFY collectionChanged)
    Q_PROPERTY(QString description READ description NOTIFY collectionChanged)
    Q_PROPERTY(QString bookName READ bookName NOTIFY collectionChanged)
    Q_PROPERTY(QString error READ error NOTIFY collectionChanged)
    Q_PROPERTY(qint64 categoryId READ categoryId NOTIFY collectionChanged)
    Q_PROPERTY(QString fullUrl READ fullUrl NOTIFY collectionChanged)
    Q_PROPERTY(int categoryCount READ categoryCount NOTIFY collectionChanged)
    Q_PROPERTY(int poemCount READ poemCount NOTIFY collectionChanged)

public:
    enum Role {
        EntryTypeRole = Qt::UserRole + 1,
        EntryIdRole,
        SortOrderRole,
        TitleRole,
        FullUrlRole
    };

    explicit CollectionListModel(CatalogRepository *repository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const { return m_entries.size(); }
    QString title() const { return m_category.title; }
    QString description() const { return m_category.description; }
    QString bookName() const { return m_category.bookName; }
    QString error() const { return m_error; }
    qint64 categoryId() const { return m_category.id; }
    QString fullUrl() const { return m_category.fullUrl; }
    int categoryCount() const { return m_categoryCount; }
    int poemCount() const { return m_poemCount; }
    Q_INVOKABLE bool loadCategory(const QString &fullUrl);
    Q_INVOKABLE void clear();

signals:
    void collectionChanged();

private:
    struct Entry
    {
        QString type;
        qint64 id = 0;
        int sortOrder = 0;
        QString title;
        QString fullUrl;
    };

    CatalogRepository *m_repository;
    CategoryRecord m_category;
    QList<Entry> m_entries;
    QString m_error;
    int m_categoryCount = 0;
    int m_poemCount = 0;
};
