#include "CatalogPaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStorageInfo>
#include <algorithm>

namespace {
constexpr auto catalogName = "parsinama-catalog.sqlite";
}

QString CatalogPaths::installedCatalogPath()
{
#ifdef Q_OS_ANDROID
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
        .filePath(QString::fromLatin1(catalogName));
#else
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
#endif
}

bool CatalogPaths::installBundledCatalog(const QStringList &sourcePaths, const QString &manifestPath,
                                         const QString &destinationPath, QString *error,
                                         const std::function<void(qint64, qint64)> &progress)
{
    if (error) error->clear();
    QFile manifestFile(manifestPath);
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("فهرست نسخهٔ داده‌های همراه برنامه خوانده نشد.");
        return false;
    }
    const QList<QByteArray> fields = manifestFile.readAll().trimmed().toLower().split(' ');
    if (fields.size() != 3) {
        if (error) *error = QStringLiteral("فهرست داده‌های همراه برنامه نامعتبر است.");
        return false;
    }
    const QByteArray expected = fields.at(0);
    bool sizeOk = false;
    bool countOk = false;
    const qint64 total = fields.at(1).toLongLong(&sizeOk);
    const int count = fields.at(2).toInt(&countOk);
    if (expected.size() != 64 || !std::all_of(expected.cbegin(), expected.cend(), [](char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        }) || !sizeOk || total <= 0 || !countOk || count <= 0 || count != sourcePaths.size()) {
        if (error) *error = QStringLiteral("اثر انگشت داده‌های همراه برنامه نامعتبر است.");
        return false;
    }
    qint64 availableBytes = 0;
    for (const QString &part : sourcePaths) {
        QFile source(part);
        if (!source.open(QIODevice::ReadOnly) || source.size() <= 0) {
            if (error) *error = QStringLiteral("بخشی از داده‌های شعر در بستهٔ برنامه پیدا نشد.");
            return false;
        }
        availableBytes += source.size();
    }
    if (availableBytes != total) {
        if (error) *error = QStringLiteral("حجم داده‌های همراه برنامه با فهرست آن سازگار نیست.");
        return false;
    }
    const QString sidecarPath = destinationPath + QStringLiteral(".sha256");
    QFile sidecar(sidecarPath);
    if (QFileInfo(destinationPath).size() == total && sidecar.open(QIODevice::ReadOnly)
        && sidecar.readAll().trimmed() == expected) {
        if (progress) progress(total, total);
        return true;
    }
    const QFileInfo destination(destinationPath);
    if (!QDir().mkpath(destination.absolutePath())) {
        if (error) *error = QStringLiteral("پوشهٔ خصوصی برنامه ساخته نشد: %1").arg(destination.absolutePath());
        return false;
    }
    const QStorageInfo storage(destination.absolutePath());
    if (storage.isValid() && storage.isReady() && storage.bytesAvailable() < total + 16 * 1024 * 1024) {
        if (error) *error = QStringLiteral("فضای خالی کافی برای داده‌های شعر وجود ندارد. دست‌کم %1 گیگابایت فضا آزاد کنید و برنامه را دوباره باز کنید.")
            .arg(QString::number(double(total + 16 * 1024 * 1024) / (1024 * 1024 * 1024), 'f', 1));
        return false;
    }
    QSaveFile output(destinationPath);
    if (!output.open(QIODevice::WriteOnly)) {
        if (error) *error = QStringLiteral("نوشتن داده‌های شعر ممکن نیست. فضای خالی دستگاه را بررسی کنید: %1").arg(output.errorString());
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    qint64 copied = 0;
    if (progress) progress(0, total);
    for (const QString &part : sourcePaths) {
        QFile source(part);
        if (!source.open(QIODevice::ReadOnly)) {
            if (error) *error = QStringLiteral("باز کردن بخشی از داده‌های شعر ناموفق بود.");
            output.cancelWriting();
            return false;
        }
        while (!source.atEnd()) {
            const QByteArray chunk = source.read(1024 * 1024);
            if (chunk.isEmpty()) {
                if (error) *error = QStringLiteral("خواندن داده‌های همراه برنامه ناتمام ماند: %1").arg(source.errorString());
                output.cancelWriting();
                return false;
            }
            hash.addData(chunk);
            if (output.write(chunk) != chunk.size()) {
                if (error) *error = QStringLiteral("فضای خالی دستگاه یا مجوز نوشتن را بررسی کنید: %1").arg(output.errorString());
                output.cancelWriting();
                return false;
            }
            copied += chunk.size();
            if (progress) progress(copied, total);
        }
    }
    if (copied != total || hash.result().toHex() != expected) {
        if (error) *error = QStringLiteral("کپی داده‌های شعر کامل یا معتبر نیست. برنامه را دوباره باز کنید.");
        output.cancelWriting();
        return false;
    }
    if (!output.commit()) {
        if (error) *error = QStringLiteral("ذخیرهٔ داده‌های شعر ناموفق بود. فضای خالی دستگاه را بررسی کنید: %1").arg(output.errorString());
        return false;
    }
    QSaveFile version(sidecarPath);
    if (!version.open(QIODevice::WriteOnly) || version.write(expected) != expected.size()
        || !version.commit()) {
        if (error) *error = QStringLiteral("ثبت نسخهٔ داده‌های شعر ناموفق بود. برنامه را دوباره باز کنید.");
        return false;
    }
    return true;
}

QString CatalogPaths::resolve(const QStringList &arguments, QString *error,
                              const std::function<void(qint64, qint64)> &progress)
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
#ifdef Q_OS_ANDROID
    const QString destination = installedCatalogPath();
    QFile manifest(QStringLiteral("assets:/parsinama-catalog.manifest"));
    if (!manifest.open(QIODevice::ReadOnly)) {
        if (error) *error = QStringLiteral("فهرست داده‌های شعر در بستهٔ برنامه پیدا نشد.");
        return {};
    }
    const QList<QByteArray> fields = manifest.readAll().trimmed().split(' ');
    bool countOk = false;
    const int count = fields.size() == 3 ? fields.at(2).toInt(&countOk) : 0;
    if (!countOk || count <= 0 || count > 1000) {
        if (error) *error = QStringLiteral("فهرست داده‌های شعر نامعتبر است.");
        return {};
    }
    QStringList parts;
    for (int part = 0; part < count; ++part) {
        parts.append(QStringLiteral("assets:/parsinama-catalog-%1.part").arg(part, 4, 10, QLatin1Char('0')));
    }
    if (!installBundledCatalog(parts, QStringLiteral("assets:/parsinama-catalog.manifest"),
                               destination, error, progress)) return {};
    return destination;
#else
    const QString localCatalog = QDir(QCoreApplication::applicationDirPath())
        .filePath(QStringLiteral("parsinama-catalog.sqlite"));
    if (QFileInfo::exists(localCatalog)) {
        return localCatalog;
    }
    return installedCatalogPath();
#endif
}
