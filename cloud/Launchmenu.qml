import QtQuick 2.0

Rectangle {
    id:menu
    x:5
    anchors.fill: parent
    color: "black"
//    border.color: "white"
//    border.width: 1
    property int iconsize: 48 * cloud.getValueFor("hiResRatio","1")

    ListModel {
        id: menuModel
        property var actions : {
            "New Pocket":       function(){ showroomNew.visible = true; },
            "New Extension":    function(){ showroomExt.connectorsearch='';showroomExt.visible = true; },
            "Settings":         function(){ settings.visible = true; },
            "Code Editor":      function(){ sendDev(); },
            "Save":             function(){ sendSave(); },
            "Load":             function(){ sendLoad(); },
            "Cloud":            function(){ nav.hide(); cloudShow(); },
            "Bookcase":         function(){ sendBook(); },
            "Close All":        function(){ sendCloseAll(); },
            "About PockEmul":   function(){ about.visible = true; },
            "Check for Updates":function(){ sendCheck(); },
            "Exit":             function(){ sendExit(); }
        }
        ListElement {
            libelle: "New Pocket"
            imageName: "qrc:/core/pocket.png"
        }
        ListElement {
            libelle: "New Extension"
            imageName: "qrc:/core/ext.png"
        }
        ListElement {
            libelle: "Code Editor"
            imageName: "qrc:/core/dev.png"
        }
        ListElement {
            libelle: "Save"
            imageName: "qrc:/core/save.png"
        }
        ListElement {
            libelle: "Load"
            imageName: "qrc:/core/load.png"
        }
        ListElement {
            libelle: "Cloud"
            imageName: "qrc:/core/cloud-white.png"
        }
        ListElement {
            libelle: "Bookcase"
            imageName: "qrc:/core/bookcase.png"
        }
        ListElement {
            libelle: "Close All"
            imageName: "qrc:/core/close.png"
        }
        ListElement {
            libelle: "Settings"
            imageName: "qrc:/TabbedQuickApp/pics/settings-white.png"
            separatorBefore: true
        }
        ListElement {
            libelle: "About PockEmul"
            imageName: "qrc:/TabbedQuickApp/pics/white-about-256.png"
        }
        ListElement {
            libelle: "Check for Updates"
            imageName: ""
        }
        ListElement {
            libelle: "Exit"
            imageName: "qrc:/core/exit.png"
        }
    }

    Component {
        id: menuDelegate
        Item {
            width: menu.width
            height: image.height + (separatorBefore ? separator.height : 0)
            Column {
                spacing: 0
                Rectangle {
                    id: separator
                    visible : separatorBefore
                    height: 10
                    width: menu.width*.9
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "white"
                    border.color: "black"
                    border.width: 4
                }

                Row {
                    id: row
                    spacing: 10
                    Image {
                        id: image
                        source: imageName
                        width:menu.iconsize
                        height:menu.iconsize
                    }
                    Text {
                        text: libelle
                        color: "white"
                        font { family: "Helvetica"; pointSize: 14; bold: false }
                    }
                }
            }

            MouseArea {
                enabled: nav.open
                anchors.fill: parent
                onClicked: {
                    nav.hide();
                    menuModel.actions[libelle]();
                }
            }
        }
    }


    ListView {
        anchors.fill: parent
        model: menuModel
        delegate: menuDelegate

    }
}

