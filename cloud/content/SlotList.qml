import QtQuick 2.0
import QtQuick.Layouts 1.1

ColumnLayout {
    id: listLayout
//    anchors.top: parent.top
//    anchors.bottom: lowLayout.top
//    anchors.right: parent.right

    property alias slotListModel : slotListModel
    property alias slotListView : slotListView

    width: 300
    height: parent.height
    anchors.margins: 5
    spacing: 5

    Component.onCompleted: slotListView.currentIndex = -1

    Text {
        text: "Devices :"
        color: "white"
        font { family: "Helvetica"; pointSize: 16; bold: false }
        renderType: Text.NativeRendering
    }

    Rectangle {
        id: slotList
        Layout.fillWidth: true
        Layout.fillHeight: true

        color: "white"
        Component {
            id: slDelegate
            Rectangle {
                id: wrapper
                width: slotListView.width
                height: childrenRect.height
                color: "transparent"
                Row {
                    spacing: 20
                    Text {
                        text: sLabel
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 12; bold: false }
                        width: dp(150)
                    }
                    Text {
                        text: sSize
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 10; bold: false }
                        width: dp(25)
                    }
                    Text {
                        text: sStart
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 10; bold: false }
                        width: 75
                    }
                    Text {
                        text: sEnd
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 10; bold: false }
                        width: 75
                    }
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("clic",index);
                        slotListView.currentIndex = index;
                        loadFile.trigger();
                    }
                }
            }
        }

        ListView {
            id: slotListView
            anchors.fill: parent
            clip: true
            model: slotListModel
            highlight: Rectangle {color: "red"; radius: 5 }
            delegate: slDelegate
            section.property: "name"
            section.criteria: ViewSection.FullString
            section.delegate:
                Rectangle {
                width: slotListView.width
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

    ListModel {
        id: slotListModel
    }
}
