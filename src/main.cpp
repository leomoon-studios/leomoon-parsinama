#include <QCoreApplication>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

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

    QQmlApplicationEngine engine;
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
        if (window->objectName() != QStringLiteral("mainWindow")
            || window->property("title").toString() != QString::fromUtf8(applicationName)
            || !welcome || welcome->property("text").toString() != QString::fromUtf8(welcomeText)) {
            qCritical("The Persian application shell did not load correctly");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    return application.exec();
}
