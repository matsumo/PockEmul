import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    property bool on: true
    width: 360
    height: 640
    color: "#d9d3b5"

    Text {
        id: info
        text: "Tap on the switch to toggle it " + (on ? "OFF" : "ON")
        anchors { horizontalCenter: parent.horizontalCenter; bottom: toggle.top; }
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

    Column {
        id: toggle
        width: 260
        anchors.centerIn: parent
        spacing: 8
        Item {
            height: 40
            anchors.left: parent.left
            anchors.right: parent.right
            Text {
                id: switchLabel
                anchors.left: parent.left
                anchors.right: themeSwitch.left
                height: parent.height
                text: qsTr("Property")
                color: "#666666"
                verticalAlignment: Text.AlignVCenter
            }
            Comp.Switch {
                id: themeSwitch
                anchors.right: parent.right
                height: parent.height
                spacing: 8
                textOn: qsTr("On")
                textOff: qsTr("Off")
                fontColor: "#666666"
                onSwitched: container.on = position
            }
        }
    }
}
