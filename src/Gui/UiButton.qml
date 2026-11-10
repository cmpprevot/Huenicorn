import QtQuick
import QtQuick.Controls

// Modern button with three kinds:
//   "primary" - signature gradient fill + soft glow
//   "ghost"   - bordered, transparent fill (default look)
//   "danger"  - red tint on hover
Item {
    id: root

    property string text: ""
    property string kind: "ghost"

    // `enabled` is inherited from Item: disabling it also stops event
    // delivery to the MouseArea below.

    signal clicked()

    implicitWidth: label.implicitWidth + 36
    implicitHeight: 38

    // soft glow behind primary buttons
    Rectangle {
        visible: root.kind === "primary" && root.enabled
        anchors.fill: bg
        anchors.margins: -6
        radius: Theme.radiusControl + 8
        color: Theme.glowAccent
        opacity: ma.containsMouse ? 0.9 : 0.55
        z: -1
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: Theme.radiusControl

        color: {
            if (!root.enabled) return Theme.surfaceActive;
            switch (root.kind) {
            case "primary": return Theme.accentA;
            case "danger": return ma.pressed ? Qt.rgba(0.98, 0.45, 0.52, 0.28) : "transparent";
            default: return ma.pressed ? Theme.surfaceActive : (ma.containsMouse ? Theme.surfaceHover : "transparent");
            }
        }
        border.color: {
            if (!root.enabled) return Theme.border;
            if (root.kind === "primary") return "transparent";
            if (root.kind === "danger") return ma.containsMouse ? Theme.danger : Theme.border;
            return ma.containsMouse ? Theme.borderStrong : Theme.border;
        }
        border.width: root.kind === "primary" ? 0 : 1

        // gradient overlay for the primary kind
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: root.kind === "primary" && root.enabled
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Theme.accentA }
                GradientStop { position: 1.0; color: Theme.accentB }
            }
        }
        // hover sheen
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            visible: root.enabled && ma.containsMouse && root.kind !== "primary"
            color: Qt.rgba(1, 1, 1, 0.04)
        }
    }

    Label {
        id: label
        anchors.centerIn: parent
        text: root.text
        font.pixelSize: Theme.fontBody
        font.bold: root.kind === "primary"
        color: {
            if (!root.enabled) return Theme.textFaint;
            if (root.kind === "primary") return "#ffffff";
            if (root.kind === "danger" && ma.containsMouse) return "#ffffff";
            return Theme.text;
        }
    }

    MouseArea {
        id: ma
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
