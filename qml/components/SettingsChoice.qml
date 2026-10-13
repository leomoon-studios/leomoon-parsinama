import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Button {
    id: control
    required property var appTheme
    property bool selected: false
    property bool hasSwatch: false
    property color swatchColor: "transparent"

    implicitHeight: UiMetrics.px(44)
    implicitWidth: UiMetrics.px(130)
    focusPolicy: Qt.StrongFocus
    hoverEnabled: true
    Accessible.name: text

    contentItem: RowLayout {
        spacing: UiMetrics.px(8)
        Rectangle {
            visible: control.hasSwatch
            Layout.preferredWidth: UiMetrics.px(16)
            Layout.preferredHeight: UiMetrics.px(16)
            radius: UiMetrics.px(8)
            color: control.swatchColor
            border.color: control.appTheme.border
        }
        Text {
            Layout.fillWidth: true
            text: control.text
            font.family: control.font.family
            font.pixelSize: UiMetrics.px(15)
            color: control.appTheme.foreground
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
    background: Rectangle {
        radius: UiMetrics.px(10)
        color: control.selected || control.down
            ? control.appTheme.surfaceRaised : control.appTheme.surface
        border.color: control.activeFocus ? control.appTheme.focus
            : control.selected ? control.appTheme.accent : control.appTheme.border
        border.width: control.activeFocus ? 2 : 1
    }
}
