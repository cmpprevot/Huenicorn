import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Screen preview with draggable per-channel UV zones and gamma slider
ColumnLayout {
    id: screenArea
    spacing: 12

    property real aspect: huenicorn.displayWidth > 0 && huenicorn.displayHeight > 0
        ? huenicorn.displayWidth / huenicorn.displayHeight
        : 16 / 9

    property var selectedChannel: {
        huenicorn.channelsVersion; // re-evaluate when channels are reloaded
        if (huenicorn.selectedChannelId < 0) {
            return {};
        }
        return huenicorn.channelData(huenicorn.selectedChannelId);
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true

        // soft frame glow around the preview
        Rectangle {
            anchors.centerIn: screen
            width: screen.width + 18
            height: screen.height + 18
            radius: Theme.radiusSmall + 10
            color: Theme.glowAccent
            opacity: 0.35
            z: -1
        }

        Rectangle {
            id: screen
            anchors.centerIn: parent
            width: Math.min(parent.width, parent.height * screenArea.aspect)
            height: width / screenArea.aspect
            color: "#04060a"
            border.color: Theme.borderStrong
            border.width: 1
            radius: Theme.radiusSmall

            // subtle inner vignette
            Rectangle {
                anchors.fill: parent
                radius: parent.radius - 1
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.03) }
                    GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.25) }
                }
            }

            // grid
            Repeater {
                model: 7
                delegate: Rectangle {
                    x: (index + 1) / 8 * screen.width
                    width: 1
                    height: screen.height
                    color: Theme.border
                    opacity: 0.45
                }
            }
            Repeater {
                model: 7
                delegate: Rectangle {
                    y: (index + 1) / 8 * screen.height
                    width: screen.width
                    height: 1
                    color: Theme.border
                    opacity: 0.45
                }
            }

            // Click on the preview: select the smallest zone containing the
            // point (so a small zone stays selectable even when covered by a
            // larger one). Declared before the zones so handle MouseAreas
            // keep their drags.
            MouseArea {
                anchors.fill: parent
                onClicked: function(mouse) {
                    var px = mouse.x / screen.width;
                    var py = mouse.y / screen.height;
                    var bestId = -1;
                    var bestArea = Infinity;
                    var count = huenicorn.channels.count;
                    for (var i = 0; i < count; ++i) {
                        var c = huenicorn.channels.get(i);
                        if (!c.active) continue;
                        if (px < c.uvMinX || px > c.uvMaxX || py < c.uvMinY || py > c.uvMaxY) continue;
                        var area = (c.uvMaxX - c.uvMinX) * (c.uvMaxY - c.uvMinY);
                        if (c.channelId === huenicorn.selectedChannelId || area < bestArea) {
                            bestId = c.channelId;
                            bestArea = area;
                        }
                    }
                    if (bestId >= 0) {
                        huenicorn.selectedChannelId = bestId;
                    }
                }
            }

            // channel zones
            Repeater {
                model: huenicorn.channels
                delegate: Rectangle {
                    id: zone
                    property int channelId: model.channelId
                    property bool selected: huenicorn.selectedChannelId === channelId

                    visible: model.active
                    z: selected ? 1 : 0
                    x: model.uvMinX * screen.width
                    y: model.uvMinY * screen.height
                    width: Math.max(0, (model.uvMaxX - model.uvMinX) * screen.width)
                    height: Math.max(0, (model.uvMaxY - model.uvMinY) * screen.height)
                    radius: 6
                    color: Theme.channelColor(channelId)
                    opacity: selected ? 0.45 : 0.15
                    border.color: Theme.channelColor(channelId)
                    border.width: selected ? 2 : 1

                    // soft halo around the zone
                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width + 16
                        height: parent.height + 16
                        radius: parent.radius + 8
                        color: parent.color
                        opacity: zone.selected ? 0.16 : 0.05
                    }

                    // corner handles (only for the selected channel)
                    Repeater {
                        model: [
                            { corner: 0, hx: 0.0, hy: 0.0 },
                            { corner: 1, hx: 1.0, hy: 0.0 },
                            { corner: 2, hx: 0.0, hy: 1.0 },
                            { corner: 3, hx: 1.0, hy: 1.0 }
                        ]
                        delegate: Item {
                            visible: zone.selected
                            width: 18
                            height: 18
                            x: modelData.hx * zone.width - width / 2
                            y: modelData.hy * zone.height - height / 2

                            Rectangle {
                                anchors.fill: parent
                                anchors.margins: -4
                                radius: (width + 8) / 2
                                color: Theme.shadow
                            }
                            Rectangle {
                                anchors.centerIn: parent
                                width: 16
                                height: 16
                                radius: 8
                                color: Theme.channelColor(zone.channelId)
                                border.color: "#ffffff"
                                border.width: 2
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.SizeAllCursor
                                onClicked: huenicorn.selectedChannelId = zone.channelId
                                onPositionChanged: function(mouse) {
                                    var p = mapToItem(screen, mouse.x, mouse.y);
                                    huenicorn.setChannelUV(zone.channelId, modelData.corner,
                                                           p.x / screen.width, p.y / screen.height);
                                }
                            }
                        }
                    }
                }
            }

            // overlay legend chip
            Rectangle {
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.topMargin: 10
                radius: Theme.pill
                color: Qt.rgba(4, 6, 10, 0.72)
                border.color: Theme.border
                border.width: 1
                implicitWidth: legendLabel.implicitWidth + 24
                implicitHeight: legendLabel.implicitHeight + 12

                Label {
                    id: legendLabel
                    anchors.centerIn: parent
                    color: Theme.textDim
                    font.pixelSize: Theme.fontSmall
                    text: {
                        if (huenicorn.selectedChannelId < 0) {
                            return huenicorn.channels && huenicorn.channels.rowCount() > 0
                                ? "Select a light on the right to edit its screen zone"
                                : "No channel available for the current entertainment area";
                        }
                        var c = screenArea.selectedChannel;
                        var w = Math.round((c.uvMaxX - c.uvMinX) * 100);
                        var h = Math.round((c.uvMaxY - c.uvMinY) * 100);
                        return c.label + "  ·  " + w + "% x " + h + "% of the screen";
                    }
                }
            }
        }
    }

    // gamma slider for the selected channel
    RowLayout {
        Layout.fillWidth: true
        visible: huenicorn.selectedChannelId >= 0
        spacing: 12

        Label {
            text: "Gamma"
            color: Theme.textDim
            font.pixelSize: Theme.fontBody
        }
        UiSlider {
            Layout.fillWidth: true
            from: 0
            to: 1000
            value: {
                var c = screenArea.selectedChannel;
                if (c.gammaFactor === undefined) {
                    return 500;
                }
                return (c.gammaFactor + 1) / 2 * 1000;
            }
            onMoved: function(v) {
                var c = screenArea.selectedChannel;
                if (c.channelId !== undefined) {
                    huenicorn.setChannelGammaFactor(c.channelId, v / 1000 * 2 - 1);
                }
            }
        }
        Label {
            color: Theme.text
            font.pixelSize: Theme.fontSmall
            Layout.preferredWidth: 40
            text: {
                var c = screenArea.selectedChannel;
                return c.gammaFactor !== undefined ? (Math.round(c.gammaFactor * 100) / 100).toFixed(2) : "";
            }
        }
    }
}
