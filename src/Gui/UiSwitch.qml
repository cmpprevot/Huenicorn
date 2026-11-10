import QtQuick

// Pill switch with gradient track when on. The `checked` property is never
// assigned internally, so bindings like `checked: model.active` stay alive;
// user interaction only emits toggled(newValue).
Item {
    id: root

    property bool checked: false
    readonly property bool hovered: ma.containsMouse

    signal toggled(bool isChecked)

    implicitWidth: 46
    implicitHeight: 26

    Rectangle {
        id: track
        anchors.fill: parent
        radius: Theme.pill
        color: root.checked ? Theme.accentA : Theme.surfaceActive
        border.color: root.checked ? "transparent" : Theme.border
        border.width: 1

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: root.checked
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Theme.accentA }
                GradientStop { position: 1.0; color: Theme.accentB }
            }
        }
    }

    // glow when on
    Rectangle {
        visible: root.checked
        anchors.centerIn: track
        width: track.width + 10
        height: track.height + 10
        radius: Theme.pill
        color: Theme.glowAccent
        opacity: 0.5
        z: -1
    }

    Rectangle {
        id: knob
        width: 20
        height: 20
        radius: 10
        anchors.verticalCenter: parent.verticalCenter
        x: root.checked ? root.width - width - 3 : 3
        color: root.checked ? "#ffffff" : Theme.textDim

        Behavior on x {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
        Behavior on color {
            ColorAnimation { duration: 160 }
        }
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggled(!root.checked)
    }
}
