#include "CatalogStartup.h"
#include "CatalogPaths.h"

#include <QFutureWatcher>
#include <QMetaObject>
#include <QtConcurrent>

CatalogStartup::CatalogStartup(QObject *parent) : QObject(parent) {}
CatalogStartup::~CatalogStartup()
{
    if (m_watcher) m_watcher->waitForFinished();
}

void CatalogStartup::start(const QStringList &arguments)
{
    auto *watcher = new QFutureWatcher<CatalogStartupResult>(this);
    m_watcher = watcher;
    connect(watcher, &QFutureWatcher<CatalogStartupResult>::finished, this, [this, watcher] {
        const CatalogStartupResult result = watcher->result();
        m_watcher = nullptr;
        watcher->deleteLater();
        emit catalogPrepared(result.path, result.error);
    });
    watcher->setFuture(QtConcurrent::run([this, arguments] {
        CatalogStartupResult result;
        result.path = CatalogPaths::resolve(arguments, &result.error,
            [this](qint64 copied, qint64 total) {
                if (total <= 0) return;
                const int percent = int(copied * 100 / total);
                QMetaObject::invokeMethod(this, [this, percent] {
                    if (m_progress == percent) return;
                    m_progress = percent;
                    m_statusText = QStringLiteral("کپی داده‌های شعر: %1٪. برنامه را باز نگه دارید.").arg(percent);
                    emit changed();
                }, Qt::QueuedConnection);
            });
        return result;
    }));
}

void CatalogStartup::setCatalogOpened(bool opened, const QString &error)
{
    m_busy = false;
    m_ready = opened;
    m_statusText = opened ? QString{} : error;
    emit changed();
}
