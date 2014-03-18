import QtQuick 1.1

Column {
    spacing: 20
    anchors.fill: parent

    Rectangle {height: 20 }

    TextButton {
        id: saveCurrentSessionButton
        text: "Save current session"
        expand: false
        font.pointSize: 16
        onClicked: {
            save_pml("un titre provisoire","une description provisoire");
        }
    }

    TextButton {
        text: "upload Session File"
        expand: false
        font.pointSize: 16
        onClicked: {
            cloud.showFileDialog();
        }
    }

    TextButton {
        text: "Send message"
        expand: false
        font.pointSize: 16
        onClicked: {
//            cloud.showFileDialog();
        }
    }

//    TextButton {
//        text: "analyse sensors"
//        expand: true
//        font.pointSize: 16
//        onClicked: {
//            var types = Sensors.sensorTypes();
//            text = types.join(", ");

//        }
//    }


//                TextButton {
//                    text: "Create shortcut"
//                    onClicked: cloud.addShortcut("-r PC-1475");
//                }


}
