#pragma once

#include <QString>

class UserDataPaths
{
public:
    static QString directory(const QString &configBasePath = {});
    static QString settingsFile(const QString &configBasePath = {});
    static QString bookmarksFile(const QString &configBasePath = {});
    static bool ensureDirectory(const QString &configBasePath = {});
};
