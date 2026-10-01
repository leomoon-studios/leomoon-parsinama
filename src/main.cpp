#include "data/CatalogPaths.h"
#include "data/CatalogRepository.h"
#include "data/CollectionListModel.h"
#include "data/PoemLoader.h"
#include "data/PoetListModel.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
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

    QQmlApplicationEngine engine;
    engine.setInitialProperties({
        {QStringLiteral("catalogRepository"), QVariant::fromValue(&catalogRepository)},
        {QStringLiteral("poetListModel"), QVariant::fromValue(&poetListModel)},
        {QStringLiteral("collectionListModel"), QVariant::fromValue(&collectionListModel)},
        {QStringLiteral("poemLoader"), QVariant::fromValue(&poemLoader)}
    });
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &application, [] { QCoreApplication::exit(EXIT_FAILURE); },
                     Qt::QueuedConnection);
    engine.loadFromModule(QStringLiteral("LeoMoon.ParsiNama"), QStringLiteral("Main"));

    if (engine.rootObjects().isEmpty()) {
        return EXIT_FAILURE;
    }

    if (application.arguments().contains(QStringLiteral("--smoke-test"))) {
        const QObject *window = engine.rootObjects().constFirst();
        const QObject *welcome = window->findChild<QObject *>(QStringLiteral("welcomeLabel"));
        const QObject *status = window->findChild<QObject *>(QStringLiteral("catalogStatus"));
        if (window->objectName() != QStringLiteral("mainWindow")
            || window->property("title").toString() != QString::fromUtf8(applicationName)
            || !welcome || welcome->property("text").toString() != QString::fromUtf8(welcomeText)
            || !status || status->property("text").toString().isEmpty()) {
            qCritical("The Persian application shell did not load correctly");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return application.exec();
}
