import QtQuick 2.0


Rectangle {
    width: 500
    height: 200
    color: "black"

    Component {
            id: menuDelegate
            Item {
                width: grid.cellWidth; height: grid.cellHeight
                Column {
                    anchors.fill: parent
                    Image { source: portrait; anchors.horizontalCenter: parent.horizontalCenter }
                    Text { text: name; anchors.horizontalCenter: parent.horizontalCenter }
                }
            }
        }

    ListModel {
        id: gridModel
        ListElement {
            name: "Private Cloud"
            portrait: "../TabbedQuickApp/pics/private-cloud-white.png"
        }
        ListElement {
            name: "Public Cloud"
            portrait: "../TabbedQuickApp/pics/public-cloud-white.png"
        }
        ListElement {
            name: "Action"
            portrait: "../TabbedQuickApp/pics/action-white.png"
        }
        ListElement {
            name: "Private Cloud"
            portrait: "../TabbedQuickApp/pics/settings-white.png"
        }
        ListElement {
            name: "Private Cloud"
            portrait: "../TabbedQuickApp/pics/back-white.png"
        }
    }

    GridView
    {
        id: grid
        anchors.fill: parent
        //    cellWidth: (parent.width)/2;
        cellHeight: cellWidth;
        width: parent.width; height: parent.height
         highlight: Rectangle { color: "lightsteelblue"; radius: 5 }
        model: gridModel
        delegate: menuDelegate


    }

}
