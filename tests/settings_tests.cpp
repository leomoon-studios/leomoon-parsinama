#include "services/SettingsStore.h"
#include "services/UserDataPaths.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class SettingsTests final : public QObject
{
    Q_OBJECT

private slots:
    void pathsUseHomeDirectory();
    void persistsThemeAndReadingSize();
    void rejectsInvalidSettings();
};

void SettingsTests::pathsUseHomeDirectory()
{
    QCOMPARE(UserDataPaths::directory(), QDir(QDir::homePath()).filePath(QStringLiteral("leomoon-parsinama")));
    QTemporaryDir home;
    QVERIFY(home.isValid());
    QCOMPARE(UserDataPaths::settingsFile(home.path()),
             QDir(home.path()).filePath(QStringLiteral("leomoon-parsinama/settings.json")));
}

void SettingsTests::persistsThemeAndReadingSize()
{
    QTemporaryDir home;
    QVERIFY(home.isValid());
    {
        SettingsStore settings(home.path());
        QCOMPARE(settings.theme(), QStringLiteral("dark"));
        QCOMPARE(settings.readingSize(), 22);
        QVERIFY(QDir(UserDataPaths::directory(home.path())).exists());
        settings.setTheme(QStringLiteral("light"));
        settings.setReadingSize(29);
        QVERIFY(settings.error().isEmpty());
        settings.setReadingSize(100);
        QCOMPARE(settings.readingSize(), 40);
        settings.setReadingSize(29);
    }
    SettingsStore restored(home.path());
    QCOMPARE(restored.theme(), QStringLiteral("light"));
    QCOMPARE(restored.readingSize(), 29);
    restored.toggleTheme();
    QCOMPARE(restored.theme(), QStringLiteral("dark"));
    QFile file(restored.filePath());
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonObject document = QJsonDocument::fromJson(file.readAll()).object();
    QCOMPARE(document.value(QStringLiteral("version")).toInt(), 1);
    QCOMPARE(document.value(QStringLiteral("theme")).toString(), QStringLiteral("dark"));
    QCOMPARE(document.value(QStringLiteral("readingSize")).toInt(), 29);
    QVERIFY(!QFile::exists(restored.filePath() + QStringLiteral(".tmp")));
}

void SettingsTests::rejectsInvalidSettings()
{
    QTemporaryDir home;
    QVERIFY(home.isValid());
    SettingsStore settings(home.path());
    QFile file(settings.filePath());
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("{\"version\":999,\"theme\":\"light\",\"readingSize\":21}") > 0);
    file.close();
    QVERIFY(!settings.reload());
    QVERIFY(!settings.error().isEmpty());
    QCOMPARE(settings.theme(), QStringLiteral("dark"));
}

QTEST_GUILESS_MAIN(SettingsTests)

#include "settings_tests.moc"
