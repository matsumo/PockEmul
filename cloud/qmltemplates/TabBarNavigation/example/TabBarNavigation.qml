import QtQuick 1.0
import "../component" as Comp

Item {
    id: container
    width: 360
    height: 640

    // App layout: content pane and tab bar at the bottom which changes the content of the content pane
    Item {
        id: contentPane
        clip: true
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: tabBar.top
        }

        // View switcher component, handles the view switching and animation
        Comp.ViewSwitcher {
            id: viewSwitcher
            // Rooted in contentPane
            root: contentPane
        }

        // The actual views
        Comp.ViewLoader {
            id: page1
            viewSource: "Page1.qml"
            keepLoaded: true
        }

        Comp.ViewLoader {
            id: page2
            viewSource: "Page2.qml"
            keepLoaded: true
        }

        Comp.ViewLoader {
            id: page3
            viewSource: "Page3.qml"
            keepLoaded: true
        }
        Comp.ViewLoader {
            id: page4
            viewSource: "Page4.qml"
            keepLoaded: true
        }
    }

    Comp.TabBar {
        id: tabBar

        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        height: 80

        button1Text: qsTr("Page1");
        button2Text: qsTr("Page2");
        button3Text: qsTr("Page3");
        button4Text: qsTr("Page4");

        onTabButtonClicked: {
            console.log("Tab-bar button clicked: " + buttonName);
            if (buttonName == "page1Button") {
                viewSwitcher.switchView(page1,0, "instant");
            } else if (buttonName == "page2Button") {
                viewSwitcher.switchView(page2,0, "instant");
            } else if (buttonName == "page3Button") {
                viewSwitcher.switchView(page3,0, "instant");
            } else if (buttonName == "page4Button") {
                viewSwitcher.switchView(page4,0, "instant");
            }

        }
    }
}
