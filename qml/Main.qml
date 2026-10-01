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
    property string page: "poets"
    property string selectedPoetName: ""
    property string selectedPoetUrl: ""

    width: 1120
    height: 760
    minimumWidth: 500
    minimumHeight: 440
    visible: true
    title: "LeoMoon ParsiNama"
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
                        text: "LeoMoon ParsiNama"
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
                symbol: "☷"
                hint: "شاعران"
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.page = "poets"
            }
            HeaderAction {
                objectName: "favoritesButton"
                symbol: "☆"
                hint: "نشانک‌ها"
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
            }
            HeaderAction {
                objectName: "searchButton"
                symbol: "⌕"
                hint: "جستجو"
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
            }
            HeaderAction {
                objectName: "printButton"
                symbol: "⎙"
                hint: "چاپ"
                visible: !root.compactHeader
                enabled: false
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
            }
            HeaderAction {
                objectName: "themeButton"
                symbol: root.settingsStore.theme === "dark" ? "☼" : "☾"
                hint: root.settingsStore.theme === "dark" ? "حالت روشن" : "حالت تیره"
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.settingsStore.toggleTheme()
            }
            HeaderAction {
                objectName: "settingsButton"
                symbol: "⚙"
                hint: "تنظیمات"
                visible: !root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: root.page = "settings"
            }
            HeaderAction {
                objectName: "moreButton"
                symbol: "⋮"
                hint: "گزینه‌های بیشتر"
                visible: root.compactHeader
                surfaceColor: colors.surface
                textColor: colors.foreground
                borderColor: colors.border
                focusColor: colors.focus
                onClicked: overflowMenu.popup()
                Menu {
                    id: overflowMenu
                    MenuItem { text: "شاعران"; onTriggered: root.page = "poets" }
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
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.poetListModel
                        clip: true
                        spacing: 4
                        boundsBehavior: Flickable.StopAtBounds
                        delegate: ItemDelegate {
                            id: poetRow
                            required property string name
                            required property string fullUrl
                            width: poetList.width
                            text: name
                            font.family: typography.family
                            palette.buttonText: fullUrl === root.selectedPoetUrl
                                ? colors.accentText : colors.foreground
                            focusPolicy: Qt.StrongFocus
                            Accessible.name: name
                            background: Rectangle {
                                radius: 9
                                color: poetRow.fullUrl === root.selectedPoetUrl ? colors.accent
                                    : poetRow.hovered ? colors.surfaceRaised : colors.surface
                                border.color: poetRow.activeFocus ? colors.focus : "transparent"
                                border.width: poetRow.activeFocus ? 2 : 0
                            }
                            onClicked: root.selectPoet(fullUrl, name)
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
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 16
                    Label {
                        objectName: "welcomeLabel"
                        text: root.page === "settings" ? "تنظیمات" : root.page === "collection"
                            ? root.selectedPoetName : "به لئومون پارسی‌نما خوش آمدید"
                        color: colors.foreground
                        font.pixelSize: 24
                        font.weight: Font.DemiBold
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignRight
                        wrapMode: Text.Wrap
                    }
                    Label {
                        visible: root.page === "poets"
                        text: "برای خواندن، شاعری را از فهرست انتخاب کنید."
                        color: colors.muted
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
                        visible: root.page === "collection"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: root.collectionListModel
                        clip: true
                        spacing: 4
                        delegate: ItemDelegate {
                            id: collectionRow
                            required property string title
                            required property string fullUrl
                            required property string entryType
                            width: collectionList.width
                            text: title
                            font.family: typography.family
                            palette.buttonText: colors.foreground
                            focusPolicy: Qt.StrongFocus
                            Accessible.name: title
                            background: Rectangle {
                                radius: 9
                                color: collectionRow.hovered ? colors.surfaceRaised : colors.surface
                                border.color: collectionRow.activeFocus ? colors.focus : "transparent"
                                border.width: collectionRow.activeFocus ? 2 : 0
                            }
                            onClicked: {
                                if (entryType === "category")
                                    root.collectionListModel.loadCategory(fullUrl)
                                else
                                    root.poemLoader.loadByUrl(fullUrl)
                            }
                        }
                    }
                    Item { Layout.fillHeight: true; visible: root.page !== "collection" }
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
