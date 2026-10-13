import QtQuick
import QtQuick.Controls

MenuItem {
    id: control
    required property var appTheme

    implicitWidth: UiMetrics.px(218)
    implicitHeight: UiMetrics.px(42)
    leftPadding: UiMetrics.px(14)
    rightPadding: UiMetrics.px(14)
    font.pixelSize: UiMetrics.px(15)
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
        radius: UiMetrics.px(7)
        color: control.highlighted && control.enabled
            ? control.appTheme.surfaceRaised : "transparent"
        border.color: control.activeFocus ? control.appTheme.focus : "transparent"
        border.width: control.activeFocus ? 2 : 0
    }
}
