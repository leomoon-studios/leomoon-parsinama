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
    required property var settingsStore
    readonly property bool compactHeader: width < 850
    readonly property bool compactBrowse: width < 760
    readonly property bool bundledFontReady: typography.ready
    readonly property bool bundledFontError: typography.failed
    readonly property bool bundledIconFontReady: typography.iconReady
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

            Image {
                objectName: "appLogo"
                source: "qrc:/qt/qml/LeoMoon/ParsiNama/assets/app-icon.svg"
                sourceSize.width: 60
                sourceSize.height: 60
                Layout.preferredWidth: root.compactHeader ? 48 : 60
                Layout.preferredHeight: root.compactHeader ? 48 : 60
                fillMode: Image.PreserveAspectFit
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
                objectName: "themeButton"
                symbol: root.settingsStore.theme === "dark" ? "\ue518" : "\ue51c"
                hint: root.settingsStore.theme === "dark" ? "حالت روشن" : "حالت تیره"
                font.family: typography.iconFamily
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.settingsStore.toggleTheme()
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
                                palette.buttonText: poetEntry.fullUrl === root.selectedPoetUrl
                                    ? colors.accentText : colors.foreground
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
                    RowLayout {
                        visible: root.page === "settings"
                        Label { text: "اندازهٔ متن"; color: colors.foreground }
                        Button {
                            text: "−"
                            Accessible.name: "کوچک کردن متن"
                            onClicked: root.settingsStore.setReadingSize(root.settingsStore.readingSize - 1)
                        }
                        Label { text: root.settingsStore.readingSize.toString(); color: colors.foreground }
                        Button {
                            text: "+"
                            Accessible.name: "بزرگ کردن متن"
                            onClicked: root.settingsStore.setReadingSize(root.settingsStore.readingSize + 1)
                        }
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
                                palette.buttonText: colors.foreground
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
                    Item { Layout.fillHeight: true; visible: root.page === "settings" }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 40
            radius: 10
            color: colors.surface
            border.color: colors.border
            Label {
                objectName: "catalogStatus"
                anchors.fill: parent
                anchors.margins: 10
                text: root.settingsStore.error !== "" ? root.settingsStore.error
                    : root.poemLoader.loading || root.poemLoader.error !== ""
                    ? root.poemLoader.statusText : root.catalogRepository.statusText
                color: colors.muted
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideMiddle
            }
        }
    }
}
