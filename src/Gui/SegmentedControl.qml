import QtQuick
import QtQuick.Controls

// Pill segmented navigation with a sliding gradient indicator.
Item {
    id: root

    property var model: []
    property int currentIndex: 0

    signal selected(int index)

    implicitHeight: 40

    function segmentWidth() {
        return (root.width - 8) / Math.max(1, root.model.length);
    }

    Rectangle {
        anchors.fill: parent
        radius: Theme.pill
        color: Theme.surfaceActive
        border.color: Theme.border
        border.width: 1
    }

    Rectangle {
        id: indicator
        y: 4
        height: parent.height - 8
        width: root.segmentWidth()
        x: 4 + root.currentIndex * root.segmentWidth()
        radius: Theme.pill
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: Theme.accentA }
            GradientStop { position: 1.0; color: Theme.accentB }
        }

        Behavior on x { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
        Behavior on width { NumberAnimation { duration: 180; easing.type: Easing.OutCubic } }
    }

    Repeater {
        model: root.model
        delegate: Item {
            x: 4 + index * root.segmentWidth()
            y: 0
            width: root.segmentWidth()
            height: root.height

            Label {
                anchors.centerIn: parent
                text: modelData
                font.pixelSize: Theme.fontBody
                font.bold: index === root.currentIndex
                color: index === root.currentIndex ? "#ffffff" : Theme.textDim
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.currentIndex = index;
                    root.selected(index);
                }
            }
        }
    }
}
