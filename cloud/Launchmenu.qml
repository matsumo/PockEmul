import QtQuick 2.0

Rectangle {
    id:menu
    x:5
    anchors.fill: parent
    color: "black"
//    border.color: "white"
//    border.width: 1
    property int iconsize: 48 * cloud.getValueFor("hiResRatio","1")
    property alias currentIndex : menuListView.currentIndex

    property bool android : Qt.platform.os==='android'

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
            "Fit to screen":    function(){ fit() ;},
            "Toggle Fullscreen":function(){ toggleFullscreen() ;},
            "Logic Analyser":   function(){ logic.visible = true; /*analyser() ;*/},
            "Help":             function(){ about.visible = true; },
            "Check for Updates":function(){ sendCheck(); },
            "Exit":             function(){ sendExit(); }
        }
        property var visible : {
            "New Pocket":       true,
            "New Extension":    true,
            "Settings":         true,
            "Code Editor":      !android,
            "Save":             true,
            "Load":             true,
            "Cloud":            true,
            "Bookcase":         true,
            "Close All":        true,
            "Fit to screen":    true,
            "Toggle Fullscreen":!android,
            "Logic Analyser":   !android,
            "Help":             true,
            "Check for Updates":true,
            "Exit":             true
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
            libelle: "Load"
            imageName: "qrc:/core/load.png"
        }
        ListElement {
            libelle: "Save"
            imageName: "qrc:/core/save.png"
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
            libelle: "Code Editor"
            imageName: "qrc:/core/dev.png"
        }
        ListElement {
            libelle: "Logic Analyser"
            imageName: "qrc:/core/analyser.png"
        }
        ListElement {
            libelle: "Close All"
            imageName: "qrc:/core/close.png"
        }
        ListElement {
            libelle: "Fit to screen"
            imageName: "qrc:/core/fit_to_width.png"
        }
        ListElement {
            libelle: "Toggle Fullscreen"
            imageName: "qrc:/core/expand.png"
        }

        ListElement {
            libelle: "Settings"
            imageName: "qrc:/TabbedQuickApp/pics/settings-white.png"
            separatorBefore: true
        }
        ListElement {
            libelle: "Help"
            imageName: "qrc:/TabbedQuickApp/pics/help.png"
        }
        ListElement {
            libelle: "Check for Updates"
            imageName: "qrc:/core/check.png"
        }
        ListElement {
            libelle: "Exit"
            imageName: "qrc:/core/exit.png"
        }
    }

    Component {
        id: menuDelegate

        Item {
            id: itemDelegate
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
                        text:
                            libelle
//                            (itemDelegate.activeFocus ? "I have active focus!" : "I do not have active focus")
                        color: "white"
                        height:menu.iconsize
                        verticalAlignment: Text.AlignVCenter
                        font { family: "Helvetica"; pointSize: 14; bold: false }
                    }
                }
            }

            function select() {
                nav.hide();
                menuModel.actions[libelle]();
            }

            MouseArea {
                enabled: nav.open
                anchors.fill: parent
                onClicked: itemDelegate.select()
            }
            Keys.onPressed: {
                    if (event.key === Qt.Key_Return) {
                        console.log('Key Return was pressed');
                        select();
                        event.accepted = true;
                    }
                    if ((event.key === Qt.Key_Escape) ||
                        (event.key === Qt.Key_Backspace) ) {
                        console.log('Key Escape was pressed');
                        nav.hide();
                        event.accepted = true;
                    }
                }

        }
    }


    ListView {
        id: menuListView
        anchors.fill: parent
        model: menuModel
        focus: menu.focus
        spacing: 5
        delegate: menuDelegate
        highlight: Rectangle { color: "darkslategrey"; radius: 5 }

    }

    Component.onCompleted: {
        // fetch listelement and apply filter

        for (var i=0; i < menuModel.count; i++) {
            var visible = menuModel.visible[menuModel.get(i).libelle];
//            console.log(menuModel.get(i).libelle,visible);
            if (!visible) {
                menuModel.remove(i);
                i--;
            }
        }
    }
}

