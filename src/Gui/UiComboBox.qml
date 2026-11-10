import QtQuick
import QtQuick.Controls

// ComboBox with a dark styled box, chevron indicator and an elevated popup.
// The popup/delegate follow the canonical Qt Quick Controls pattern
// (popup list currentIndex bound to highlightedIndex, delegate uses
// required model/index) so hover and keyboard highlight always work.
// API is the standard ComboBox one (model, textRole, valueRole, currentIndex,
// currentValue, displayText, activated(index), indexOfValue, textAt).
ComboBox {
    id: root

    implicitHeight: 40

    background: Rectangle {
        radius: Theme.radiusControl
        color: root.hovered || root.popup.opened ? Theme.surfaceHover : Theme.background
        border.color: root.popup.opened || root.activeFocus ? Theme.accentA : Theme.border
        border.width: root.popup.opened || root.activeFocus ? 1.5 : 1
    }

    contentItem: Label {
        text: root.displayText
        color: Theme.text
        leftPadding: 12
        rightPadding: root.indicator.width + 12
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        font.pixelSize: Theme.fontBody
    }

    indicator: UiChevron {
        x: root.width - width - root.rightPadding - 2
        y: root.topPadding + (root.availableHeight - height) / 2
        color: Theme.textDim
    }

    popup: Popup {
        y: root.height + 6
        width: root.width
        implicitHeight: contentItem.implicitHeight + 8
        padding: 4

        background: Rectangle {
            color: Theme.surfaceElevated
            border.color: Theme.borderStrong
            border.width: 1
            radius: Theme.radiusSmall
        }

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.delegateModel
            currentIndex: root.highlightedIndex
            highlightMoveDuration: 0
            delegate: root.delegate
            ScrollIndicator.vertical: ScrollIndicator {}
        }
    }

    delegate: ItemDelegate {
        id: itemDelegate

        required property var model
        required property int index

        width: ListView.view ? ListView.view.width : root.width
        hoverEnabled: true
        highlighted: root.highlightedIndex === index

        background: Rectangle {
            color: itemDelegate.highlighted ? Theme.surfaceActive
                 : itemDelegate.hovered ? Theme.surfaceHover
                 : "transparent"
            radius: Theme.radiusSmall - 2
        }
        contentItem: Label {
            text: itemDelegate.model[root.textRole] !== undefined
                  ? itemDelegate.model[root.textRole]
                  : root.textAt(itemDelegate.index)
            color: itemDelegate.highlighted ? "#ffffff" : Theme.text
            font.pixelSize: Theme.fontBody
            leftPadding: 10
            rightPadding: 10
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
