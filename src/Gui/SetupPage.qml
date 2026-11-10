import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Initial setup wizard: bridge discovery / address validation, then user pairing
Item {
    id: setupPage

    property int step: 1
    property bool busy: false
    property string errorMessage: ""
    property var detectedBridges: []

    UiCard {
        anchors.centerIn: parent
        width: Math.min(540, parent.width - 48)
        height: contentColumn.implicitHeight + 2 * 24

        ColumnLayout {
            id: contentColumn
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16

            // logo with gradient ring
            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 92
                Layout.preferredHeight: 92

                Rectangle {
                    anchors.fill: parent
                    radius: 26
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Theme.accentA }
                        GradientStop { position: 0.55; color: Theme.accentB }
                        GradientStop { position: 1.0; color: Theme.accentC }
                    }
                }
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 4
                    radius: 22
                    color: Theme.surface
                    Image {
                        anchors.centerIn: parent
                        source: "qrc:/qt/qml/huenicorn_UI/logo.svg"
                        sourceSize.width: 56
                        sourceSize.height: 56
                        fillMode: Image.PreserveAspectFit
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "Welcome to Huenicorn"
                font.pixelSize: Theme.fontTitle
                font.bold: true
                color: Theme.text
            }

            // step indicator
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 8

                Rectangle {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 6
                    radius: 3
                    color: Theme.accentA
                }
                Rectangle {
                    Layout.preferredWidth: 26
                    Layout.preferredHeight: 6
                    radius: 3
                    color: setupPage.step >= 2 ? Theme.accentB : Theme.surfaceActive
                }
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                color: Theme.textDim
                font.pixelSize: Theme.fontBody
                text: step === 1
                    ? "Connect Huenicorn to your Philips Hue bridge."
                    : "Press the link button on your Hue bridge, then click Pair."
            }

            // Step 1: bridge address
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10
                visible: step === 1

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    TextField {
                        id: bridgeAddressField
                        Layout.fillWidth: true
                        implicitHeight: 42
                        placeholderText: "Bridge IP address (e.g. 192.168.1.10)"
                        color: Theme.text
                        placeholderTextColor: Theme.textFaint
                        selectedTextColor: "#ffffff"
                        selectionColor: Theme.accentA
                        font.pixelSize: Theme.fontBody
                        leftPadding: 14
                        rightPadding: 14
                        background: Rectangle {
                            color: bridgeAddressField.hovered || bridgeAddressField.activeFocus
                                ? Theme.surfaceHover : Theme.background
                            border.color: bridgeAddressField.activeFocus ? Theme.accentA : Theme.border
                            border.width: bridgeAddressField.activeFocus ? 1.5 : 1
                            radius: Theme.radiusControl
                        }
                    }

                    UiButton {
                        text: "Auto-detect"
                        kind: "ghost"
                        enabled: !setupPage.busy
                        onClicked: {
                            setupPage.busy = true;
                            setupPage.errorMessage = "";
                            var result = huenicorn.autodetectBridges();
                            setupPage.busy = false;
                            if (result.succeeded && result.bridges.length > 0) {
                                setupPage.detectedBridges = result.bridges;
                                bridgeAddressField.text = result.bridges[0].internalipaddress;
                            } else {
                                setupPage.errorMessage = result.error !== undefined
                                    ? result.error
                                    : "No bridge detected. Enter the IP address manually.";
                            }
                        }
                    }
                }

                UiButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    text: "Continue"
                    kind: "primary"
                    enabled: !setupPage.busy && bridgeAddressField.text.trim() !== ""
                    onClicked: {
                        setupPage.busy = true;
                        setupPage.errorMessage = "";
                        var ok = huenicorn.validateBridgeAddress(bridgeAddressField.text);
                        setupPage.busy = false;
                        if (ok) {
                            setupPage.step = 2;
                        } else {
                            setupPage.errorMessage = "Could not reach a Hue bridge at this address.";
                        }
                    }
                }
            }

            // Step 2: pairing
            UiButton {
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                visible: step === 2
                text: "Pair with bridge"
                kind: "primary"
                enabled: !setupPage.busy
                onClicked: {
                    setupPage.busy = true;
                    setupPage.errorMessage = "";
                    var result = huenicorn.registerNewUser();
                    setupPage.busy = false;
                    if (!result.succeeded) {
                        setupPage.errorMessage = result.error !== undefined
                            ? result.error
                            : "Pairing failed. Make sure the bridge link button was pressed.";
                    }
                    // On success the controller switches setupRequired off and
                    // Main.qml automatically loads the main page.
                }
            }

            RowLayout {
                Layout.fillWidth: true
                visible: setupPage.busy
                spacing: 10

                BusyIndicator {
                    Layout.alignment: Qt.AlignVCenter
                    running: setupPage.busy
                    palette.dark: Theme.accentA
                }
                Label {
                    text: "Contacting bridge..."
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                }
            }

            Label {
                Layout.fillWidth: true
                visible: setupPage.errorMessage !== ""
                wrapMode: Text.WordWrap
                color: Theme.danger
                font.pixelSize: Theme.fontSmall
                text: setupPage.errorMessage
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
                text: "Huenicorn " + huenicorn.version + "  ·  config: " + huenicorn.configFilePath
            }
        }
    }
}
