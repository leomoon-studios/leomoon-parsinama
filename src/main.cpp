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
        for (int attempt = 0; attempt < 100 && !window->property("bundledFontReady").toBool(); ++attempt) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
        }
        if (window->objectName() != QStringLiteral("mainWindow")
            || window->property("title").toString() != QString::fromUtf8(applicationName)
            || !welcome || welcome->property("text").toString() != QString::fromUtf8(welcomeText)
            || !status || status->property("text").toString().isEmpty()
            || !window->property("bundledFontReady").toBool()
            || !poetPane || !contentPane || !moreButton || !themeButton
            || !appLogo || appLogo->property("status").toInt() != 1
            || window->property("compactHeader").toBool()
            || moreButton->property("visible").toBool()
            || !poetPane->property("visible").toBool()
            || !contentPane->property("visible").toBool()) {
            qCritical("The Persian application shell did not load correctly");
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
