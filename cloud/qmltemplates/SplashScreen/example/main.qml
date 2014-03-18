import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    width: 360
    height: 640
    color: "#d9d3b5"

    Text {
        text: "Exit"
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: container.height / 4 }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                Qt.quit();
            }
        }
    }

    // Splash screen is full screen but visible only at the start
    Comp.SplashScreen {
        id: splashScreen

        // The size should always be defined.
        anchors.fill: container

        // Put splash screen on top of everything. Otherwise you would need to lay it out as the last objects
        // in the layout for it to be on top of all other objects. But with z-property we get same results.
        z: 100

        // By default splash screen is not visible.
        opacity: 0.0

        // Splash screen timeout.
        timeout: 2500

        // Define our own graphics to be used instead of the default
        // one in SplashScreen (although these are exactly the same :).
        image: "gfx/splash_screen.png"

        // When splash screen times out, do something, e.g. change the state.
	// Here's just a debug print for testing purposes.
        onSplashTimeout: {
            console.log("Splash Screen finished");
        }

        // Trigger the SplashScreen!
        show: true
    }
}
