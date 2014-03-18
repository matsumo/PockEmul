import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    width: 360
    height: 640
    color: "#d9d3b5"

    Text {
        id: text
        text: "Click and hold here"
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: container.height / 4 }

        MouseArea {
            anchors.fill: parent
            // Trigger showing the note by calling its show() function.
            onClicked: {
                console.log("Clicked at " + mouseX + ", " + mouseY);
                popup.text = "This is a popup";
                popup.posX = text.x + mouseX + text.width/2;
                popup.posY = text.y + mouseY;
                popup.show();
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

    Comp.PopUp {
        id: popup
        onClicked: {
            console.log("Popup clicked")
        }
    }
}
