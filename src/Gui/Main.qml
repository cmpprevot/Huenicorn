import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1200
    height: 760
    minimumWidth: 960
    minimumHeight: 600
    visible: true
    title: "Huenicorn"
    color: Theme.backgroundDeep

    // ---- ambient background: deep gradient + two drifting glow blobs ----
    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Theme.backgroundDeep }
            GradientStop { position: 0.6; color: Theme.background }
            GradientStop { position: 1.0; color: Theme.backgroundAlt }
        }
    }
    GlowBlob {
        id: blobViolet
        x: -width * 0.25
        y: -height * 0.35
        width: root.width * 0.75
        height: width
        glowColor: "#8b5cf6"
        intensity: 0.20
    }
    GlowBlob {
        id: blobCyan
        x: root.width * 0.72
        y: root.height * 0.62
        width: root.width * 0.62
        height: width
        glowColor: "#22d3ee"
        intensity: 0.13
    }

    // very slow drift so the background feels alive
    SequentialAnimation {
        running: true
        loops: Animation.Infinite
        NumberAnimation { target: blobViolet; property: "x"; to: root.width * 0.02; duration: 16000; easing.type: Easing.InOutSine }
        NumberAnimation { target: blobViolet; property: "x"; to: -blobViolet.width * 0.25; duration: 16000; easing.type: Easing.InOutSine }
    }
    SequentialAnimation {
        running: true
        loops: Animation.Infinite
        NumberAnimation { target: blobCyan; property: "y"; to: root.height * 0.55; duration: 20000; easing.type: Easing.InOutSine }
        NumberAnimation { target: blobCyan; property: "y"; to: root.height * 0.62; duration: 20000; easing.type: Easing.InOutSine }
    }

    Loader {
        id: pageLoader
        anchors.fill: parent
        sourceComponent: huenicorn.setupRequired ? setupPageComponent : mainPageComponent
    }

    Component {
        id: setupPageComponent
        SetupPage {}
    }

    Component {
        id: mainPageComponent
        MainPage {}
    }
}
