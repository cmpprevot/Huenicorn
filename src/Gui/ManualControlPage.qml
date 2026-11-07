import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: manualPage

    // Current manual state (mirrors what was last sent to the bridge)
    property bool power: true
    property int brightness: 80
    property string mode: "color" // "color" | "white"
    property int hue: 210
    property int saturation: 100
    property int kelvin: 4000

    // False while Component.onCompleted restores the last saved state, so the
    // checked bindings below don't fire applyManual() on page load
    property bool ready: false

    // Approximate Kelvin -> RGB conversion (Tanner Helland) for the preview swatch
    function kelvinToRgb(k) {
        var t = k / 100.0;
        var r, g, b;
        if(t <= 66){
            r = 255;
            g = 99.4708025861 * Math.log(t) - 161.1195681661;
        }
        else{
            r = 329.698727446 * Math.pow(t - 60, -0.1332047592);
            g = 288.1221695283 * Math.pow(t - 60, -0.0755148492);
        }
        if(t >= 66){
            b = 255;
        }
        else if(t <= 19){
            b = 0;
        }
        else{
            b = 138.5177312231 * Math.log(t - 10) - 305.0447927307;
        }
        function clamp(v){ return Math.max(0, Math.min(255, v)); }
        return Qt.rgba(clamp(r) / 255, clamp(g) / 255, clamp(b) / 255, 1);
    }

    function currentColor() {
        if(mode === "white"){
            return kelvinToRgb(kelvin);
        }
        return Qt.hsla(hue / 360.0, saturation / 100.0, 0.5, 1.0);
    }

    function applyManual() {
        var state = { "on": power, "brightness": brightness, "mode": mode,
                      "hue": hue, "saturation": saturation, "kelvin": kelvin };
        if(mode === "color"){
            var c = Qt.hsla(hue / 360.0, saturation / 100.0, 0.5, 1.0);
            state.r = Math.round(c.r * 255);
            state.g = Math.round(c.g * 255);
            state.b = Math.round(c.b * 255);
        }
        else{
            state.mirek = Math.round(1000000 / kelvin);
        }
        huenicorn.setManualLight(state);
    }

    // Sliders fire continuously while dragged: debounce the bridge requests
    function scheduleApply() {
        applyTimer.restart();
    }

    Timer {
        id: applyTimer
        interval: 250
        onTriggered: manualPage.applyManual()
    }

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
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.spacing
        spacing: Theme.spacing

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Manual light control"
                color: Theme.text
                font.pixelSize: 16
                font.bold: true
                Layout.fillWidth: true
            }
            Rectangle {
                Layout.preferredWidth: 44
                Layout.preferredHeight: 44
                radius: 8
                color: manualPage.currentColor()
                border.color: Theme.border
                border.width: 1
            }
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Theme.textDim
            font.pixelSize: 12
            text: "Applies to all lights of the current entertainment area. Applying a manual state pauses ambilight; resume it from the Ambilight tab."
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "Power"
                color: Theme.text
                Layout.fillWidth: true
            }
            Switch {
                id: powerSwitch
                checked: manualPage.power
                onToggled: {
                    if(!manualPage.ready) return;
                    manualPage.power = checked;
                    manualPage.applyManual();
                }
            }
        }

        Label { text: "Brightness"; color: Theme.textDim; font.pixelSize: 12 }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Slider {
                id: brightnessSlider
                Layout.fillWidth: true
                from: 0
                to: 100
                stepSize: 1
                value: manualPage.brightness
                onMoved: {
                    manualPage.brightness = Math.round(value);
                    manualPage.scheduleApply();
                }
            }
            Label {
                text: manualPage.brightness + " %"
                color: Theme.text
                font.pixelSize: 12
                Layout.preferredWidth: 48
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            RadioButton {
                id: colorModeRadio
                text: "Color"
                checked: manualPage.mode === "color"
                onToggled: if(checked && manualPage.ready){ manualPage.mode = "color"; manualPage.applyManual(); }
            }
            RadioButton {
                id: whiteModeRadio
                text: "White (temperature)"
                checked: manualPage.mode === "white"
                onToggled: if(checked && manualPage.ready){ manualPage.mode = "white"; manualPage.applyManual(); }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            currentIndex: manualPage.mode === "color" ? 0 : 1

            // Color mode: hue + saturation
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing

                Label { text: "Hue"; color: Theme.textDim; font.pixelSize: 12 }
                Slider {
                    id: hueSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 360
                    stepSize: 1
                    value: manualPage.hue
                    onMoved: {
                        manualPage.hue = Math.round(value);
                        manualPage.scheduleApply();
                    }
                    background: Rectangle {
                        x: hueSlider.leftPadding
                        y: hueSlider.availableHeight / 2 - height / 2
                        width: hueSlider.availableWidth
                        height: 8
                        radius: 4
                        color: Theme.surfaceActive
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.00; color: "#ff0000" }
                            GradientStop { position: 0.17; color: "#ffff00" }
                            GradientStop { position: 0.33; color: "#00ff00" }
                            GradientStop { position: 0.50; color: "#00ffff" }
                            GradientStop { position: 0.67; color: "#0000ff" }
                            GradientStop { position: 0.83; color: "#ff00ff" }
                            GradientStop { position: 1.00; color: "#ff0000" }
                        }
                    }
                }

                Label { text: "Saturation"; color: Theme.textDim; font.pixelSize: 12 }
                Slider {
                    id: saturationSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 100
                    stepSize: 1
                    value: manualPage.saturation
                    onMoved: {
                        manualPage.saturation = Math.round(value);
                        manualPage.scheduleApply();
                    }
                    background: Rectangle {
                        x: saturationSlider.leftPadding
                        y: saturationSlider.availableHeight / 2 - height / 2
                        width: saturationSlider.availableWidth
                        height: 8
                        radius: 4
                        color: Theme.surfaceActive
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: Qt.hsla(manualPage.hue / 360.0, 0.0, 0.5, 1.0) }
                            GradientStop { position: 1.0; color: Qt.hsla(manualPage.hue / 360.0, 1.0, 0.5, 1.0) }
                        }
                    }
                }
            }

            // White mode: color temperature
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Theme.spacing

                Label { text: "Color temperature"; color: Theme.textDim; font.pixelSize: 12 }
                Slider {
                    id: kelvinSlider
                    Layout.fillWidth: true
                    from: 2000
                    to: 6500
                    stepSize: 50
                    value: manualPage.kelvin
                    onMoved: {
                        manualPage.kelvin = Math.round(value);
                        manualPage.scheduleApply();
                    }
                    background: Rectangle {
                        x: kelvinSlider.leftPadding
                        y: kelvinSlider.availableHeight / 2 - height / 2
                        width: kelvinSlider.availableWidth
                        height: 8
                        radius: 4
                        color: Theme.surfaceActive
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: manualPage.kelvinToRgb(2000) }
                            GradientStop { position: 0.5; color: manualPage.kelvinToRgb(4000) }
                            GradientStop { position: 1.0; color: manualPage.kelvinToRgb(6500) }
                        }
                    }
                }
                Label {
                    text: manualPage.kelvin + " K"
                    color: Theme.text
                    font.pixelSize: 12
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
