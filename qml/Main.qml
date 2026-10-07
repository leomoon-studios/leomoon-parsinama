pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    objectName: "mainWindow"
    required property var catalogRepository
    required property var poetListModel
    required property var collectionListModel
    required property var poemLoader
    required property var searchRepository
    required property var navigationController
    required property var settingsStore
    required property var bookmarkStore
    property var printService: null
    property var catalogStartup: null
    property bool simulatePhoneForSmokeTest: false
    readonly property bool printingSupported: Qt.platform.os !== "android"
    readonly property bool phoneLayout: (Qt.platform.os === "android" || simulatePhoneForSmokeTest)
        && (width < 600 || height < 500)
    readonly property bool compactHeader: width < 760 || phoneLayout
    readonly property bool compactBrowse: width < 760 || phoneLayout
    readonly property bool shortAndroidView: (Qt.platform.os === "android" || simulatePhoneForSmokeTest)
        && height < 500
    readonly property bool stackedReaderToolbar: compactBrowse && !shortAndroidView
    readonly property real keyboardInset: Qt.platform.os === "android" && Qt.inputMethod.visible
        && Qt.inputMethod.keyboardRectangle.height > 0
        ? Math.max(0, height - Qt.inputMethod.keyboardRectangle.y) : 0
    readonly property bool bundledFontReady: typography.ready
    readonly property bool bundledFontError: typography.failed
    readonly property bool bundledIconFontReady: typography.iconReady
    readonly property color accentColor: colors.accent
    readonly property color accentTextColor: colors.accentText
    readonly property color foregroundColor: colors.foreground
    readonly property color surfaceColor: colors.surface
    readonly property bool canPrintCurrentPoem: printingSupported && page === "poem"
        && printService !== null && printService.available
    readonly property bool canHandleAndroidBack: navigationDrawer.visible || overflowMenu.visible || printMenu.visible
        || breadcrumbs.overflowVisible || Qt.inputMethod.visible || page !== "poets"
    readonly property string statusMessage: catalogStartup !== null && !catalogStartup.busy
        && !catalogStartup.ready ? catalogStartup.statusText
        : bookmarkStore.error !== "" ? bookmarkStore.error
        : printService !== null && printService.error !== "" ? printService.error
        : settingsStore.error !== "" ? settingsStore.error
        : navigationController.error !== "" ? navigationController.error
        : poemLoader.loading || poemLoader.error !== "" ? poemLoader.statusText
        : catalogRepository.ready ? "" : catalogRepository.statusText
    property string page: "poets"
    readonly property bool auxiliaryPage: page === "settings" || page === "favorites" || page === "search"
    readonly property string currentContentTitle: page === "settings" ? "تنظیمات"
        : page === "favorites" ? "نشانک‌ها"
        : page === "search" ? "جستجو" : navigationController.title
    property var pageReturnStack: []
    onPageChanged: {
        if (page !== "poem")
            printMenu.close()
        if (page !== "search" && Qt.platform.os === "android")
            Qt.callLater(() => {
                if (page !== "search") {
                    searchPage.releaseQueryFocus()
                    root.contentItem.forceActiveFocus()
                    Qt.inputMethod.hide()
                }
            })
    }
    property string selectedPoetName: ""
    property string selectedPoetUrl: ""

    width: 1120
    height: 760
    minimumWidth: Qt.platform.os === "android" || simulatePhoneForSmokeTest ? 0 : 500
    minimumHeight: Qt.platform.os === "android" || simulatePhoneForSmokeTest ? 0 : 440
    visible: true
    onClosing: (event) => {
        if (Qt.platform.os === "android" && canHandleAndroidBack) {
            event.accepted = false
            handleAndroidBack()
        }
    }
    title: "لئومون پارسی‌نما"
    color: colors.background
    font.family: typography.family

    LayoutMirroring.enabled: true
    LayoutMirroring.childrenInherit: true

    function selectPoet(fullUrl, name) {
        if (navigationController.openPoet(fullUrl, poetList.contentY, collectionScrollOffset()))
            page = "poet"
    }

    function showPoets() {
        navigationController.openPoets(poetList.contentY, collectionScrollOffset())
        selectedPoetName = ""
        selectedPoetUrl = ""
        poetFilter.text = ""
        page = "poets"
    }

    function showAuxiliaryPage(target) {
        if (page === target) {
            if (Qt.platform.os !== "android")
                returnFromAuxiliaryPage()
            return
        }
        pageReturnStack = pageReturnStack.concat([page])
        page = target
    }

    function returnFromAuxiliaryPage() {
        if (!auxiliaryPage)
            return
        if (pageReturnStack.length > 0) {
            const previousPage = pageReturnStack[pageReturnStack.length - 1]
            pageReturnStack = pageReturnStack.slice(0, -1)
            page = previousPage
        } else {
            page = navigationController.page
        }
        if (!auxiliaryPage) {
            selectedPoetName = navigationController.poetName
            selectedPoetUrl = navigationController.poetUrl
        }
        if (Qt.platform.os !== "android")
            root.contentItem.forceActiveFocus()
    }

    function showFavorites() {
        selectedPoetName = ""
        selectedPoetUrl = ""
        showAuxiliaryPage("favorites")
    }

    function showSettings() {
        showAuxiliaryPage("settings")
    }

    function showSearch() {
        if (page === "search" && Qt.platform.os !== "android") {
            returnFromAuxiliaryPage()
            return
        }
        const oldPage = page
        searchPage.availablePoetUrl = oldPage === "poet" || oldPage === "collection"
            || oldPage === "poem" ? navigationController.poetUrl : ""
        searchPage.availableCategoryUrl = ""
        if (oldPage === "collection")
            searchPage.availableCategoryUrl = navigationController.url
        else if (oldPage === "poem") {
            const crumbs = navigationController.breadcrumbs
            if (crumbs.length >= 3)
                searchPage.availableCategoryUrl = crumbs[crumbs.length - 2].url || ""
        }
        searchPage.scope = "all"
        showAuxiliaryPage("search")
        searchPage.refresh()
        Qt.callLater(() => searchPage.focusQuery())
    }

    function openFavorite(entryType, fullUrl) {
        let opened = false
        if (entryType === "poet")
            opened = navigationController.openPoet(fullUrl, poetList.contentY, collectionScrollOffset())
        else if (entryType === "collection")
            opened = navigationController.openCategory(fullUrl, poetList.contentY, collectionScrollOffset())
        else if (entryType === "poem")
            opened = navigationController.openPoem(fullUrl, poetList.contentY, collectionScrollOffset())
        if (opened) {
            page = navigationController.page
            selectedPoetName = navigationController.poetName
            selectedPoetUrl = navigationController.poetUrl
        }
    }

    function openCategory(fullUrl) {
        navigationController.openCategory(fullUrl, poetList.contentY, collectionScrollOffset())
    }

    function openPoem(fullUrl) {
        navigationController.openPoem(fullUrl, poetList.contentY, collectionScrollOffset())
    }

    function previousPoem() {
        navigationController.openPreviousPoem(poetList.contentY, collectionScrollOffset())
    }

    function nextPoem() {
        navigationController.openNextPoem(poetList.contentY, collectionScrollOffset())
    }

    function printCurrentPoem() {
        if (canPrintCurrentPoem)
            printService.printCurrentPoem()
    }

    function openPrintMenu() {
        if (!canPrintCurrentPoem)
            return
        const anchor = root.compactHeader ? moreButton : printButton
        printMenu.popup(anchor, 0, anchor.height + 8)
    }


    function handleAndroidBack() {
        if (navigationDrawer.visible) {
            navigationDrawer.close()
            return
        }
        if (overflowMenu.visible) {
            overflowMenu.close()
            return
        }
        if (printMenu.visible) {
            printMenu.close()
            return
        }
        if (breadcrumbs.overflowVisible) {
            breadcrumbs.closeOverflow()
            return
        }
        if (Qt.inputMethod.visible) {
            Qt.inputMethod.hide()
            return
        }
        if (pageReturnStack.length > 0) {
            const previousPage = pageReturnStack[pageReturnStack.length - 1]
            pageReturnStack = pageReturnStack.slice(0, -1)
            page = previousPage
            return
        }
        if (page !== navigationController.page) {
            page = navigationController.page
            return
        }
        if (page !== "poets" && navigationController.canGoBack)
            navigationController.back(poetList.contentY, collectionScrollOffset())
        else if (page !== "poets")
            showPoets()
    }

    Shortcut {
        sequence: "Back"
        context: Qt.ApplicationShortcut
        enabled: Qt.platform.os === "android" && root.canHandleAndroidBack
        onActivated: root.handleAndroidBack()
    }

    Shortcut {
        objectName: "desktopAuxiliaryEscape"
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        enabled: Qt.platform.os !== "android" && root.auxiliaryPage
            && !navigationDrawer.visible && !overflowMenu.visible && !printMenu.visible
            && !breadcrumbs.overflowVisible && !settingsPage.readingSizePopupVisible
        onActivated: root.returnFromAuxiliaryPage()
    }

    Shortcut {
        sequence: StandardKey.Print
        enabled: root.canPrintCurrentPoem
        onActivated: root.openPrintMenu()
    }

    function openBreadcrumb(index) {
        navigationController.openBreadcrumb(index, poetList.contentY, collectionScrollOffset())
    }

    function collectionScrollOffset() {
        return Math.max(0, collectionList.contentY
            + (collectionList.headerItem ? collectionList.headerItem.height : 0))
    }

    function restoreCollectionScroll() {
        collectionList.contentY = root.navigationController.collectionScroll
            - (collectionList.headerItem ? collectionList.headerItem.height : 0)
    }

    function restorePoemScroll() {
        if (!poemScrollPending || page !== "poem" || poemLoader.loading)
            return
        poemList.contentY = -(poemList.headerItem ? poemList.headerItem.height : 0)
        poemScrollPending = false
    }

    property bool poemScrollPending: false

    Timer {
        id: collectionScrollTimer
        interval: 30
        onTriggered: root.restoreCollectionScroll()
    }

    Timer {
        id: poemScrollTimer
        interval: 30
        onTriggered: root.restorePoemScroll()
    }

    Connections {
        target: root.navigationController
        function onStateChanged() {
            root.pageReturnStack = []
            root.page = root.navigationController.page
            root.selectedPoetName = root.navigationController.poetName
            root.selectedPoetUrl = root.navigationController.poetUrl
            root.poemScrollPending = root.page === "poem"
            poemList.resetScrollAnchor()
            poemScrollTimer.stop()
            Qt.callLater(() => {
                poetList.contentY = root.navigationController.poetScroll
                root.restoreCollectionScroll()
                collectionScrollTimer.restart()
                if (root.poemScrollPending && !root.poemLoader.loading)
                    poemScrollTimer.restart()
            })
        }
    }

    Connections {
        target: root.poemLoader
        function onRequestFinished() {
            if (root.poemScrollPending)
                poemScrollTimer.restart()
        }
    }

    AppTheme {
        id: colors
        darkMode: root.settingsStore.theme === "dark"
        accentPreset: root.settingsStore.accentPreset
    }

    Typography { id: typography }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.phoneLayout ? 10 : root.compactHeader ? 16 : 20
        anchors.bottomMargin: (root.phoneLayout ? 10 : root.compactHeader ? 16 : 20)
            + root.keyboardInset
        spacing: root.phoneLayout ? 8 : root.shortAndroidView ? 8 : 18

        RowLayout {
            objectName: "header"
            Layout.fillWidth: true
            spacing: 12

            Image {
                id: appLogo
                objectName: "appLogo"
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                source: "qrc:/qt/qml/LeoMoon/ParsiNama/assets/app-icon.svg"
                sourceSize: Qt.size(256, 256)
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
                Accessible.name: "نشان لئومون پارسی‌نما"
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Label {
                        objectName: "headerTitle"
                        text: "لئومون پارسی‌نما"
                        color: colors.foreground
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                        Layout.maximumWidth: parent.width - (applicationVersion.visible
                            ? applicationVersion.implicitWidth + parent.spacing : 0)
                        Layout.alignment: Qt.AlignBaseline
                    }
                    Label {
                        id: applicationVersion
                        objectName: "headerVersion"
                        text: "v" + Qt.application.version
                        visible: !root.phoneLayout
                        color: colors.muted
                        font.pixelSize: 12
                        LayoutMirroring.enabled: false
                        Layout.alignment: Qt.AlignBaseline
                    }
                    Item { Layout.fillWidth: true }
                }
                Label {
                    objectName: "headerSubtitle"
                    Layout.fillWidth: true
                    text: "گنجینه شعر پارسی"
                    visible: !root.phoneLayout
                    color: colors.muted
                    font.pixelSize: 12
                    elide: Text.ElideRight
                }
            }
            HeaderAction {
                objectName: "poetsButton"
                symbol: "\ue865"
                hint: "شاعران"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.showPoets()
            }
            HeaderAction {
                objectName: "favoritesButton"
                symbol: "\ue866"
                hint: "نشانک‌ها"
                selected: root.page === "favorites"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.showFavorites()
            }
            HeaderAction {
                objectName: "searchButton"
                symbol: "\ue8b6"
                hint: "جستجو"
                selected: root.page === "search"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.showSearch()
            }
            HeaderAction {
                id: printButton
                objectName: "printButton"
                symbol: "\ue8ad"
                hint: "چاپ"
                font.family: typography.iconFamily
                visible: root.printingSupported && !root.compactHeader
                enabled: root.canPrintCurrentPoem
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.openPrintMenu()
            }
            HeaderAction {
                objectName: "settingsButton"
                symbol: "\ue8b8"
                hint: "تنظیمات"
                selected: root.page === "settings"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.showSettings()
            }
            HeaderAction {
                id: moreButton
                objectName: "moreButton"
                symbol: "\ue5d4"
                hint: root.phoneLayout ? "باز کردن فهرست" : "گزینه‌های بیشتر"
                font.family: typography.iconFamily
                visible: root.compactHeader
                Layout.minimumWidth: 44
                Layout.minimumHeight: 44
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                contentItem: Item {
                    Text {
                        anchors.fill: parent
                        visible: !root.phoneLayout
                        text: moreButton.symbol
                        color: moreButton.enabled ? moreButton.textColor : moreButton.borderColor
                        font.family: typography.iconFamily
                        font.pixelSize: 24
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Column {
                        anchors.centerIn: parent
                        visible: root.phoneLayout
                        spacing: 4
                        Repeater {
                            model: 3
                            Rectangle {
                                width: 20
                                height: 2
                                radius: 1
                                color: moreButton.enabled ? moreButton.textColor : moreButton.borderColor
                            }
                        }
                    }
                }
                onClicked: {
                    if (root.phoneLayout) {
                        if (root.page === "search") {
                            searchPage.releaseQueryFocus()
                            root.contentItem.forceActiveFocus()
                            Qt.inputMethod.hide()
                        }
                        navigationDrawer.open()
                    } else {
                        overflowMenu.popup(moreButton, 0, moreButton.height + 8)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Rectangle {
                id: poetPane
                objectName: "poetPane"
                visible: !root.compactBrowse || root.page === "poets"
                Layout.preferredWidth: root.compactBrowse ? 0 : 280
                Layout.fillWidth: root.compactBrowse
                Layout.fillHeight: true
                radius: 18
                color: colors.surface
                border.color: colors.border
                clip: true
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8
                    Label {
                        text: "شاعران"
                        color: colors.foreground
                        font.pixelSize: 19
                        font.weight: Font.DemiBold
                    }
                    TextField {
                        id: poetFilter
                        objectName: "poetFilter"
                        LayoutMirroring.enabled: false
                        Layout.fillWidth: true
                        font.family: typography.family
                        font.pixelSize: 15
                        color: colors.foreground
                        horizontalAlignment: Text.AlignRight
                        inputMethodHints: Qt.ImhNoPredictiveText
                        Accessible.name: "جستجوی شاعر"
                        onTextChanged: root.poetListModel.setFilterText(text)
                        Text {
                            objectName: "poetFilterPlaceholder"
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            LayoutMirroring.enabled: false
                            visible: poetFilter.length === 0 && !poetFilter.activeFocus
                            text: "جستجوی شاعر"
                            color: colors.muted
                            font: poetFilter.font
                            horizontalAlignment: Text.AlignRight
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 9
                            color: colors.surface
                            border.color: poetFilter.activeFocus ? colors.focus : colors.border
                            border.width: poetFilter.activeFocus ? 2 : 1
                        }
                    }
                    ListView {
                        id: poetList
                        objectName: "poetList"
                        readonly property real rowGutter: poetScrollBar.width + 8
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.poetListModel
                        clip: true
                        spacing: 4
                        boundsBehavior: Flickable.StopAtBounds
                        ScrollBar.vertical: AppScrollBar {
                            id: poetScrollBar
                            objectName: "poetScrollBar"
                            visible: poetList.contentHeight > poetList.height + 1
                            trackColor: colors.surfaceRaised
                            thumbColor: colors.muted
                            activeThumbColor: colors.accent
                        }
                        delegate: Item {
                            id: poetEntry
                            required property string name
                            required property string fullUrl
                            width: poetList.width
                            height: poetRow.implicitHeight
                            LayoutMirroring.enabled: false
                            ItemDelegate {
                                id: poetRow
                                objectName: "poetRow"
                                x: poetList.rowGutter
                                width: poetEntry.width - poetList.rowGutter - 4
                                LayoutMirroring.enabled: true
                                text: poetEntry.name
                                font.family: typography.family
                                implicitHeight: Math.max(48,
                                    contentItem.implicitHeight + topPadding + bottomPadding)
                                hoverEnabled: true
                                contentItem: Text {
                                    objectName: "poetRowText"
                                    LayoutMirroring.enabled: false
                                    text: poetRow.text
                                    font: poetRow.font
                                    color: poetEntry.fullUrl === root.selectedPoetUrl
                                        ? colors.accentText : colors.foreground
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                                focusPolicy: Qt.StrongFocus
                                Accessible.name: poetEntry.name
                                background: Rectangle {
                                    radius: 9
                                    color: poetEntry.fullUrl === root.selectedPoetUrl ? colors.accent
                                        : poetRow.pressed || poetRow.hovered
                                            ? colors.surfaceRaised : colors.surface
                                    border.color: poetRow.activeFocus ? colors.focus
                                        : poetEntry.fullUrl === root.selectedPoetUrl
                                            ? colors.accent : colors.border
                                    border.width: poetRow.activeFocus ? 2 : 1
                                }
                                onClicked: root.selectPoet(poetEntry.fullUrl, poetEntry.name)
                            }
                        }
                    }
                }
            }

            Rectangle {
                objectName: "contentPane"
                visible: !root.compactBrowse || root.page !== "poets"
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: root.phoneLayout ? 12 : 18
                color: colors.surface
                border.color: colors.border
                Item {
                    objectName: "welcomePanel"
                    anchors.fill: parent
                    visible: root.page === "poets"
                    ColumnLayout {
                        objectName: "welcomeContent"
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 48, 560)
                        spacing: 12
                        Label {
                            objectName: "welcomeLabel"
                            text: "به لئومون پارسی‌نما خوش آمدید"
                            color: colors.foreground
                            font.pixelSize: 27
                            font.weight: Font.DemiBold
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                        Label {
                            text: "برای خواندن، شاعری را از فهرست انتخاب کنید."
                            color: colors.muted
                            font.pixelSize: 16
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.Wrap
                        }
                    }
                }
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: root.phoneLayout ? (root.shortAndroidView ? 6 : 8) : 14
                    spacing: root.phoneLayout ? (root.shortAndroidView ? 6 : 8) : 16
                    visible: root.page !== "poets"
                    Item {
                        id: readerToolbar
                        objectName: "readerToolbar"
                        Layout.fillWidth: true
                        Layout.preferredHeight: root.stackedReaderToolbar
                            ? (root.phoneLayout ? 84 : 86) : 44
                        visible: root.page === "poet" || root.page === "collection" || root.page === "poem"
                        readonly property real landscapeCrumbWidth: Math.max(160,
                            Math.min(breadcrumbs.fullCrumbWidth, 250))
                        TextMetrics {
                            id: inlineTitleMeasure
                            text: root.currentContentTitle
                            font.family: typography.family
                            font.pixelSize: 24
                            font.weight: Font.DemiBold
                        }
                        readonly property bool inlinePhoneTitle: root.phoneLayout
                            && root.shortAndroidView
                            && (root.page === "poet" || root.page === "collection" || root.page === "poem")
                            && inlineTitleMeasure.advanceWidth + actionRow.width
                                + landscapeCrumbWidth + 24 <= width
                        BreadcrumbBar {
                            id: breadcrumbs
                            objectName: "breadcrumbBar"
                            x: root.stackedReaderToolbar ? 0
                                : readerToolbar.inlinePhoneTitle
                                    ? parent.width - readerToolbar.landscapeCrumbWidth
                                    : actionRow.width + 8
                            width: root.stackedReaderToolbar ? parent.width
                                : readerToolbar.inlinePhoneTitle
                                    ? readerToolbar.landscapeCrumbWidth
                                    : parent.width - actionRow.width - 8
                            height: 38
                            navigationController: root.navigationController
                            appTheme: colors
                            typography: typography
                            phoneMode: root.phoneLayout
                            onActivated: (index) => root.openBreadcrumb(index)
                        }
                        RowLayout {
                            id: actionRow
                            objectName: "readerActionArea"
                            x: 0
                            y: root.stackedReaderToolbar ? (root.phoneLayout ? 40 : 42) : 0
                            width: implicitWidth
                            height: 44
                            spacing: 8
                            HeaderAction {
                                objectName: "toggleFavoriteButton"
                                enabled: root.bookmarkStore.canFavoriteCurrent
                                symbol: "\ue866"
                                hint: root.bookmarkStore.currentFavorite ? "حذف نشانک" : "افزودن نشانک"
                                font.family: typography.iconFamily
                                surfaceColor: colors.surface
                                textColor: root.bookmarkStore.currentFavorite ? colors.accent : colors.foreground
                                borderColor: colors.border
                                focusColor: colors.focus
                                onClicked: root.bookmarkStore.toggleCurrent()
                            }
                            HeaderAction {
                                objectName: "previousPoemButton"
                                visible: root.page === "poem"
                                enabled: root.navigationController.hasPreviousPoem
                                symbol: "\ue5c8"
                                hint: "شعر پیشین"
                                font.family: typography.iconFamily
                                surfaceColor: colors.surface
                                textColor: colors.foreground
                                borderColor: colors.border
                                focusColor: colors.focus
                                onClicked: root.previousPoem()
                            }
                            HeaderAction {
                                objectName: "nextPoemButton"
                                visible: root.page === "poem"
                                enabled: root.navigationController.hasNextPoem
                                symbol: "\ue5c4"
                                hint: "شعر بعدی"
                                font.family: typography.iconFamily
                                surfaceColor: colors.surface
                                textColor: colors.foreground
                                borderColor: colors.border
                                focusColor: colors.focus
                                onClicked: root.nextPoem()
                            }
                        }
                        Label {
                            id: inlineTitle
                            objectName: "landscapeContentTitle"
                            visible: readerToolbar.inlinePhoneTitle
                            x: actionRow.width + 8
                            y: 0
                            width: breadcrumbs.x - x - 8
                            height: 44
                            text: root.currentContentTitle
                            color: colors.foreground
                            font.pixelSize: 24
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                    Label {
                        objectName: "contentTitle"
                        visible: !readerToolbar.inlinePhoneTitle
                        LayoutMirroring.enabled: false
                        text: root.currentContentTitle
                        color: colors.foreground
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.Wrap
                    }
                    SettingsPage {
                        id: settingsPage
                        objectName: "settingsPage"
                        visible: root.page === "settings"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        settingsStore: root.settingsStore
                        appTheme: colors
                        typography: typography
                    }
                    FavoritesPage {
                        objectName: "favoritesPage"
                        visible: root.page === "favorites"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        bookmarkStore: root.bookmarkStore
                        appTheme: colors
                        typography: typography
                        onActivated: (entryType, fullUrl) => root.openFavorite(entryType, fullUrl)
                        onRemoveRequested: (entryType, fullUrl) => root.bookmarkStore.remove(entryType, fullUrl)
                    }
                    SearchPage {
                        id: searchPage
                        objectName: "searchPage"
                        visible: root.page === "search"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        searchRepository: root.searchRepository
                        appTheme: colors
                        typography: typography
                        onActivated: (entryType, fullUrl) => root.openFavorite(entryType, fullUrl)
                    }
                    ListView {
                        id: collectionList
                        objectName: "collectionList"
                        readonly property real rowGutter: collectionScrollBar.width + 8
                        visible: root.page === "poet" || root.page === "collection"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.collectionListModel
                        clip: true
                        spacing: 4
                        onContentHeightChanged: {
                            if (visible && root.navigationController.collectionScroll === 0
                                && contentY <= 0)
                                collectionScrollTimer.restart()
                        }
                        onMovementStarted: collectionScrollTimer.stop()
                        header: ColumnLayout {
                            width: collectionList.width - collectionList.rowGutter - 4
                            x: collectionList.rowGutter
                            spacing: 16
                            Label {
                                objectName: "poetBiography"
                                visible: root.page === "poet" && root.navigationController.poetDescription !== ""
                                LayoutMirroring.enabled: false
                                Layout.fillWidth: true
                                text: root.navigationController.poetDescription
                                color: colors.foreground
                                font.pixelSize: 15
                                horizontalAlignment: Text.AlignRight
                                wrapMode: Text.Wrap
                            }
                            Label {
                                visible: root.navigationController.bookName !== ""
                                LayoutMirroring.enabled: false
                                text: root.navigationController.bookName
                                color: colors.muted
                                font.pixelSize: 15
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignRight
                            }
                            Label {
                                visible: root.navigationController.description !== ""
                                    && (root.page !== "poet"
                                        || root.navigationController.description !== root.navigationController.poetDescription)
                                LayoutMirroring.enabled: false
                                Layout.fillWidth: true
                                text: root.navigationController.description
                                color: colors.foreground
                                font.pixelSize: 15
                                horizontalAlignment: Text.AlignRight
                                wrapMode: Text.Wrap
                            }
                            Label {
                                LayoutMirroring.enabled: false
                                text: root.collectionListModel.categoryCount + " مجموعه · "
                                    + root.collectionListModel.poemCount + " شعر"
                                color: colors.muted
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                Layout.bottomMargin: 12
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                        ScrollBar.vertical: AppScrollBar {
                            id: collectionScrollBar
                            objectName: "collectionScrollBar"
                            visible: collectionList.contentHeight > collectionList.height + 1
                            trackColor: colors.surfaceRaised
                            thumbColor: colors.muted
                            activeThumbColor: colors.accent
                        }
                        delegate: Item {
                            id: collectionEntry
                            required property string title
                            required property string fullUrl
                            required property string entryType
                            width: collectionList.width
                            height: collectionRow.implicitHeight
                            LayoutMirroring.enabled: false
                            ItemDelegate {
                                id: collectionRow
                                objectName: "collectionRow"
                                x: collectionList.rowGutter
                                width: collectionEntry.width - collectionList.rowGutter - 4
                                LayoutMirroring.enabled: true
                                text: collectionEntry.title
                                font.family: typography.family
                                implicitHeight: Math.max(48,
                                    contentItem.implicitHeight + topPadding + bottomPadding)
                                hoverEnabled: true
                                contentItem: Text {
                                    objectName: "collectionRowText"
                                    LayoutMirroring.enabled: false
                                    text: collectionRow.text
                                    font: collectionRow.font
                                    color: colors.foreground
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                                focusPolicy: Qt.StrongFocus
                                Accessible.name: collectionEntry.title
                                background: Rectangle {
                                    radius: 9
                                    color: collectionRow.pressed || collectionRow.hovered
                                        ? colors.surfaceRaised : colors.surface
                                    border.color: collectionRow.activeFocus ? colors.focus : colors.border
                                    border.width: collectionRow.activeFocus ? 2 : 1
                                }
                                onClicked: {
                                    if (collectionEntry.entryType === "category")
                                        root.openCategory(collectionEntry.fullUrl)
                                    else
                                        root.openPoem(collectionEntry.fullUrl)
                                }
                            }
                        }
                    }
                    Connections {
                        target: collectionList.headerItem
                        function onHeightChanged() {
                            if (collectionList.visible
                                && root.navigationController.collectionScroll === 0
                                && collectionList.contentY <= 0)
                                collectionScrollTimer.restart()
                        }
                    }
                    PoemPage {
                        id: poemList
                        visible: root.page === "poem"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        poemLoader: root.poemLoader
                        navigationController: root.navigationController
                        settingsStore: root.settingsStore
                        appTheme: colors
                        typography: typography
                        phoneMode: root.phoneLayout
                        onContentHeightChanged: {
                            if (root.poemScrollPending && !root.poemLoader.loading)
                                poemScrollTimer.restart()
                        }
                        onMovementStarted: {
                            root.poemScrollPending = false
                            poemScrollTimer.stop()
                            poemList.recordUserScroll()
                        }
                    }
                    Connections {
                        target: poemList.headerItem
                        function onHeightChanged() {
                            if (root.poemScrollPending && !root.poemLoader.loading)
                                poemScrollTimer.restart()
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            visible: root.statusMessage !== ""
            implicitHeight: 40
            radius: 10
            color: colors.surface
            border.color: colors.border
            Label {
                objectName: "catalogStatus"
                anchors.fill: parent
                anchors.margins: 10
                LayoutMirroring.enabled: false
                text: root.statusMessage
                color: colors.muted
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        z: 100
        visible: root.catalogStartup !== null && !root.catalogStartup.ready
        color: colors.background
        ColumnLayout {
            anchors.centerIn: parent
            width: Math.min(parent.width - 40, 440)
            spacing: 16
            Label {
                Layout.fillWidth: true
                text: "آماده‌سازی کتابخانهٔ آفلاین"
                font.pixelSize: 22
                font.bold: true
                color: colors.foreground
                horizontalAlignment: Text.AlignHCenter
            }
            Label {
                Layout.fillWidth: true
                text: root.catalogStartup === null ? "" : root.catalogStartup.statusText
                color: root.catalogStartup !== null && root.catalogStartup.busy
                    ? colors.muted : colors.foreground
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
            ProgressBar {
                Layout.fillWidth: true
                visible: root.catalogStartup !== null && root.catalogStartup.busy
                from: 0
                to: 100
                value: root.catalogStartup === null ? 0 : root.catalogStartup.progress
            }
        }
    }

    Drawer {
        id: navigationDrawer
        objectName: "navigationDrawer"
        edge: Qt.RightEdge
        y: 0
        width: Math.min(320, root.width * 0.84)
        height: root.height
        padding: 0
        modal: true
        dim: true
        interactive: root.phoneLayout
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        onClosed: {
            if (root.page !== "search" && Qt.platform.os === "android") {
                searchPage.releaseQueryFocus()
                root.contentItem.forceActiveFocus()
                Qt.inputMethod.hide()
            }
        }
        Overlay.modal: Rectangle { color: "#80000000" }
        background: Rectangle {
            radius: 0
            color: colors.surface
            border.color: colors.border
        }
        contentItem: Item {
            LayoutMirroring.enabled: false
            LayoutMirroring.childrenInherit: true
            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 6
                Label {
                    width: parent.width
                    height: 52
                    text: "فهرست"
                    color: colors.foreground
                    font.family: typography.family
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                }
                Repeater {
                    model: [
                        { title: "شاعران", target: "poets", icon: "\ue865" },
                        { title: "نشانک‌ها", target: "favorites", icon: "\ue866" },
                        { title: "جستجو", target: "search", icon: "\ue8b6" },
                        { title: "تنظیمات", target: "settings", icon: "\ue8b8" }
                    ]
                    delegate: Button {
                        id: drawerDestination
                        objectName: "drawerDestination"
                        required property var modelData
                        readonly property bool selected: modelData.target === "poets"
                            ? root.page !== "favorites" && root.page !== "search" && root.page !== "settings"
                            : root.page === modelData.target
                        width: parent.width
                        height: 56
                        text: modelData.title
                        Accessible.name: text
                        onClicked: {
                            navigationDrawer.close()
                            if (modelData.target === "poets") root.showPoets()
                            else if (modelData.target === "favorites") root.showFavorites()
                            else if (modelData.target === "search") root.showSearch()
                            else root.showSettings()
                        }
                        contentItem: Item {
                            Row {
                                anchors.right: parent.right
                                anchors.rightMargin: 16
                                anchors.verticalCenter: parent.verticalCenter
                                layoutDirection: Qt.RightToLeft
                                spacing: 12
                                Text {
                                    text: drawerDestination.modelData.icon
                                    color: drawerDestination.selected ? colors.accent : colors.foreground
                                    font.family: typography.iconFamily
                                    font.pixelSize: 24
                                }
                                Text {
                                    text: drawerDestination.text
                                    color: colors.foreground
                                    font.family: typography.family
                                    font.pixelSize: 17
                                }
                            }
                        }
                        background: Rectangle {
                            radius: 12
                            color: drawerDestination.selected || drawerDestination.down
                                ? colors.surfaceRaised : "transparent"
                            border.color: drawerDestination.selected ? colors.accent : "transparent"
                        }
                    }
                }
            }
        }
    }

    Menu {
        id: overflowMenu
        objectName: "overflowMenu"
        width: 230
        topPadding: 6
        bottomPadding: 6
        leftPadding: 6
        rightPadding: 6
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            objectName: "overflowMenuBackground"
            radius: 11
            color: colors.surface
            border.color: colors.border
        }
        AppMenuItem {
            objectName: "overflowPoetsItem"
            appTheme: colors
            text: "شاعران"
            onTriggered: root.showPoets()
        }
        AppMenuItem { appTheme: colors; text: "نشانک‌ها"; onTriggered: root.showFavorites() }
        AppMenuItem { appTheme: colors; text: "جستجو"; onTriggered: root.showSearch() }
        AppMenuItem {
            appTheme: colors
            text: "چاپ"
            visible: root.printingSupported
            implicitHeight: root.printingSupported ? 42 : 0
            height: implicitHeight
            enabled: root.canPrintCurrentPoem
            onTriggered: Qt.callLater(() => root.openPrintMenu())
        }
        AppMenuItem {
            appTheme: colors
            text: "تنظیمات"
            onTriggered: root.showSettings()
        }
    }

    Menu {
        id: printMenu
        objectName: "printMenu"
        width: 230
        topPadding: 6
        bottomPadding: 6
        leftPadding: 6
        rightPadding: 6
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle {
            radius: 11
            color: colors.surface
            border.color: colors.border
        }
        AppMenuItem {
            appTheme: colors
            text: "ذخیرهٔ پی‌دی‌اف"
            visible: root.printingSupported
            onTriggered: root.printService.choosePdfDestination()
        }
        AppMenuItem {
            appTheme: colors
            text: "چاپ با چاپگر"
            visible: root.printingSupported
            onTriggered: root.printCurrentPoem()
        }
    }
}
