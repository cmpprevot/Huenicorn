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

    // Target scope of the manual commands: 0 = all lights of the entertainment
    // area, 1 = a single channel (the per-segment scope for gradient strips),
    // 2 = a single light
    property int targetScope: 0
    property int targetChannelId: -1
    property string targetLightId: ""

    // The channel scope only matters when some light is held by several
    // channels (e.g. one segment per channel of a gradient strip); when every
    // light is 1:1 with its channel it is redundant with the light scope
    readonly property bool channelScopeUseful: {
        var lights = huenicorn.manualLights;
        for(var i = 0; i < lights.length; ++i)
            if(lights[i].channelCount > 1) return true;
        return false;
    }

    // Map displayed segmented control indexes to target scope ids; display
    // order is All lights, Light, Channel (Channel offered only when useful)
    function scopeFromIndex(index) {
        if(channelScopeUseful) return index === 1 ? 2 : (index === 2 ? 1 : 0);
        return index === 1 ? 2 : 0;
    }
    function indexFromScope(scope) {
        if(channelScopeUseful) return scope === 1 ? 2 : (scope === 2 ? 1 : 0);
        return scope === 2 ? 1 : 0;
    }

    // Preselect the first channel / light - validated against the current
    // entertainment area (switching areas may invalidate the previous choice) -
    // so that the target of the manual commands is always meaningful
    function ensureDefaultTargets() {
        var channelCount = huenicorn.channels.count;
        var foundChannel = false;
        for(var c = 0; c < channelCount; ++c){
            if(huenicorn.channels.get(c).channelId === targetChannelId){
                foundChannel = true;
                break;
            }
        }
        if(!foundChannel && channelCount > 0)
            targetChannelId = huenicorn.channels.get(0).channelId;

        var lights = huenicorn.manualLights;
        var foundLight = false;
        for(var l = 0; l < lights.length; ++l){
            if(lights[l].lightId === targetLightId){
                foundLight = true;
                break;
            }
        }
        if(!foundLight && lights.length > 0)
            targetLightId = lights[0].lightId;
    }
    Connections {
        target: huenicorn
        function onChannelsChanged() { manualPage.ensureDefaultTargets(); }
    }

    // The channel scope may not be offered in this entertainment area (every
    // light is 1:1 with its channel): fall back to the equivalent light target
    function demoteChannelScope() {
        if(targetScope !== 1 || channelScopeUseful) return;
        var lights = huenicorn.manualLights;
        for(var i = 0; i < lights.length; ++i){
            if(lights[i].channelId === targetChannelId){
                targetScope = 2;
                targetLightId = lights[i].lightId;
                break;
            }
        }
        if(targetScope === 1) targetScope = 0;
        huenicorn.manualTargetChannelId = targetScope === 1 ? targetChannelId : -1;
        huenicorn.manualTargetLightId = targetScope === 2 ? targetLightId : "";
    }
    onChannelScopeUsefulChanged: demoteChannelScope()

    readonly property color currentColor: hexColor !== ""
        ? huenicorn.colorFromHex(hexColor)
        : huenicorn.manualColor(mode, hue, saturation, kelvin)

    // Sends the current state to the bridge (hex value when one is set)
    function applyNow() {
        huenicorn.manualTargetChannelId = manualPage.targetScope === 1 ? manualPage.targetChannelId : -1;
        huenicorn.manualTargetLightId = manualPage.targetScope === 2 ? manualPage.targetLightId : "";
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
            if("lightId" in s && s.lightId !== ""){
                targetScope = 2;
                targetLightId = s.lightId;
            }
            else if("channelId" in s && s.channelId >= 0){
                targetScope = 1;
                targetChannelId = Math.round(s.channelId);
            }
        }
        // The channel scope may not be offered in this entertainment area:
        // fall back to the equivalent light target
        ensureDefaultTargets();
        demoteChannelScope();
        huenicorn.manualTargetChannelId = targetScope === 1 ? targetChannelId : -1;
        huenicorn.manualTargetLightId = targetScope === 2 ? targetLightId : "";
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
            text: (manualPage.targetScope === 0
                ? "Applies to all zones of the current entertainment area."
                : manualPage.targetScope === 1
                    ? "Applies to channel " + (manualPage.targetChannelId + 1) + " only."
                    : "Applies to the selected light only.")
                + (huenicorn.manualMode
                    ? "  Manual mode active: ambilight is suspended; resume it from the Ambilight toggle."
                    : "")
        }

        UiSectionLabel { text: "Target" }
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            SegmentedControl {
                Layout.preferredWidth: 380
                model: manualPage.channelScopeUseful
                    ? ["All lights", "Light", "Channel"]
                    : ["All lights", "Light"]
                currentIndex: manualPage.indexFromScope(manualPage.targetScope)
                onSelected: function(index) {
                    if(!manualPage.ready) return;
                    manualPage.targetScope = manualPage.scopeFromIndex(index);
                    manualPage.ensureDefaultTargets();
                    manualPage.applyNow();
                }
            }

            UiComboBox {
                id: lightCombo
                Layout.fillWidth: true
                visible: manualPage.targetScope === 2
                model: huenicorn.manualLights
                textRole: "name"
                valueRole: "lightId"
                currentIndex: indexOfValue(manualPage.targetLightId)
                onActivated: {
                    manualPage.targetLightId = currentValue;
                    if(manualPage.ready) manualPage.applyNow();
                }
            }

            UiComboBox {
                id: channelCombo
                Layout.fillWidth: true
                visible: manualPage.targetScope === 1
                model: huenicorn.channels
                textRole: "label"
                valueRole: "channelId"
                currentIndex: indexOfValue(manualPage.targetChannelId)
                onActivated: {
                    manualPage.targetChannelId = currentValue;
                    if(manualPage.ready) manualPage.applyNow();
                }
                // show the channel label followed by the names of its lights
                contentItem: Label {
                    text: channelCombo.currentIndex >= 0
                        ? channelCombo.currentText + "  ·  " + huenicorn.channels.get(channelCombo.currentIndex).devices.join(", ")
                        : ""
                    color: Theme.text
                    leftPadding: 12
                    rightPadding: channelCombo.indicator.width + 12
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    font.pixelSize: Theme.fontBody
                }
                delegate: ItemDelegate {
                    id: channelDelegate

                    required property var model
                    required property int index

                    width: channelCombo.width
                    hoverEnabled: true
                    highlighted: channelCombo.highlightedIndex === index

                    background: Rectangle {
                        color: channelDelegate.highlighted ? Theme.surfaceActive
                             : channelDelegate.hovered ? Theme.surfaceHover
                             : "transparent"
                        radius: Theme.radiusSmall - 2
                    }
                    contentItem: Label {
                        text: model.label + "  ·  " + model.devices.join(", ")
                        color: channelDelegate.highlighted ? "#ffffff" : Theme.text
                        font.pixelSize: Theme.fontBody
                        leftPadding: 10
                        rightPadding: 10
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                    }
                }
            }

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
