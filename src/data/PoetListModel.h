#pragma once

#include "CatalogRepository.h"

#include <QAbstractListModel>

class PoetListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

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
    Q_INVOKABLE void reload();

signals:
    void countChanged();

private:
    CatalogRepository *m_repository;
    QList<PoetRecord> m_poets;
};
