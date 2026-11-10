import QtQuick

// Soft radial glow faked with concentric circles (works on every renderer,
// no shader / GraphicalEffects dependency). Brightest at the center.
Item {
    id: root

    property color glowColor: "#8b5cf6"
    property real intensity: 0.16 // alpha at the very center
    property int layers: 7

    Repeater {
        model: root.layers
        delegate: Rectangle {
            anchors.centerIn: parent
            property real t: (index + 1) / root.layers
            width: root.width * t
            height: root.height * t
            radius: width / 2
            color: root.glowColor
            opacity: root.intensity * t * t
        }
    }
}
