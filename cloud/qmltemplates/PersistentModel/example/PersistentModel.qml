import QtQuick 1.0
import "../component" as Comp

Item {
    id: container
    width: 360
    height: 640

    Comp.PersistentModel {
        id: model
    }

    ListView {
        anchors.top:  container.top
        anchors.bottom: buttons.top
        anchors.left: container.left
        anchors.right: container.right
        model: model
        delegate: Component {
            Item {
                width: container.width
                height: 20
                Text {
                    color: "black"
                    anchors.fill: parent
                    text: itemName
                }
                Repeater {
                    model: attributes
                    Text {
                        anchors.right:  parent.right
                        color: "blue"
                        text: subItemName
                    }
                }
            }
        }
    }

    Row {
        id: buttons
        anchors.bottom: container.bottom
        width: parent.width
        height: 20
        Rectangle {
            width: parent.width/2
            height: parent.height
            Text {
                color: "green"
                anchors.fill:  parent
                text: "Click here to add a new item"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: model.addItem("New item " + Math.floor(1000*Math.random()), "Sub item "+ Math.floor(1000*Math.random()))
            }
        }
        Rectangle {
            width: parent.width/2
            height: parent.height
            Text {
                color: "red"
                anchors.fill:  parent
                text: "Click here to remove last item"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: model.remove(model.count-1)
            }
        }
    }
}
