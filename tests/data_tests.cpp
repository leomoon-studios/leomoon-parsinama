#include "data/CatalogPaths.h"
#include "data/CatalogRepository.h"
#include "data/CollectionListModel.h"
#include "data/NavigationController.h"
#include "data/PoemLoader.h"
#include "data/PoetListModel.h"
#include "services/BookmarkStore.h"
#include "services/UserDataPaths.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

class DataTests final : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void repositoryAndModels();
    void navigationAndFiltering();
    void missingAndIncompatibleCatalog();
    void largePoemLoadsAsynchronously();
    void bookmarksPersistAndNavigate();
    void bookmarksHandleMissingAndInvalidFiles();
    void fullCatalogQueries();

private:
    QTemporaryDir m_temporary;
    QString m_fixturePath;
};

void DataTests::initTestCase()
{
    QVERIFY(m_temporary.isValid());
    m_fixturePath = m_temporary.filePath(QStringLiteral("fixture.sqlite"));
    const QString connection = QStringLiteral("fixture_writer");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(m_fixturePath);
        QVERIFY(database.open());
        {
            QSqlQuery query(database);
            const QStringList statements {
                QStringLiteral("CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL)"),
                QStringLiteral("CREATE TABLE poets (id INTEGER PRIMARY KEY, sort_order INTEGER, slug TEXT, name TEXT, nickname TEXT, full_url TEXT, description TEXT)"),
                QStringLiteral("CREATE TABLE categories (id INTEGER PRIMARY KEY, poet_id INTEGER, parent_id INTEGER, title TEXT, full_url TEXT, description TEXT, book_name TEXT)"),
                QStringLiteral("CREATE TABLE category_children (parent_id INTEGER, child_id INTEGER, sort_order INTEGER)"),
                QStringLiteral("CREATE TABLE poems (id INTEGER PRIMARY KEY, poet_id INTEGER, cat_id INTEGER, title TEXT, full_title TEXT, full_url TEXT, data_json BLOB)"),
                QStringLiteral("CREATE TABLE category_poems (category_id INTEGER, poem_id INTEGER, sort_order INTEGER)"),
                QStringLiteral("INSERT INTO metadata VALUES ('catalog_schema_version', '1'), ('source_schema_version', '1'), ('source_digest_sha256', 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'), ('poets_count', '3'), ('poems_count', '2')"),
                QStringLiteral("INSERT INTO poets VALUES (2, 0, 'hafez', 'حافظ شیرازی', 'حافظ', '/hafez', 'زندگی‌نامه'), (4, 1, 'ferdousi', 'فردوسی', 'فردوسی', '/ferdousi', ''), (222, 2, 'sepehri', 'سهراب سپهری', 'سهراب سپهری', '/sepehri', '')"),
                QStringLiteral("INSERT INTO categories VALUES (9, 2, NULL, 'حافظ', '/hafez', '', 'دیوان حافظ'), (24, 2, 9, 'غزلیات', '/hafez/ghazal', '', ''), (32, 4, NULL, 'فردوسی', '/ferdousi', '', ''), (33, 4, 32, 'شاهنامه', '/ferdousi/shahname', '', ''), (34, 4, 33, 'آغاز کتاب', '/ferdousi/shahname/aghaz', '', ''), (3410, 222, NULL, 'سپهری', '/sepehri', '', '')"),
                QStringLiteral("INSERT INTO category_children VALUES (9, 24, 0), (32, 33, 0), (33, 34, 0)")
            };
            for (const QString &statement : statements) {
                QVERIFY2(query.exec(statement), qPrintable(query.lastError().text()));
            }

            QJsonArray verses;
            const QString longLine(400, QChar(0x0634));
            for (int order = 1; order <= 2500; ++order) {
                verses.append(QJsonObject{
                    {QStringLiteral("VOrder"), order},
                    {QStringLiteral("Position"), order % 2 ? QStringLiteral("Right") : QStringLiteral("Left")},
                    {QStringLiteral("Text"), longLine},
                    {QStringLiteral("CoupletIndex"), (order - 1) / 2},
                    {QStringLiteral("SectionIndex1"), 0}
                });
            }
            const QJsonObject poem {
                {QStringLiteral("Id"), 500},
                {QStringLiteral("CatId"), 34},
                {QStringLiteral("Title"), QStringLiteral("شعر بلند")},
                {QStringLiteral("FullUrl"), QStringLiteral("/ferdousi/shahname/aghaz/long")},
                {QStringLiteral("PoemSummary"), QStringLiteral("خلاصه\n  متن")},
                {QStringLiteral("Metre"), QJsonObject{{QStringLiteral("Rhythm"), QStringLiteral("وزن آزمایشی")}}},
                {QStringLiteral("Sections"), QJsonArray{QJsonObject{
                    {QStringLiteral("Index"), 0}, {QStringLiteral("Number"), 1},
                    {QStringLiteral("SectionType"), QStringLiteral("WholePoem")},
                    {QStringLiteral("PlainText"), QStringLiteral("بخش یک")}}}},
                {QStringLiteral("Verses"), verses}
            };
            QVERIFY(query.prepare(QStringLiteral("INSERT INTO poems VALUES (?, ?, ?, ?, ?, ?, ?)")));
            query.addBindValue(500);
            query.addBindValue(4);
            query.addBindValue(34);
            query.addBindValue(QStringLiteral("شعر بلند"));
            query.addBindValue(QStringLiteral("فردوسی » شعر بلند"));
            query.addBindValue(QStringLiteral("/ferdousi/shahname/aghaz/long"));
            query.addBindValue(QJsonDocument(poem).toJson(QJsonDocument::Compact));
            QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
            QVERIFY(query.exec(QStringLiteral("INSERT INTO category_poems VALUES (34, 500, 0)")));

            const QJsonObject shortPoem {
                {QStringLiteral("Id"), 501}, {QStringLiteral("CatId"), 3410},
                {QStringLiteral("Title"), QStringLiteral("شعر کوتاه")},
                {QStringLiteral("FullUrl"), QStringLiteral("/sepehri/short")},
                {QStringLiteral("Sections"), QJsonArray{}},
                {QStringLiteral("Verses"), QJsonArray{}}
            };
            QVERIFY(query.prepare(QStringLiteral("INSERT INTO poems VALUES (?, ?, ?, ?, ?, ?, ?)")));
            query.addBindValue(501);
            query.addBindValue(222);
            query.addBindValue(3410);
            query.addBindValue(QStringLiteral("شعر کوتاه"));
            query.addBindValue(QStringLiteral("سپهری » شعر کوتاه"));
            query.addBindValue(QStringLiteral("/sepehri/short"));
            query.addBindValue(QJsonDocument(shortPoem).toJson(QJsonDocument::Compact));
            QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
            QVERIFY(query.exec(QStringLiteral("INSERT INTO category_poems VALUES (3410, 501, 0)")));

            const QJsonObject rootPoem {
                {QStringLiteral("Id"), 502}, {QStringLiteral("CatId"), 9},
                {QStringLiteral("Title"), QStringLiteral("شعر ریشه")},
                {QStringLiteral("FullUrl"), QStringLiteral("/hafez/root")},
                {QStringLiteral("Sections"), QJsonArray{}},
                {QStringLiteral("Verses"), QJsonArray{}}
            };
            QVERIFY(query.prepare(QStringLiteral("INSERT INTO poems VALUES (?, ?, ?, ?, ?, ?, ?)")));
            query.addBindValue(502);
            query.addBindValue(2);
            query.addBindValue(9);
            query.addBindValue(QStringLiteral("شعر ریشه"));
            query.addBindValue(QStringLiteral("حافظ » شعر ریشه"));
            query.addBindValue(QStringLiteral("/hafez/root"));
            query.addBindValue(QJsonDocument(rootPoem).toJson(QJsonDocument::Compact));
            QVERIFY2(query.exec(), qPrintable(query.lastError().text()));
            QVERIFY(query.exec(QStringLiteral("INSERT INTO category_poems VALUES (9, 502, 0)")));
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);
}

