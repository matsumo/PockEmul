import QtQuick 1.0
import "../component" as Comp

Rectangle {
    id: container

    width: 360
    height: 640
    color: "#d9d3b5"

    Text {
        id: info
        text: "Flick the reels to change selected date"
        anchors { horizontalCenter: parent.horizontalCenter; bottom: datePicker.top; }
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
        id: datePicker
        width: 260
        anchors.centerIn: parent

        Row {
            width: parent.width

            Text {
                id: yearText
                width: (parent.width-2*parent.spacing)*0.4
                text: "Year"
                color: "#666666"
            }
            Text {
                id: monthText
                width: (parent.width-2*parent.spacing)*0.3
                text: "Month"
                color: "#666666"
            }
            Text {
                id: dayText
                width: (parent.width-2*parent.spacing)*0.3
                text: "Day"
                color: "#666666"
            }
        }
        Comp.DateReel {
            id: dateReel
            width: parent.width
        }
    }
}
