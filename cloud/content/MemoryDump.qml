import QtQuick 2.0
import HexEditor 1.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

Rectangle {
    //    id: root


    //    width: 800
    //    height: 600

    //    LoadData { id: load }
    property alias slotListModel : slotList.slotListModel

    color: "black"

    Action {
        id: loadFile
        onTriggered: {
            //            load.loadFile(file.text, hexModel)
            var _item = slotList.slotListModel.get(slotList.slotListView.currentIndex);
            console.log(_item,_item.idpocket,_item.sIndex);
            main.loadSlot(_item.idpocket,_item.sIndex,hexModel);
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

    RowLayout {
        anchors.fill: parent
        anchors.margins: 10

        SlotList {
            id: slotList
            Layout.minimumWidth: 400
            Layout.maximumWidth: 400
        }

        ColumnLayout {

            //        onHeightChanged: console.log("height column",height,parent.height);
            RowLayout {
                width: parent.width
                TextField {
                    id: file
                    Layout.fillWidth: true
                    placeholderText: "file name"
                    onAccepted:  loadFile.trigger()
                }

                TextButton {
                    text: "load"
                    onClicked: loadFile.trigger()
                }
            }

            RowLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                implicitWidth: 10
                implicitHeight: 10
                //            onHeightChanged: console.log("height group",height,parent.height);




                HexEdit {
                    id: editor
                    Layout.fillWidth: true
                    Layout.fillHeight: true
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
                LineInput {
                    id: pattern
                    label: "search: "
                    text: "regular expression pattern"
                    Layout.fillWidth: true
                    //                onAccepted: {
                    //                    searchFirstAction.trigger()
                    //                }
                }

                TextButton {
                    text: "FindFirst"
                    onClicked: searchFirstAction.trigger();
                }
                TextButton {
                    text: "FindNext"
                    onClicked: searchNextAction.trigger();
                }
            }
        }
    }

    function loadData() {

    }
}
