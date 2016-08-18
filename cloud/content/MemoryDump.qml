import QtQuick 2.0
import HexEditor 1.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Item {
    id: root


    width: 800
    height: 600

//    LoadData { id: load }

    Action {
        id: loadFile
        onTriggered: {
//            load.loadFile(file.text, hexModel)
            main.loadSlot(0,0,hexModel);
        }
    }
    Action {
        id: searchFirstAction
        onTriggered: {
            hexModel.search(pattern.text, true)
        }
    }

    Action {
        id: searchNextAction
        onTriggered: {
            hexModel.search(pattern.text, false)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        RowLayout {
            width: parent.width
            TextField {
                id: file
                Layout.fillWidth: true
                placeholderText: "file name"
                onAccepted:  loadFile.trigger()
            }

            Button {
                text: "load"
                action: loadFile
            }
        }

        GroupBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            implicitWidth: 10

            HexEdit {
                id: editor
                anchors.fill: parent
                hexModel: HexModel {id: hexModel}
                visibleLines: 16
            }
        }

        RowLayout {
            CheckBox {
                text: "InsertMode"
                onClicked : {
                    if (checked)
                        hexModel.mode = HexModel.InsertMode
                    else
                        hexModel.mode = HexModel.BrowseMode
                }
            }
            Label{ text: "search: " }
            TextField {
                id: pattern
                placeholderText: "regular expression pattern"
                Layout.fillWidth: true
                onAccepted: {
                    searchFirstAction.trigger()
                }
            }

            Button {
                text: "FindFirst"
                action: searchFirstAction
            }
            Button {
                text: "FindNext"
                action: searchNextAction
            }
        }
    }

    function loadData() {

    }
}
