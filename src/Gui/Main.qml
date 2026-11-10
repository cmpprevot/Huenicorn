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
    // Client-side decorations: the generic Wayland/X11 frame is replaced by
    // the custom title bar below; the taskbar icon is set in main.cpp via
    // QGuiApplication::setWindowIcon (the Huenicorn logo)
    flags: Qt.FramelessWindowHint

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
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: titleBar.bottom
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

    // ---- custom title bar ----
    Rectangle {
        id: titleBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 44
        color: Theme.surface
        border.color: Theme.border
        border.width: 0
        z: 10

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 1
            color: Theme.border
        }

        RowLayout {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: 12
            spacing: 10

            Image {
                source: "qrc:/qt/qml/huenicorn_UI/logo.svg"
                sourceSize.width: 22
                sourceSize.height: 22
                fillMode: Image.PreserveAspectFit
            }
            Label {
                text: "Huenicorn"
                color: Theme.text
                font.pixelSize: 14
                font.bold: true
            }
            Label {
                text: "v" + huenicorn.version
                color: Theme.textFaint
                font.pixelSize: Theme.fontSmall
            }
        }

        // drag area (declared BEFORE the buttons so the buttons stay on top
        // and receive their own clicks)
        MouseArea {
            anchors.fill: parent
            anchors.rightMargin: 130 // leave room for the window controls
            onPressed: root.startSystemMove()
            onDoubleClicked: root.visibility === Window.Maximized ? root.showNormal() : root.showMaximized()
        }

        // window controls
        RowLayout {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 0

            // minimize
            Item {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 44
                Rectangle {
                    anchors.fill: parent
                    color: minMa.containsMouse ? Theme.surfaceHover : "transparent"
                }
                Rectangle {
                    anchors.centerIn: parent
                    width: 11
                    height: 2
                    radius: 1
                    color: minMa.containsMouse ? Theme.text : Theme.textDim
                }
                MouseArea {
                    id: minMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.showMinimized()
                }
            }
            // maximize / restore
            Item {
                Layout.preferredWidth: 42
                Layout.preferredHeight: 44
                Rectangle {
                    anchors.fill: parent
                    color: maxMa.containsMouse ? Theme.surfaceHover : "transparent"
                }
                Rectangle {
                    anchors.centerIn: parent
                    width: 11
                    height: 11
                    color: "transparent"
                    border.color: maxMa.containsMouse ? Theme.text : Theme.textDim
                    border.width: 2
                    radius: 1
                }
                MouseArea {
                    id: maxMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.visibility === Window.Maximized ? root.showNormal() : root.showMaximized()
                }
            }
            // close
            Item {
                Layout.preferredWidth: 46
                Layout.preferredHeight: 44
                Rectangle {
                    anchors.fill: parent
                    color: closeMa.containsMouse ? Theme.danger : "transparent"
                }
                Item {
                    anchors.centerIn: parent
                    width: 12
                    height: 12
                    Rectangle {
                        anchors.centerIn: parent
                        width: 13
                        height: 2
                        radius: 1
                        color: closeMa.containsMouse ? "#ffffff" : Theme.textDim
                        rotation: 45
                    }
                    Rectangle {
                        anchors.centerIn: parent
                        width: 13
                        height: 2
                        radius: 1
                        color: closeMa.containsMouse ? "#ffffff" : Theme.textDim
                        rotation: -45
                    }
                }
                MouseArea {
                    id: closeMa
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.close()
                }
            }
        }
    }

    // ---- resize grips (frameless window has no WM borders) ----
    Item {
        anchors.fill: parent
        z: 100
        visible: root.visibility === Window.Windowed

        Repeater {
            model: [
                { edges: Qt.TopEdge,    x: 12, y: 0, w: root.width - 24, h: 6,  cursor: Qt.SizeVerCursor },
                { edges: Qt.BottomEdge, x: 12, y: root.height - 6, w: root.width - 24, h: 6, cursor: Qt.SizeVerCursor },
                { edges: Qt.LeftEdge,   x: 0, y: 12, w: 6, h: root.height - 24, cursor: Qt.SizeHorCursor },
                { edges: Qt.RightEdge,  x: root.width - 6, y: 12, w: 6, h: root.height - 24, cursor: Qt.SizeHorCursor },
                { edges: Qt.TopEdge | Qt.LeftEdge,     x: 0, y: 0, w: 12, h: 12, cursor: Qt.SizeFDiagCursor },
                { edges: Qt.TopEdge | Qt.RightEdge,    x: root.width - 12, y: 0, w: 12, h: 12, cursor: Qt.SizeBDiagCursor },
                { edges: Qt.BottomEdge | Qt.LeftEdge,  x: 0, y: root.height - 12, w: 12, h: 12, cursor: Qt.SizeBDiagCursor },
                { edges: Qt.BottomEdge | Qt.RightEdge, x: root.width - 12, y: root.height - 12, w: 12, h: 12, cursor: Qt.SizeFDiagCursor }
            ]
            delegate: MouseArea {
                x: modelData.x
                y: modelData.y
                width: modelData.w
                height: modelData.h
                cursorShape: modelData.cursor
                onPressed: root.startSystemResize(modelData.edges)
            }
        }
    }
}
