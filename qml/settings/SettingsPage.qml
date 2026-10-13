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
    readonly property var uiScaleOptions: ["1.0×", "1.1×", "1.2×", "1.3×", "1.4×", "1.5×", "1.6×", "1.7×", "1.8×", "1.9×", "2.0×", "2.1×", "2.2×", "2.3×", "2.4×", "2.5×", "2.6×", "2.7×", "2.8×", "2.9×", "3.0×"]
    readonly property bool readingSizePopupVisible: sizePopup.visible

    clip: true
    contentWidth: availableWidth
    leftPadding: UiMetrics.px(20)
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
        spacing: UiMetrics.px(14)

        Label {
            text: "ظاهر"
            color: root.appTheme.foreground
            font.pixelSize: UiMetrics.px(18)
            font.weight: Font.DemiBold
        }
        GridLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: root.availableWidth
            columns: root.availableWidth >= UiMetrics.px(300) ? 2 : 1
            columnSpacing: UiMetrics.px(8)
            rowSpacing: UiMetrics.px(8)
            SettingsChoice {
                objectName: "lightThemeChoice"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "روشن"
                selected: root.settingsStore.theme === "light"
                onClicked: root.settingsStore.setTheme("light")
            }
            SettingsChoice {
                objectName: "darkThemeChoice"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "تیره"
                selected: root.settingsStore.theme === "dark"
                onClicked: root.settingsStore.setTheme("dark")
            }
        }

        Label {
            text: "رنگ تأکید"
            color: root.appTheme.foreground
            font.pixelSize: UiMetrics.px(18)
            font.weight: Font.DemiBold
        }
        GridLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: root.availableWidth
            columns: width >= UiMetrics.px(600) ? 3 : width >= UiMetrics.px(320) ? 2 : 1
            columnSpacing: UiMetrics.px(8)
            rowSpacing: UiMetrics.px(8)
            SettingsChoice {
                objectName: "accentChoice_purple"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
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
                Layout.minimumWidth: 0
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
                Layout.minimumWidth: 0
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
                Layout.minimumWidth: 0
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
                Layout.minimumWidth: 0
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
                Layout.minimumWidth: 0
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
            text: "مقیاس برنامه"
            color: root.appTheme.foreground
            font.pixelSize: UiMetrics.px(18)
            font.weight: Font.DemiBold
        }
        ComboBox {
            id: uiScaleSelector
            objectName: "uiScaleSelector"
            Layout.preferredWidth: Math.min(UiMetrics.px(180), root.availableWidth)
            implicitHeight: UiMetrics.px(44)
            model: root.uiScaleOptions
            currentIndex: Math.round(root.settingsStore.uiScale * 10) - 10
            onActivated: (index) => {
                uiScaleSelector.popup.close()
                root.settingsStore.setUiScale((index + 10) / 10)
            }
            font.family: root.typography.family
            font.pixelSize: UiMetrics.px(16)
            Accessible.name: "مقیاس برنامه"

            contentItem: Text {
                objectName: "uiScaleText"
                LayoutMirroring.enabled: false
                text: uiScaleSelector.currentText
                font: uiScaleSelector.font
                color: root.appTheme.foreground
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                rightPadding: UiMetrics.px(14)
                leftPadding: uiScaleSelector.indicator.width + UiMetrics.px(14)
            }
            indicator: Text {
                x: UiMetrics.px(12)
                anchors.verticalCenter: parent.verticalCenter
                text: "\ue5c5"
                color: root.appTheme.foreground
                font.family: root.typography.iconFamily
                font.pixelSize: UiMetrics.px(24)
            }
            background: Rectangle {
                radius: UiMetrics.px(10)
                color: root.appTheme.surface
                border.color: uiScaleSelector.activeFocus ? root.appTheme.focus : root.appTheme.border
                border.width: uiScaleSelector.activeFocus ? 2 : 1
            }
            delegate: ItemDelegate {
                id: uiScaleOption
                required property int index
                width: uiScaleSelector.width - uiScaleSelector.popup.leftPadding - uiScaleSelector.popup.rightPadding
                height: UiMetrics.px(40)
                highlighted: uiScaleSelector.highlightedIndex === index
                contentItem: Text {
                    LayoutMirroring.enabled: false
                    text: root.uiScaleOptions[uiScaleOption.index]
                    font: uiScaleSelector.font
                    color: uiScaleOption.highlighted ? root.appTheme.accentText : root.appTheme.foreground
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: UiMetrics.px(12)
                }
                background: Rectangle {
                    radius: UiMetrics.px(7)
                    color: uiScaleOption.highlighted ? root.appTheme.accent : root.appTheme.surface
                }
            }
            popup: Popup {
                id: uiScalePopup
                objectName: "uiScalePopup"
                y: uiScaleSelector.height
                width: uiScaleSelector.width
                implicitHeight: Math.min(uiScaleList.contentHeight + topPadding + bottomPadding,
                    UiMetrics.px(280), Math.max(UiMetrics.px(80), root.availableHeight - UiMetrics.px(16)))
                padding: UiMetrics.px(4)
                contentItem: ListView {
                    id: uiScaleList
                    objectName: "uiScaleList"
                    clip: true
                    implicitHeight: contentHeight
                    model: uiScaleSelector.popup.visible ? uiScaleSelector.delegateModel : null
                    currentIndex: uiScaleSelector.highlightedIndex
                    ScrollBar.vertical: AppScrollBar {
                        trackColor: root.appTheme.surfaceRaised
                        thumbColor: root.appTheme.muted
                        activeThumbColor: root.appTheme.accent
                    }
                }
                background: Rectangle {
                    radius: UiMetrics.px(10)
                    color: root.appTheme.surface
                    border.color: root.appTheme.border
                }
            }
        }

        Label {
            text: "قلم شعر"
            color: root.appTheme.foreground
            font.pixelSize: UiMetrics.px(18)
            font.weight: Font.DemiBold
        }
        GridLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            Layout.maximumWidth: root.availableWidth
            columns: root.availableWidth >= UiMetrics.px(300) ? 2 : 1
            columnSpacing: UiMetrics.px(8)
            rowSpacing: UiMetrics.px(8)
            SettingsChoice {
                objectName: "poemFontChoiceVazirmatn"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "وزیرمتن"
                selected: root.settingsStore.poemFont === "vazirmatn"
                onClicked: root.settingsStore.setPoemFont("vazirmatn")
            }
            SettingsChoice {
                objectName: "poemFontChoiceIranNastaliq"
                Layout.fillWidth: true
                Layout.minimumWidth: 0
                Layout.preferredWidth: 0
                appTheme: root.appTheme
                text: "ایران‌نستعلیق"
                selected: root.settingsStore.poemFont === "irannastaliq"
                onClicked: root.settingsStore.setPoemFont("irannastaliq")
            }
        }

        Label {
            text: "اندازهٔ متن شعر"
            color: root.appTheme.foreground
            font.pixelSize: UiMetrics.px(18)
            font.weight: Font.DemiBold
        }
        ComboBox {
            id: sizeSelector
            objectName: "readingSizeSelector"
            Layout.preferredWidth: Math.min(UiMetrics.px(180), root.availableWidth)
            implicitHeight: UiMetrics.px(44)
            model: root.fontSizes
            currentIndex: root.fontSizes.indexOf(root.settingsStore.readingSize)
            onActivated: (index) => root.settingsStore.setReadingSize(root.fontSizes[index])
            font.family: root.typography.family
            font.pixelSize: UiMetrics.px(16)
            Accessible.name: "اندازهٔ متن شعر"

            contentItem: Text {
                objectName: "readingSizeText"
                LayoutMirroring.enabled: false
                text: sizeSelector.currentText + " پیکسل"
                font: sizeSelector.font
                color: root.appTheme.foreground
                horizontalAlignment: Text.AlignRight
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                rightPadding: UiMetrics.px(14)
                leftPadding: sizeSelector.indicator.width + UiMetrics.px(14)
            }
            indicator: Text {
                objectName: "readingSizeIndicator"
                x: UiMetrics.px(12)
                anchors.verticalCenter: parent.verticalCenter
                text: "\ue5c5"
                color: root.appTheme.foreground
                font.family: root.typography.iconFamily
                font.pixelSize: UiMetrics.px(24)
            }
            background: Rectangle {
                radius: UiMetrics.px(10)
                color: root.appTheme.surface
                border.color: sizeSelector.activeFocus ? root.appTheme.focus : root.appTheme.border
                border.width: sizeSelector.activeFocus ? 2 : 1
            }
            delegate: ItemDelegate {
                id: sizeOption
                required property int index
                width: sizeSelector.width - sizeSelector.popup.leftPadding - sizeSelector.popup.rightPadding
                height: UiMetrics.px(40)
                highlighted: sizeSelector.highlightedIndex === index
                contentItem: Text {
                    objectName: "readingSizeOptionText"
                    LayoutMirroring.enabled: false
                    text: root.fontSizes[sizeOption.index] + " پیکسل"
                    font: sizeSelector.font
                    color: sizeOption.highlighted ? root.appTheme.accentText : root.appTheme.foreground
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    rightPadding: UiMetrics.px(12)
                }
                background: Rectangle {
                    radius: UiMetrics.px(7)
                    color: sizeOption.highlighted ? root.appTheme.accent : root.appTheme.surface
                }
            }
            popup: Popup {
                id: sizePopup
                objectName: "readingSizePopup"
                y: sizeSelector.height
                width: sizeSelector.width
                implicitHeight: Math.min(sizeList.contentHeight + topPadding + bottomPadding, UiMetrics.px(280))
                padding: UiMetrics.px(4)
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
                    radius: UiMetrics.px(10)
                    color: root.appTheme.surface
                    border.color: root.appTheme.border
                }
            }
        }
        Label {
            objectName: "readingSizePreview"
            text: "بنی‌آدم اعضای یکدیگرند"
            LayoutMirroring.enabled: false
            font.family: root.settingsStore.poemFont === "irannastaliq"
                ? root.typography.nastaliqFamily : root.typography.family
            font.pixelSize: UiMetrics.px(root.settingsStore.readingSize)
            color: root.appTheme.foreground
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }
        Button {
            id: hidePoemTranslationsSwitch
            objectName: "hidePoemTranslationsSwitch"
            Layout.fillWidth: true
            Layout.topMargin: UiMetrics.px(12)
            LayoutMirroring.enabled: false
            implicitHeight: UiMetrics.px(48)
            text: "پنهان کردن معنی بیت‌ها"
            checked: root.settingsStore.hidePoemTranslations
            onClicked: root.settingsStore.setHidePoemTranslations(!checked)
            Accessible.name: text
            Accessible.role: Accessible.CheckBox
            Accessible.checked: checked
            font.family: root.typography.family
            font.pixelSize: UiMetrics.px(16)
            padding: 0
            contentItem: Item {
                Text {
                    id: translationLabel
                    objectName: "translationLabel"
                    x: parent.width - width - UiMetrics.px(16)
                    y: (parent.height - height) / 2
                    text: hidePoemTranslationsSwitch.text
                    font: hidePoemTranslationsSwitch.font
                    color: root.appTheme.foreground
                    horizontalAlignment: Text.AlignRight
                }
                Rectangle {
                    id: translationIndicator
                    objectName: "translationIndicator"
                    x: UiMetrics.px(12)
                    y: (parent.height - height) / 2
                    width: UiMetrics.px(42)
                    height: UiMetrics.px(24)
                    radius: UiMetrics.px(12)
                    color: hidePoemTranslationsSwitch.checked
                        ? root.appTheme.accent : root.appTheme.surfaceRaised
                    border.color: hidePoemTranslationsSwitch.checked
                        ? root.appTheme.accent : root.appTheme.border
                    Rectangle {
                        x: hidePoemTranslationsSwitch.checked ? UiMetrics.px(3) : parent.width - width - UiMetrics.px(3)
                        y: UiMetrics.px(3)
                        width: UiMetrics.px(18)
                        height: UiMetrics.px(18)
                        radius: UiMetrics.px(9)
                        color: hidePoemTranslationsSwitch.checked
                            ? root.appTheme.accentText : root.appTheme.muted
                    }
                }
            }
            background: Rectangle {
                radius: UiMetrics.px(10)
                color: hidePoemTranslationsSwitch.down
                    ? root.appTheme.surfaceRaised : root.appTheme.surface
                border.color: hidePoemTranslationsSwitch.activeFocus
                    ? root.appTheme.focus : root.appTheme.border
                border.width: hidePoemTranslationsSwitch.activeFocus ? 2 : 1
            }
        }
        Label {
            objectName: "settingsVersion"
            Layout.fillWidth: true
            LayoutMirroring.enabled: false
            text: "نسخهٔ برنامه: v" + Qt.application.version
            color: root.appTheme.muted
            font.pixelSize: UiMetrics.px(14)
            horizontalAlignment: Text.AlignRight
        }
    }
}
