import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    objectName: "mainWindow"
    width: 960
    height: 720
    minimumWidth: 520
    minimumHeight: 400
    visible: true
    title: "LeoMoon ParsiNama"

    LayoutMirroring.enabled: true
    LayoutMirroring.childrenInherit: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 32

        Label {
            objectName: "welcomeLabel"
            Layout.fillWidth: true
            text: "به لئومون پارسی‌نما خوش آمدید"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
