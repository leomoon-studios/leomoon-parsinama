pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var searchRepository
    required property var appTheme
    required property var typography
    property string availablePoetUrl: ""
    property string availableCategoryUrl: ""
    property string scope: "all"
    property string resultType: ""
    signal activated(string entryType, string fullUrl)

    function focusQuery() {
        queryField.forceActiveFocus(Qt.ShortcutFocusReason)
        queryField.selectAll()
    }

    onVisibleChanged: {
        if (visible)
            Qt.callLater(() => root.focusQuery())
    }

    function refresh() {
        searchRepository.search(queryField.text,
            scope === "poet" || scope === "collection" ? availablePoetUrl : "",
            scope === "collection" ? availableCategoryUrl : "", resultType)
    }

    Timer {
        id: searchDelay
        interval: 220
        onTriggered: root.refresh()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        TextField {
            id: queryField
            objectName: "searchQuery"
            Layout.fillWidth: true
            LayoutMirroring.enabled: false
            horizontalAlignment: Text.AlignRight
            font.family: root.typography.family
            font.pixelSize: 17
            color: root.appTheme.foreground
            Accessible.name: "جستجو در شعرها"
            onTextChanged: searchDelay.restart()
            Text {
                objectName: "searchQueryPlaceholder"
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                LayoutMirroring.enabled: false
                visible: queryField.length === 0
                text: "جستجو در شاعران، مجموعه‌ها و شعرها"
                color: root.appTheme.muted
                font: queryField.font
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                radius: 10
                color: root.appTheme.surface
                border.color: queryField.activeFocus ? root.appTheme.focus : root.appTheme.border
                border.width: queryField.activeFocus ? 2 : 1
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 7
            SettingsChoice {
                appTheme: root.appTheme
                text: "همهٔ منابع"
                selected: root.scope === "all"
                onClicked: { root.scope = "all"; searchDelay.restart() }
            }
            SettingsChoice {
                visible: root.availablePoetUrl !== ""
                appTheme: root.appTheme
                text: "این شاعر"
                selected: root.scope === "poet"
                onClicked: { root.scope = "poet"; searchDelay.restart() }
            }
            SettingsChoice {
                visible: root.availableCategoryUrl !== ""
                appTheme: root.appTheme
                text: "این مجموعه"
                selected: root.scope === "collection"
                onClicked: { root.scope = "collection"; searchDelay.restart() }
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 7
            Repeater {
                model: [
                    { "label": "همه", "value": "" },
                    { "label": "شاعران", "value": "poet" },
                    { "label": "مجموعه‌ها", "value": "collection" },
                    { "label": "شعرها", "value": "poem" }
                ]
                delegate: SettingsChoice {
                    required property var modelData
                    appTheme: root.appTheme
                    text: modelData.label
                    selected: root.resultType === modelData.value
                    onClicked: { root.resultType = modelData.value; searchDelay.restart() }
                }
            }
        }
        Label {
            Layout.fillWidth: true
            LayoutMirroring.enabled: false
            horizontalAlignment: Text.AlignRight
            color: root.appTheme.muted
            font.family: root.typography.family
            text: root.searchRepository.loading && root.searchRepository.count === 0
                ? "در حال جستجو…"
                : root.searchRepository.error !== "" ? root.searchRepository.error
                : queryField.text.trim() === "" ? "برای جستجو، واژه‌ای بنویسید."
                : root.searchRepository.totalCount + " نتیجه"
        }
        ListView {
            id: results
            objectName: "searchResults"
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.searchRepository
            clip: true
            spacing: 6
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: AppScrollBar {
                visible: results.contentHeight > results.height + 1
                trackColor: root.appTheme.surfaceRaised
                thumbColor: root.appTheme.muted
                activeThumbColor: root.appTheme.accent
            }
            delegate: ItemDelegate {
                id: row
                required property string entryType
                required property string fullUrl
                required property string title
                required property string context
                required property string snippet
                width: results.width - 22
                x: 18
                implicitHeight: textColumn.implicitHeight + 20
                hoverEnabled: true
                Accessible.name: title + "، " + context
                onClicked: root.activated(entryType === "verse" ? "poem" : entryType, fullUrl)
                background: Rectangle {
                    radius: 10
                    color: row.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                    border.color: row.activeFocus ? root.appTheme.focus : root.appTheme.border
                }
                contentItem: Column {
                    id: textColumn
                    spacing: 3
                    Text {
                        width: parent.width
                        text: row.title
                        textFormat: Text.PlainText
                        color: root.appTheme.foreground
                        font.family: root.typography.family
                        font.pixelSize: 17
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                    Text {
                        width: parent.width
                        text: row.context
                        textFormat: Text.PlainText
                        color: root.appTheme.muted
                        font.family: root.typography.family
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                    Text {
                        visible: row.entryType === "verse"
                        width: parent.width
                        text: row.snippet
                        textFormat: Text.PlainText
                        color: root.appTheme.foreground
                        font.family: root.typography.family
                        font.pixelSize: 15
                        horizontalAlignment: Text.AlignRight
                        elide: Text.ElideRight
                    }
                }
            }
            footer: Button {
                visible: root.searchRepository.hasMore && !root.searchRepository.loading
                width: results.width
                text: "نمایش نتیجه‌های بیشتر"
                font.family: root.typography.family
                onClicked: root.searchRepository.loadMore()
            }
        }
        Label {
            visible: queryField.text.trim() !== "" && !root.searchRepository.loading
                && root.searchRepository.totalCount === 0 && root.searchRepository.error === ""
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: "نتیجه‌ای پیدا نشد."
            color: root.appTheme.muted
            font.family: root.typography.family
        }
    }
}