void DataTests::navigationAndFiltering()
{
    CatalogRepository repository;
    QVERIFY(repository.openCatalog(m_fixturePath));
    PoetListModel poets(&repository);
    QCOMPARE(poets.count(), 3);
    poets.setFilterText(QStringLiteral("حافظ"));
    QCOMPARE(poets.count(), 1);
    QCOMPARE(poets.data(poets.index(0, 0), PoetListModel::FullUrlRole).toString(), QStringLiteral("/hafez"));
    poets.setFilterText(QStringLiteral("فردوسی"));
    QCOMPARE(poets.count(), 1);
    poets.setFilterText({});
    QCOMPARE(poets.count(), 3);
    QCOMPARE(poets.data(poets.index(0, 0), PoetListModel::FullUrlRole).toString(), QStringLiteral("/hafez"));

    CollectionListModel collection(&repository);
    PoemLoader poemLoader(m_fixturePath);
    NavigationController navigation(&repository, &collection, &poemLoader);
    QCOMPARE(navigation.page(), QStringLiteral("poets"));
    QCOMPARE(navigation.breadcrumbs().size(), 1);
    QVERIFY(navigation.openPoet(QStringLiteral("/hafez"), 125));
    QCOMPARE(navigation.page(), QStringLiteral("poet"));
    QCOMPARE(navigation.poetDescription(), QStringLiteral("زندگی‌نامه"));
    QCOMPARE(collection.categoryCount(), 1);
    QCOMPARE(collection.poemCount(), 1);
    QCOMPARE(collection.data(collection.index(1, 0), CollectionListModel::FullUrlRole).toString(),
             QStringLiteral("/hafez/root"));
    QVERIFY(navigation.openPoem(QStringLiteral("/hafez/root"), 125, 44));
    QCOMPARE(navigation.page(), QStringLiteral("poem"));
    QCOMPARE(navigation.breadcrumbs().size(), 3);
    QVERIFY(navigation.back(125, 44));
    QCOMPARE(navigation.page(), QStringLiteral("poet"));
    QCOMPARE(navigation.collectionScroll(), 44);

    QVERIFY(navigation.openPoet(QStringLiteral("/sepehri"), 280));
    QCOMPARE(collection.categoryCount(), 0);
    QCOMPARE(collection.poemCount(), 1);
    QVERIFY(navigation.openPoet(QStringLiteral("/ferdousi"), 310));
    QVERIFY(navigation.openCategory(QStringLiteral("/ferdousi/shahname"), 310, 90));
    QCOMPARE(navigation.breadcrumbs().size(), 3);
    QVERIFY(navigation.openCategory(QStringLiteral("/ferdousi/shahname/aghaz"), 310, 240));
    QCOMPARE(navigation.breadcrumbs().size(), 4);
    QVERIFY(navigation.openPoem(QStringLiteral("/ferdousi/shahname/aghaz/long"), 310, 430));
    QCOMPARE(navigation.breadcrumbs().size(), 5);
    QCOMPARE(navigation.poemPosition(), 1);
    QCOMPARE(navigation.poemCount(), 1);
    QVERIFY(navigation.back(310, 430));
    QCOMPARE(navigation.page(), QStringLiteral("collection"));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname/aghaz"));
    QCOMPARE(navigation.collectionScroll(), 430);
    QVERIFY(navigation.forward(310, 390));
    QCOMPARE(navigation.page(), QStringLiteral("poem"));
    QVERIFY(navigation.openBreadcrumb(2, 310, 430));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname"));
    QVERIFY(!navigation.canGoForward());
    QVERIFY(navigation.back(310, 90));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname/aghaz/long"));
    QVERIFY(navigation.forward(310, 430));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname"));
    QVERIFY(!navigation.openCategory(QStringLiteral("/missing")));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname"));
    QCOMPARE(collection.fullUrl(), QStringLiteral("/ferdousi/shahname"));
    QVERIFY(navigation.openBreadcrumb(1));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi"));
    QVERIFY(navigation.openCategory(QStringLiteral("/ferdousi/shahname/aghaz")));
    QVERIFY(navigation.openBreadcrumb(3));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname/aghaz"));
    QVERIFY(navigation.openPoem(QStringLiteral("/ferdousi/shahname/aghaz/long")));
    QVERIFY(navigation.openBreadcrumb(4));
    QCOMPARE(navigation.url(), QStringLiteral("/ferdousi/shahname/aghaz/long"));
    QVERIFY(navigation.openBreadcrumb(0));
    QCOMPARE(navigation.page(), QStringLiteral("poets"));
}

