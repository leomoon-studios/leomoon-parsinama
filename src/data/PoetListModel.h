#pragma once

#include "CatalogRepository.h"

#include <QAbstractListModel>

class PoetListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    enum Role {
        PoetIdRole = Qt::UserRole + 1,
        SortOrderRole,
        SlugRole,
        NameRole,
        NicknameRole,
        FullUrlRole,
        DescriptionRole
    };

    explicit PoetListModel(CatalogRepository *repository, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count() const { return m_poets.size(); }
    QString filterText() const { return m_filterText; }
    Q_INVOKABLE void setFilterText(const QString &text);
    Q_INVOKABLE void reload();

signals:
    void countChanged();
    void filterTextChanged();

private:
    CatalogRepository *m_repository;
    QList<PoetRecord> m_poets;
    QList<PoetRecord> m_allPoets;
    QString m_filterText;
};
