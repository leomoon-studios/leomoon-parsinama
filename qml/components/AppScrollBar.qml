import QtQuick
import QtQuick.Controls

ScrollBar {
    id: control
    property color trackColor: "#222632"
    property color thumbColor: "#aeb5c3"
    property color activeThumbColor: "#9b8cff"

    policy: ScrollBar.AsNeeded
    implicitWidth: 12
    padding: 3
    minimumSize: 0.08

    contentItem: Rectangle {
        implicitWidth: 6
        radius: 3
        color: control.pressed || control.hovered ? control.activeThumbColor : control.thumbColor
    }
    background: Rectangle {
        radius: 6
        color: control.trackColor
    }
}
