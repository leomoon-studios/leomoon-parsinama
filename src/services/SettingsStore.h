#pragma once

#include <QObject>
#include <QString>

class SettingsStore final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY settingsChanged)
    Q_PROPERTY(QString accentPreset READ accentPreset WRITE setAccentPreset NOTIFY settingsChanged)
    Q_PROPERTY(int readingSize READ readingSize WRITE setReadingSize NOTIFY settingsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)

public:
    explicit SettingsStore(QString configBasePath = {}, QObject *parent = nullptr);

    QString theme() const { return m_theme; }
    QString accentPreset() const { return m_accentPreset; }
    int readingSize() const { return m_readingSize; }
    QString error() const { return m_error; }
    QString filePath() const;

    Q_INVOKABLE void setTheme(const QString &theme);
    Q_INVOKABLE void setAccentPreset(const QString &preset);
    Q_INVOKABLE void setReadingSize(int size);
    Q_INVOKABLE void toggleTheme();
    bool reload();

signals:
    void settingsChanged();
    void errorChanged();

private:
    bool save();
    void setError(const QString &error);

    QString m_configBasePath;
    QString m_theme = QStringLiteral("light");
    QString m_accentPreset = QStringLiteral("purple");
    int m_readingSize = 16;
    QString m_error;
};
