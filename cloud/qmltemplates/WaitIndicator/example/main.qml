import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    width: 360
    height: 640
    color: "#d9d3b5"

    property bool loading: false

    Text {
        text: "Click here"
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: container.height / 4 }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                container.loading = true;
                timer.running = true;
            }
        }
    }

    Text {
        text: "Exit"
        anchors { right: container.right; top: container.top; rightMargin: 10; topMargin: 10; }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                Qt.quit();
            }
        }
    }

    // Timer, that simulates a loading process.
    Timer {
      id: timer

      interval: 5000
      running: false
      repeat: false
      onTriggered: { container.loading = false; console.log("WaitIndicator should be stopped now.")}
    }

    // Wait indicator is not visible by default, only when container.loading == true.
    Comp.WaitIndicator {
        id: waitIndicator

        // Always define the size for the WaitIndicator.
        anchors.fill: parent

        // Start showing the WaitIndicator immediately.
        // Otherwise it'll wait for one second before starting the indicator.
        delay: 0

        // Make sure it's on top of everything, so that the
        // Fader will capture key clicks
        z: 120

        // In real application, the WaitIndicator could be bound to
        // some "loading" appState variable etc.
        show: container.loading
    }
}
