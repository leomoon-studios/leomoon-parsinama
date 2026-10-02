#include "NavigationController.h"

#include "CollectionListModel.h"
#include "PoemLoader.h"

#include <QVariantMap>

#include <algorithm>

namespace {
QVariantMap crumb(const QString &title, const QString &type, const QString &url)
{
    return {{QStringLiteral("title"), title},
            {QStringLiteral("type"), type},
            {QStringLiteral("url"), url}};
}
}

NavigationController::NavigationController(CatalogRepository *repository,
                                           CollectionListModel *collection,
                                           PoemLoader *poemLoader, QObject *parent)
    : QObject(parent), m_repository(repository), m_collection(collection), m_poemLoader(poemLoader)
{
    applyLocation({});
}

QString NavigationController::page() const
{
    switch (m_current.kind) {
    case Kind::Poets: return QStringLiteral("poets");
    case Kind::Poet: return QStringLiteral("poet");
    case Kind::Collection: return QStringLiteral("collection");
    case Kind::Poem: return QStringLiteral("poem");
    }
    return {};
}

void NavigationController::setError(const QString &error)
{
    if (m_error == error) {
        return;
    }
    m_error = error;
    emit errorChanged();
}

bool NavigationController::applyLocation(const Location &location)
{
    if (!m_collection || !m_poemLoader) {
        setError(QStringLiteral("مرور داده‌های شعر آماده نیست."));
        return false;
    }

    if (location.kind == Kind::Poets) {
        m_collection->clear();
        m_poemLoader->clear();
        m_title.clear();
        m_poetName.clear();
        m_poetUrl.clear();
        m_poetDescription.clear();
        m_description.clear();
        m_bookName.clear();
        m_poemPosition = 0;
        m_poemCount = 0;
        m_previousPoemUrl.clear();
        m_nextPoemUrl.clear();
        m_breadcrumbs = {crumb(QStringLiteral("شاعران"), QStringLiteral("poets"), {})};
        m_current = location;
        setError({});
        emit stateChanged();
        return true;
    }

    if (!m_repository || !m_repository->ready()) {
        setError(QStringLiteral("پایگاه دادهٔ شعر آماده نیست."));
        return false;
    }

    std::optional<CategoryRecord> category;
    std::optional<PoemRecord> poem;
    if (location.kind == Kind::Poet) {
        category = m_repository->categoryByUrl(location.url);
    } else if (location.kind == Kind::Collection) {
        category = m_repository->categoryByUrl(location.url);
    } else {
        poem = m_repository->poemByUrl(location.url);
        if (poem) {
            category = m_repository->categoryById(poem->categoryId);
        }
    }
    if (!category || (location.kind == Kind::Poet && category->parentId != 0)) {
        setError(QStringLiteral("مجموعهٔ درخواستی پیدا نشد."));
        return false;
    }
    const auto poet = m_repository->poetById(category->poetId);
    if (!poet || (poem && poem->poetId != poet->id)) {
        setError(QStringLiteral("شاعر درخواستی پیدا نشد."));
        return false;
    }

    QList<CategoryRecord> lineage;
    CategoryRecord ancestor = *category;
    for (int depth = 0; depth < 64; ++depth) {
        lineage.prepend(ancestor);
        if (ancestor.parentId == 0) {
            break;
        }
        const auto parent = m_repository->categoryById(ancestor.parentId);
        if (!parent || parent->poetId != poet->id || parent->id == ancestor.id) {
            setError(QStringLiteral("مسیر مجموعهٔ شعر معتبر نیست."));
            return false;
        }
        ancestor = *parent;
    }
    if (lineage.isEmpty() || lineage.constFirst().parentId != 0
        || lineage.constFirst().fullUrl != poet->fullUrl) {
        setError(QStringLiteral("مسیر مجموعهٔ شعر کامل نیست."));
        return false;
    }

    if (!m_collection->loadCategory(category->fullUrl)) {
        setError(m_collection->error());
        return false;
    }
    if (poem) {
        m_poemLoader->loadByUrl(poem->fullUrl);
    } else {
        m_poemLoader->clear();
    }

    m_current = location;
    m_poetName = poet->name;
    m_poetUrl = poet->fullUrl;
    m_poetDescription = poet->description;
    m_title = poem ? poem->title : location.kind == Kind::Poet ? poet->name : category->title;
    m_description = category->description;
    m_bookName = category->bookName;
    m_poemPosition = 0;
    m_poemCount = m_collection->poemCount();
    m_previousPoemUrl.clear();
    m_nextPoemUrl.clear();
    if (poem) {
        const QList<PoemRecord> orderedPoems = m_repository->categoryPoems(category->id);
        m_poemCount = orderedPoems.size();
        for (int index = 0; index < orderedPoems.size(); ++index) {
            if (orderedPoems.at(index).fullUrl != poem->fullUrl) {
                continue;
            }
            m_poemPosition = index + 1;
            if (index > 0) m_previousPoemUrl = orderedPoems.at(index - 1).fullUrl;
            if (index + 1 < orderedPoems.size()) m_nextPoemUrl = orderedPoems.at(index + 1).fullUrl;
            break;
        }
    }
    m_breadcrumbs = {crumb(QStringLiteral("شاعران"), QStringLiteral("poets"), {}),
                     crumb(poet->name, QStringLiteral("poet"), poet->fullUrl)};
    for (int index = 1; index < lineage.size(); ++index) {
        const CategoryRecord &part = lineage.at(index);
        m_breadcrumbs.append(crumb(part.title, QStringLiteral("collection"), part.fullUrl));
    }
    if (poem) {
        m_breadcrumbs.append(crumb(poem->title, QStringLiteral("poem"), poem->fullUrl));
    }
    setError({});
    emit stateChanged();
    return true;
}

