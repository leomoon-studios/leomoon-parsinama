#include "services/SettingsStore.h"
#include "services/UserDataPaths.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

class SettingsTests final : public QObject
{
    Q_OBJECT

private slots:
    void pathsUsePlatformConfigDirectory();
    void createsDefaultSettingsOnFirstRun();
    void persistsThemeAndReadingSize();
    void rejectsInvalidSettings();
    void readsPreviousSettingsVersion();
};

void SettingsTests::pathsUsePlatformConfigDirectory()
{
    QCOMPARE(UserDataPaths::directory(),
             QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation))
                 .filePath(QStringLiteral("leomoon-parsinama")));
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    QCOMPARE(UserDataPaths::settingsFile(configBase.path()),
             QDir(configBase.path()).filePath(QStringLiteral("leomoon-parsinama/settings.json")));
    QCOMPARE(UserDataPaths::bookmarksFile(configBase.path()),
             QDir(configBase.path()).filePath(QStringLiteral("leomoon-parsinama/bookmarks.json")));
    const QStringList platformBases {
        QStringLiteral("/home/reader/.config"),
        QStringLiteral("/Users/reader/Library/Preferences"),
        QStringLiteral("C:/Users/reader/AppData/Local")
    };
    for (const QString &base : platformBases) {
        QCOMPARE(UserDataPaths::bookmarksFile(base),
                 QDir(base).filePath(QStringLiteral("leomoon-parsinama/bookmarks.json")));
    }
}

void SettingsTests::createsDefaultSettingsOnFirstRun()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    const QString path = UserDataPaths::settingsFile(configBase.path());
    QVERIFY(!QFile::exists(path));

    SettingsStore settings(configBase.path());
    QVERIFY(settings.error().isEmpty());
    QCOMPARE(settings.filePath(), path);
    QCOMPARE(settings.theme(), QStringLiteral("light"));
    QCOMPARE(settings.accentPreset(), QStringLiteral("purple"));
    QCOMPARE(settings.readingSize(), 16);

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonObject document = QJsonDocument::fromJson(file.readAll()).object();
    QCOMPARE(document.value(QStringLiteral("version")).toInt(), 2);
    QCOMPARE(document.value(QStringLiteral("theme")).toString(), QStringLiteral("light"));
    QCOMPARE(document.value(QStringLiteral("accentPreset")).toString(), QStringLiteral("purple"));
    QCOMPARE(document.value(QStringLiteral("readingSize")).toInt(), 16);
}

void SettingsTests::persistsThemeAndReadingSize()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    {
        SettingsStore settings(configBase.path());
        QCOMPARE(settings.theme(), QStringLiteral("light"));
        QCOMPARE(settings.accentPreset(), QStringLiteral("purple"));
        QCOMPARE(settings.readingSize(), 16);
        QVERIFY(QDir(UserDataPaths::directory(configBase.path())).exists());
        settings.setTheme(QStringLiteral("dark"));
        settings.setAccentPreset(QStringLiteral("teal"));
        settings.setAccentPreset(QStringLiteral("invalid"));
        QCOMPARE(settings.accentPreset(), QStringLiteral("teal"));
        settings.setReadingSize(29);
        QVERIFY(settings.error().isEmpty());
        settings.setReadingSize(100);
        QCOMPARE(settings.readingSize(), 40);
        settings.setReadingSize(29);
    }
    SettingsStore restored(configBase.path());
    QCOMPARE(restored.theme(), QStringLiteral("dark"));
    QCOMPARE(restored.accentPreset(), QStringLiteral("teal"));
    QCOMPARE(restored.readingSize(), 29);
    restored.toggleTheme();
    QCOMPARE(restored.theme(), QStringLiteral("light"));
    QFile file(restored.filePath());
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QJsonObject document = QJsonDocument::fromJson(file.readAll()).object();
    QCOMPARE(document.value(QStringLiteral("version")).toInt(), 2);
    QCOMPARE(document.value(QStringLiteral("theme")).toString(), QStringLiteral("light"));
    QCOMPARE(document.value(QStringLiteral("accentPreset")).toString(), QStringLiteral("teal"));
    QCOMPARE(document.value(QStringLiteral("readingSize")).toInt(), 29);
    QVERIFY(!QFile::exists(restored.filePath() + QStringLiteral(".tmp")));
}

void SettingsTests::readsPreviousSettingsVersion()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    QVERIFY(UserDataPaths::ensureDirectory(configBase.path()));
    QFile file(UserDataPaths::settingsFile(configBase.path()));
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("{\"version\":1,\"theme\":\"light\",\"readingSize\":29}") > 0);
    file.close();
    SettingsStore settings(configBase.path());
    QVERIFY(settings.error().isEmpty());
    QCOMPARE(settings.theme(), QStringLiteral("light"));
    QCOMPARE(settings.readingSize(), 29);
    QCOMPARE(settings.accentPreset(), QStringLiteral("purple"));
}

void SettingsTests::rejectsInvalidSettings()
{
    QTemporaryDir configBase;
    QVERIFY(configBase.isValid());
    SettingsStore settings(configBase.path());
    QFile file(settings.filePath());
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("{\"version\":999,\"theme\":\"light\",\"readingSize\":21}") > 0);
    file.close();
    QVERIFY(!settings.reload());
    QVERIFY(!settings.error().isEmpty());
    QCOMPARE(settings.theme(), QStringLiteral("light"));
}

QTEST_GUILESS_MAIN(SettingsTests)

#include "settings_tests.moc"