void DataTests::repositoryAndModels()
{
    const qint64 originalSize = QFileInfo(m_fixturePath).size();
    {
        CatalogRepository repository;
        QVERIFY(repository.openCatalog(m_fixturePath));
        QVERIFY(repository.ready());
        QCOMPARE(repository.poets().size(), 3);
        QCOMPARE(repository.poetByUrl(QStringLiteral("/hafez"))->nickname, QStringLiteral("حافظ"));
        QCOMPARE(repository.categoryByUrl(QStringLiteral("/ferdousi/shahname/aghaz"))->parentId, 33);
        QCOMPARE(repository.categoryPoems(3410).constFirst().fullUrl, QStringLiteral("/sepehri/short"));
        QCOMPARE(repository.poemByUrl(QStringLiteral("/ferdousi/shahname/aghaz/long"))->categoryId, 34);

        PoetListModel poets(&repository);
        QCOMPARE(poets.count(), 3);
        QCOMPARE(poets.data(poets.index(0, 0), PoetListModel::FullUrlRole).toString(), QStringLiteral("/hafez"));
        CollectionListModel collection(&repository);
        QVERIFY(collection.loadCategory(QStringLiteral("/ferdousi/shahname")));
        QCOMPARE(collection.count(), 1);
        QCOMPARE(collection.data(collection.index(0, 0), CollectionListModel::FullUrlRole).toString(),
                 QStringLiteral("/ferdousi/shahname/aghaz"));
        QVERIFY(collection.loadCategory(QStringLiteral("/sepehri")));
        QCOMPARE(collection.data(collection.index(0, 0), CollectionListModel::EntryTypeRole).toString(),
                 QStringLiteral("poem"));
    }
    QCOMPARE(QFileInfo(m_fixturePath).size(), originalSize);
}

