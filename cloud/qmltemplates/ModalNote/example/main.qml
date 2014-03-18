import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    width: 360
    height: 640
    color: "#d9d3b5"

    Text {
        text: "Click here"
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: container.height / 4 }

        MouseArea {
            anchors.fill: parent
            // Trigger showing the note by calling its show() function.
            onClicked: { note.show() }
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

    // Example on using ModalNote component. You can define e.g. the text to be
    // shown and the time, how long the note will stay on screen.
    Comp.ModalNote {
        id: note

        // Set the note to fill the whole area. The width & height should
        // always be replaced when creating a ModalNote.
        anchors.fill: parent

        // Define the shown text on the note.
        text: "This is an important note!"

        // Show the note for 3 seconds
        noteShowTime: 3000
    }
}
