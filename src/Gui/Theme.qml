pragma Singleton
import QtQuick

// Central design system for the Huenicorn Qt GUI ("Pulse" dark theme).
// All colors, radii, spacing and typography tokens live here so every
// screen and component stays visually consistent.
QtObject {
    // ---- backgrounds -------------------------------------------------
    readonly property color backgroundDeep: "#07090f"
    readonly property color background: "#0b0e16"
    readonly property color backgroundAlt: "#10141f"

    // ---- surfaces (elevation scale) ----------------------------------
    readonly property color surface: "#131826"
    readonly property color surfaceHover: "#1a2133"
    readonly property color surfaceActive: "#232c44"
    readonly property color surfaceElevated: "#1a2133"

    // ---- borders ------------------------------------------------------
    readonly property color border: "#252e47"
    readonly property color borderStrong: "#36436a"

    // ---- text ----------------------------------------------------------
    readonly property color text: "#eef1f8"
    readonly property color textDim: "#9aa4ba"
    readonly property color textFaint: "#646e84"

    // ---- signature gradient (violet -> magenta -> cyan) ---------------
    readonly property color accentA: "#8b5cf6"
    readonly property color accentB: "#d946ef"
    readonly property color accentC: "#22d3ee"

    // ---- semantic ------------------------------------------------------
    readonly property color danger: "#fb7185"
    readonly property color success: "#34d399"

    // translucent overlays used for fake glow / shadow (no effects lib needed)
    readonly property color glowAccent: Qt.rgba(0.55, 0.36, 0.96, 0.35)
    readonly property color shadow: Qt.rgba(0, 0, 0, 0.45)

    // ---- shape ---------------------------------------------------------
    readonly property int radius: 16        // cards
    readonly property int radiusControl: 12 // buttons, inputs
    readonly property int radiusSmall: 8
    readonly property int pill: 999

    // ---- spacing -------------------------------------------------------
    readonly property int spacing: 12
    readonly property int padding: 18

    // ---- typography ------------------------------------------------------
    readonly property int fontTitle: 22
    readonly property int fontHeader: 18
    readonly property int fontBody: 13
    readonly property int fontSmall: 11

    // Deterministic, well-spread per-channel color (golden angle hue
    // progression) used for zone rectangles and list dots.
    function channelColor(channelId) {
        var hue = (channelId * 137.508) % 360;
        if (hue < 0) hue += 360;
        return Qt.hsla(hue / 360.0, 0.85, 0.62, 1.0);
    }
}