void DataTests::missingAndIncompatibleCatalog()
{
    const QString missing = m_temporary.filePath(QStringLiteral("missing.sqlite"));
    CatalogRepository repository;
    QVERIFY(!repository.openCatalog(missing));
    QVERIFY(!QFileInfo::exists(missing));
    QVERIFY(!repository.error().isEmpty());

    const QString incompatible = m_temporary.filePath(QStringLiteral("incompatible.sqlite"));
    QVERIFY(QFile::copy(m_fixturePath, incompatible));
    const QString connection = QStringLiteral("incompatible_writer");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(incompatible);
        QVERIFY(database.open());
        {
            QSqlQuery query(database);
            QVERIFY(query.exec(QStringLiteral("UPDATE metadata SET value='999' WHERE key='catalog_schema_version'")));
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);
    QVERIFY(!repository.openCatalog(incompatible));
    QVERIFY(repository.error().contains(QStringLiteral("نسخه")));

    QString pathError;
    QVERIFY(CatalogPaths::resolve({QStringLiteral("app"), QStringLiteral("--catalog")}, &pathError).isEmpty());
    QVERIFY(!pathError.isEmpty());
    QCOMPARE(CatalogPaths::resolve({QStringLiteral("app"), QStringLiteral("--catalog"), m_fixturePath}),
             m_fixturePath);
}

void DataTests::largePoemLoadsAsynchronously()
{
    PoemLoader loader(m_fixturePath);
    QSignalSpy finished(&loader, &PoemLoader::requestFinished);
    loader.loadByUrl(QStringLiteral("/ferdousi/shahname/aghaz/long"));
    QVERIFY(loader.loading());
    QCOMPARE(finished.size(), 0);
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 10000);
    QVERIFY(finished.constFirst().at(0).toBool());
    QVERIFY(!loader.loading());
    QCOMPARE(loader.verses()->rowCount(), 2500);
    QCOMPARE(loader.readingRows()->rowCount(), 1250);
    QCOMPARE(loader.sections()->rowCount(), 1);
    QCOMPARE(loader.metre(), QStringLiteral("وزن آزمایشی"));
    QCOMPARE(loader.summary(), QStringLiteral("خلاصه متن"));
    QCOMPARE(loader.verses()->data(loader.verses()->index(0, 0), VerseListModel::PositionRole).toString(),
             QStringLiteral("Right"));
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::PairedRole).toBool(), true);
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::RightTextRole).toString(),
             loader.verses()->data(loader.verses()->index(0, 0), VerseListModel::TextRole).toString());
    loader.loadByUrl(QStringLiteral("/missing/poem"));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 2, 10000);
    QVERIFY(!finished.at(1).at(0).toBool());
    QVERIFY(!loader.error().isEmpty());
    QCOMPARE(loader.verses()->rowCount(), 0);
    QCOMPARE(loader.readingRows()->rowCount(), 0);
}

