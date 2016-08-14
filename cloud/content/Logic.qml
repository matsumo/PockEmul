import QtQuick 2.3

Rectangle {
    id: root
    anchors.fill: parent
    color: "black"

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: lowLayout.top
        anchors.left: parent.left
        anchors.right: watchPointsList.left
        anchors.margins: 5

        Image {
            id: image

            fillMode: Image.Stretch
            source: "image://Logic/"
            sourceSize.width: parent.width
            sourceSize.height: parent.height
            scale: 1
            smooth: true
            mipmap: true
            antialiasing: true
            cache: false;
        }
    }
    Rectangle {
        id: watchPointsList
        width: 300
        height: parent.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: lowLayout.top
        anchors.margins: 5

        color: "white"
    }
    Rectangle {
        id: lowLayout
        anchors.bottom: parent.bottom
        height: 50
        anchors.margins: 5

        Text {
            id: labelElement
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.verticalCenter: parent.verticalCenter

            text: "Capture :"
            color: "white"
            font { family: "Helvetica"; pointSize: 16; bold: false }
            renderType: Text.NativeRendering
        }

        Switch {
            id: captureSwitch
            anchors.left: labelElement.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.margins: 5
            objectName: "Capture"
            saveToSettings: false
            bheight: 50 //buttonElement.height*.9
            bwidth: 50 * 2
            on: false
            onToggleState: console.log("toggle");

        }
    }

    Component.onCompleted: {
        console.log("size",width,height);
    }

}
