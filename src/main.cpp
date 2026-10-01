#include "data/CatalogPaths.h"
#include "data/CatalogRepository.h"
#include "data/CollectionListModel.h"
#include "data/NavigationController.h"
#include "data/PoemLoader.h"
#include "data/PoetListModel.h"
#include "services/SettingsStore.h"

#include <QCoreApplication>
#include <QColor>
#include <QEventLoop>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickItem>
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
    NavigationController navigationController(&catalogRepository, &collectionListModel, &poemLoader);
    const bool smokeTest = application.arguments().contains(QStringLiteral("--smoke-test"));
    QTemporaryDir smokeConfigBase;
    SettingsStore settingsStore(smokeTest ? smokeConfigBase.path() : QString{});
    if (smokeTest && (settingsStore.theme() != QLatin1String("light")
        || !QFileInfo::exists(settingsStore.filePath()))) {
        qCritical("The first launch did not create light-theme settings");
        return EXIT_FAILURE;
    }

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings, &application,
                     [](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings) {
            std::fprintf(stderr, "%s\n", qPrintable(warning.toString()));
        }
    });
    engine.setInitialProperties({
        {QStringLiteral("catalogRepository"), QVariant::fromValue(&catalogRepository)},
        {QStringLiteral("poetListModel"), QVariant::fromValue(&poetListModel)},
        {QStringLiteral("collectionListModel"), QVariant::fromValue(&collectionListModel)},
        {QStringLiteral("poemLoader"), QVariant::fromValue(&poemLoader)},
        {QStringLiteral("navigationController"), QVariant::fromValue(&navigationController)},
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
        QObject *header = window->findChild<QObject *>(QStringLiteral("header"));
        QObject *poetPane = window->findChild<QObject *>(QStringLiteral("poetPane"));
        QObject *contentPane = window->findChild<QObject *>(QStringLiteral("contentPane"));
        QObject *moreButton = window->findChild<QObject *>(QStringLiteral("moreButton"));
        QObject *settingsButton = window->findChild<QObject *>(QStringLiteral("settingsButton"));
        QObject *settingsPage = window->findChild<QObject *>(QStringLiteral("settingsPage"));
        QObject *readingSizeSelector = window->findChild<QObject *>(QStringLiteral("readingSizeSelector"));
        QObject *lightThemeChoice = window->findChild<QObject *>(QStringLiteral("lightThemeChoice"));
        QObject *appLogo = window->findChild<QObject *>(QStringLiteral("appLogo"));
        QObject *headerTitle = window->findChild<QObject *>(QStringLiteral("headerTitle"));
        QObject *welcomeContent = window->findChild<QObject *>(QStringLiteral("welcomeContent"));
        QObject *poetScrollBar = window->findChild<QObject *>(QStringLiteral("poetScrollBar"));
        QObject *collectionScrollBar = window->findChild<QObject *>(QStringLiteral("collectionScrollBar"));
        QObject *collectionList = window->findChild<QObject *>(QStringLiteral("collectionList"));
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
            || !poetScrollBar || !collectionScrollBar || !collectionList
            || poetScrollBar->property("policy").toInt() != Qt::ScrollBarAlwaysOn
            || collectionScrollBar->property("policy").toInt() != Qt::ScrollBarAlwaysOn
            || !status
            || (catalogRepository.ready() && !status->property("text").toString().isEmpty())
            || (!catalogRepository.ready() && status->property("text").toString().isEmpty())
            || !window->property("bundledFontReady").toBool()
            || !window->property("bundledIconFontReady").toBool()
            || !header || !poetPane || !contentPane || !moreButton || !settingsButton
            || !settingsPage || !readingSizeSelector || !lightThemeChoice
            || !appLogo || appLogo->property("color").value<QColor>()
                != window->property("accentColor").value<QColor>()
            || window->property("compactHeader").toBool()
            || moreButton->property("visible").toBool()
            || !poetPane->property("visible").toBool()
            || !contentPane->property("visible").toBool()
            || settingsButton->property("x").toReal() > 20) {
            std::fprintf(stderr, "The Persian application shell did not load correctly. settings=%p size=%p light=%p\n",
                         static_cast<void *>(settingsPage), static_cast<void *>(readingSizeSelector),
                         static_cast<void *>(lightThemeChoice));
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
            auto *poetRowItem = qobject_cast<QQuickItem *>(poetRow);
            auto *collectionRowItem = qobject_cast<QQuickItem *>(collectionRow);
            auto *poetPaneItem = qobject_cast<QQuickItem *>(poetPane);
            auto *contentPaneItem = qobject_cast<QQuickItem *>(contentPane);
            if (poetRowItem && collectionRowItem && poetPaneItem && contentPaneItem) {
                const qreal poetInset = poetRowItem->mapToItem(poetPaneItem, QPointF{}).x();
                const qreal collectionInset = collectionRowItem->mapToItem(contentPaneItem, QPointF{}).x();
                const qreal poetTrailingInset = poetPaneItem->width() - poetInset - poetRowItem->width();
                const qreal collectionTrailingInset = contentPaneItem->width()
                    - collectionInset - collectionRowItem->width();
                if (qAbs(poetInset - collectionInset) > 1
                    || qAbs(poetTrailingInset - collectionTrailingInset) > 1) {
                    qCritical("The poet and collection rows have inconsistent side gutters");
                    return EXIT_FAILURE;
                }
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
        const QColor lightColor = window->property("color").value<QColor>();
        settingsStore.setTheme(QStringLiteral("dark"));
        QCoreApplication::processEvents();
        if (window->property("color").value<QColor>() == lightColor) {
            qCritical("The theme switch did not update the window");
            return EXIT_FAILURE;
        }
        settingsStore.setTheme(QStringLiteral("light"));
        QCoreApplication::processEvents();
        if (window->property("color").value<QColor>() != lightColor) {
            qCritical("The light theme did not restore the window color");
            return EXIT_FAILURE;
        }
        const QColor lightForeground = window->property("foregroundColor").value<QColor>();
        QObject *poetRowText = window->findChild<QObject *>(QStringLiteral("poetRowText"));
        if (poetRowText && (poetRowText->property("color").value<QColor>() != lightForeground
            || poetRowText->property("effectiveHorizontalAlignment").toInt() != Qt::AlignRight)) {
            qCritical("The light theme did not update poet row text");
            return EXIT_FAILURE;
        }
        if (catalogRepository.ready() && collectionListModel.loadCategory(QStringLiteral("/ferdousi"))) {
            window->setProperty("page", QStringLiteral("collection"));
            QCoreApplication::processEvents();
            QObject *collectionRowText = window->findChild<QObject *>(QStringLiteral("collectionRowText"));
            if (collectionRowText && (collectionRowText->property("color").value<QColor>() != lightForeground
                || collectionRowText->property("effectiveHorizontalAlignment").toInt() != Qt::AlignRight)) {
                qCritical("The light theme did not update collection row text");
                return EXIT_FAILURE;
            }
        }
        const QColor originalAccent = window->property("accentColor").value<QColor>();
        settingsStore.setAccentPreset(QStringLiteral("teal"));
        QCoreApplication::processEvents();
        QObject *logoGlyph = window->findChild<QObject *>(QStringLiteral("appLogoGlyph"));
        if (window->property("accentColor").value<QColor>() == originalAccent
            || appLogo->property("color").value<QColor>() != window->property("accentColor").value<QColor>()
            || !logoGlyph || logoGlyph->property("fillColor").value<QColor>()
                != window->property("accentTextColor").value<QColor>()) {
            qCritical("The accent choice did not update the theme");
            return EXIT_FAILURE;
        }
        window->setProperty("page", QStringLiteral("settings"));
        QCoreApplication::processEvents();
        QObject *settingsScrollBar = window->findChild<QObject *>(QStringLiteral("settingsScrollBar"));
        if (!settingsScrollBar || settingsScrollBar->property("visible").toBool()) {
            qCritical("The settings scrollbar is visible when the content fits");
            return EXIT_FAILURE;
        }
        window->setProperty("height", 500);
        QCoreApplication::processEvents();
        if (!settingsScrollBar->property("visible").toBool()
            || settingsScrollBar->property("height").toReal()
                < settingsPage->property("availableHeight").toReal() - 1) {
            qCritical("The settings scrollbar did not fill the viewport when scrolling is needed");
            return EXIT_FAILURE;
        }
        window->setProperty("height", 760);
        QCoreApplication::processEvents();
        QObject *tealAccentChoice = window->findChild<QObject *>(QStringLiteral("accentChoice_teal"));
        QObject *readingSizeText = window->findChild<QObject *>(QStringLiteral("readingSizeText"));
        QObject *readingSizePopup = window->findChild<QObject *>(QStringLiteral("readingSizePopup"));
        QObject *readingSizePopupBackground = window->findChild<QObject *>(QStringLiteral("readingSizePopupBackground"));
        QObject *readingSizePreview = window->findChild<QObject *>(QStringLiteral("readingSizePreview"));
        if (!settingsPage->property("visible").toBool()
            || !lightThemeChoice->property("selected").toBool()
            || !tealAccentChoice || !tealAccentChoice->property("selected").toBool()
            || !readingSizeText || readingSizeText->property("color").value<QColor>() != lightForeground
            || readingSizeText->property("effectiveHorizontalAlignment").toInt() != Qt::AlignRight
            || !readingSizePreview
            || readingSizePreview->property("effectiveHorizontalAlignment").toInt() != Qt::AlignRight
            || !readingSizePopup || !readingSizePopupBackground
            || readingSizePopupBackground->property("color").value<QColor>()
                != window->property("surfaceColor").value<QColor>()
            || readingSizeSelector->property("currentIndex").toInt() != settingsStore.readingSize() - 16) {
            std::fprintf(stderr, "The settings page did not load correctly: page=%d light=%d teal=%d size=%d expected=%d\n",
                         settingsPage->property("visible").toBool(), lightThemeChoice->property("selected").toBool(),
                         tealAccentChoice && tealAccentChoice->property("selected").toBool(),
                         readingSizeSelector->property("currentIndex").toInt(), settingsStore.readingSize() - 16);
            return EXIT_FAILURE;
        }
        QMetaObject::invokeMethod(readingSizePopup, "open");
        QCoreApplication::processEvents();
        QObject *sizeList = window->findChild<QObject *>(QStringLiteral("readingSizeList"));
        if (!readingSizePopup->property("visible").toBool()
            || readingSizePopup->property("height").toReal() < 80
            || !sizeList || sizeList->property("count").toInt() != 25) {
            qCritical("The reading-size menu did not open with visible options");
            return EXIT_FAILURE;
        }
        QMetaObject::invokeMethod(readingSizePopup, "close");
        settingsStore.setReadingSize(29);
        QCoreApplication::processEvents();
        if (readingSizeSelector->property("currentIndex").toInt() != 13) {
            std::fprintf(stderr, "The reading-size selector did not follow the saved value: %d\n",
                         readingSizeSelector->property("currentIndex").toInt());
            return EXIT_FAILURE;
        }
        QMetaObject::invokeMethod(window, "showPoets");
        window->setProperty("width", 800);
        QCoreApplication::processEvents();
        if (window->property("compactHeader").toBool()
            || !settingsButton->property("visible").toBool()
            || moreButton->property("visible").toBool()
            || settingsButton->property("x").toReal() > 20) {
            qCritical("The full toolbar disappeared while there was room for it");
            return EXIT_FAILURE;
        }
        window->setProperty("width", 600);
        QCoreApplication::processEvents();
        QObject *overflowMenu = window->findChild<QObject *>(QStringLiteral("overflowMenu"));
        QObject *overflowMenuBackground = window->findChild<QObject *>(QStringLiteral("overflowMenuBackground"));
        QObject *overflowPoetsItem = window->findChild<QObject *>(QStringLiteral("overflowPoetsItem"));
        QObject *menuItemText = overflowPoetsItem
            ? overflowPoetsItem->findChild<QObject *>(QStringLiteral("menuItemText")) : nullptr;
        if (!window->property("compactHeader").toBool()
            || !window->property("compactBrowse").toBool()
            || !moreButton->property("visible").toBool()
            || moreButton->property("x").toReal() > 20
            || !overflowMenu || !overflowMenuBackground || !overflowPoetsItem || !menuItemText
            || overflowMenuBackground->property("color").value<QColor>()
                != window->property("surfaceColor").value<QColor>()
            || menuItemText->property("color").value<QColor>()
                != window->property("foregroundColor").value<QColor>()
            || menuItemText->property("effectiveHorizontalAlignment").toInt() != Qt::AlignRight
            || !poetPane->property("visible").toBool()
            || contentPane->property("visible").toBool()) {
            qCritical("The compact shell did not lay out correctly");
            return EXIT_FAILURE;
        }
        if (!QMetaObject::invokeMethod(moreButton, "clicked")) {
            qCritical("The compact menu button could not be activated");
            return EXIT_FAILURE;
        }
        QCoreApplication::processEvents();
        if (!overflowMenu->property("visible").toBool()) {
            qCritical("The compact menu did not open");
            return EXIT_FAILURE;
        }
        settingsStore.setTheme(QStringLiteral("dark"));
        QCoreApplication::processEvents();
        if (overflowMenuBackground->property("color").value<QColor>()
                != window->property("surfaceColor").value<QColor>()
            || menuItemText->property("color").value<QColor>()
                != window->property("foregroundColor").value<QColor>()) {
            qCritical("The compact menu did not follow the dark theme");
            return EXIT_FAILURE;
        }
        QMetaObject::invokeMethod(overflowMenu, "close");
        if (catalogRepository.ready()) {
            poetListModel.setFilterText(QStringLiteral("حافظ"));
            if (poetListModel.count() != 1) {
                qCritical("The poet directory filter did not narrow the results");
                return EXIT_FAILURE;
            }
            poetListModel.setFilterText({});
            if (!navigationController.openPoet(QStringLiteral("/hafez"))) {
                qCritical("The poet page did not open");
                return EXIT_FAILURE;
            }
            QCoreApplication::processEvents();
            QObject *biography = window->findChild<QObject *>(QStringLiteral("poetBiography"));
            QObject *breadcrumbs = window->findChild<QObject *>(QStringLiteral("breadcrumbBar"));
            if (window->property("page").toString() != QLatin1String("poet")
                || !biography || biography->property("text").toString().isEmpty()
                || !breadcrumbs || !breadcrumbs->property("visible").toBool()) {
                qCritical("The poet biography or breadcrumbs did not appear");
                return EXIT_FAILURE;
            }
            const auto ghazals = catalogRepository.categoryByUrl(QStringLiteral("/hafez/ghazal"));
            if (!ghazals || !navigationController.openCategory(ghazals->fullUrl, 0, 180)
                || navigationController.breadcrumbs().size() != 3) {
                qCritical("The nested collection did not open");
                return EXIT_FAILURE;
            }
            const auto poems = catalogRepository.categoryPoems(ghazals->id);
            if (poems.isEmpty() || !navigationController.openPoem(poems.constFirst().fullUrl, 0, 240)
                || navigationController.breadcrumbs().size() != 4
                || !navigationController.back(0, 240)
                || navigationController.collectionScroll() != 240) {
                qCritical("Poem navigation did not preserve its collection position");
                return EXIT_FAILURE;
            }
            QCoreApplication::processEvents();
            if (qAbs(collectionList->property("contentY").toReal() - 240) > 1) {
                qCritical("The collection list did not restore its scroll position");
                return EXIT_FAILURE;
            }
            if (!navigationController.openCategory(QStringLiteral("/ferdousi/shahname/aghaz"))) {
                qCritical("The deep collection did not open");
                return EXIT_FAILURE;
            }
            QCoreApplication::processEvents();
            if (!breadcrumbs->property("collapsed").toBool()
                || !QMetaObject::invokeMethod(breadcrumbs, "openOverflow")) {
                qCritical("Deep breadcrumbs did not collapse into an accessible menu");
                return EXIT_FAILURE;
            }
            QCoreApplication::processEvents();
            if (!breadcrumbs->property("overflowVisible").toBool()) {
                qCritical("The collapsed breadcrumb menu did not open");
                return EXIT_FAILURE;
            }
            QObject *breadcrumbOverflowPopup = window->findChild<QObject *>(
                QStringLiteral("breadcrumbOverflowPopup"));
            QMetaObject::invokeMethod(breadcrumbOverflowPopup, "close");
        }
        return EXIT_SUCCESS;
    }

    return application.exec();
}
