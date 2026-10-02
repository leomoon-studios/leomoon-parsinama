pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

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
    required property var printService
    readonly property bool compactHeader: width < 760
    readonly property bool compactBrowse: width < 760
    readonly property bool bundledFontReady: typography.ready
    readonly property bool bundledFontError: typography.failed
    readonly property bool bundledIconFontReady: typography.iconReady
    readonly property color accentColor: colors.accent
    readonly property color accentTextColor: colors.accentText
    readonly property color foregroundColor: colors.foreground
    readonly property color surfaceColor: colors.surface
    readonly property bool canPrintCurrentPoem: page === "poem" && printService.available
    readonly property string statusMessage: bookmarkStore.error !== "" ? bookmarkStore.error
        : printService.error !== "" ? printService.error
        : settingsStore.error !== "" ? settingsStore.error
        : navigationController.error !== "" ? navigationController.error
        : poemLoader.loading || poemLoader.error !== "" ? poemLoader.statusText
        : catalogRepository.ready ? "" : catalogRepository.statusText
    property string page: "poets"
    onPageChanged: {
        if (page !== "poem")
            printMenu.close()
    }
    property string selectedPoetName: ""
    property string selectedPoetUrl: ""

    width: 1120
    height: 760
    minimumWidth: 500
    minimumHeight: 440
    visible: true
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

    function showFavorites() {
        selectedPoetName = ""
        selectedPoetUrl = ""
        page = "favorites"
    }

    function showSearch() {
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
        page = "search"
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
        poemList.contentY = -(poemList.headerItem ? poemList.headerItem.height : 0)
    }

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
            root.page = root.navigationController.page
            root.selectedPoetName = root.navigationController.poetName
            root.selectedPoetUrl = root.navigationController.poetUrl
            Qt.callLater(() => {
                poetList.contentY = root.navigationController.poetScroll
                root.restoreCollectionScroll()
                collectionScrollTimer.restart()
                if (root.page === "poem") {
                    root.restorePoemScroll()
                    poemScrollTimer.restart()
                }
            })
        }
    }

    Connections {
        target: root.poemLoader
        function onRequestFinished() {
            if (root.page === "poem")
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
        anchors.margins: root.compactHeader ? 16 : 20
        spacing: 18

        RowLayout {
            objectName: "header"
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                id: appLogo
                objectName: "appLogo"
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: 11
                color: colors.accent
                clip: true
                Shape {
                    width: 256
                    height: 256
                    transformOrigin: Item.TopLeft
                    scale: appLogo.width / 256
                    ShapePath {
                        objectName: "appLogoGlyph"
                        fillColor: colors.accentText
                        strokeColor: "transparent"
                        PathSvg { path: "m138.938 151.188 8.53 8.53-8.53 8.532-8.477-8.531zm-19.032 0 8.531 8.53-8.53 8.532-8.532-8.531zm9.516 14 8.531 8.476-8.531 8.531-8.531-8.53zm9.023-19.688h-12.687q-12.797 0-22.531-2.242-9.735-2.242-15.204-8.04-5.468-5.796-5.468-16.515 0-4.703 1.039-9.187 1.094-4.485 2.68-8.313l12.25 4.594q-.876 2.516-1.805 5.797-.875 3.281-.875 6.289 0 5.633 3.937 8.422 3.938 2.734 10.719 3.61 6.781.874 15.258.874h12.797q6.398 0 11.32-.656 4.922-.711 7.71-2.735 2.845-2.023 2.845-6.07 0-3.5-.82-7.984-.82-4.54-1.805-8.696l13.398-3.39q2.297 10.281 2.297 19.195 0 8.04-2.734 13.016-2.68 4.976-7.493 7.601-4.812 2.57-11.156 3.5t-13.672.93" }
                    }
                }
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
                        Layout.maximumWidth: parent.width - applicationVersion.implicitWidth - parent.spacing
                        Layout.alignment: Qt.AlignBaseline
                    }
                    Label {
                        id: applicationVersion
                        text: "v" + Qt.application.version
                        color: colors.muted
                        font.pixelSize: 12
                        LayoutMirroring.enabled: false
                        Layout.alignment: Qt.AlignBaseline
                    }
                    Item { Layout.fillWidth: true }
                }
                Label {
                    Layout.fillWidth: true
                    text: "گنجینهٔ شعر فارسی"
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
                visible: !root.compactHeader
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
                font.family: typography.iconFamily
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.page = "settings"
            }
            HeaderAction {
                id: moreButton
                objectName: "moreButton"
                symbol: "\ue5d4"
                hint: "گزینه‌های بیشتر"
                font.family: typography.iconFamily
                visible: root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: overflowMenu.popup(moreButton, 0, moreButton.height + 8)
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
                                        : poetRow.hovered ? colors.surfaceRaised : colors.surface
                                    border.color: poetRow.activeFocus ? colors.focus : "transparent"
                                    border.width: poetRow.activeFocus ? 2 : 0
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
                radius: 18
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
                    anchors.margins: 14
                    spacing: 16
                    visible: root.page !== "poets"
                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            objectName: "contentTitle"
                            LayoutMirroring.enabled: false
                            text: root.page === "settings" ? "تنظیمات"
                                : root.page === "favorites" ? "نشانک‌ها"
                                : root.page === "search" ? "جستجو" : root.navigationController.title
                            color: colors.foreground
                            font.pixelSize: 24
                            font.weight: Font.DemiBold
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            wrapMode: Text.Wrap
                        }
                        HeaderAction {
                            objectName: "toggleFavoriteButton"
                            visible: root.page === "poet" || root.page === "collection" || root.page === "poem"
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
                    BreadcrumbBar {
                        objectName: "breadcrumbBar"
                        visible: root.page === "poet" || root.page === "collection" || root.page === "poem"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 38
                        navigationController: root.navigationController
                        appTheme: colors
                        typography: typography
                        onActivated: (index) => root.openBreadcrumb(index)
                    }
                    SettingsPage {
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
                                    color: collectionRow.hovered ? colors.surfaceRaised : colors.surface
                                    border.color: collectionRow.activeFocus ? colors.focus : "transparent"
                                    border.width: collectionRow.activeFocus ? 2 : 0
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
                        onContentHeightChanged: {
                            if (visible && contentY <= 0)
                                poemScrollTimer.restart()
                        }
                        onMovementStarted: poemScrollTimer.stop()
                    }
                    Connections {
                        target: poemList.headerItem
                        function onHeightChanged() {
                            if (poemList.visible && poemList.contentY <= 0)
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
            enabled: root.canPrintCurrentPoem
            onTriggered: Qt.callLater(() => root.openPrintMenu())
        }
        AppMenuItem {
            appTheme: colors
            text: "تنظیمات"
            onTriggered: root.page = "settings"
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
            onTriggered: root.printService.choosePdfDestination()
        }
        AppMenuItem {
            appTheme: colors
            text: "چاپ با چاپگر"
            onTriggered: root.printCurrentPoem()
        }
    }
}
