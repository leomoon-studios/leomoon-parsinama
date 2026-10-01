#pragma once

#include <QObject>
#include <QString>

class SettingsStore final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY settingsChanged)
    Q_PROPERTY(int readingSize READ readingSize WRITE setReadingSize NOTIFY settingsChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString filePath READ filePath CONSTANT)

public:
    explicit SettingsStore(QString homePath = {}, QObject *parent = nullptr);

    QString theme() const { return m_theme; }
    int readingSize() const { return m_readingSize; }
    QString error() const { return m_error; }
    QString filePath() const;

    Q_INVOKABLE void setTheme(const QString &theme);
    Q_INVOKABLE void setReadingSize(int size);
    Q_INVOKABLE void toggleTheme();
    bool reload();

signals:
    void settingsChanged();
    void errorChanged();

private:
    bool save();
    void setError(const QString &error);

    QString m_homePath;
    QString m_theme = QStringLiteral("dark");
    int m_readingSize = 22;
    QString m_error;
};
