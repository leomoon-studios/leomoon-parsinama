#include "PoetListModel.h"

namespace {
QString normalizedName(QString text)
{
    return text.trimmed().toCaseFolded()
        .replace(QChar(0x064A), QChar(0x06CC))
        .replace(QChar(0x0643), QChar(0x06A9))
        .remove(QChar(0x0640));
}
}

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
    m_allPoets = m_repository ? m_repository->poets() : QList<PoetRecord>{};
    setFilterText(m_filterText);
}

void PoetListModel::setFilterText(const QString &text)
{
    const QString nextFilter = normalizedName(text);
    if (nextFilter == m_filterText && m_poets.size() == m_allPoets.size() && nextFilter.isEmpty()) {
        return;
    }
    beginResetModel();
    m_poets.clear();
    for (const PoetRecord &poet : m_allPoets) {
        if (nextFilter.isEmpty() || normalizedName(poet.name).contains(nextFilter)
            || normalizedName(poet.nickname).contains(nextFilter)) {
            m_poets.append(poet);
        }
    }
    endResetModel();
    if (m_filterText != nextFilter) {
        m_filterText = nextFilter;
        emit filterTextChanged();
    }
    emit countChanged();
}
