#include "PoetListModel.h"

PoetListModel::PoetListModel(CatalogRepository *repository, QObject *parent)
    : QAbstractListModel(parent), m_repository(repository)
{
    reload();
}

int PoetListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_poets.size();
}

QVariant PoetListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_poets.size()) {
        return {};
    }
    const PoetRecord &poet = m_poets.at(index.row());
    switch (role) {
    case PoetIdRole: return poet.id;
    case SortOrderRole: return poet.sortOrder;
    case SlugRole: return poet.slug;
    case NameRole: return poet.name;
    case NicknameRole: return poet.nickname;
    case FullUrlRole: return poet.fullUrl;
    case DescriptionRole: return poet.description;
    default: return {};
    }
}

QHash<int, QByteArray> PoetListModel::roleNames() const
{
    return {
        {PoetIdRole, "poetId"}, {SortOrderRole, "sortOrder"},
        {SlugRole, "slug"}, {NameRole, "name"}, {NicknameRole, "nickname"},
        {FullUrlRole, "fullUrl"}, {DescriptionRole, "description"}
    };
}

void PoetListModel::reload()
{
    beginResetModel();
    m_poets = m_repository ? m_repository->poets() : QList<PoetRecord>{};
    endResetModel();
    emit countChanged();
}
