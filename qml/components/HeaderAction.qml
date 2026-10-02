import QtQuick
import QtQuick.Controls

Button {
    id: control
    property string symbol: ""
    property string hint: ""
    property color surfaceColor: "#191c25"
    property color textColor: "#f3f4f8"
    property color borderColor: "#343946"
    property color focusColor: "#b9aeff"

    implicitWidth: 44
    implicitHeight: 44
    padding: 0
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus
    Accessible.name: hint
    ToolTip.text: hint
    ToolTip.visible: hovered && hint !== ""
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
        border.color: control.activeFocus ? control.focusColor : control.borderColor
        border.width: control.activeFocus ? 2 : 1
    }
}
