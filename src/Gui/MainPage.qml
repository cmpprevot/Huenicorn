import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: mainPage

    property bool savedRecently: false

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        // ---------------- header ----------------
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            // logo tile
            Item {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: -5
                    radius: Theme.radiusControl + 7
                    color: Theme.glowAccent
                    opacity: 0.6
                    z: -1
                }
                Rectangle {
                    anchors.fill: parent
                    radius: Theme.radiusControl
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Theme.accentA }
                        GradientStop { position: 1.0; color: Theme.accentB }
                    }
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/qt/qml/huenicorn_UI/logo.svg"
                        sourceSize.width: 26
                        sourceSize.height: 26
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }

            ColumnLayout {
                spacing: 1
                Label {
                    text: "Huenicorn"
                    font.pixelSize: 20
                    font.bold: true
                    color: Theme.text
                }
                Label {
                    text: "Ambilight for Philips Hue"
                    font.pixelSize: Theme.fontSmall
                    color: Theme.textFaint
                }
            }

            // version chip
            Rectangle {
                radius: Theme.pill
                color: Theme.surfaceActive
                border.color: Theme.border
                border.width: 1
                implicitWidth: versionChipLabel.implicitWidth + 16
                implicitHeight: 24
                Label {
                    id: versionChipLabel
                    anchors.centerIn: parent
                    text: "v" + huenicorn.version
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            Item { Layout.fillWidth: true }

            SegmentedControl {
                id: nav
                Layout.preferredWidth: 300
                model: ["Ambilight", "Manual control"]
            }

            // ambilight master toggle (pill with glowing dot)
            Item {
                id: ambilightPill
                Layout.preferredWidth: pillRow.implicitWidth + 30
                Layout.preferredHeight: 38
                readonly property bool active: huenicorn.streamingActive

                Rectangle {
                    visible: ambilightPill.active && huenicorn.running
                    anchors.fill: pillBg
                    anchors.margins: -6
                    radius: Theme.pill
                    color: Theme.glowAccent
                    z: -1
                }
                Rectangle {
                    id: pillBg
                    anchors.fill: parent
                    radius: Theme.pill
                    color: !huenicorn.running ? Theme.surfaceActive
                        : ambilightPill.active ? Theme.accentA
                        : (pillMa.containsMouse ? Theme.surfaceHover : Theme.surfaceActive)
                    border.color: ambilightPill.active && huenicorn.running ? "transparent" : Theme.border
                    border.width: 1
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        visible: ambilightPill.active && huenicorn.running
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: Theme.accentA }
                            GradientStop { position: 1.0; color: Theme.accentB }
                        }
                    }
                }
                RowLayout {
                    id: pillRow
                    anchors.centerIn: parent
                    spacing: 8
                    Rectangle {
                        id: pillDot
                        Layout.preferredWidth: 9
                        Layout.preferredHeight: 9
                        radius: 5
                        color: ambilightPill.active && huenicorn.running ? "#ffffff" : Theme.textDim
                        SequentialAnimation on opacity {
                            running: ambilightPill.active && huenicorn.running
                            loops: Animation.Infinite
                            onStopped: pillDot.opacity = 1.0
                            NumberAnimation { to: 0.35; duration: 900; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InOutSine }
                        }
                    }
                    Label {
                        text: "Ambilight"
                        font.pixelSize: Theme.fontBody
                        font.bold: true
                        color: ambilightPill.active && huenicorn.running ? "#ffffff" : Theme.text
                    }
                    Label {
                        text: ambilightPill.active ? "On" : "Off"
                        font.pixelSize: Theme.fontBody
                        font.bold: true
                        color: ambilightPill.active && huenicorn.running ? "#ffffff" : Theme.textDim
                    }
                }
                MouseArea {
                    id: pillMa
                    anchors.fill: parent
                    enabled: huenicorn.running
                    hoverEnabled: true
                    cursorShape: huenicorn.running ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: huenicorn.setStreamingActive(!huenicorn.streamingActive)
                }
            }

            UiButton {
                text: "Shut down"
                kind: "danger"
                onClicked: confirmStopDialog.open()
            }
        }

        // ---------------- waiting-for-capture banner ----------------
        Rectangle {
            Layout.fillWidth: true
            visible: !huenicorn.running
            radius: Theme.radiusControl
            color: Qt.rgba(0.13, 0.83, 0.93, 0.07)
            border.color: Qt.rgba(0.13, 0.83, 0.93, 0.35)
            border.width: 1
            implicitHeight: bannerLabel.implicitHeight + 18

            Label {
                id: bannerLabel
                anchors.centerIn: parent
                width: parent.width - 32
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Theme.text
                font.pixelSize: Theme.fontBody
                text: "Waiting for screen capture — pick your screen in the portal dialog and click \"Share\"."
            }
        }

        // ---------------- ambilight tab ----------------
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16
            visible: nav.currentIndex === 0

            // left: screen preview card
            UiCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ScreenArea {
                    anchors.fill: parent
                    anchors.margins: Theme.padding
                }
            }

            // right sidebar (scrollable, so controls never overlap or get cut off)
            Flickable {
                id: sidebarFlick
                Layout.preferredWidth: 360
                Layout.maximumWidth: 360
                Layout.fillWidth: false
                Layout.fillHeight: true
                clip: true
                contentWidth: sidebarColumn.width
                contentHeight: sidebarColumn.height
                boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: sidebarColumn
                width: sidebarFlick.width
                height: Math.max(implicitHeight, sidebarFlick.height)
                spacing: 10

                UiSectionLabel { text: "Entertainment area" }
                UiComboBox {
                    Layout.fillWidth: true
                    model: huenicorn.entertainmentConfigurations
                    textRole: "name"
                    valueRole: "id"
                    currentIndex: indexOfValue(huenicorn.currentEntertainmentConfigurationId)
                    onActivated: huenicorn.selectEntertainmentConfiguration(currentValue)
                }

                UiSectionLabel { text: "Lights" }
                UiCard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.preferredHeight: 220
                    Layout.minimumHeight: 140

                    ListView {
                        id: channelsList
                        anchors.fill: parent
                        anchors.margins: 8
                        clip: true
                        model: huenicorn.channels
                        spacing: 6

                        delegate: Item {
                            width: channelsList.width
                            height: 56

                            Rectangle {
                                anchors.fill: parent
                                radius: Theme.radiusSmall
                                color: huenicorn.selectedChannelId === model.channelId ? Theme.surfaceActive
                                    : (rowMa.containsMouse ? Theme.surfaceHover : "transparent")
                                border.color: huenicorn.selectedChannelId === model.channelId ? Theme.accentA : "transparent"
                                border.width: 1.5
                            }

                            MouseArea {
                                id: rowMa
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: huenicorn.selectedChannelId = model.channelId
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10

                                Item {
                                    Layout.preferredWidth: 14
                                    Layout.preferredHeight: 14
                                    Rectangle {
                                        visible: model.active
                                        anchors.centerIn: parent
                                        width: 22
                                        height: 22
                                        radius: 11
                                        color: Theme.channelColor(model.channelId)
                                        opacity: 0.25
                                    }
                                    Rectangle {
                                        anchors.centerIn: parent
                                        width: 12
                                        height: 12
                                        radius: 6
                                        color: Theme.channelColor(model.channelId)
                                        opacity: model.active ? 1.0 : 0.25
                                    }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Label {
                                        text: model.label
                                        color: Theme.text
                                        font.pixelSize: Theme.fontBody
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                    Label {
                                        text: model.devices.length > 0 ? model.devices.join(", ") : "no light"
                                        color: Theme.textDim
                                        font.pixelSize: Theme.fontSmall
                                        elide: Text.ElideRight
                                        Layout.fillWidth: true
                                    }
                                }
                                UiSwitch {
                                    checked: model.active
                                    onToggled: function(isChecked) {
                                        huenicorn.setChannelActivity(model.channelId, isChecked)
                                    }
                                }
                            }
                        }
                    }
                }

                // advanced settings
                UiCard {
                    Layout.fillWidth: true
                    // The content below is anchored, so it does not contribute
                    // to the card's implicit height: bind the preferred height
                    // to the content layout explicitly, otherwise the card
                    // collapses to 0 and its content overlaps the footer.
                    Layout.preferredHeight: advancedContent.implicitHeight + 28

                    ColumnLayout {
                        id: advancedContent
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            UiSectionLabel {
                                text: "Advanced settings"
                                Layout.fillWidth: true
                            }
                            UiSwitch {
                                id: advancedSwitch
                                onToggled: function(isChecked) { advancedSwitch.checked = isChecked }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            visible: advancedSwitch.checked

                            Label { text: "Subsample resolution"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            UiComboBox {
                                id: subsampleCombo
                                Layout.fillWidth: true
                                model: huenicorn.subsampleCandidates
                                onActivated: huenicorn.setSubsampleWidth(model[index].x)
                                Connections {
                                    target: huenicorn
                                    function onDisplayInfoChanged() {
                                        for (var i = 0; i < subsampleCombo.count; i++) {
                                            if (subsampleCombo.model[i].x === huenicorn.subsampleWidth) {
                                                subsampleCombo.currentIndex = i;
                                                break;
                                            }
                                        }
                                    }
                                }
                                delegate: ItemDelegate {
                                    width: subsampleCombo.width
                                    hoverEnabled: true
                                    highlighted: subsampleCombo.highlightedIndex === index
                                    background: Rectangle {
                                        color: parent.highlighted ? Theme.surfaceActive
                                             : parent.hovered ? Theme.surfaceHover
                                             : "transparent"
                                        radius: Theme.radiusSmall - 2
                                    }
                                    contentItem: Label {
                                        text: modelData.x + " x " + modelData.y
                                            + "  (" + Math.round(modelData.x / Math.max(1, huenicorn.displayWidth) * 100) + "%)"
                                        color: parent.highlighted ? "#ffffff" : Theme.text
                                        font.pixelSize: Theme.fontBody
                                        leftPadding: 10
                                        rightPadding: 10
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                                contentItem: Label {
                                    text: subsampleCombo.currentIndex >= 0 && subsampleCombo.model.length > 0
                                        ? subsampleCombo.model[subsampleCombo.currentIndex].x + " x " + subsampleCombo.model[subsampleCombo.currentIndex].y
                                        : ""
                                    color: Theme.text
                                    font.pixelSize: Theme.fontBody
                                    leftPadding: 12
                                    rightPadding: subsampleCombo.indicator.width + 12
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                            }

                            Label { text: "Interpolation"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            UiComboBox {
                                Layout.fillWidth: true
                                model: huenicorn.interpolations
                                textRole: "name"
                                valueRole: "value"
                                currentIndex: model ? model.findIndex(function(m) { return m.value === huenicorn.interpolation; }) : -1
                                onActivated: huenicorn.setInterpolation(currentValue)
                            }

                            Label { text: "Refresh rate (Hz)"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            UiSpinBox {
                                from: 1
                                to: Math.max(1, huenicorn.maxRefreshRate)
                                value: huenicorn.refreshRate
                                onValueModified: huenicorn.setRefreshRate(value)
                            }

                            Label { text: "Transition smoothing"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                UiSlider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 95
                                    stepSize: 1
                                    value: huenicorn.transitionSmoothing
                                    onMoved: function(v) { huenicorn.setTransitionSmoothing(Math.round(v)) }
                                }
                                Label {
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSmall
                                    text: Math.round(huenicorn.transitionSmoothing) + " %"
                                }
                            }
                        }
                    }
                }

                // footer
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    UiButton {
                        text: "Save profile"
                        kind: "primary"
                        onClicked: {
                            huenicorn.saveProfile();
                            mainPage.savedRecently = true;
                            savedTimer.restart();
                        }
                    }
                    Label {
                        visible: mainPage.savedRecently
                        text: "Saved"
                        color: Theme.success
                        font.pixelSize: Theme.fontSmall
                        font.bold: true
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: huenicorn.displayWidth + "x" + huenicorn.displayHeight
                            + "  ·  " + huenicorn.refreshRate + " Hz"
                        color: Theme.textFaint
                        font.pixelSize: Theme.fontSmall
                    }
                }
            }
            }
        }

        // ---------------- manual control tab ----------------
        ManualControlPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: nav.currentIndex === 1
        }
    }

    Timer {
        id: savedTimer
        interval: 2500
        onTriggered: mainPage.savedRecently = false
    }

    Dialog {
        id: confirmStopDialog
        anchors.centerIn: Overlay.overlay
        modal: true
        title: ""
        padding: 22

        background: Rectangle {
            color: Theme.surfaceElevated
            border.color: Theme.borderStrong
            border.width: 1
            radius: Theme.radius
        }

        contentItem: ColumnLayout {
            spacing: 14

            Label {
                text: "Shut down Huenicorn?"
                font.pixelSize: Theme.fontHeader
                font.bold: true
                color: Theme.text
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                color: Theme.textDim
                font.pixelSize: Theme.fontBody
                text: "Do you really want to stop Huenicorn?"
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 10
                Item { Layout.fillWidth: true }
                UiButton {
                    text: "Cancel"
                    kind: "ghost"
                    onClicked: confirmStopDialog.close()
                }
                UiButton {
                    text: "Shut down"
                    kind: "danger"
                    onClicked: {
                        confirmStopDialog.close();
                        huenicorn.quit();
                    }
                }
            }
        }
    }
}
