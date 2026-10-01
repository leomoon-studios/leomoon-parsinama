pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var navigationController
    required property var appTheme
    required property var typography
    readonly property bool collapsed: width < 650 && navigationController.breadcrumbs.length > 3
    readonly property bool overflowVisible: hiddenCrumbs.visible
    signal activated(int index)

    function openOverflow() {
        if (collapsed)
            hiddenCrumbs.open()
    }

    implicitHeight: 38
    clip: true

    RowLayout {
        anchors.fill: parent
        spacing: 5
        Repeater {
            model: root.navigationController.breadcrumbs
            delegate: RowLayout {
                required property int index
                required property var modelData
                visible: !root.collapsed || index === 0
                    || index >= root.navigationController.breadcrumbs.length - 2
                spacing: 5

                Button {
                    id: crumbButton
                    Layout.maximumWidth: root.collapsed ? Math.max(90, (root.width - 50) / 3) : 220
                    implicitHeight: 34
                    text: parent.modelData.title
                    font.family: root.typography.family
                    onClicked: root.activated(parent.index)
                    contentItem: Text {
                        LayoutMirroring.enabled: false
                        text: crumbButton.text
                        font: crumbButton.font
                        color: root.appTheme.foreground
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    background: Rectangle {
                        radius: 8
                        color: crumbButton.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                        border.color: root.appTheme.border
                    }
                }
                Button {
                    id: ellipsisButton
                    objectName: "breadcrumbOverflowButton"
                    visible: root.collapsed && parent.index === 0
                    implicitWidth: 34
                    implicitHeight: 34
                    text: "…"
                    font.family: root.typography.family
                    onClicked: root.openOverflow()
                    contentItem: Text {
                        text: ellipsisButton.text
                        color: root.appTheme.foreground
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        radius: 8
                        color: ellipsisButton.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                        border.color: root.appTheme.border
                    }
                }
            }
        }
        Item { Layout.fillWidth: true }
    }

    Popup {
        id: hiddenCrumbs
        objectName: "breadcrumbOverflowPopup"
        x: Math.max(0, root.width - width - 40)
        y: root.height + 4
        width: 220
        padding: 5
        modal: true
        dim: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        contentItem: Column {
            Repeater {
                model: Math.max(0, root.navigationController.breadcrumbs.length - 3)
                delegate: Button {
                    id: hiddenButton
                    required property int index
                    readonly property var entry: root.navigationController.breadcrumbs[index + 1]
                    width: hiddenCrumbs.availableWidth
                    implicitHeight: 36
                    text: entry.title
                    font.family: root.typography.family
                    onClicked: {
                        hiddenCrumbs.close()
                        root.activated(index + 1)
                    }
                    contentItem: Text {
                        LayoutMirroring.enabled: false
                        text: hiddenButton.text
                        font: hiddenButton.font
                        color: root.appTheme.foreground
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                    background: Rectangle {
                        radius: 7
                        color: hiddenButton.hovered ? root.appTheme.surfaceRaised : root.appTheme.surface
                    }
                }
            }
        }
        background: Rectangle {
            radius: 10
            color: root.appTheme.surface
            border.color: root.appTheme.border
        }
    }
}
