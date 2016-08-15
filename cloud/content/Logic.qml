import QtQuick 2.3
import QtQuick.Layouts 1.1

Rectangle {
    id: root
    anchors.fill: parent
    color: "black"

    property alias source : imageLogic.source
    property alias watchPointModel : watchPointModel

    Rectangle {
        anchors.top: parent.top
        anchors.bottom: lowLayout.top
        anchors.left: parent.left
        anchors.right: listLayout.left
        anchors.margins: 5

        Image {
            id: imageLogic

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
    ColumnLayout {
        id: listLayout
        anchors.top: parent.top
        anchors.bottom: lowLayout.top
        anchors.right: parent.right

        width: 300
        anchors.margins: 5
        spacing: 5
        Text {
            text: "Devices :"
            color: "white"
            font { family: "Helvetica"; pointSize: 16; bold: false }
            renderType: Text.NativeRendering
        }

        Rectangle {
            id: pocketList
            Layout.fillWidth: true
            Layout.fillHeight: true

            color: "white"
            Component {
                id: wpDelegate
                Rectangle {
                    id: wrapper
                    width: pocketListView.width
                    height: childrenRect.height
                    color: "transparent"
                    Text {
                        text: wpName
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 14; bold: false }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            console.log("clic",index);
//                            wrapper.ListView.view.currentIndex = index;
                            pocketListView.currentIndex = index;
                            logicObj.slotChangeWatchPoint(wpIndex);
                        }
                    }
                }
            }

            ListView {
                id: pocketListView
                anchors.fill: parent
                clip: true
                model: watchPointModel
                highlight: Rectangle {color: "red"; radius: 5 }
                delegate: wpDelegate
                section.property: "name"
                section.criteria: ViewSection.FullString
                section.delegate:
                    Rectangle {
                    width: pocketListView.width
                    height: childrenRect.height
                    color: "black"
                    Text {
                        text: section
                        color: "white"
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 16; bold: true }
                    }
                }
                onCurrentIndexChanged: console.log("current:",currentIndex)
            }
        }
    }
    Row {
        id: lowLayout
        anchors.bottom: parent.bottom
        height: 50
        anchors.margins: 5
        spacing: 5

        Text {
            id: labelElement
//            anchors.top: parent.top
//            anchors.bottom: parent.bottom
//            anchors.verticalCenter: parent.verticalCenter

            text: "Capture :"
            color: "white"
            font { family: "Helvetica"; pointSize: 16; bold: false }
            renderType: Text.NativeRendering
        }

        Switch {
            id: captureSwitch
//            anchors.left: labelElement.right
//            anchors.top: parent.top
//            anchors.bottom: parent.bottom
//            anchors.margins: 5
            objectName: "Capture"
            saveToSettings: false
            bheight: 50 //buttonElement.height*.9
            bwidth: 50 * 2
            on: false
            onToggleState: {
                logicObj.updatecapture(value === "on" ? 2:0);
                console.log("toggle");
            }
        }
        Rectangle {
            width: 100
        }

        TextButton {
            text: "Zoom +"
            expand: false
            font.pointSize: 16
            onClicked: {
                logicObj.zoomin();
            }
        }
        TextButton {
            text: "Zoom -"
            expand: false
            font.pointSize: 16
            onClicked: {
                logicObj.zoomout();
            }
        }
        Rectangle {
            width: 200
        }

        TextButton {
            text: "Fit Markers"
            expand: false
            font.pointSize: 16
            onClicked: {
                logicObj.fitmarkers();
            }
        }

    }

    Component.onCompleted: {
        console.log("size",width,height);
    }

    ListModel {
        id: watchPointModel
    }
}
