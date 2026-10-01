pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ListView {
    id: reader
    objectName: "poemList"
    required property var poemLoader
    required property var navigationController
    required property var settingsStore
    required property var appTheme
    required property var typography

    model: poemLoader.readingRows
    clip: true
    spacing: 8
    boundsBehavior: Flickable.StopAtBounds

    header: ColumnLayout {
        width: reader.width - 20
        x: 20
        spacing: 12
        Label {
            LayoutMirroring.enabled: false
            Layout.fillWidth: true
            text: "شعر " + reader.navigationController.poemPosition
                + " از " + reader.navigationController.poemCount
            color: reader.appTheme.muted
            font.family: reader.typography.family
            font.pixelSize: 14
            horizontalAlignment: Text.AlignRight
        }
        Label {
            visible: reader.poemLoader.metre !== ""
            LayoutMirroring.enabled: false
            Layout.fillWidth: true
            text: "وزن: " + reader.poemLoader.metre
            color: reader.appTheme.muted
            font.family: reader.typography.family
            font.pixelSize: 14
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.Wrap
        }
        Label {
            objectName: "poemSummary"
            visible: reader.poemLoader.summary !== ""
            LayoutMirroring.enabled: false
            Layout.fillWidth: true
            Layout.bottomMargin: 12
            text: reader.poemLoader.summary
            textFormat: Text.PlainText
            color: reader.appTheme.muted
            font.family: reader.typography.family
            font.pixelSize: 15
            horizontalAlignment: Text.AlignJustify
            wrapMode: Text.Wrap
        }
    }

    ScrollBar.vertical: AppScrollBar {
        objectName: "poemScrollBar"
        visible: reader.contentHeight > reader.height + 1
        trackColor: reader.appTheme.surfaceRaised
        thumbColor: reader.appTheme.muted
        activeThumbColor: reader.appTheme.accent
    }

    delegate: Item {
        id: verseEntry
        required property string kind
        required property bool paired
        required property string rightText
        required property string leftText
        required property string text
        required property string position
        required property string note
        property bool noteExpanded: false
        readonly property bool stacked: reader.width < 700
        width: reader.width
        height: readingContent.height + (kind === "section" ? 24 : 14)

        Item {
            id: readingContent
            LayoutMirroring.enabled: false
            width: Math.min(Math.max(0, verseEntry.width - 48), 1000)
            anchors.horizontalCenter: parent.horizontalCenter
            height: verseEntry.kind === "section" ? sectionLabel.implicitHeight
                : verseEntry.paired
                    ? (verseEntry.stacked
                        ? rightVerse.implicitHeight + leftVerse.implicitHeight + 10
                        : Math.max(rightVerse.implicitHeight, leftVerse.implicitHeight))
                        + (noteBlock.visible ? noteBlock.height : 0)
                    : singleVerse.implicitHeight + (noteBlock.visible ? noteBlock.height : 0)

            Text {
                id: sectionLabel
                visible: verseEntry.kind === "section"
                width: parent.width
                text: verseEntry.text
                textFormat: Text.PlainText
                color: reader.appTheme.muted
                font.family: reader.typography.family
                font.pixelSize: Math.max(17, reader.settingsStore.readingSize - 2)
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                id: rightVerse
                visible: verseEntry.kind === "verse" && verseEntry.paired
                width: verseEntry.stacked ? readingContent.width : (readingContent.width - 28) / 2
                x: verseEntry.stacked ? 0 : readingContent.width - width
                text: verseEntry.rightText
                textFormat: Text.PlainText
                color: reader.appTheme.foreground
                font.family: reader.typography.family
                font.pixelSize: reader.settingsStore.readingSize
                horizontalAlignment: verseEntry.stacked ? Text.AlignHCenter
                    : lineCount > 1 ? Text.AlignJustify : Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Text {
                id: leftVerse
                visible: verseEntry.kind === "verse" && verseEntry.paired
                width: verseEntry.stacked ? readingContent.width : (readingContent.width - 28) / 2
                y: verseEntry.stacked ? rightVerse.implicitHeight + 10 : 0
                text: verseEntry.leftText
                textFormat: Text.PlainText
                color: reader.appTheme.foreground
                font.family: reader.typography.family
                font.pixelSize: reader.settingsStore.readingSize
                horizontalAlignment: verseEntry.stacked ? Text.AlignHCenter
                    : lineCount > 1 ? Text.AlignJustify : Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Text {
                id: singleVerse
                visible: verseEntry.kind === "verse" && !verseEntry.paired
                width: readingContent.width
                text: verseEntry.text
                textFormat: Text.PlainText
                color: reader.appTheme.foreground
                font.family: reader.typography.family
                font.pixelSize: reader.settingsStore.readingSize
                horizontalAlignment: verseEntry.position === "Paragraph"
                    || verseEntry.position === "Comment"
                    ? Text.AlignJustify : Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            Column {
                id: noteBlock
                visible: verseEntry.kind === "verse" && verseEntry.note !== ""
                width: parent.width
                y: verseEntry.paired
                    ? (verseEntry.stacked
                        ? rightVerse.implicitHeight + leftVerse.implicitHeight + 10
                        : Math.max(rightVerse.implicitHeight, leftVerse.implicitHeight))
                    : singleVerse.implicitHeight
                spacing: 4
                Button {
                    text: verseEntry.noteExpanded ? "بستن معنی" : "معنی بیت"
                    font.family: reader.typography.family
                    font.pixelSize: 13
                    onClicked: verseEntry.noteExpanded = !verseEntry.noteExpanded
                    background: Item {}
                    contentItem: Text {
                        text: parent.text
                        color: reader.appTheme.accent
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                    }
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    visible: verseEntry.noteExpanded
                    width: parent.width
                    text: verseEntry.note
                    textFormat: Text.PlainText
                    color: reader.appTheme.muted
                    font.family: reader.typography.family
                    font.pixelSize: Math.max(14, reader.settingsStore.readingSize - 4)
                    horizontalAlignment: Text.AlignJustify
                    wrapMode: Text.Wrap
                }
            }
        }
    }
}
