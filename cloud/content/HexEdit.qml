import QtQuick 2.0
import HexEditor 1.0

Item {
    id: root
    property HexModel hexModel
    property int visibleLines : 16
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
        ListView {
            id: line
            property int normalWidth : line.width / (HexModel.LineSize + 2)
            property int addressWidth : normalWidth * 2

            height: lineHeight
            width: hexViewer.width
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
                radius: 3

                Text {
                    anchors.centerIn: parent
                    text: lineData[index]
                    font.pixelSize: Math.min(parent.height / 2, parent.width / 2)
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
