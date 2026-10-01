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
    required property var navigationController
    required property var settingsStore
    readonly property bool compactHeader: width < 760
    readonly property bool compactBrowse: width < 760
    readonly property bool bundledFontReady: typography.ready
    readonly property bool bundledFontError: typography.failed
    readonly property bool bundledIconFontReady: typography.iconReady
    readonly property color accentColor: colors.accent
    readonly property color accentTextColor: colors.accentText
    readonly property color foregroundColor: colors.foreground
    readonly property color surfaceColor: colors.surface
    readonly property string statusMessage: settingsStore.error !== "" ? settingsStore.error
        : navigationController.error !== "" ? navigationController.error
        : poemLoader.loading || poemLoader.error !== "" ? poemLoader.statusText
        : catalogRepository.ready ? "" : catalogRepository.statusText
    property string page: "poets"
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
        if (navigationController.openPoet(fullUrl, poetList.contentY, collectionList.contentY))
            page = "poet"
    }

    function showPoets() {
        navigationController.openPoets(poetList.contentY, collectionList.contentY)
        selectedPoetName = ""
        selectedPoetUrl = ""
        poetFilter.text = ""
        page = "poets"
    }

    function openCategory(fullUrl) {
        navigationController.openCategory(fullUrl, poetList.contentY, collectionList.contentY)
    }

    function openPoem(fullUrl) {
        navigationController.openPoem(fullUrl, poetList.contentY, collectionList.contentY)
    }

    function openBreadcrumb(index) {
        navigationController.openBreadcrumb(index, poetList.contentY, collectionList.contentY)
    }

    function goBack() {
        navigationController.back(poetList.contentY, collectionList.contentY)
    }

    function goForward() {
        navigationController.forward(poetList.contentY, collectionList.contentY)
    }

    Connections {
        target: root.navigationController
        function onStateChanged() {
            root.page = root.navigationController.page
            root.selectedPoetName = root.navigationController.poetName
            root.selectedPoetUrl = root.navigationController.poetUrl
            Qt.callLater(() => {
                poetList.contentY = root.navigationController.poetScroll
                collectionList.contentY = root.navigationController.collectionScroll
            })
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
        anchors.margins: root.compactHeader ? 16 : 24
        spacing: 18

        RowLayout {
            objectName: "header"
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                id: appLogo
                objectName: "appLogo"
                Layout.preferredWidth: root.compactHeader ? 48 : 60
                Layout.preferredHeight: root.compactHeader ? 48 : 60
                radius: width / 4
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
                spacing: 0
                RowLayout {
                    spacing: 10
                    Label {
                        objectName: "headerTitle"
                        text: "لئومون پارسی‌نما"
                        color: colors.foreground
                        font.pixelSize: root.compactHeader ? 21 : 28
                        font.weight: Font.DemiBold
                        elide: Text.ElideRight
                    }
                    Label {
                        text: "v" + Qt.application.version
                        color: colors.muted
                        font.pixelSize: 14
                    }
                }
                Label {
                    text: "گنجینهٔ شعر فارسی"
                    color: colors.muted
                    font.pixelSize: 15
                    elide: Text.ElideRight
                }
            }
            Item { Layout.fillWidth: true }
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
                symbol: "\ue87d"
                hint: "نشانک‌ها"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
            }
            HeaderAction {
                objectName: "searchButton"
                symbol: "\ue8b6"
                hint: "جستجو"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
            }
            HeaderAction {
                objectName: "printButton"
                symbol: "\ue8ad"
                hint: "چاپ"
                font.family: typography.iconFamily
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
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
                        placeholderText: "جستجوی شاعر"
                        font.family: typography.family
                        font.pixelSize: 15
                        color: colors.foreground
                        placeholderTextColor: colors.muted
                        horizontalAlignment: Text.AlignRight
                        inputMethodHints: Qt.ImhNoPredictiveText
                        onTextChanged: root.poetListModel.setFilterText(text)
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
                    Label {
                        visible: root.page !== "poets" && root.page !== "settings"
                        text: "مسیر مجموعه"
                        color: colors.muted
                        font.pixelSize: 14
                    }
                    ListView {
                        id: sidebarPath
                        objectName: "sidebarPath"
                        visible: root.page !== "poets" && root.page !== "settings"
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(160, count * 38)
                        model: root.navigationController.sidebarPath
                        clip: true
                        spacing: 4
                        delegate: Button {
                            id: pathButton
                            required property int index
                            required property var modelData
                            width: sidebarPath.width
                            implicitHeight: 34
                            text: modelData.title
                            font.family: typography.family
                            onClicked: root.openBreadcrumb(index + 1)
                            contentItem: Text {
                                LayoutMirroring.enabled: false
                                text: pathButton.text
                                font: pathButton.font
                                color: colors.foreground
                                horizontalAlignment: Text.AlignRight
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideRight
                            }
                            background: Rectangle {
                                radius: 7
                                color: pathButton.hovered ? colors.surfaceRaised : colors.surface
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
                            text: root.page === "settings" ? "تنظیمات" : root.navigationController.title
                            color: colors.foreground
                            font.pixelSize: 24
                            font.weight: Font.DemiBold
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                            wrapMode: Text.Wrap
                        }
                        HeaderAction {
                            objectName: "backButton"
                            visible: root.page !== "settings"
                            enabled: root.navigationController.canGoBack
                            symbol: "\ue5c8"
                            hint: "بازگشت"
                            font.family: typography.iconFamily
                            surfaceColor: colors.surface
                            textColor: colors.foreground
                            borderColor: colors.border
                            focusColor: colors.focus
                            onClicked: root.goBack()
                        }
                        HeaderAction {
                            objectName: "forwardButton"
                            visible: root.page !== "settings"
                            enabled: root.navigationController.canGoForward
                            symbol: "\ue5c4"
                            hint: "پیش‌روی"
                            font.family: typography.iconFamily
                            surfaceColor: colors.surface
                            textColor: colors.foreground
                            borderColor: colors.border
                            focusColor: colors.focus
                            onClicked: root.goForward()
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
                    ScrollView {
                        visible: root.page === "poet" && root.navigationController.poetDescription !== ""
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(150, biographyLabel.implicitHeight + 12)
                        contentWidth: availableWidth
                        clip: true
                        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                        Label {
                            id: biographyLabel
                            objectName: "poetBiography"
                            LayoutMirroring.enabled: false
                            width: parent.width
                            text: root.navigationController.poetDescription
                            color: colors.foreground
                            font.pixelSize: 15
                            horizontalAlignment: Text.AlignRight
                            wrapMode: Text.Wrap
                        }
                    }
                    Label {
                        visible: (root.page === "poet" || root.page === "collection")
                            && root.navigationController.bookName !== ""
                        LayoutMirroring.enabled: false
                        text: root.navigationController.bookName
                        color: colors.muted
                        font.pixelSize: 15
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    Label {
                        visible: (root.page === "poet" || root.page === "collection")
                            && root.navigationController.description !== ""
                        LayoutMirroring.enabled: false
                        text: root.navigationController.description
                        color: colors.foreground
                        font.pixelSize: 15
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }
                    Label {
                        visible: root.page === "poet" || root.page === "collection"
                        LayoutMirroring.enabled: false
                        text: root.collectionListModel.categoryCount + " مجموعه · "
                            + root.collectionListModel.poemCount + " شعر"
                        color: colors.muted
                        font.pixelSize: 14
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
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
                        ScrollBar.vertical: AppScrollBar {
                            id: collectionScrollBar
                            objectName: "collectionScrollBar"
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
                    Label {
                        visible: collectionList.visible && root.collectionListModel.count > 0
                        LayoutMirroring.enabled: false
                        text: "مورد " + Math.max(1, collectionList.indexAt(1, collectionList.contentY + 1) + 1)
                            + " از " + root.collectionListModel.count
                        color: colors.muted
                        font.pixelSize: 13
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                    }
                    ColumnLayout {
                        visible: root.page === "poem"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Label {
                            LayoutMirroring.enabled: false
                            text: "شعر " + root.navigationController.poemPosition
                                + " از " + root.navigationController.poemCount
                            color: colors.muted
                            font.pixelSize: 14
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                        Label {
                            visible: root.poemLoader.summary !== ""
                            LayoutMirroring.enabled: false
                            text: root.poemLoader.summary
                            color: colors.muted
                            font.pixelSize: 15
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignJustify
                            wrapMode: Text.Wrap
                        }
                        ListView {
                            id: poemList
                            objectName: "poemList"
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            model: root.poemLoader.readingRows
                            clip: true
                            spacing: 8
                            ScrollBar.vertical: AppScrollBar {
                                trackColor: colors.surfaceRaised
                                thumbColor: colors.muted
                                activeThumbColor: colors.accent
                            }
                            delegate: Item {
                                id: verseEntry
                                required property bool paired
                                required property string rightText
                                required property string leftText
                                required property string text
                                readonly property bool stacked: poemList.width < 700
                                width: poemList.width
                                height: readingContent.height + 14
                                Item {
                                    id: readingContent
                                    LayoutMirroring.enabled: false
                                    width: Math.min(Math.max(0, verseEntry.width - 48), 1000)
                                    anchors.horizontalCenter: parent.horizontalCenter
                                    height: verseEntry.paired
                                        ? verseEntry.stacked
                                            ? rightVerse.implicitHeight + leftVerse.implicitHeight + 10
                                            : Math.max(rightVerse.implicitHeight, leftVerse.implicitHeight)
                                        : singleVerse.implicitHeight
                                    Text {
                                        id: rightVerse
                                        visible: verseEntry.paired
                                        width: verseEntry.stacked ? readingContent.width
                                            : (readingContent.width - 28) / 2
                                        x: verseEntry.stacked ? 0 : readingContent.width - width
                                        text: verseEntry.rightText
                                        color: colors.foreground
                                        font.family: typography.family
                                        font.pixelSize: root.settingsStore.readingSize
                                        horizontalAlignment: verseEntry.stacked ? Text.AlignRight
                                            : lineCount > 1 ? Text.AlignJustify : Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                    }
                                    Text {
                                        id: leftVerse
                                        visible: verseEntry.paired
                                        width: verseEntry.stacked ? readingContent.width
                                            : (readingContent.width - 28) / 2
                                        y: verseEntry.stacked ? rightVerse.implicitHeight + 10 : 0
                                        text: verseEntry.leftText
                                        color: colors.foreground
                                        font.family: typography.family
                                        font.pixelSize: root.settingsStore.readingSize
                                        horizontalAlignment: verseEntry.stacked ? Text.AlignRight
                                            : lineCount > 1 ? Text.AlignJustify : Text.AlignHCenter
                                        wrapMode: Text.Wrap
                                    }
                                    Text {
                                        id: singleVerse
                                        visible: !verseEntry.paired
                                        width: readingContent.width
                                        text: verseEntry.text
                                        color: colors.foreground
                                        font.family: typography.family
                                        font.pixelSize: root.settingsStore.readingSize
                                        horizontalAlignment: Text.AlignRight
                                        wrapMode: Text.Wrap
                                    }
                                }
                            }
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
        AppMenuItem { appTheme: colors; text: "نشانک‌ها"; enabled: false }
        AppMenuItem { appTheme: colors; text: "جستجو"; enabled: false }
        AppMenuItem { appTheme: colors; text: "چاپ"; enabled: false }
        AppMenuItem {
            appTheme: colors
            text: "تنظیمات"
            onTriggered: root.page = "settings"
        }
    }
}