void DataTests::bookmarksPersistAndNavigate()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    CatalogRepository repository;
    QVERIFY(repository.openCatalog(m_fixturePath));
    CollectionListModel collection(&repository);
    PoemLoader loader(m_fixturePath);
    NavigationController navigation(&repository, &collection, &loader);
    const QString deepCollection = QStringLiteral("/ferdousi/shahname/aghaz");
    const QString deepPoem = deepCollection + QStringLiteral("/long");
    {
        BookmarkStore bookmarks(&repository, &navigation, configBase.path());
        QCOMPARE(bookmarks.count(), 0);
        QVERIFY(QFile::exists(bookmarks.filePath()));
        QVERIFY(!bookmarks.canFavoriteCurrent());
        QVERIFY(bookmarks.add(QStringLiteral("poet"), QStringLiteral("/ferdousi")));
        QVERIFY(bookmarks.add(QStringLiteral("collection"), deepCollection));
        QVERIFY(navigation.openPoem(deepPoem));
        QVERIFY(bookmarks.canFavoriteCurrent());
        QVERIFY(!bookmarks.currentFavorite());
        QVERIFY(bookmarks.toggleCurrent());
        QVERIFY(bookmarks.currentFavorite());
        QCOMPARE(bookmarks.count(), 3);
        QVERIFY(!bookmarks.add(QStringLiteral("poem"), deepPoem));
        QCOMPARE(bookmarks.count(), 3);
        QCOMPARE(bookmarks.data(bookmarks.index(0, 0), BookmarkStore::ContextRole).toString(),
                 QStringLiteral("فردوسی » شاهنامه » آغاز کتاب"));
        QVERIFY(bookmarks.remove(QStringLiteral("poet"), QStringLiteral("/ferdousi")));
        QCOMPARE(bookmarks.count(), 2);
    }
    CatalogRepository reopenedRepository;
    QVERIFY(reopenedRepository.openCatalog(m_fixturePath));
    CollectionListModel reopenedCollection(&reopenedRepository);
    PoemLoader reopenedLoader(m_fixturePath);
    NavigationController reopenedNavigation(&reopenedRepository, &reopenedCollection, &reopenedLoader);
    BookmarkStore restored(&reopenedRepository, &reopenedNavigation, configBase.path());
    QCOMPARE(restored.count(), 2);
    QVERIFY(restored.contains(QStringLiteral("poem"), deepPoem));
    QCOMPARE(restored.data(restored.index(0, 0), BookmarkStore::AvailableRole).toBool(), true);
    QVERIFY(reopenedNavigation.openPoem(restored.data(restored.index(0, 0), BookmarkStore::UrlRole).toString()));
    QCOMPARE(reopenedNavigation.page(), QStringLiteral("poem"));
    QCOMPARE(reopenedNavigation.breadcrumbs().size(), 5);
    QCOMPARE(reopenedNavigation.breadcrumbs().at(3).toMap().value(QStringLiteral("url")).toString(), deepCollection);
    QVERIFY(restored.currentFavorite());
    QVERIFY(restored.toggleCurrent());
    QVERIFY(!restored.currentFavorite());
    QCOMPARE(restored.count(), 1);
}

