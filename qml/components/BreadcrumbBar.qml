pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    required property var navigationController
    required property var appTheme
    required property var typography
    readonly property real fullCrumbWidth: widthProbe.implicitWidth + 12
    readonly property bool collapsed: navigationController.breadcrumbs.length > 3
        && fullCrumbWidth > width
    readonly property var hiddenEntries: navigationController.breadcrumbs.slice(1, -2)
    readonly property bool overflowVisible: hiddenCrumbs.visible
    signal activated(int index)

    function openOverflow() {
        if (collapsed)
            hiddenCrumbs.open()
    }

    implicitHeight: 38
    clip: true

    Row {
        id: widthProbe
        opacity: 0
        enabled: false
        height: 0
        spacing: 5
        Repeater {
            model: root.navigationController.breadcrumbs
            delegate: Text {
                required property var modelData
                text: modelData && modelData.title ? modelData.title : ""
                font.family: root.typography.family
                font.pixelSize: 15
                width: Math.min(220, implicitWidth + 24)
            }
        }
    }

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
                    text: parent.modelData && parent.modelData.title ? parent.modelData.title : ""
                    font.family: root.typography.family
                    font.pixelSize: 15
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
                model: root.hiddenEntries
                delegate: Button {
                    id: hiddenButton
                    required property int index
                    required property var modelData
                    width: hiddenCrumbs.availableWidth
                    implicitHeight: 36
                    text: modelData && modelData.title ? modelData.title : ""
                    font.family: root.typography.family
                    onClicked: {
                        if (!hiddenButton.modelData)
                            return
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
