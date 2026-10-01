import QtQuick

QtObject {
    id: root
    readonly property bool ready: fontLoader.status === FontLoader.Ready
    readonly property bool failed: fontLoader.status === FontLoader.Error
    readonly property string family: ready ? fontLoader.name : "sans-serif"
    readonly property bool iconReady: iconFontLoader.status === FontLoader.Ready
    readonly property string iconFamily: iconReady ? iconFontLoader.name : "sans-serif"
    readonly property FontLoader fontLoader: FontLoader {
        id: fontLoader
        source: "qrc:/qt/qml/LeoMoon/ParsiNama/assets/fonts/Vazirmatn%5Bwght%5D.ttf"
    }
    readonly property FontLoader iconFontLoader: FontLoader {
        id: iconFontLoader
        source: "qrc:/qt/qml/LeoMoon/ParsiNama/assets/fonts/MaterialSymbolsRounded.ttf"
    }
}
