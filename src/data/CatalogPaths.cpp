#include "CatalogPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

QString CatalogPaths::installedCatalogPath()
{
    const QDir executableDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MACOS)
    return QFileInfo(executableDir.filePath(QStringLiteral("../Resources/parsinama-catalog.sqlite")))
        .absoluteFilePath();
#elif defined(Q_OS_WIN)
    return executableDir.filePath(QStringLiteral("data/parsinama-catalog.sqlite"));
#else
    return QFileInfo(executableDir.filePath(QStringLiteral("../share/leomoon-parsinama/parsinama-catalog.sqlite")))
        .absoluteFilePath();
#endif
}

QString CatalogPaths::resolve(const QStringList &arguments, QString *error)
{
    if (error) {
        error->clear();
    }
    const qsizetype index = arguments.indexOf(QStringLiteral("--catalog"));
    if (index >= 0) {
        if (index + 1 >= arguments.size() || arguments.at(index + 1).startsWith(QLatin1String("--"))) {
            if (error) {
                *error = QStringLiteral("گزینهٔ --catalog به مسیر پایگاه داده نیاز دارد.");
            }
            return {};
        }
        return QFileInfo(arguments.at(index + 1)).absoluteFilePath();
    }
    const QString overridePath = qEnvironmentVariable("PARSINAMA_CATALOG_PATH");
    if (!overridePath.isEmpty()) {
        return QFileInfo(overridePath).absoluteFilePath();
    }
    const QString localCatalog = QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("parsinama-catalog.sqlite"));
    if (QFileInfo::exists(localCatalog)) {
        return localCatalog;
    }
    return installedCatalogPath();
}
