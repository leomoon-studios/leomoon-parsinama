import QtQuick
import QtQuick.Controls

Button {
    id: control
    property string symbol: ""
    property string hint: ""
    property bool selected: false
    property color surfaceColor: "#191c25"
    property color textColor: "#f3f4f8"
    property color borderColor: "#343946"
    property color focusColor: "#b9aeff"

    implicitWidth: 44
    implicitHeight: 44
    padding: 0
    hoverEnabled: Qt.platform.os !== "android"
    focusPolicy: Qt.platform.os === "android" ? Qt.NoFocus : Qt.StrongFocus
    Accessible.name: hint
    ToolTip.text: hint
    ToolTip.visible: Qt.platform.os !== "android" && hovered && hint !== ""
    ToolTip.delay: 550

    contentItem: Text {
        text: control.symbol
        color: control.enabled ? control.textColor : control.borderColor
        font.pixelSize: 24
        font.family: control.font.family
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    background: Rectangle {
        radius: 11
        color: control.down ? control.borderColor : control.surfaceColor
        border.color: control.activeFocus || control.selected ? control.focusColor : control.borderColor
        border.width: control.activeFocus || control.selected ? 2 : 1
    }
}
