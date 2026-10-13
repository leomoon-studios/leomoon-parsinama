pragma Singleton

import QtQuick

QtObject {
    property real scale: 1.0

    function px(value) {
        return Math.round(value * scale)
    }
}
