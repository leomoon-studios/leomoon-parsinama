pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    id: root
    required property var settingsStore
    required property var appTheme
    required property var typography
    readonly property var fontSizes: [16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40]

    clip: true
    contentWidth: availableWidth
    leftPadding: 20
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
    ScrollBar.vertical: AppScrollBar {
        objectName: "settingsScrollBar"
        policy: ScrollBar.AsNeeded
        height: root.availableHeight
        visible: root.contentHeight > root.availableHeight + 1
        trackColor: root.appTheme.surfaceRaised
        thumbColor: root.appTheme.muted
        activeThumbColor: root.appTheme.accent
    }

    ColumnLayout {
        width: root.availableWidth
        spacing: 14

        Label {
            text: "ظاهر"
            color: root.appTheme.foreground
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            SettingsChoice {
                objectName: "lightThemeChoice"
                Layout.fillWidth: true
                appTheme: root.appTheme
                text: "روشن"
                selected: root.settingsStore.theme === "light"
                onClicked: root.settingsStore.setTheme("light")
            }
            SettingsChoice {
                objectName: "darkThemeChoice"
                Layout.fillWidth: true
                appTheme: root.appTheme
                text: "تیره"
                selected: root.settingsStore.theme === "dark"
                onClicked: root.settingsStore.setTheme("dark")
            }
        }

        Label {
            text: "رنگ تأکید"
            color: root.appTheme.foreground
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }
        GridLayout {
            Layout.fillWidth: true
            columns: width >= 600 ? 3 : 2
            columnSpacing: 8
            rowSpacing: 8
            SettingsChoice {
                objectName: "accentChoice_purple"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "بنفش"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("purple")
                selected: root.settingsStore.accentPreset === "purple"
                onClicked: root.settingsStore.setAccentPreset("purple")
            }
            SettingsChoice {
                objectName: "accentChoice_slate"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "خاکستری"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("slate")
                selected: root.settingsStore.accentPreset === "slate"
                onClicked: root.settingsStore.setAccentPreset("slate")
            }
            SettingsChoice {
                objectName: "accentChoice_faint"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "ملایم"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("faint")
                selected: root.settingsStore.accentPreset === "faint"
                onClicked: root.settingsStore.setAccentPreset("faint")
            }
            SettingsChoice {
                objectName: "accentChoice_blue"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "آبی"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("blue")
                selected: root.settingsStore.accentPreset === "blue"
                onClicked: root.settingsStore.setAccentPreset("blue")
            }
            SettingsChoice {
                objectName: "accentChoice_teal"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "سبزآبی"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("teal")
                selected: root.settingsStore.accentPreset === "teal"
                onClicked: root.settingsStore.setAccentPreset("teal")
            }
            SettingsChoice {
                objectName: "accentChoice_rose"
                Layout.fillWidth: true
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "سرخ"
                hasSwatch: true
                swatchColor: root.appTheme.accentFor("rose")
                selected: root.settingsStore.accentPreset === "rose"
                onClicked: root.settingsStore.setAccentPreset("rose")
            }
        }

        Label {
            text: "اندازهٔ متن شعر"
            color: root.appTheme.foreground
            font.pixelSize: 18
            font.weight: Font.DemiBold
        }
        ComboBox {
            id: sizeSelector
            objectName: "readingSizeSelector"
            Layout.preferredWidth: 180
            implicitHeight: 44
            model: root.fontSizes
            currentIndex: root.fontSizes.indexOf(root.settingsStore.readingSize)
            onActivated: (index) => root.settingsStore.setReadingSize(root.fontSizes[index])
            font.family: root.typography.family
            font.pixelSize: 16
            Accessible.name: "اندازهٔ متن شعر"

            contentItem: Text {
                objectName: "readingSizeText"
                LayoutMirroring.enabled: false
                text: sizeSelector.currentText + " پیکسل"
                font: sizeSelector.font
                color: root.appTheme.foreground
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                rightPadding: 14
                leftPadding: sizeSelector.indicator.width + 14
            }
            indicator: Text {
                x: 12
                anchors.verticalCenter: parent.verticalCenter
                text: "▾"
                color: root.appTheme.foreground
                font.pixelSize: 18
            }
            background: Rectangle {
                radius: 10
                color: root.appTheme.surface
                border.color: sizeSelector.activeFocus ? root.appTheme.focus : root.appTheme.border
                border.width: sizeSelector.activeFocus ? 2 : 1
            }
            delegate: ItemDelegate {
                id: sizeOption
                required property int index
                width: sizeSelector.width - sizeSelector.popup.leftPadding - sizeSelector.popup.rightPadding
                height: 40
                highlighted: sizeSelector.highlightedIndex === index
                contentItem: Text {
                    objectName: "readingSizeOptionText"
                    LayoutMirroring.enabled: false
                    text: root.fontSizes[sizeOption.index] + " پیکسل"
                    font: sizeSelector.font
                    color: sizeOption.highlighted ? root.appTheme.accentText : root.appTheme.foreground
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: 12
                }
                background: Rectangle {
                    radius: 7
                    color: sizeOption.highlighted ? root.appTheme.accent : root.appTheme.surface
                }
            }
            popup: Popup {
                id: sizePopup
                objectName: "readingSizePopup"
                y: sizeSelector.height
                width: sizeSelector.width
                implicitHeight: Math.min(sizeList.contentHeight + topPadding + bottomPadding, 280)
                padding: 4
                contentItem: ListView {
                    id: sizeList
                    objectName: "readingSizeList"
                    clip: true
                    implicitHeight: contentHeight
                    model: sizeSelector.popup.visible ? sizeSelector.delegateModel : null
                    currentIndex: sizeSelector.highlightedIndex
                    ScrollBar.vertical: AppScrollBar {
                        trackColor: root.appTheme.surfaceRaised
                        thumbColor: root.appTheme.muted
                        activeThumbColor: root.appTheme.accent
                    }
                }
                background: Rectangle {
                    objectName: "readingSizePopupBackground"
                    radius: 10
                    color: root.appTheme.surface
                    border.color: root.appTheme.border
                }
            }
        }
        Label {
            objectName: "readingSizePreview"
            text: "بنی‌آدم اعضای یکدیگرند"
            LayoutMirroring.enabled: false
            font.pixelSize: root.settingsStore.readingSize
            color: root.appTheme.foreground
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }
    }
}
