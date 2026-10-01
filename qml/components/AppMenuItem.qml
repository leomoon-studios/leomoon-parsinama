import QtQuick
import QtQuick.Controls

MenuItem {
    id: control
    required property var appTheme

    implicitWidth: 218
    implicitHeight: 42
    leftPadding: 14
    rightPadding: 14
    hoverEnabled: true
    Accessible.name: text

    contentItem: Text {
        objectName: "menuItemText"
        LayoutMirroring.enabled: false
        text: control.text
        font: control.font
        color: control.enabled ? control.appTheme.foreground : control.appTheme.muted
        opacity: control.enabled ? 1 : 0.55
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 7
        color: control.highlighted && control.enabled
            ? control.appTheme.surfaceRaised : "transparent"
        border.color: control.activeFocus ? control.appTheme.focus : "transparent"
        border.width: control.activeFocus ? 2 : 0
    }
}
