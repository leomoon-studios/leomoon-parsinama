#include "UserDataPaths.h"

#include <QDir>

QString UserDataPaths::directory(const QString &homePath)
{
    const QString home = homePath.isEmpty() ? QDir::homePath() : homePath;
    return QDir(home).filePath(QStringLiteral("leomoon-parsinama"));
}

QString UserDataPaths::settingsFile(const QString &homePath)
{
    return QDir(directory(homePath)).filePath(QStringLiteral("settings.json"));
}

bool UserDataPaths::ensureDirectory(const QString &homePath)
{
    return QDir().mkpath(directory(homePath));
}
