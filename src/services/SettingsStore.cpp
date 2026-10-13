#include "SettingsStore.h"

#include "UserDataPaths.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <cmath>
#include <utility>

namespace {
constexpr int settingsVersion = 2;
constexpr int minimumReadingSize = 16;
constexpr int maximumReadingSize = 40;
constexpr int minimumUiScaleTenths = 10;
constexpr int maximumUiScaleTenths = 30;

int validUiScaleTenths(qreal scale)
{
    if (!std::isfinite(scale) || scale < minimumUiScaleTenths / 10.0
        || scale > maximumUiScaleTenths / 10.0)
        return -1;
    const qreal tenths = scale * 10;
    const int rounded = qRound(tenths);
    return rounded >= minimumUiScaleTenths && rounded <= maximumUiScaleTenths
        && std::abs(tenths - rounded) < 0.000001 ? rounded : -1;
}

bool validAccentPreset(const QString &preset)
{
    return preset == QLatin1String("purple") || preset == QLatin1String("slate")
        || preset == QLatin1String("faint") || preset == QLatin1String("blue")
        || preset == QLatin1String("teal") || preset == QLatin1String("rose");
}

bool validPoemFont(const QString &font)
{
    return font == QLatin1String("vazirmatn") || font == QLatin1String("irannastaliq");
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
    const QJsonValue uiScale = object.value(QStringLiteral("uiScale"));
    const int uiScaleTenths = uiScale.isUndefined() ? minimumUiScaleTenths
        : uiScale.isDouble() ? validUiScaleTenths(uiScale.toDouble()) : -1;
    const QJsonValue poemFont = object.value(QStringLiteral("poemFont"));
    const QJsonValue hidePoemTranslations = object.value(QStringLiteral("hidePoemTranslations"));
    if ((version != 1 && version != settingsVersion)
        || (theme != QLatin1String("dark") && theme != QLatin1String("light"))
        || !validAccentPreset(accentPreset)
        || !readingSize.isDouble()
        || readingSize.toDouble() != readingSize.toInt()
        || readingSize.toInt() < minimumReadingSize
        || readingSize.toInt() > maximumReadingSize
        || uiScaleTenths < minimumUiScaleTenths
        || (!poemFont.isUndefined()
            && (!poemFont.isString() || !validPoemFont(poemFont.toString())))
        || (!hidePoemTranslations.isUndefined() && !hidePoemTranslations.isBool())) {
        setError(QStringLiteral("نسخه یا مقدارهای تنظیمات معتبر نیست."));
        return false;
    }
    m_theme = theme;
    m_accentPreset = accentPreset;
    m_readingSize = readingSize.toInt();
    m_uiScaleTenths = uiScaleTenths;
    m_poemFont = poemFont.isUndefined() ? QStringLiteral("vazirmatn") : poemFont.toString();
    m_hidePoemTranslations = hidePoemTranslations.toBool(false);
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
        {QStringLiteral("readingSize"), m_readingSize},
        {QStringLiteral("uiScale"), m_uiScaleTenths / 10.0},
        {QStringLiteral("poemFont"), m_poemFont},
        {QStringLiteral("hidePoemTranslations"), m_hidePoemTranslations}
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

void SettingsStore::setUiScale(qreal scale)
{
    const int tenths = validUiScaleTenths(scale);
    if (tenths < minimumUiScaleTenths || m_uiScaleTenths == tenths)
        return;
    m_uiScaleTenths = tenths;
    emit settingsChanged();
    save();
}

void SettingsStore::setPoemFont(const QString &font)
{
    if (!validPoemFont(font) || m_poemFont == font) {
        return;
    }
    m_poemFont = font;
    emit settingsChanged();
    save();
}

void SettingsStore::setHidePoemTranslations(bool hide)
{
    if (m_hidePoemTranslations == hide) {
        return;
    }
    m_hidePoemTranslations = hide;
    emit settingsChanged();
    save();
}

void SettingsStore::toggleTheme()
{
    setTheme(m_theme == QLatin1String("dark") ? QStringLiteral("light") : QStringLiteral("dark"));
}
