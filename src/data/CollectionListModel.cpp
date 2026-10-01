#include "CollectionListModel.h"

CollectionListModel::CollectionListModel(CatalogRepository *repository, QObject *parent)
    : QAbstractListModel(parent), m_repository(repository)
{
}

int CollectionListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant CollectionListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return {};
    }
    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case EntryTypeRole: return entry.type;
    case EntryIdRole: return entry.id;
    case SortOrderRole: return entry.sortOrder;
    case TitleRole: return entry.title;
    case FullUrlRole: return entry.fullUrl;
    default: return {};
    }
}

QHash<int, QByteArray> CollectionListModel::roleNames() const
{
    return {
        {EntryTypeRole, "entryType"}, {EntryIdRole, "entryId"},
        {SortOrderRole, "sortOrder"}, {TitleRole, "title"},
        {FullUrlRole, "fullUrl"}
    };
}

bool CollectionListModel::loadCategory(const QString &fullUrl)
{
    if (!m_repository || !m_repository->ready()) {
        m_error = QStringLiteral("پایگاه دادهٔ شعر آماده نیست.");
        emit collectionChanged();
        return false;
    }
    const auto category = m_repository->categoryByUrl(fullUrl);
    if (!category) {
        m_error = QStringLiteral("مجموعهٔ درخواستی پیدا نشد.");
        emit collectionChanged();
        return false;
    }

    QList<Entry> entries;
    const auto children = m_repository->childCategories(category->id);
    for (qsizetype position = 0; position < children.size(); ++position) {
        const auto &child = children.at(position);
        entries.append({QStringLiteral("category"), child.id, int(position),
                        child.title, child.fullUrl});
    }
    const auto poems = m_repository->categoryPoems(category->id);
    for (const auto &poem : poems) {
        entries.append({QStringLiteral("poem"), poem.id, poem.sortOrder,
                        poem.title, poem.fullUrl});
    }

    beginResetModel();
    m_category = *category;
    m_entries = std::move(entries);
    m_error.clear();
    m_categoryCount = int(children.size());
    m_poemCount = int(poems.size());
    endResetModel();
    emit collectionChanged();
    return true;
}

void CollectionListModel::clear()
{
    beginResetModel();
    m_category = {};
    m_entries.clear();
    m_categoryCount = 0;
    m_poemCount = 0;
    m_error.clear();
    endResetModel();
    emit collectionChanged();
}
