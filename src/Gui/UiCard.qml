import QtQuick

// Elevated surface card: soft drop shadow (faked with translucent layers,
// so no Qt5Compat/GraphicalEffects dependency), 1px border and a subtle
// top highlight. Children placed inside land in the card body.
// The card is always sized by its parent (Layout fill/preferred or
// explicit width/height): anchored content does not contribute to the
// implicit size, so implicitWidth/Height intentionally stay at 0.
Item {
    id: root

    default property alias contentData: card.data

    Rectangle {
        x: 0
        y: 4
        width: card.width
        height: card.height
        radius: card.radius + 8
        color: Qt.rgba(0, 0, 0, 0.28)
        z: -2
    }
    Rectangle {
        x: 0
        y: 2
        width: card.width
        height: card.height
        radius: card.radius + 6
        color: Qt.rgba(0, 0, 0, 0.32)
        z: -1
    }

    Rectangle {
        id: card
        anchors.fill: parent
        color: Theme.surface
        border.color: Theme.border
        border.width: 1
        radius: Theme.radius

        // top inner highlight
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 1
            anchors.leftMargin: card.radius
            anchors.rightMargin: card.radius
            height: 1
            color: Qt.rgba(1, 1, 1, 0.05)
        }
    }
}
