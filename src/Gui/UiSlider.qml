import QtQuick

// Custom slider: rounded track, gradient/fill portion and a glowing knob.
// `value` is never assigned internally while dragging, so external bindings
// on `value` stay alive; drags report through moved(newValue).
Item {
    id: root

    property real from: 0
    property real to: 100
    property real stepSize: 0      // 0 = continuous
    property real value: 0
    property var trackGradient: null // optional Gradient for the filled part
    property color fillColor: Theme.accentA

    property bool dragging: false
    property real visualValue: value

    signal moved(real newValue)

    implicitHeight: 28

    onValueChanged: if (!dragging) visualValue = value

    function snap(v) {
        if (stepSize > 0)
            v = Math.round(v / stepSize) * stepSize;
        return Math.min(to, Math.max(from, v));
    }
    function ratio() {
        return Math.min(1, Math.max(0, (visualValue - from) / Math.max(1e-9, to - from)));
    }
    function updateFromX(x) {
        var r = Math.min(1, Math.max(0, (x - knob.width / 2) / track.width));
        visualValue = snap(from + r * (to - from));
        moved(visualValue);
    }

    Rectangle {
        id: track
        anchors.verticalCenter: parent.verticalCenter
        x: knob.width / 2
        width: parent.width - knob.width
        height: 6
        radius: 3
        color: Theme.surfaceActive
    }

    Rectangle {
        id: fillColorRect
        anchors.left: track.left
        anchors.verticalCenter: track.verticalCenter
        height: track.height
        radius: track.radius
        width: root.ratio() * track.width
        visible: root.trackGradient === null
        color: root.fillColor
    }
    // When a gradient is provided, it spans the FULL track width (rainbow
    // bars etc.); the knob alone indicates the current position.
    Rectangle {
        id: fillGradientRect
        anchors.left: track.left
        anchors.verticalCenter: track.verticalCenter
        height: track.height
        radius: track.radius
        width: track.width
        visible: root.trackGradient !== null
        gradient: root.trackGradient
    }

    // knob glow
    Rectangle {
        visible: root.dragging || knobMa.containsMouse
        anchors.centerIn: knob
        width: knob.width + 12
        height: knob.height + 12
        radius: (knob.width + 12) / 2
        color: Theme.glowAccent
        opacity: 0.7
        z: -1
    }

    Rectangle {
        id: knob
        width: 18
        height: 18
        radius: 9
        anchors.verticalCenter: parent.verticalCenter
        x: track.x + root.ratio() * track.width - width / 2
        color: "#ffffff"
        border.color: root.fillColor
        border.width: 2
    }

    MouseArea {
        id: knobMa
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed: function(mouse) {
            root.dragging = true;
            root.updateFromX(mouse.x);
        }
        onPositionChanged: function(mouse) {
            if (root.dragging) root.updateFromX(mouse.x);
        }
        onReleased: root.dragging = false
    }
}
