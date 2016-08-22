import QtQuick 2.0
import HexEditor 1.0

Item {
    id: root
    property HexModel hexModel
    property int visibleLines : 32
    property int lineHeight: height / visibleLines

    function search(pattern) {
        hexModel.search(pattern,false)
    }

    Connections {
        target: hexModel
        onAddressChange: {
            hexViewer.currentIndex = address / HexModel.LineSize
            hexViewer.positionViewAtIndex(hexViewer.currentIndex, ListView.Contain)
        }
    }

    Component {
        id: drawLine
        Row {
            spacing: 2

            property int normalWidth : line.width / (HexModel.LineSize + 4)
            property int normalWidthChar : linechar.width / (HexModel.LineSize*1.5 + 0)
            property int addressWidth : normalWidth * 3


        ListView {
            id: line
//            anchors.left: parent.left
//            visible: false

            height: lineHeight
            width: hexViewer.width / 2
            orientation: ListView.Horizontal
            interactive: false
            model: lineData
            delegate: Rectangle {
                property bool addressField : index % (HexModel.LineSize + 1) === 0
                property bool selected : (index === hexModel.offset) && (lineData[0] === hexModel.address)

                color: addressField ? "#AAAAAA" : !selected ? "#F7F7F7" : (hexModel.mode === HexModel.InsertMode) ? "black" : "#CCCCCC"
                border.width: 1
                border.color: "#CCCCCC"
                width: addressField ?  addressWidth : normalWidth
                height: lineHeight
//                radius: 3

                Text {
                    id: textData
//                    anchors.centerIn: parent
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom

                    text: lineData[index]
                    font.pixelSize: Math.min(parent.height / 1, parent.width / (addressField ? 6:2))
                    color: selected && (hexModel.mode === HexModel.InsertMode) ? "white" : "black"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked : {
                        hexModel.address = lineData[0]
                        hexModel.offset = index
                        hexViewer.forceActiveFocus()
                    }
                }
            }
        }

        ListView {
            id: linechar

            height: lineHeight
            width: hexViewer.width / 2
            orientation: ListView.Horizontal
            interactive: false
            model: lineChar
            delegate: Rectangle {

                property bool selected : (index === hexModel.offset) && (lineChar[0] === hexModel.address)

//                color:  !selected ? "#F7F7F7" : (hexModel.mode === HexModel.InsertMode) ? "black" : "#CCCCCC"
                color:  selected ? "#F7F7F7" : (hexModel.mode !== HexModel.InsertMode) ? "black" : "#CCCCCC"
                border.width: 0
                border.color: "#CCCCCC"
                width: normalWidthChar
                height: lineHeight
//                radius: 3

                Text {
                    id: textChar
//                    anchors.centerIn: parent
                    anchors.horizontalCenter:  parent.horizontalCenter
                    anchors.bottom: parent.bottom

                    text: lineChar[index]
                    font.pixelSize: Math.min(parent.height / 1, parent.width / 1)
                    font.bold: true
//                    color: selected && (hexModel.mode === HexModel.InsertMode) ? "white" : "black"
                    color: selected && (hexModel.mode === HexModel.InsertMode) ? "black" : "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked : {
//                        hexModel.address = lineChar[0]
                        hexModel.offset = index
                        hexViewer.forceActiveFocus()
                    }
                }
            }
        }

        }

    }

    function showStatusBar() {
        return (hexModel.mode === HexModel.SearchMode) || ((hexModel.mode === HexModel.BrowseMode) && (hexModel.entered.length > 0))
    }

    Column {
        anchors.fill: parent
        ListView {
            id: hexViewer
            width: parent.width
            height: parent.height - lineHeight
            model: hexModel
            clip: true
            delegate : drawLine
            focus: true
            Keys.onPressed: {
                hexModel.keyPress(event.key, event.text)
            }
        }
        Rectangle {
            width: hexViewer.width - HexModel.LineSize
            height: showStatusBar() ? lineHeight * 2/3 : 0
            color: "#777777"
            radius: 5
            clip: true
            Text {
                id: label
                text: hexModel.prompt +  hexModel.entered
                anchors.centerIn: parent
                color: "white"
            }
            Behavior on height {
                NumberAnimation {duration: 100}
            }
        }
    }
}
