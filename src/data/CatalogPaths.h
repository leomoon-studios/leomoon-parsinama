#pragma once

#include <QString>
#include <QStringList>
#include <functional>

class CatalogPaths final
{
public:
    static QString installedCatalogPath();
    static QString resolve(const QStringList &arguments, QString *error = nullptr,
                           const std::function<void(qint64, qint64)> &progress = {});
    static bool installBundledCatalog(const QString &sourcePath, const QString &checksumPath,
                                      const QString &destinationPath, QString *error,
                                      const std::function<void(qint64, qint64)> &progress = {});
};
