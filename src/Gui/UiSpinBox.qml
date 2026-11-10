import QtQuick
import QtQuick.Controls

// SpinBox with a dark styled box and chevron up/down buttons.
// API is the standard SpinBox one (from, to, value, valueModified(value)).
SpinBox {
    id: root

    implicitWidth: 110
    implicitHeight: 40

    background: Rectangle {
        radius: Theme.radiusControl
        color: root.hovered || root.activeFocus ? Theme.surfaceHover : Theme.background
        border.color: root.activeFocus ? Theme.accentA : Theme.border
        border.width: root.activeFocus ? 1.5 : 1
    }

    contentItem: TextInput {
        z: 2
        text: root.textFromValue(root.value, root.locale)
        font.pixelSize: Theme.fontBody
        color: Theme.text
        selectionColor: Theme.accentA
        selectedTextColor: "#ffffff"
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !root.editable
        validator: root.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }

    up.indicator: UiChevron {
        x: root.width - width - 6
        y: (root.height - height) / 2
        up: true
        color: Theme.textDim
    }

    down.indicator: UiChevron {
        x: 6
        y: (root.height - height) / 2
        up: false
        color: Theme.textDim
    }
}
