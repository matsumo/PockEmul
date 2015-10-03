import QtQuick 2.0
import "TabbedQuickApp"
import "content"
import "."
//import QtSensors 5.3 as Sensors

Rectangle {
    width: 360
    height: 640

    id: hx20rc
    signal sendWarning(string test)
    signal close

    property bool   isPortrait: false

    property color backGroundColor: "white"
    property color textColor : "black"
    property color inputBorderColor: "black"
    property color buttonBorderColor: "orange"
    property color textButtonColor: "black"

    property string auth_token: ""

    Component.onCompleted: {
        console.log("start");
    }

    onWidthChanged: {
//        isPortrait = cloud.isPortraitOrientation();
        //        console.log(isPortrait)
    }

    ListModel {
        id: slotModel
        ListElement {
            name: "Slot 1"
            size: 0
        }
        ListElement {
            name: "Slot 2"
            size: 0
        }
        ListElement {
            name: "Slot 3"
            size: 0
        }
        ListElement {
            name: "Slot 4"
            size: 0
        }
        ListElement {
            name: "Slot 5"
            size: 0
        }
    }
    Component {
        id: slotDelegate
        Item {
            width: 200; height: 50
            Text { id: nameSlot; text: name }
            Text { text: 'size :' + size; anchors.left: nameSlot.right }
//            Row {
//                anchors.top: nameSlot.bottom
//                spacing: 5
//                Text { text: "Attributes:" }
//                Repeater {
//                    model: attributes
//                    Text { text: description }
//                }
//            }
        }
    }
    Row {
        Rectangle {
            id: slotView
            width: 220; height: hx20rc.height
            color: "#efefef"

            ListView {
                id: slots
                focus: true
                anchors.fill: parent
                model: slotModel
                clip:true
                header: refreshButtonDelegate
                delegate: slotDelegate
                highlight: Rectangle { color: "steelblue" }
                highlightMoveVelocity: 9999999
            }

            ScrollBar {
                scrollArea: slots; height: slots.height; width: 8
                anchors.right: slots.right
            }
        }

//        ListView {
//            id: list
//            width: hx20rc.width - slotView.width;
//            height: hx20rc.height
//            interactive: true;
//            clip: true
//            model: pmlModel
//            delegate: NewsDelegate2 {}
//        }
    }

    TextButton {
        text: "Close"
        expand: false
        font.pointSize: 16
        onClicked: {
            hx20rc.close();
        }
    }
    Message {
        id:message
        anchors.top: parent.top
        anchors.left: parent.left
        width : parent.width
        height: 30
    }
}
