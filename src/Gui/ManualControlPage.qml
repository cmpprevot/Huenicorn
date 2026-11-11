import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: manualPage

    // Raw UI fields of the manual state. All conversions (HSL/RGB for the
    // "color" mode, Kelvin/mirek for the "white" mode) and the bridge payload
    // are computed in C++ by the controller.
    property bool power: true
    property int brightness: 80
    property string mode: "color" // "color" | "white"
    property int hue: 210
    property int saturation: 100
    property int kelvin: 4000

    // Exact hex chosen in the hex field: when set it wins over the hue/sat
    // sliders so the precise RGB value is sent to the bridge
    property string hexColor: ""

    readonly property color currentColor: hexColor !== ""
        ? huenicorn.colorFromHex(hexColor)
        : huenicorn.manualColor(mode, hue, saturation, kelvin)

    // Sends the current state to the bridge (hex value when one is set)
    function applyNow() {
        if(hexColor !== "")
            huenicorn.setManualLightHex(power, brightness, hexColor);
        else
            huenicorn.setManualLight(power, brightness, mode, hue, saturation, kelvin);
    }

    // False while Component.onCompleted restores the last saved state, so the
    // handlers below don't send a spurious state to the bridge on page load
    property bool ready: false

    // Sliders fire continuously while dragged: debounce the bridge requests
    Timer {
        id: applyTimer
        interval: 250
        onTriggered: manualPage.applyNow()
    }

    // Keep the hex field in sync with the sliders (without clobbering the
    // text while the user is typing in it)
    onHueChanged: hexField.refresh()
    onSaturationChanged: hexField.refresh()
    onKelvinChanged: hexField.refresh()
    onModeChanged: hexField.refresh()
    onHexColorChanged: hexField.refresh()

    Component.onCompleted: {
        var s = huenicorn.manualLightState();
        if(s && Object.keys(s).length > 0){
            if("on" in s) power = s.on;
            if("brightness" in s) brightness = Math.round(s.brightness);
            if("mode" in s) mode = s.mode;
            if("hue" in s) hue = Math.round(s.hue);
            if("saturation" in s) saturation = Math.round(s.saturation);
            if("kelvin" in s) kelvin = Math.round(s.kelvin);
        }
        ready = true;
        hexField.refresh();
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 16

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Manual light control"
                color: Theme.text
                font.pixelSize: Theme.fontHeader
                font.bold: true
                Layout.fillWidth: true
            }
            Rectangle {
                id: headerSwatch
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 20
                color: manualPage.currentColor
                border.color: Qt.rgba(1, 1, 1, 0.18)
                border.width: 1
                // dim with brightness, hue untouched
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 1
                    radius: parent.radius - 1
                    color: "black"
                    opacity: 1.0 - manualPage.brightness / 100.0
                }
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.textDim
            font.pixelSize: Theme.fontSmall
            text: "Applies to all lights of the current entertainment area. Applying a manual state pauses ambilight; resume it from the Ambilight tab."
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            // left card: color orb, power, mode
            UiCard {
                Layout.preferredWidth: 300
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 18

                    // big glowing color orb
                    Item {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 130
                        Layout.preferredHeight: 130

                        Rectangle {
                            anchors.centerIn: parent
                            width: 158
                            height: 158
                            radius: 79
                            color: manualPage.currentColor
                            opacity: 0.16 * manualPage.brightness / 100.0
                        }
                        Rectangle {
                            anchors.centerIn: parent
                            width: 144
                            height: 144
                            radius: 72
                            color: manualPage.currentColor
                            opacity: 0.28 * manualPage.brightness / 100.0
                        }
                        Rectangle {
                            id: orb
                            anchors.fill: parent
                            radius: 65
                            color: manualPage.currentColor
                            border.color: Qt.rgba(1, 1, 1, 0.18)
                            border.width: 1
                        }
                        // dim the orb with brightness, hue untouched
                        Rectangle {
                            anchors.fill: orb
                            anchors.margins: 1
                            radius: orb.radius - 1
                            color: "black"
                            opacity: 1.0 - manualPage.brightness / 100.0
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        color: Theme.textDim
                        font.pixelSize: Theme.fontBody
                        text: manualPage.mode === "color"
                            ? manualPage.hue + "°  ·  " + manualPage.saturation + "% sat"
                            : manualPage.kelvin + " K"
                    }

                    // exact hex code of the current color, editable
                    TextField {
                        id: hexField
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 120
                        implicitHeight: 34
                        horizontalAlignment: Text.AlignHCenter
                        font.family: "monospace"
                        font.pixelSize: Theme.fontSmall
                        color: Theme.text
                        placeholderText: "#RRGGBB"
                        placeholderTextColor: Theme.textFaint
                        selectedTextColor: "#ffffff"
                        selectionColor: Theme.accentA
                        background: Rectangle {
                            color: hexField.hovered || hexField.activeFocus
                                ? Theme.surfaceHover : Theme.background
                            border.color: hexField.activeFocus ? Theme.accentA : Theme.border
                            border.width: hexField.activeFocus ? 1.5 : 1
                            radius: Theme.radiusSmall
                        }
                        function refresh() {
                            if(activeFocus) return;
                            text = manualPage.hexColor !== ""
                                ? manualPage.hexColor
                                : huenicorn.manualColorHex(manualPage.mode, manualPage.hue,
                                                            manualPage.saturation, manualPage.kelvin);
                        }
                        onAccepted: {
                            var r = huenicorn.hexToHsl(text);
                            if(r.valid){
                                manualPage.mode = "color";
                                manualPage.hue = r.hue;
                                manualPage.saturation = r.saturation;
                                manualPage.hexColor = r.hex;
                                manualPage.applyNow();
                            }
                            refresh();
                        }
                        onEditingFinished: refresh()
                    }

                    Item { Layout.fillHeight: true }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        Label {
                            text: "Power"
                            color: Theme.text
                            font.pixelSize: Theme.fontBody
                            Layout.fillWidth: true
                        }
                        UiSwitch {
                            checked: manualPage.power
                            onToggled: function(isChecked) {
                                if(!manualPage.ready) return;
                                manualPage.power = isChecked;
                                manualPage.applyNow();
                            }
                        }
                    }

                    UiSectionLabel { text: "Mode" }
                    SegmentedControl {
                        Layout.fillWidth: true
                        model: ["Color", "White"]
                        currentIndex: manualPage.mode === "color" ? 0 : 1
                        onSelected: function(index) {
                            if(!manualPage.ready) return;
                            manualPage.mode = index === 0 ? "color" : "white";
                            if(manualPage.mode === "white") manualPage.hexColor = "";
                            manualPage.applyNow();
                        }
                    }
                }
            }

            // right card: sliders
            UiCard {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 16

                    Label { text: "Brightness"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12
                        UiSlider {
                            Layout.fillWidth: true
                            from: 0
                            to: 100
                            stepSize: 1
                            value: manualPage.brightness
                            onMoved: function(v) {
                                manualPage.brightness = Math.round(v);
                                applyTimer.restart();
                            }
                        }
                        Label {
                            text: manualPage.brightness + " %"
                            color: Theme.text
                            font.pixelSize: Theme.fontSmall
                            Layout.preferredWidth: 48
                        }
                    }

                    StackLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        currentIndex: manualPage.mode === "color" ? 0 : 1

                        // Color mode: hue + saturation
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Label { text: "Hue"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                UiSlider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 360
                                    stepSize: 1
                                    value: manualPage.hue
                                    trackGradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.00; color: "#ff0000" }
                                        GradientStop { position: 0.17; color: "#ffff00" }
                                        GradientStop { position: 0.33; color: "#00ff00" }
                                        GradientStop { position: 0.50; color: "#00ffff" }
                                        GradientStop { position: 0.67; color: "#0000ff" }
                                        GradientStop { position: 0.83; color: "#ff00ff" }
                                        GradientStop { position: 1.00; color: "#ff0000" }
                                    }
                                    onMoved: function(v) {
                                        manualPage.hue = Math.round(v);
                                        manualPage.hexColor = ""; // slider takes over the exact value
                                        applyTimer.restart();
                                    }
                                }
                                Label {
                                    text: manualPage.hue + "°"
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSmall
                                    Layout.preferredWidth: 48
                                }
                            }

                            Label { text: "Saturation"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                UiSlider {
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 100
                                    stepSize: 1
                                    value: manualPage.saturation
                                    trackGradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: huenicorn.manualColor("color", manualPage.hue, 0, 0) }
                                        GradientStop { position: 1.0; color: huenicorn.manualColor("color", manualPage.hue, 100, 0) }
                                    }
                                    onMoved: function(v) {
                                        manualPage.saturation = Math.round(v);
                                        manualPage.hexColor = ""; // slider takes over the exact value
                                        applyTimer.restart();
                                    }
                                }
                                Label {
                                    text: manualPage.saturation + " %"
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSmall
                                    Layout.preferredWidth: 48
                                }
                            }
                        }

                        // White mode: color temperature
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            Label { text: "Color temperature"; color: Theme.textDim; font.pixelSize: Theme.fontSmall }
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12
                                UiSlider {
                                    Layout.fillWidth: true
                                    from: 2000
                                    to: 6500
                                    stepSize: 50
                                    value: manualPage.kelvin
                                    trackGradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop { position: 0.0; color: huenicorn.manualColor("white", 0, 0, 2000) }
                                        GradientStop { position: 0.5; color: huenicorn.manualColor("white", 0, 0, 4000) }
                                        GradientStop { position: 1.0; color: huenicorn.manualColor("white", 0, 0, 6500) }
                                    }
                                    onMoved: function(v) {
                                        manualPage.kelvin = Math.round(v);
                                        applyTimer.restart();
                                    }
                                }
                                Label {
                                    text: manualPage.kelvin + " K"
                                    color: Theme.text
                                    font.pixelSize: Theme.fontSmall
                                    Layout.preferredWidth: 56
                                }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }
    }
}
