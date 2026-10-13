import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control
    property color trackColor: "#222632"
    property color thumbColor: "#aeb5c3"
    property color activeThumbColor: "#9b8cff"

    policy: ScrollBar.AsNeeded
    implicitWidth: UiMetrics.px(12)
    padding: UiMetrics.px(3)
    minimumSize: 0.08

    contentItem: Rectangle {
        implicitWidth: UiMetrics.px(6)
        radius: UiMetrics.px(3)
        color: control.pressed || control.hovered ? control.activeThumbColor : control.thumbColor
    }
    background: Rectangle {
        radius: UiMetrics.px(6)
        color: control.trackColor
    }
}
