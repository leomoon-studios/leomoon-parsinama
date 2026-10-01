#pragma once

#include <QString>

class UserDataPaths
{
public:
    static QString directory(const QString &homePath = {});
    static QString settingsFile(const QString &homePath = {});
    static bool ensureDirectory(const QString &homePath = {});
};
