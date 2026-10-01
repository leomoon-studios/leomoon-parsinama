#include "data/CatalogPaths.h"
#include "data/CatalogRepository.h"
#include "data/CollectionListModel.h"
#include "data/PoemLoader.h"
#include "data/PoetListModel.h"
#include "services/SettingsStore.h"

#include <QCoreApplication>
#include <QColor>
#include <QEventLoop>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QTemporaryDir>
#include <QVariant>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

constexpr auto applicationName = "LeoMoon ParsiNama";
constexpr auto windowTitle = "لئومون پارسی‌نما";
constexpr auto welcomeText = "به لئومون پارسی‌نما خوش آمدید";

} // namespace

int main(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index) {
        if (std::strcmp(argv[index], "--version") == 0) {
            std::printf("%s %s\n", applicationName, PARSINAMA_VERSION);
            return EXIT_SUCCESS;
        }
    }

    QCoreApplication::setOrganizationName(QStringLiteral("LeoMoon Studios"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("leomoon-studios.com"));
    QCoreApplication::setApplicationName(QString::fromUtf8(applicationName));
    QCoreApplication::setApplicationVersion(QStringLiteral(PARSINAMA_VERSION));

    QGuiApplication application(argc, argv);
    QGuiApplication::setDesktopFileName(QStringLiteral(PARSINAMA_APP_ID));
    QGuiApplication::setWindowIcon(QIcon(QStringLiteral(":/qt/qml/LeoMoon/ParsiNama/assets/app-icon.svg")));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QString pathError;
    const QString catalogPath = CatalogPaths::resolve(application.arguments(), &pathError);
    if (!pathError.isEmpty()) {
        qCritical().noquote() << pathError;
        return EXIT_FAILURE;
    }
    CatalogRepository catalogRepository;
    catalogRepository.openCatalog(catalogPath);
    PoetListModel poetListModel(&catalogRepository);
    CollectionListModel collectionListModel(&catalogRepository);
    PoemLoader poemLoader(catalogPath);
    const bool smokeTest = application.arguments().contains(QStringLiteral("--smoke-test"));
    QTemporaryDir smokeHome;
    SettingsStore settingsStore(smokeTest ? smokeHome.path() : QString{});

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &application,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            qWarning().noquote() << warning.toString();
        }
    });
    engine.setInitialProperties({
        {QStringLiteral("catalogRepository"), QVariant::fromValue(&catalogRepository)},
        {QStringLiteral("poetListModel"), QVariant::fromValue(&poetListModel)},
        {QStringLiteral("collectionListModel"), QVariant::fromValue(&collectionListModel)},
        {QStringLiteral("poemLoader"), QVariant::fromValue(&poemLoader)},
        {QStringLiteral("settingsStore"), QVariant::fromValue(&settingsStore)}
    });
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &application, [] { QCoreApplication::exit(EXIT_FAILURE); },
                     Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("LeoMoon.ParsiNama"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        std::fprintf(stderr, "QML root window could not be created.\n");
        return EXIT_FAILURE;
    }

    if (smokeTest) {
        QObject *window = engine.rootObjects().constFirst();
        QObject *welcome = window->findChild<QObject *>(QStringLiteral("welcomeLabel"));
        QObject *status = window->findChild<QObject *>(QStringLiteral("catalogStatus"));
        QObject *poetPane = window->findChild<QObject *>(QStringLiteral("poetPane"));
        QObject *contentPane = window->findChild<QObject *>(QStringLiteral("contentPane"));
        QObject *moreButton = window->findChild<QObject *>(QStringLiteral("moreButton"));
        QObject *themeButton = window->findChild<QObject *>(QStringLiteral("themeButton"));
        QObject *appLogo = window->findChild<QObject *>(QStringLiteral("appLogo"));
        QObject *headerTitle = window->findChild<QObject *>(QStringLiteral("headerTitle"));
        QObject *welcomeContent = window->findChild<QObject *>(QStringLiteral("welcomeContent"));
        QObject *poetScrollBar = window->findChild<QObject *>(QStringLiteral("poetScrollBar"));
        QObject *collectionScrollBar = window->findChild<QObject *>(QStringLiteral("collectionScrollBar"));
        QObject *poetRow = window->findChild<QObject *>(QStringLiteral("poetRow"));
        for (int attempt = 0; attempt < 100
             && (!window->property("bundledFontReady").toBool()
                 || !window->property("bundledIconFontReady").toBool()); ++attempt) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        }
        if (window->objectName() != QStringLiteral("mainWindow")
            || window->property("title").toString() != QString::fromUtf8(windowTitle)
            || !headerTitle || headerTitle->property("text").toString() != QString::fromUtf8(windowTitle)
            || !welcome || welcome->property("text").toString() != QString::fromUtf8(welcomeText)
            || !welcomeContent
            || !poetScrollBar || !collectionScrollBar
            || poetScrollBar->property("policy").toInt() != Qt::ScrollBarAlwaysOn
            || collectionScrollBar->property("policy").toInt() != Qt::ScrollBarAlwaysOn
            || !status || status->property("text").toString().isEmpty()
            || !window->property("bundledFontReady").toBool()
            || !window->property("bundledIconFontReady").toBool()
            || !poetPane || !contentPane || !moreButton || !themeButton
            || !appLogo || appLogo->property("status").toInt() != 1
            || window->property("compactHeader").toBool()
            || moreButton->property("visible").toBool()
            || !poetPane->property("visible").toBool()
            || !contentPane->property("visible").toBool()) {
            qCritical("The Persian application shell did not load correctly");
            return EXIT_FAILURE;
        }
        const qreal welcomeCenterX = welcomeContent->property("x").toReal()
            + welcomeContent->property("width").toReal() / 2;
        const qreal welcomeCenterY = welcomeContent->property("y").toReal()
            + welcomeContent->property("height").toReal() / 2;
        if (qAbs(welcomeCenterX - contentPane->property("width").toReal() / 2) > 1
            || qAbs(welcomeCenterY - contentPane->property("height").toReal() / 2) > 1) {
            qCritical("The welcome content is not centered");
            return EXIT_FAILURE;
        }
        if (poetRow
            && poetRow->property("x").toReal()
                < poetScrollBar->property("x").toReal()
                    + poetScrollBar->property("width").toReal() + 4) {
            qCritical("The poet row overlaps the scrollbar gutter");
            return EXIT_FAILURE;
        }
        if (catalogRepository.ready() && collectionListModel.loadCategory(QStringLiteral("/ferdousi"))) {
            window->setProperty("page", QStringLiteral("collection"));
            QCoreApplication::processEvents();
            QObject *collectionRow = window->findChild<QObject *>(QStringLiteral("collectionRow"));
            if (collectionRow
                && collectionRow->property("x").toReal()
                    < collectionScrollBar->property("x").toReal()
                        + collectionScrollBar->property("width").toReal() + 4) {
                qCritical("The collection row overlaps the scrollbar gutter");
                return EXIT_FAILURE;
            }
            QMetaObject::invokeMethod(window, "showPoets");
        }
        window->setProperty("selectedPoetName", QStringLiteral("حافظ"));
        window->setProperty("selectedPoetUrl", QStringLiteral("/hafez"));
        window->setProperty("page", QStringLiteral("collection"));
        if (!QMetaObject::invokeMethod(window, "showPoets")
            || !window->property("selectedPoetName").toString().isEmpty()
            || !window->property("selectedPoetUrl").toString().isEmpty()
            || window->property("page").toString() != QStringLiteral("poets")) {
            qCritical("Returning to poets did not clear the previous selection");
            return EXIT_FAILURE;
        }
        const QColor darkColor = window->property("color").value<QColor>();
        settingsStore.toggleTheme();
        QCoreApplication::processEvents();
        if (window->property("color").value<QColor>() == darkColor) {
            qCritical("The theme switch did not update the window");
            return EXIT_FAILURE;
        }
        window->setProperty("width", 600);
        QCoreApplication::processEvents();
        if (!window->property("compactHeader").toBool()
            || !window->property("compactBrowse").toBool()
            || !moreButton->property("visible").toBool()
            || !poetPane->property("visible").toBool()
            || contentPane->property("visible").toBool()) {
            qCritical("The compact shell did not lay out correctly");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return application.exec();
}
