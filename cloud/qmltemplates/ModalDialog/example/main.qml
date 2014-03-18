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
            onClicked: { dialog.show() }
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

    // Visual is an item that defines some constants for the application
    // look and feel, e.g. the font sizes & colors etc.
    Visual {
        id: visual
    }

    // Example on using ModalDialog component.
    Comp.ModalDialog {
        id: dialog

        // Shown text can be set by modifying "text" property.
        text: "Click OK to Accept this dialog. To send it away, click Cancel."
        // Always remember to define the size for the dialog.
        anchors.fill: parent

        // Demonstrating how one could keep application wide variables saved
        // in one place and use them conveniently all around the application.
        fontName: visual.defaultFontFamily
        fontColor: visual.defaultFontColor
        fontColorButton: visual.defaultFontColorButton
        fontSize: visual.defaultFontSize

        // Use these if you would like to change the Button look and feel.
        // See Visual.qml how these are defined. Remember to modify also the
        // ModalDialog.qml, since this functionality is disabled currently.
//        buttonBackground: visual.buttonComponent
//        buttonBackgroundPressed: visual.buttonPressedComponent

        onAccepted: {
            // When dialog is accepted, e.g. the data model could be updated.
            // Just debug print here to show how to use the ModalDialog signals.
            console.log("Dialog accepted signal received!");
        }
        onCancelled: {
            console.log("Dialog cancelled signal received.")
        }
    }
}
