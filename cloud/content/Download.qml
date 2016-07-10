import QtQuick 2.0
import QtQuick.Controls 1.4

Item {
    property alias minimumValue: progressbar.minimumValue
    property alias maximumValue: progressbar.maximumValue
    property alias value: progressbar.value

//    spacing: 10
    height: cancelPB.height
    ProgressBar {
        id:progressbar
        anchors.left: parent.left
        anchors.right: cancelPB.left
        height: cancelPB.height
        minimumValue: 0
        maximumValue: 100
        value: 50
    }
    TextButton {
        id: cancelPB
        anchors.right: parent.right
        expand: false
        text: "Cancel"
        color: "white"

    }

    Component.onCompleted: {
        console.log("Download.qml: Completed");
    }
}
