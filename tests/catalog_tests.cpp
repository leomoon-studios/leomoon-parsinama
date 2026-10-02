#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

class CatalogTests final : public QObject
{
    Q_OBJECT

private slots:
    void importsOrderedCollectionsAndAliasPoems();

private:
    static bool writeObject(const QString &path, const QJsonObject &object);
    static int runBuilder(const QString &source, const QString &output, QByteArray *errors);
};

bool CatalogTests::writeObject(const QString &path, const QJsonObject &object)
{
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        return false;
    }
    QFile file(path);
    return file.open(QIODevice::WriteOnly)
        && file.write(QJsonDocument(object).toJson(QJsonDocument::Compact)) > 0;
}

int CatalogTests::runBuilder(const QString &source, const QString &output, QByteArray *errors)
{
    QProcess process;
    process.setProgram(qEnvironmentVariable("PARSINAMA_CATALOG_BUILDER"));
    process.setArguments({QStringLiteral("--data-root"), source,
                          QStringLiteral("--output"), output});
    process.start();
    if (!process.waitForStarted(5000) || !process.waitForFinished(10000)) {
        *errors = QByteArray("Catalog builder did not finish");
        return -1;
    }
    *errors = process.readAllStandardError();
    return process.exitCode();
}

void CatalogTests::importsOrderedCollectionsAndAliasPoems()
{
    QTemporaryDir temporary;
    QVERIFY(temporary.isValid());
    const QString data = temporary.filePath(QStringLiteral("data"));
    const auto path = [&data](const QString &relative) {
        return QDir(data).filePath(relative);
    };
    const auto poemRef = [](int id, const QString &title, const QString &url) {
        return QJsonObject{{QStringLiteral("Id"), id},
                           {QStringLiteral("Title"), title},
                           {QStringLiteral("FullUrl"), url}};
    };

    QVERIFY(writeObject(path(QStringLiteral("manifest.json")),
                        {{QStringLiteral("SchemaVersion"), 1},
                         {QStringLiteral("GeneratedAtUtc"), QStringLiteral("2026-01-01T00:00:00Z")},
                         {QStringLiteral("PoetsCount"), 1},
                         {QStringLiteral("PoemsCount"), 2},
                         {QStringLiteral("Poets"), QJsonArray{QJsonObject{
                              {QStringLiteral("Id"), 1},
                              {QStringLiteral("Nickname"), QStringLiteral("شاعر")},
                              {QStringLiteral("FullUrl"), QStringLiteral("/test")}}}}}));
    QVERIFY(writeObject(path(QStringLiteral("poets/test/poet.json")),
                        {{QStringLiteral("Id"), 1},
                         {QStringLiteral("Name"), QStringLiteral("شاعر آزمایشی")},
                         {QStringLiteral("Nickname"), QStringLiteral("شاعر")},
                         {QStringLiteral("FullUrl"), QStringLiteral("/test")}}));
    QVERIFY(writeObject(path(QStringLiteral("poets/test/_cat.json")),
                        {{QStringLiteral("Id"), 10},
                         {QStringLiteral("PoetId"), 1},
                         {QStringLiteral("Title"), QStringLiteral("شاعر")},
                         {QStringLiteral("FullUrl"), QStringLiteral("/test")},
                         {QStringLiteral("ChildCats"), QJsonArray{poemRef(11, QStringLiteral("کتاب"),
                                                                           QStringLiteral("/test/book"))}},
                         {QStringLiteral("Poems"), QJsonArray{poemRef(100, QStringLiteral("ریشه"),
                                                                       QStringLiteral("/test/root"))}}}));
    QVERIFY(writeObject(path(QStringLiteral("poets/test/book/_cat.json")),
                        {{QStringLiteral("Id"), 11},
                         {QStringLiteral("PoetId"), 1},
                         {QStringLiteral("ParentId"), 10},
                         {QStringLiteral("Title"), QStringLiteral("کتاب")},
                         {QStringLiteral("FullUrl"), QStringLiteral("/test/book")},
                         {QStringLiteral("ChildCats"), QJsonArray{}},
                         {QStringLiteral("Poems"), QJsonArray{poemRef(101, QStringLiteral("بخش یک"),
                                                                       QStringLiteral("/test/alias/one"))}}}));
    for (const auto &item : {
             std::tuple{QStringLiteral("poets/test/root.json"), 100, 10,
                        QStringLiteral("ریشه"), QStringLiteral("/test/root")},
             std::tuple{QStringLiteral("poets/test/alias/one.json"), 101, 11,
                        QStringLiteral("بخش یک"), QStringLiteral("/test/alias/one")}}) {
        QVERIFY(writeObject(path(std::get<0>(item)),
                            {{QStringLiteral("Id"), std::get<1>(item)},
                             {QStringLiteral("CatId"), std::get<2>(item)},
                             {QStringLiteral("Title"), std::get<3>(item)},
                             {QStringLiteral("FullUrl"), std::get<4>(item)},
                             {QStringLiteral("Sections"), QJsonArray{}},
                             {QStringLiteral("Verses"), std::get<1>(item) == 100
                                  ? QJsonArray{QJsonObject{{QStringLiteral("Text"), QStringLiteral("مِي‌گويم كی ۱۲")}}}
                                  : QJsonArray{}}}));
    }

    const QString output = temporary.filePath(QStringLiteral("catalog.sqlite"));
    QByteArray errors;
    QCOMPARE(runBuilder(data, output, &errors), 0);
    QVERIFY2(QFileInfo::exists(output), errors.constData());

    const QString connection = QStringLiteral("catalog_test_readonly");
    {
        QSqlDatabase database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
        database.setDatabaseName(output);
        database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
        QVERIFY2(database.open(), qPrintable(database.lastError().text()));
        QSqlQuery query(database);
        QVERIFY(query.exec(QStringLiteral("SELECT c.full_url, p.full_url, cp.sort_order "
                                          "FROM category_poems cp "
                                          "JOIN categories c ON c.id=cp.category_id "
                                          "JOIN poems p ON p.id=cp.poem_id "
                                          "ORDER BY c.id")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/test"));
        QCOMPARE(query.value(1).toString(), QStringLiteral("/test/root"));
        QCOMPARE(query.value(2).toInt(), 0);
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/test/book"));
        QCOMPARE(query.value(1).toString(), QStringLiteral("/test/alias/one"));
        QVERIFY(!query.next());
        query.finish();
        QVERIFY(query.exec(QStringLiteral("SELECT value FROM metadata WHERE key='poems_count'")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("2"));
        query.finish();
        QVERIFY(query.exec(QStringLiteral("SELECT full_url, original_text, normalized_text FROM search_fts "
                                          "WHERE search_fts MATCH '\"میگویم\"*'")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("/test/root"));
        QCOMPARE(query.value(1).toString(), QStringLiteral("مِي‌گويم كی ۱۲"));
        QCOMPARE(query.value(2).toString(), QStringLiteral("میگویم کی 12"));
        QVERIFY(!query.next());
        query.finish();
        QVERIFY(query.exec(QStringLiteral("SELECT value FROM metadata WHERE key='catalog_schema_version'")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("2"));
        query.finish();
        database.close();
    }
    QSqlDatabase::removeDatabase(connection);

    const QString invalid = temporary.filePath(QStringLiteral("invalid.sqlite"));
    QVERIFY(writeObject(path(QStringLiteral("poets/test/book/_cat.json")),
                        {{QStringLiteral("Id"), 11},
                         {QStringLiteral("PoetId"), 1},
                         {QStringLiteral("ParentId"), 10},
                         {QStringLiteral("Title"), QStringLiteral("کتاب")},
                         {QStringLiteral("FullUrl"), QStringLiteral("/test/book")},
                         {QStringLiteral("ChildCats"), QJsonArray{}},
                         {QStringLiteral("Poems"), QJsonArray{poemRef(101, QStringLiteral("بخش یک"),
                                                                       QStringLiteral("/test/../escape"))}}}));
    QVERIFY(runBuilder(data, invalid, &errors) != 0);
    QVERIFY(errors.contains("Unsafe FullUrl"));
    QVERIFY(!QFileInfo::exists(invalid));
    QVERIFY(QFileInfo::exists(output));
}

QTEST_GUILESS_MAIN(CatalogTests)

#include "catalog_tests.moc"
