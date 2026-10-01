#pragma once

#include "CatalogRepository.h"

#include <QObject>
#include <QVariantList>
#include <QVector>

class CollectionListModel;
class PoemLoader;

class NavigationController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString page READ page NOTIFY stateChanged)
    Q_PROPERTY(QString url READ url NOTIFY stateChanged)
    Q_PROPERTY(QString title READ title NOTIFY stateChanged)
    Q_PROPERTY(QString poetName READ poetName NOTIFY stateChanged)
    Q_PROPERTY(QString poetUrl READ poetUrl NOTIFY stateChanged)
    Q_PROPERTY(QString poetDescription READ poetDescription NOTIFY stateChanged)
    Q_PROPERTY(QString description READ description NOTIFY stateChanged)
    Q_PROPERTY(QString bookName READ bookName NOTIFY stateChanged)
    Q_PROPERTY(QVariantList breadcrumbs READ breadcrumbs NOTIFY stateChanged)
    Q_PROPERTY(QVariantList sidebarPath READ sidebarPath NOTIFY stateChanged)
    Q_PROPERTY(qreal poetScroll READ poetScroll NOTIFY stateChanged)
    Q_PROPERTY(qreal collectionScroll READ collectionScroll NOTIFY stateChanged)
    Q_PROPERTY(int poemPosition READ poemPosition NOTIFY stateChanged)
    Q_PROPERTY(int poemCount READ poemCount NOTIFY stateChanged)
    Q_PROPERTY(bool canGoBack READ canGoBack NOTIFY historyChanged)
    Q_PROPERTY(bool canGoForward READ canGoForward NOTIFY historyChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

public:
    NavigationController(CatalogRepository *repository, CollectionListModel *collection,
                         PoemLoader *poemLoader, QObject *parent = nullptr);

    QString page() const;
    QString url() const { return m_current.url; }
    QString title() const { return m_title; }
    QString poetName() const { return m_poetName; }
    QString poetUrl() const { return m_poetUrl; }
    QString poetDescription() const { return m_poetDescription; }
    QString description() const { return m_description; }
    QString bookName() const { return m_bookName; }
    QVariantList breadcrumbs() const { return m_breadcrumbs; }
    QVariantList sidebarPath() const { return m_sidebarPath; }
    qreal poetScroll() const { return m_current.poetScroll; }
    qreal collectionScroll() const { return m_current.collectionScroll; }
    int poemPosition() const { return m_poemPosition; }
    int poemCount() const { return m_poemCount; }
    bool canGoBack() const { return !m_back.isEmpty(); }
    bool canGoForward() const { return !m_forward.isEmpty(); }
    QString error() const { return m_error; }

    Q_INVOKABLE bool openPoets(qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool openPoet(const QString &url, qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool openCategory(const QString &url, qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool openPoem(const QString &url, qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool openBreadcrumb(int index, qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool back(qreal poetScroll = 0, qreal collectionScroll = 0);
    Q_INVOKABLE bool forward(qreal poetScroll = 0, qreal collectionScroll = 0);

signals:
    void stateChanged();
    void historyChanged();
    void errorChanged();

private:
    enum class Kind { Poets, Poet, Collection, Poem };
    struct Location {
        Kind kind = Kind::Poets;
        QString url;
        qreal poetScroll = 0;
        qreal collectionScroll = 0;
    };

    bool navigate(Location target, qreal poetScroll, qreal collectionScroll);
    bool applyLocation(const Location &location);
    void setError(const QString &error);

    CatalogRepository *m_repository;
    CollectionListModel *m_collection;
    PoemLoader *m_poemLoader;
    Location m_current;
    QVector<Location> m_back;
    QVector<Location> m_forward;
    QString m_title;
    QString m_poetName;
    QString m_poetUrl;
    QString m_poetDescription;
    QString m_description;
    QString m_bookName;
    QString m_error;
    QVariantList m_breadcrumbs;
    QVariantList m_sidebarPath;
    int m_poemPosition = 0;
    int m_poemCount = 0;
};
