#include "SettingsStore.h"

#include "UserDataPaths.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <utility>

namespace {
constexpr int settingsVersion = 2;
constexpr int minimumReadingSize = 16;
constexpr int maximumReadingSize = 40;

bool validAccentPreset(const QString &preset)
{
    return preset == QLatin1String("purple") || preset == QLatin1String("slate")
        || preset == QLatin1String("faint") || preset == QLatin1String("blue")
        || preset == QLatin1String("teal") || preset == QLatin1String("rose");
}
}

SettingsStore::SettingsStore(QString configBasePath, QObject *parent)
    : QObject(parent), m_configBasePath(std::move(configBasePath))
{
    reload();
}

QString SettingsStore::filePath() const
{
    return UserDataPaths::settingsFile(m_configBasePath);
}

void SettingsStore::setError(const QString &error)
{
    if (m_error == error) {
        return;
    }
    m_error = error;
    emit errorChanged();
}

bool SettingsStore::reload()
{
    if (!UserDataPaths::ensureDirectory(m_configBasePath)) {
        setError(QStringLiteral("ساخت پوشهٔ تنظیمات ممکن نشد."));
        return false;
    }
    QFile file(filePath());
    if (!file.exists()) {
        return save();
    }
    if (!file.open(QIODevice::ReadOnly)) {
        setError(QStringLiteral("خواندن تنظیمات ممکن نشد."));
        return false;
    }
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setError(QStringLiteral("پروندهٔ تنظیمات معتبر نیست."));
        return false;
    }
    const QJsonObject object = document.object();
    const int version = object.value(QStringLiteral("version")).toInt();
    const QString theme = object.value(QStringLiteral("theme")).toString();
    const QString accentPreset = version == 1 ? QStringLiteral("purple")
        : object.value(QStringLiteral("accentPreset")).toString();
    const QJsonValue readingSize = object.value(QStringLiteral("readingSize"));
    if ((version != 1 && version != settingsVersion)
        || (theme != QLatin1String("dark") && theme != QLatin1String("light"))
        || !validAccentPreset(accentPreset)
        || !readingSize.isDouble()
        || readingSize.toDouble() != readingSize.toInt()
        || readingSize.toInt() < minimumReadingSize
        || readingSize.toInt() > maximumReadingSize) {
        setError(QStringLiteral("نسخه یا مقدارهای تنظیمات معتبر نیست."));
        return false;
    }
    m_theme = theme;
    m_accentPreset = accentPreset;
    m_readingSize = readingSize.toInt();
    setError({});
    emit settingsChanged();
    return true;
}

bool SettingsStore::save()
{
    if (!UserDataPaths::ensureDirectory(m_configBasePath)) {
        setError(QStringLiteral("ساخت پوشهٔ تنظیمات ممکن نشد."));
        return false;
    }
    QSaveFile file(filePath());
    if (!file.open(QIODevice::WriteOnly)) {
        setError(QStringLiteral("نوشتن تنظیمات ممکن نشد."));
        return false;
    }
    const QJsonObject object {
        {QStringLiteral("version"), settingsVersion},
        {QStringLiteral("theme"), m_theme},
        {QStringLiteral("accentPreset"), m_accentPreset},
        {QStringLiteral("readingSize"), m_readingSize}
    };
    const QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Indented);
    if (file.write(data) != data.size()
        || !file.commit()) {
        setError(QStringLiteral("ذخیرهٔ تنظیمات ممکن نشد."));
        return false;
    }
    setError({});
    return true;
}

void SettingsStore::setTheme(const QString &theme)
{
    if ((theme != QLatin1String("dark") && theme != QLatin1String("light"))
        || m_theme == theme) {
        return;
    }
    m_theme = theme;
    emit settingsChanged();
    save();
}

void SettingsStore::setAccentPreset(const QString &preset)
{
    if (!validAccentPreset(preset) || m_accentPreset == preset) {
        return;
    }
    m_accentPreset = preset;
    emit settingsChanged();
    save();
}

void SettingsStore::setReadingSize(int size)
{
    const int bounded = qBound(minimumReadingSize, size, maximumReadingSize);
    if (m_readingSize == bounded) {
        return;
    }
    m_readingSize = bounded;
    emit settingsChanged();
    save();
}

void SettingsStore::toggleTheme()
{
    setTheme(m_theme == QLatin1String("dark") ? QStringLiteral("light") : QStringLiteral("dark"));
}