void DataTests::bookmarksHandleMissingAndInvalidFiles()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    CatalogRepository repository;
    QVERIFY(repository.openCatalog(m_fixturePath));
    CollectionListModel collection(&repository);
    PoemLoader loader(m_fixturePath);
    NavigationController navigation(&repository, &collection, &loader);
    const QString url = QStringLiteral("/ferdousi/shahname/aghaz/long");
    {
        BookmarkStore bookmarks(&repository, &navigation, configBase.path());
        QVERIFY(bookmarks.add(QStringLiteral("poem"), url));
    }

    const QString missingCatalog = m_temporary.filePath(QStringLiteral("missing-bookmark-poem.sqlite"));
    QVERIFY(QFile::copy(m_fixturePath, missingCatalog));
    const QString connection = QStringLiteral("missing_bookmark_writer");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(missingCatalog);
        QVERIFY(database.open());
        {
            QSqlQuery query(database);
            QVERIFY(query.exec(QStringLiteral("DELETE FROM category_poems WHERE poem_id = 500")));
            QVERIFY(query.exec(QStringLiteral("DELETE FROM poems WHERE id = 500")));
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);
    CatalogRepository changedRepository;
    QVERIFY(changedRepository.openCatalog(missingCatalog));
    CollectionListModel changedCollection(&changedRepository);
    PoemLoader changedLoader(missingCatalog);
    NavigationController changedNavigation(&changedRepository, &changedCollection, &changedLoader);
    BookmarkStore stale(&changedRepository, &changedNavigation, configBase.path());
    QCOMPARE(stale.count(), 1);
    QCOMPARE(stale.data(stale.index(0, 0), BookmarkStore::AvailableRole).toBool(), false);
    QCOMPARE(stale.data(stale.index(0, 0), BookmarkStore::TitleRole).toString(), QStringLiteral("شعر بلند"));
    QVERIFY(stale.remove(QStringLiteral("poem"), url));
    QCOMPARE(stale.count(), 0);

    QFile file(stale.filePath());
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    const QByteArray invalid = "{invalid";
    QCOMPARE(file.write(invalid), invalid.size());
    file.close();
    BookmarkStore corrupted(&changedRepository, &changedNavigation, configBase.path());
    QVERIFY(!corrupted.error().isEmpty());
    QVERIFY(!corrupted.add(QStringLiteral("poet"), QStringLiteral("/ferdousi")));
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(file.readAll(), invalid);

    QTemporaryDir unreadableBase;
    QVERIFY(unreadableBase.isValid());
    QVERIFY(UserDataPaths::ensureDirectory(unreadableBase.path()));
    QVERIFY(QDir().mkpath(UserDataPaths::bookmarksFile(unreadableBase.path())));
    BookmarkStore unreadable(&changedRepository, &changedNavigation, unreadableBase.path());
    QVERIFY(!unreadable.error().isEmpty());
}

