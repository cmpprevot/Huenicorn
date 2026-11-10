import QtQuick

// Small chevron drawn with two strokes (no font glyph dependency).
// Down chevron "v": left stroke "\" + right stroke "/" meeting at bottom-center.
Item {
    id: root

    property bool up: false
    property color color: Theme.textDim
    property real stroke: 7   // length of each stroke
    property real thickness: 2

    implicitWidth: 12
    implicitHeight: 8

    Rectangle {
        width: root.stroke
        height: root.thickness
        radius: root.thickness / 2
        color: root.color
        x: 0
        y: (parent.height - height) / 2
        rotation: root.up ? -45 : 45   // down: "\" stroke, up: "/" stroke
    }
    Rectangle {
        width: root.stroke
        height: root.thickness
        radius: root.thickness / 2
        color: root.color
        x: parent.width - width
        y: (parent.height - height) / 2
        rotation: root.up ? 45 : -45   // down: "/" stroke, up: "\" stroke
    }
}
