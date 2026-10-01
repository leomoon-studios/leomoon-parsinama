import QtQuick

QtObject {
    property bool darkMode: true
    readonly property color background: darkMode ? "#11131a" : "#f5f6fa"
    readonly property color surface: darkMode ? "#191c25" : "#ffffff"
    readonly property color surfaceRaised: darkMode ? "#222632" : "#eceef5"
    readonly property color foreground: darkMode ? "#f3f4f8" : "#191b22"
    readonly property color muted: darkMode ? "#aeb5c3" : "#596170"
    readonly property color border: darkMode ? "#343946" : "#d5d9e3"
    readonly property color accent: darkMode ? "#9b8cff" : "#5941d8"
    readonly property color accentText: darkMode ? "#151024" : "#ffffff"
    readonly property color focus: darkMode ? "#b9aeff" : "#4932c4"
}
