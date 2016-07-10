import QtQuick 2.0


Column {
    spacing: 20
    anchors.fill: parent

    Component.onCompleted: {
        console.log("Actions.qml: Completed",new Date());
    }

    Rectangle {height: 20 }

    TextButton {
        text: "Save current session"
        expand: false
        font.pointSize: 16
        onClicked: {
            cloudHide();
            if (cloud.askDialog("Do you confirm your upload ?",2)==2) return;
            save_pml("un titre provisoire","une description provisoire");
            cloudShow();
            renderArea.showWorkingScreen();

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
