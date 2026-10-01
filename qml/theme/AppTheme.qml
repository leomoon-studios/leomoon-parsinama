import QtQuick

QtObject {
    property bool darkMode: true
    property string accentPreset: "purple"
    readonly property var accentPalettes: ({
        purple: { dark: { accent: "#9b8cff", text: "#151024", focus: "#b9aeff" }, light: { accent: "#5941d8", text: "#ffffff", focus: "#4932c4" } },
        slate: { dark: { accent: "#929bad", text: "#151922", focus: "#b7c0cf" }, light: { accent: "#475569", text: "#ffffff", focus: "#334155" } },
        faint: { dark: { accent: "#4e535f", text: "#f3f4f8", focus: "#777e8c" }, light: { accent: "#c7ccd5", text: "#191b22", focus: "#8d95a3" } },
        blue: { dark: { accent: "#78b8ff", text: "#101c2b", focus: "#a7d0ff" }, light: { accent: "#2563b4", text: "#ffffff", focus: "#1d5096" } },
        teal: { dark: { accent: "#65cbbb", text: "#10221f", focus: "#9ce1d5" }, light: { accent: "#087b6d", text: "#ffffff", focus: "#066457" } },
        rose: { dark: { accent: "#f19ab5", text: "#2a1420", focus: "#f8bfd1" }, light: { accent: "#a23661", text: "#ffffff", focus: "#862b50" } }
    })
    readonly property var activeAccent: accentPalettes[accentPreset][darkMode ? "dark" : "light"]
    readonly property color background: darkMode ? "#11131a" : "#f5f6fa"
    readonly property color surface: darkMode ? "#191c25" : "#ffffff"
    readonly property color surfaceRaised: darkMode ? "#222632" : "#eceef5"
    readonly property color foreground: darkMode ? "#f3f4f8" : "#191b22"
    readonly property color muted: darkMode ? "#aeb5c3" : "#596170"
    readonly property color border: darkMode ? "#343946" : "#d5d9e3"
    readonly property color accent: activeAccent.accent
    readonly property color accentText: activeAccent.text
    readonly property color focus: activeAccent.focus

    function accentFor(preset) {
        return accentPalettes[preset][darkMode ? "dark" : "light"].accent
    }
}
