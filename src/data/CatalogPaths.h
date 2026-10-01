#pragma once

#include <QString>
#include <QStringList>

class CatalogPaths final
{
public:
    static QString installedCatalogPath();
    static QString resolve(const QStringList &arguments, QString *error = nullptr);
};
