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
    required property var settingsStore
    readonly property bool compactHeader: width < 850
    readonly property bool compactBrowse: width < 760
    readonly property bool bundledFontReady: typography.ready
    readonly property bool bundledFontError: typography.failed
    readonly property bool bundledIconFontReady: typography.iconReady
    readonly property color accentColor: colors.accent
    readonly property color accentTextColor: colors.accentText
    readonly property color foregroundColor: colors.foreground
    readonly property color surfaceColor: colors.surface
    readonly property string statusMessage: settingsStore.error !== "" ? settingsStore.error
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
        selectedPoetName = name
        selectedPoetUrl = fullUrl
        if (collectionListModel.loadCategory(fullUrl))
            page = "collection"
    }

    function showPoets() {
        selectedPoetName = ""
        selectedPoetUrl = ""
        collectionListModel.clear()
        poemLoader.clear()
        page = "poets"
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
                Layout.fillWidth: true
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
                objectName: "moreButton"
                symbol: "\ue5d4"
                hint: "گزینه‌های بیشتر"
                font.family: typography.iconFamily
                visible: root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: overflowMenu.popup()
                Menu {
                    id: overflowMenu
                    MenuItem { text: "شاعران"; onTriggered: root.showPoets() }
                    MenuItem { text: "نشانک‌ها"; enabled: false }
                    MenuItem { text: "جستجو"; enabled: false }
                    MenuItem { text: "چاپ"; enabled: false }
                    MenuItem { text: "تنظیمات"; onTriggered: root.page = "settings" }
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
                    anchors.margins: 24
                    spacing: 16
                    visible: root.page !== "poets"
                    Label {
                        text: root.page === "settings" ? "تنظیمات" : root.selectedPoetName
                        color: colors.foreground
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
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
                        visible: root.page === "collection"
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
                                        root.collectionListModel.loadCategory(collectionEntry.fullUrl)
                                    else
                                        root.poemLoader.loadByUrl(collectionEntry.fullUrl)
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
}