void DataTests::fullCatalogQueries()
{
    const QString path = qEnvironmentVariable("PARSINAMA_FULL_CATALOG");
    if (path.isEmpty()) {
        QSKIP("Set PARSINAMA_FULL_CATALOG to run the full-data integration check");
    }
    CatalogRepository repository;
    QVERIFY(repository.openCatalog(path));
    QCOMPARE(repository.poets().size(), 240);
    QCOMPARE(repository.poetByUrl(QStringLiteral("/hafez"))->id, 2);
    QCOMPARE(repository.categoryPoems(repository.categoryByUrl(QStringLiteral("/hafez/ghazal"))->id).size(), 495);
    const QList<PoemRecord> ghazals = repository.categoryPoems(
        repository.categoryByUrl(QStringLiteral("/hafez/ghazal"))->id);
    CollectionListModel collection(&repository);
    PoemLoader navigationLoader(path);
    NavigationController navigation(&repository, &collection, &navigationLoader);
    QVERIFY(navigation.openPoem(ghazals.first().fullUrl));
    QCOMPARE(navigation.poemPosition(), 1);
    QVERIFY(!navigation.hasPreviousPoem());
    QVERIFY(navigation.hasNextPoem());
    QVERIFY(!navigation.openPreviousPoem());
    QVERIFY(navigation.openNextPoem());
    QCOMPARE(navigation.url(), ghazals.at(1).fullUrl);
    QCOMPARE(navigation.poemPosition(), 2);
    QVERIFY(navigation.hasPreviousPoem());
    QVERIFY(navigation.hasNextPoem());
    QCOMPARE(navigation.breadcrumbs().last().toMap().value(QStringLiteral("url")).toString(), ghazals.at(1).fullUrl);
    QVERIFY(navigation.openPreviousPoem());
    QCOMPARE(navigation.url(), ghazals.first().fullUrl);
    QVERIFY(navigation.openPoem(ghazals.at(ghazals.size() / 2).fullUrl));
    QCOMPARE(navigation.poemPosition(), ghazals.size() / 2 + 1);
    QVERIFY(navigation.hasPreviousPoem());
    QVERIFY(navigation.hasNextPoem());
    QVERIFY(navigation.openPoem(ghazals.last().fullUrl));
    QCOMPARE(navigation.poemPosition(), ghazals.size());
    QVERIFY(navigation.hasPreviousPoem());
    QVERIFY(!navigation.hasNextPoem());
    QVERIFY(!navigation.openNextPoem());
    QCOMPARE(navigation.breadcrumbs().last().toMap().value(QStringLiteral("url")).toString(), ghazals.last().fullUrl);
    QVERIFY(navigation.openPreviousPoem());
    QCOMPARE(navigation.url(), ghazals.at(ghazals.size() - 2).fullUrl);
    QVERIFY(navigation.openCategory(QStringLiteral("/hafez/ghazal")));
    QVERIFY(!navigation.hasPreviousPoem());
    QVERIFY(!navigation.hasNextPoem());
    QCOMPARE(repository.categoryByUrl(QStringLiteral("/ferdousi/shahname/aghaz"))->parentId, 33);
    QCOMPARE(repository.categoryPoems(repository.categoryByUrl(QStringLiteral("/sepehri"))->id).size(), 1);
    PoemLoader loader(path);
    QSignalSpy finished(&loader, &PoemLoader::requestFinished);
    loader.loadByUrl(QStringLiteral("/azar/divan/masnavi/sh11"));
    QVERIFY(loader.loading());
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 10000);
    QVERIFY(finished.constFirst().at(0).toBool());
    QCOMPARE(loader.verses()->rowCount(), 2500);
    QCOMPARE(loader.sections()->rowCount(), 1247);
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::KindRole).toString(),
             QStringLiteral("verse"));
    loader.loadByUrl(QStringLiteral("/hafez/ghazal/sh1"));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 2, 10000);
    QVERIFY(finished.at(1).at(0).toBool());
    QCOMPARE(loader.readingRows()->rowCount(), 7);
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::PairedRole).toBool(), true);
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::KindRole).toString(),
             QStringLiteral("verse"));
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::RightTextRole).toString(),
             loader.verses()->data(loader.verses()->index(0, 0), VerseListModel::TextRole).toString());
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::LeftTextRole).toString(),
             loader.verses()->data(loader.verses()->index(1, 0), VerseListModel::TextRole).toString());
    QVERIFY(!loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::NoteRole).toString().isEmpty());
    QVERIFY(!loader.summary().contains(QLatin1Char('\n')));
    loader.loadByUrl(QStringLiteral("/amir/tarjam/sh2"));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 3, 10000);
    QVERIFY(finished.at(2).at(0).toBool());
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::KindRole).toString(),
             QStringLiteral("section"));
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::TextRole).toString(),
             QStringLiteral("بند ۱"));
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(1, 0), ReadingRowListModel::PairedRole).toBool(), true);
    bool sawRefrain = false;
    bool sawSecondBand = false;
    for (int row = 0; row < loader.readingRows()->rowCount(); ++row) {
        const QString heading = loader.readingRows()->data(loader.readingRows()->index(row, 0),
                                                         ReadingRowListModel::TextRole).toString();
        sawRefrain |= heading == QStringLiteral("بند برگردان");
        sawSecondBand |= heading == QStringLiteral("بند ۲");
    }
    QVERIFY(sawRefrain);
    QVERIFY(sawSecondBand);
    loader.loadByUrl(QStringLiteral("/shahriar/torki/sh3"));
    QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 4, 10000);
    QVERIFY(finished.at(3).at(0).toBool());
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::PairedRole).toBool(), false);
    QCOMPARE(loader.readingRows()->data(loader.readingRows()->index(0, 0), ReadingRowListModel::PositionRole).toString(),
             QStringLiteral("Single"));
}

QTEST_GUILESS_MAIN(DataTests)

#include "data_tests.moc"
