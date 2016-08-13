import QtQuick 2.0
import QtQuick.XmlListModel 2.0
import "content"
import "."
//import QtSensors 5.3 as Sensors

Rectangle {
    width: 360
    height: 640

    id: em
    signal sendWarning(string test)
    signal close

    property bool   isPortrait: false

    property color backGroundColor: "white"
    property color textColor : "black"
    property color inputBorderColor: "black"
    property color buttonBorderColor: "orange"
    property color textButtonColor: "black"

    property string auth_token: ""

    Component.onCompleted: {
        console.log("start");
    }

    onWidthChanged: {
//        isPortrait = cloud.isPortraitOrientation();
        //        console.log(isPortrait)
    }

    XmlListModel {
        id: xmlslotModel
//        source: "qrc:/hx20/hx20rcslots.xml"
        xml: hx20rc.serializeEprom()
        query: "/slots/slotitem"
        XmlRole { name: "id"; query: "@id/number()"; }
        XmlRole { name: "status"; query: "@status/string()" }
        XmlRole { name: "name"; query: "@name/string()" }
        XmlRole { name: "ext"; query: "@ext/string()" }

            // 00 - BASIC   program
            // 01 - BASIC  data
            // 02 - machine code program
        XmlRole { name: "progtype"; query: "@progtype/number()" }
            // 00 - binary file
            // FF - ASCII file
        XmlRole { name: "datatype"; query: "@progtype/number()" }
        XmlRole { name: "startadr"; query: "@startadr/string()" }
        XmlRole { name: "endadr"; query: "@endadr/string()" } // end adr +1 (4digit)
        XmlRole { name: "date"; query: "@date/string()" }    // creation date 6 digits
    }

    ListModel {
        id: slotModel
        ListElement {
            name: "Slot 1"
            statut: 0
        }
        ListElement {
            name: "Slot 2"
            statut: 0
        }
        ListElement {
            name: "Slot 3"
            statut: 0
        }
        ListElement {
            name: "Slot 4"
            statut: 0
        }
        ListElement {
            name: "Slot 5"
            statut: 0
        }
    }
    Component {
        id: slotDelegate
        Item {
            width: 200; height: 50
            Text { id: nameSlot; text: name }
//            Text { text: 'size :' + size; anchors.left: nameSlot.right }
        }
    }
    Row {
        Rectangle {
            id: slotView
            width: 220; height: em.height
            color: "#efefef"

            ListView {
                id: slots
                focus: visible
                anchors.fill: parent
                model: xmlslotModel
                clip:true
//                header: refreshButtonDelegate
                delegate: slotDelegate
                highlight: Rectangle { color: "steelblue" }
                highlightMoveVelocity: 9999999
            }

            ScrollBar {
                scrollArea: slots; height: slots.height; width: 8
                anchors.right: slots.right
            }
        }

//        ListView {
//            id: list
//            width: em.width - slotView.width;
//            height: em.height
//            interactive: true;
//            clip: true
//            model: pmlModel
//            delegate: NewsDelegate2 {}
//        }
    }

    TextButton {
        text: "Close"
        expand: false
        font.pointSize: 16
        onClicked: {
            em.close();
        }
    }

    Message {
        id:message
        anchors.top: parent.top
        anchors.left: parent.left
        width : parent.width
        height: 30
    }

    function serializerSlotModel() {
        var xml = "<slots>";
        for (var i=0; i< slotModel.count; i++) {
            var pmlItem = slotModel.get(i);
            xml += "<slotitem>";
            xml += "<id>"+pmlItem.pmlid+"</id>";
            xml += "<status>"+encodeXml(pmlItem.username)+"</status>";
            xml += "<name>"+encodeXml(pmlItem.name)+"</name>";
            xml += "<ext>"+encodeXml(pmlItem.avatar_url)+"</ext>";
            xml += "<progtype>"+encodeXml(pmlItem.objects)+"</progtype>";
            xml += "<datatype>"+encodeXml(pmlItem.listobjects)+"</datatype>";
            xml += "<startadr>"+pmlItem.access_id+"</startadr>";
            xml += "<endadr>"+pmlItem.ispublic+"</endadr>";
            xml += "<date>"+pmlItem.isdeleted+"</date>";
            xml += "</item>";
        }
        xml += "</slots>";

        return xml;
    }

    function populateSlotModel(searchText) {
//        console.log("REFRESH Model");
        list.interactive = true;
        pmlModel.clear();
        for (var i=0; i<refpmlModel.count; i++) {
            var item = refpmlModel.get(i)
//            console.log("Read: "+item.pmlid+"-"+item.title);
            if (pmlview.ispublicCloud && (item.ispublic == 0)) continue;
//            console.log("public OK");
            if ( (pmlview.objid >= 0) && (item.isdeleted == 1)) continue;

            if ( (pmlview.objid > 0) && !idInArray(pmlview.objid.toString(),item.listobjects)) continue;
//            console.log("object OK");
            if ( (pmlview.objid == -1) && (item.isdeleted != 1 )) continue;
//            console.log("Deleted OK");
            if ( (searchText !== "") && !pmlContain(item,searchText)) continue;

            pmlModel.append({   rowid : i,
                                pmlid: item.pmlid,
                               username: item.username,
                                name: item.name,
                                avatar_url: item.avatar_url,
                                objects: item.objects,
                                access_id: item.access_id,
                                ispublic: item.ispublic,
                                isdeleted: item.isdeleted,
                                title: item.title,
                                description: item.description})
//            console.log("Store: "+item.title);
        }



    }
}
