#pragma once

#include <QObject>
#include <QStringList>
#include <QFutureWatcher>

struct CatalogStartupResult { QString path; QString error; };

class CatalogStartup final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY changed)
    Q_PROPERTY(bool ready READ ready NOTIFY changed)
    Q_PROPERTY(int progress READ progress NOTIFY changed)
    Q_PROPERTY(QString statusText READ statusText NOTIFY changed)

public:
    explicit CatalogStartup(QObject *parent = nullptr);
    ~CatalogStartup() override;
    bool busy() const { return m_busy; }
    bool ready() const { return m_ready; }
    int progress() const { return m_progress; }
    QString statusText() const { return m_statusText; }
    void start(const QStringList &arguments);
    void setCatalogOpened(bool opened, const QString &error);

signals:
    void changed();
    void catalogPrepared(const QString &path, const QString &error);

private:
    bool m_busy = true;
    bool m_ready = false;
    int m_progress = 0;
    QString m_statusText = QStringLiteral("آماده‌سازی داده‌های شعر برای خواندن آفلاین…");
    QFutureWatcher<CatalogStartupResult> *m_watcher = nullptr;
};
