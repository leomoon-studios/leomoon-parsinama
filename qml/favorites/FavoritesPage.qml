pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Item {
    id: root
    required property var bookmarkStore
    required property var appTheme
    required property var typography
    signal activated(string entryType, string fullUrl)
    signal removeRequested(string entryType, string fullUrl)

    Label {
        objectName: "favoritesEmptyState"
        visible: root.bookmarkStore.count === 0
        anchors.centerIn: parent
        width: Math.max(0, parent.width - 48)
        text: "هنوز نشانکی ذخیره نشده است.\nبرای ذخیره، شعر یا مجموعه‌ای را باز کنید و نشانک را بزنید."
        color: root.appTheme.muted
        font.family: root.typography.family
        font.pixelSize: 17
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }

    ListView {
        id: list
        objectName: "favoritesList"
        visible: root.bookmarkStore.count > 0
        anchors.fill: parent
        model: root.bookmarkStore
        clip: true
        spacing: 8
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: AppScrollBar {
            id: scrollbar
            visible: list.contentHeight > list.height + 1
            trackColor: root.appTheme.surfaceRaised
            thumbColor: root.appTheme.muted
            activeThumbColor: root.appTheme.accent
        }
        delegate: Item {
            id: entry
            required property string entryType
            required property string fullUrl
            required property string title
            required property string context
            required property bool available
            width: list.width
            height: 76
            LayoutMirroring.enabled: false

            ItemDelegate {
                id: row
                objectName: "favoriteRow"
                x: scrollbar.width + 8
                width: entry.width - x - 4
                height: entry.height
                hoverEnabled: true
                focusPolicy: Qt.StrongFocus
                Accessible.name: entry.title + (entry.available ? "" : "، در داده‌ها یافت نشد")
                onClicked: {
                    if (entry.available) root.activated(entry.entryType, entry.fullUrl)
                }
                background: Rectangle {
                    radius: 10
                    color: row.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                    border.color: row.activeFocus ? root.appTheme.focus : root.appTheme.border
                    border.width: row.activeFocus ? 2 : 1
                }
                contentItem: Item {
                    LayoutMirroring.enabled: false
                    LayoutMirroring.childrenInherit: true
                    Button {
                        id: removeButton
                        objectName: "removeFavoriteButton"
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        width: 42
                        height: 42
                        enabled: true
                        Accessible.name: "حذف نشانک " + entry.title
                        ToolTip.text: "حذف نشانک"
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        onClicked: root.removeRequested(entry.entryType, entry.fullUrl)
                        contentItem: Text {
                            text: "\ue872"
                            font.family: root.typography.iconFamily
                            font.pixelSize: 21
                            color: root.appTheme.muted
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: 8
                            color: removeButton.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                        }
                    }
                    Column {
                        anchors.left: removeButton.right
                        anchors.leftMargin: 8
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text {
                            objectName: "favoriteTitle"
                            width: parent.width
                            text: entry.title
                            textFormat: Text.PlainText
                            color: entry.available ? root.appTheme.foreground : root.appTheme.muted
                            font.family: root.typography.family
                            font.pixelSize: 17
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                        Text {
                            objectName: "favoriteContext"
                            width: parent.width
                            text: entry.available ? entry.context : "دیگر در داده‌ها یافت نمی‌شود"
                            textFormat: Text.PlainText
                            color: root.appTheme.muted
                            font.family: root.typography.family
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
