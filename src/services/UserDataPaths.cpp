#include "UserDataPaths.h"

#include <QDir>
#include <QStandardPaths>

QString UserDataPaths::directory(const QString &configBasePath)
{
    const QString configBase = configBasePath.isEmpty()
        ? QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
        : configBasePath;
    return QDir(configBase).filePath(QStringLiteral("leomoon-parsinama"));
}

QString UserDataPaths::settingsFile(const QString &configBasePath)
{
    return QDir(directory(configBasePath)).filePath(QStringLiteral("settings.json"));
}

bool UserDataPaths::ensureDirectory(const QString &configBasePath)
{
    return QDir().mkpath(directory(configBasePath));
}