bool NavigationController::navigate(Location target, qreal poetScroll, qreal collectionScroll)
{
    if (target.kind == m_current.kind && target.url == m_current.url) {
        return true;
    }
    Location previous = m_current;
    previous.poetScroll = std::max<qreal>(0, poetScroll);
    previous.collectionScroll = std::max<qreal>(0, collectionScroll);
    target.poetScroll = previous.poetScroll;
    target.collectionScroll = target.kind == Kind::Poem ? previous.collectionScroll : 0;
    if (!applyLocation(target)) {
        return false;
    }
    m_back.append(previous);
    m_forward.clear();
    emit historyChanged();
    return true;
}

bool NavigationController::openPoets(qreal poetScroll, qreal collectionScroll)
{
    return navigate({Kind::Poets, {}}, poetScroll, collectionScroll);
}

bool NavigationController::openPoet(const QString &url, qreal poetScroll, qreal collectionScroll)
{
    return navigate({Kind::Poet, url}, poetScroll, collectionScroll);
}

bool NavigationController::openCategory(const QString &url, qreal poetScroll, qreal collectionScroll)
{
    const auto category = m_repository ? m_repository->categoryByUrl(url) : std::nullopt;
    if (category && category->parentId == 0) {
        return openPoet(url, poetScroll, collectionScroll);
    }
    return navigate({Kind::Collection, url}, poetScroll, collectionScroll);
}

bool NavigationController::openPoem(const QString &url, qreal poetScroll, qreal collectionScroll)
{
    return navigate({Kind::Poem, url}, poetScroll, collectionScroll);
}

bool NavigationController::openPreviousPoem(qreal poetScroll, qreal collectionScroll)
{
    return hasPreviousPoem() && openPoem(m_previousPoemUrl, poetScroll, collectionScroll);
}

bool NavigationController::openNextPoem(qreal poetScroll, qreal collectionScroll)
{
    return hasNextPoem() && openPoem(m_nextPoemUrl, poetScroll, collectionScroll);
}

bool NavigationController::openBreadcrumb(int index, qreal poetScroll, qreal collectionScroll)
{
    if (index < 0 || index >= m_breadcrumbs.size()) {
        return false;
    }
    const QVariantMap item = m_breadcrumbs.at(index).toMap();
    const QString type = item.value(QStringLiteral("type")).toString();
    const QString url = item.value(QStringLiteral("url")).toString();
    if (type == QLatin1String("poets")) return openPoets(poetScroll, collectionScroll);
    if (type == QLatin1String("poet")) return openPoet(url, poetScroll, collectionScroll);
    if (type == QLatin1String("collection")) return openCategory(url, poetScroll, collectionScroll);
    if (type == QLatin1String("poem")) return openPoem(url, poetScroll, collectionScroll);
    return false;
}

bool NavigationController::back(qreal poetScroll, qreal collectionScroll)
{
    if (m_back.isEmpty()) {
        return false;
    }
    Location current = m_current;
    current.poetScroll = std::max<qreal>(0, poetScroll);
    current.collectionScroll = std::max<qreal>(0, collectionScroll);
    const Location target = m_back.takeLast();
    if (!applyLocation(target)) {
        m_back.append(target);
        return false;
    }
    m_forward.append(current);
    emit historyChanged();
    return true;
}

bool NavigationController::forward(qreal poetScroll, qreal collectionScroll)
{
    if (m_forward.isEmpty()) {
        return false;
    }
    Location current = m_current;
    current.poetScroll = std::max<qreal>(0, poetScroll);
    current.collectionScroll = std::max<qreal>(0, collectionScroll);
    const Location target = m_forward.takeLast();
    if (!applyLocation(target)) {
        m_forward.append(target);
        return false;
    }
    m_back.append(current);
    emit historyChanged();
    return true;
}
