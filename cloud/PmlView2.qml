/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import QtQuick.XmlListModel 2.0
import QtQuick.Controls 2.0

import "content"
Rectangle {

    id: pmlview

    width: 800; height: 480

    property string currentObjid: "0"
    property bool loading: xmlpmlModel.status == XmlListModel.Loading
    property bool ispublicCloud: true

    property alias xml: xmlpmlModel.xml
    property alias source: xmlpmlModel.source
    property alias categoryListView: categories
    property alias pmlListView: list
    property alias refpmlModel: refpmlModel
    property string searchText: ""

    property int objid: 0
    property string keyword: ""
    property int ispublic: ispublicCloud ? 1 : 0
    property string cacheFileName:""

    Component.onCompleted: {
        console.log("PmlView2.qml: Completed",new Date());
    }

    onObjidChanged: {

        populatePMLModel(searchText)
    }

    onKeywordChanged: {

        populatePMLModel(searchText)
    }

    ListModel {
        id: categoryModel
    }
    SortListModel {
        id: tmpcategoryModel
        sortColumnName: "keyword"
    }

    XmlListModel {
        id: xmlpmlModel
        query: "/pmllist/pml_item"

        xml: parse.xmlPmlList

        XmlRole { name: "pmlid"; query: "guid/string()"; }
        XmlRole { name: "username"; query: "owner_username/string()" }
        XmlRole { name: "name"; query: "owner_name/string()" }
        XmlRole { name: "avatar_url"; query: "owner_avatar_url/string()" }
        XmlRole { name: "keywords"; query: "keywords/string()" }
        XmlRole { name: "access_id"; query: "access_id/number()" }
        XmlRole { name: "ispublic"; query: "ispublic/number()" }
        XmlRole { name: "isdeleted"; query: "deleted/number()" }
        XmlRole { name: "title"; query: "title/string()" }
        XmlRole { name: "description"; query: "description/string()" }
        XmlRole { name: "type"; query: "type/string()" }
        XmlRole { name: "snap_small"; query: "snapshot_small/string()" }
        XmlRole { name: "snap_medium"; query: "snapshot_medium/string()" }
        XmlRole { name: "snap_large"; query: "snapshot_large/string()" }
        XmlRole { name: "pmlfile"; query: "pmlfile/string()" }
        XmlRole { name: "createdAt"; query: "createdAt/string()" }
        XmlRole { name: "updatedAt"; query: "updatedAt/string()" }

        onStatusChanged: {
                if (status == XmlListModel.Ready) {
                    console.log("xml:",source);
//                    console.log("url ("+(ispublicCloud?"public":"private")+"):"+source);
                    console.log("xmlpmlModel onStatusChanged: START found rows:"+count);
                    refpmlModel.clear();
                    for (var i=0; i<count; i++) {
                        var item = get(i)
//                        console.log(item.objects)
                        refpmlModel.append({rowid : i,
                                           pmlid: item.pmlid,
                                           username: decodeXml(item.username),
                                           name: decodeXml(item.name),
                                           avatar_url: decodeXml(item.avatar_url),
                                           keywords: decodeXml(item.keywords),
                                           access_id: item.access_id,
                                           ispublic: item.ispublic,
                                           isdeleted: item.isdeleted,
                                           title: decodeXml(item.title),
                                           description: decodeXml(item.description),
                                           type: decodeXml(item.type),
                                           snap_small: decodeXml(item.snap_small),
                                           snap_medium: decodeXml(item.snap_medium),
                                           snap_large: decodeXml(item.snap_large),
                                           pmlfile: decodeXml(item.pmlfile),
                                           createdAt: decodeXml(item.createdAt),
                                           updatedAt: decodeXml(item.updatedAt)})
                    }

                    cloud.saveCache(cacheFileName,serializerefpmlModel());
                    populatePMLModel("");
                    populateCategoryModel("");
//                    console.log("xmlpmlModel onStatusChanged: END");
                }

            }
    }

    function insertorupdatecategoryModel(keyword) {
//        console.log("insertorupdatecategoryModel:"+keyword);
        for (var i=0; i<tmpcategoryModel.count;i++) {
            var item = tmpcategoryModel.get(i);
            if (item.keyword == keyword) {
                // found, increment
                item.counter++;
                return;
            }
        }
        // not found, create a record
        tmpcategoryModel.append({keyword: keyword,
                                 counter: 1});
    }

//    *     0 => 'Private',
//    *    -2 => 'Friends',
//    *     1 => 'Logged in users',
//    *     2 => 'Public',
    function pmlContain(pmlitem,searchText) {
        if (typeof(searchText)=='undefined') return true;
        if (searchText == "") return true;

        var searchString = "";
        searchString = pmlitem.title+" "+
                pmlitem.description+" "+
                pmlitem.username+" "+
                pmlitem.objects+" "+
                (pmlitem.access_id == 0 ? "private" : (pmlitem.access_id == -2 ? "friend" : "public")) ;
//        console.log("searchText:**"+searchText+"**");
        var tableau=searchText.toUpperCase().split(" ");
        for (var i=0; i<tableau.length; i++) {
            if ( (tableau[i]!="") && (searchString.toUpperCase().indexOf(tableau[i])>=0) ) return true;
        }
        return false;
    }

    function populateCategoryModel(searchText) {
//        console.log("populateCategoryModel");
        tmpcategoryModel.clear();
        var isdeletedCount = 0;
        var totalCount = 0;
        for (var i=0; i<refpmlModel.count; i++) {
            var item = refpmlModel.get(i)
//            console.log("Read: "+item.pmlid+"-"+item.title);
            if (pmlview.ispublicCloud && (item.ispublic == 0)) continue;

            if (item.isdeleted) { isdeletedCount++; continue; }

            if (item.type == "pml" && !typelistModel.get(0).selected) continue;
            if (item.type == "psk" && !typelistModel.get(1).selected) continue;
            if (item.type == "pum" && !typelistModel.get(2).selected) continue;

            // Apply seacrh !text is defined

            if ( (searchText != "") && !pmlContain(item,searchText)) continue;

            totalCount++;
            // fetch all item's objects
//            console.log("XML:"+item.keywords);
            var x=item.keywords


            var tableau=x.split('|');
            for (var j=1; j<tableau.length - 1; j++) {
//                console.log("j="+j)
//                var obj = tableau[j].split('|');
                insertorupdatecategoryModel(tableau[j]);
            }
//            console.log("populateCategoryModel : END");
        }

        // sort tmpcategoryModel
//        console.log("***"+tmpcategoryModel.count);
        tmpcategoryModel.quick_sort();
//        console.log("***"+tmpcategoryModel.count);
        categoryModel.clear();
        categoryModel.append({keyword: "All", counter: (totalCount-isdeletedCount)});
        // copy tmpcategoryModel to categoryModel with SORT
        for (var i=0; i<tmpcategoryModel.count;i++){
            categoryModel.append(tmpcategoryModel.get(i));
        }
//        categoryModel.append({objid: -1,name: "Recycle Bin", counter: (isdeletedCount)});
        tmpcategoryModel.clear();
    }

    function serializerefpmlModel() {
        var xml = "<listPML>";
        for (var i=0; i< refpmlModel.count; i++) {
            var pmlItem = refpmlModel.get(i);
            xml += "<item>";
            xml += "<pmlid>"+pmlItem.pmlid+"</pmlid>";
            xml += "<username>"+encodeXml(pmlItem.username)+"</username>";
            xml += "<name>"+encodeXml(pmlItem.name)+"</name>";
            xml += "<avatar_url>"+encodeXml(pmlItem.avatar_url)+"</avatar_url>";
            xml += "<keywords>"+encodeXml(pmlItem.keywords)+"</keywords>";
//            xml += "<listobjects>"+encodeXml(pmlItem.listobjects)+"</listobjects>";
            xml += "<access_id>"+pmlItem.access_id+"</access_id>";
            xml += "<ispublic>"+pmlItem.ispublic+"</ispublic>";
            xml += "<deleted>"+pmlItem.isdeleted+"</deleted>";
            xml += "<title>"+encodeXml(pmlItem.title)+"</title>";
            xml += "<description>"+encodeXml(pmlItem.description)+"</description>";
            xml += "<type>"+encodeXml(pmlItem.type)+"</type>";
            xml += "</item>";

        }
        xml += "</listPML>";

        return xml;
    }

    function populatePMLModel(searchText) {
//        console.log("REFRESH Model");
        list.interactive = true;
        pmlModel.clear();

        for (var i=0; i<refpmlModel.count; i++) {
            var item = refpmlModel.get(i)
//            console.log("Read: "+item.pmlid+"-"+item.title);
//            if (pmlview.ispublicCloud && (item.ispublic == 0)) continue;
//            console.log("public OK");
//            if ( (pmlview.objid >= 0) && (item.isdeleted == 1)) continue;

            if (item.type == "pml" && !typelistModel.get(0).selected) continue;
            if (item.type == "psk" && !typelistModel.get(1).selected) continue;
            if (item.type == "pum" && !typelistModel.get(2).selected) continue;

            if ( (pmlview.keyword != "") && (pmlview.keyword != "All") && !idInArray(pmlview.keyword,item.keywords)) continue;
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
                                description: item.description,
                                type: item.type,
                                snap_small: item.snap_small,
                                pmlfile: item.pmlfile,
                                createdAt: item.createdAt,
                                updatedAt: item.updatedAt})
//            console.log("Store: "+item.title);
        }



    }

    function idInArray(id, list) {
//        console.log("Search:"+id+" in:"+list+"  contains:"+String(list).indexOf(","+id+","));
        return (String(list).indexOf("|"+id+"|")>=0);
    }

    function refresh() {
        xmlpmlModel.xml = "";
        xmlpmlModel.reload();
    }

    function populate(searchText) {
        populateCategoryModel(searchText);
        populatePMLModel(searchText);
    }

    // Reference List Model. Contains all record
    // it might be valuable to dump it on disk when the application close
    // and load the cache at startup
    ListModel {
        id: refpmlModel;

        function appendPml(item) {
            append({rowid : refpmlModel.count,
                       pmlid: item.pmlid,
                       username: item.username,
                       name: item.name,
                       avatar_url: item.avatar_url,
                       keywords: item.keywords,
                       access_id: item.access_id,
                       ispublic: item.ispublic,
                       isdeleted: item.isdeleted,
                       title: item.title,
                       description: item.description,
                       type: item.type,
                       snap_small: item.snap_small,
                       pmlfile: item.pmlfile,
                       createdAt: item.createdAt,
                       updatedAt: item.updatedAt});
        }

        function removePml(pmlid) {
            for (var i=0; i<count;i++) {
                if (get(i).pmlid == pmlid) {
                    //                console.log("***found***");
                    remove(i);
                }
            }
        }
    }

    // Filtered ListModel
    ListModel { id: pmlModel; }



    Row {
        Rectangle {
            id: categoriesView
            width: pmlview.width /8 //pmlview.height / 8
            height: pmlview.height
            color: "#efefef"

            Column {
                id: types
                spacing: 0
                width: parent.width
                height:500
                Repeater {
                    id: repeatTypes

                    onCountChanged: {
                        /* calculate ListView dimensions based on content */
                        console.log("count:",count);
                        var listViewHeight = 0
                        var listViewWidth = 0

                        // iterate over each delegate item to get their sizes
                        for(var i=0; i<repeatTypes.count/*contentItem.children.length*/; i++) {
                            listViewHeight += types.children[i].height
                            listViewWidth  = Math.max(listViewWidth, types.children[i].width)
//                            console.log("h:",i,listViewHeight);
                        }

                        types.height = listViewHeight
//                        types.width = listViewWidth
                    }

                    Item {
                        id: listItem
                        width: types.width
                        height: text.height * 1.4

                        Text {
                            id: text
                            text: name // Title text is from the 'name' property in the model item (ListElement)
                            //                            width: parent.width
                            font { family: "Helvetica"; pointSize: 18; bold: true }
                        }

                        // The checkbox to display
                        CheckBox {
                            id: checkbox
                            checked: selected  // Checked state is from the 'selected' property in the model item
                            anchors { right: listItem.right; verticalCenter: listItem.verticalCenter }
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                checkbox.checked = ! checkbox.checked;
                                typelistModel.set(index, { "selected": checkbox.checked });
                                populate(newprivateSearchItem.text);
                            }
                        }
                    }

                    model: ListModel {
                        id: typelistModel
                        ListElement {
                            name: "Session"
                            description: "Selectable item 1"
                            selected: true
                        }
                        ListElement {
                            name: "Skin"
                            description: "Selectable item 2"
                            selected: true
                        }
                        ListElement {
                            name: "Document"
                            description: "Selectable item 3"
                            selected: true
                        }
                    }
                }
            }


            ListView {
                id: categories
                    focus: visible
                    width: parent.width
                    anchors.top: types.bottom
                    anchors.bottom: parent.bottom

                    model: categoryModel //xmlcategoryModel
                    clip:true
                    header: refreshButtonDelegate
                    delegate: CategoryDelegate {}
                    highlight: Rectangle { color: "steelblue" }
                    highlightMoveVelocity: 9999999
                }

                ScrollBar {
                    scrollArea: categories; height: categories.height; width: 8
                    anchors.right: categories.right
                    anchors.top: categories.top
                }
        }

        ListView {
            id: list
            width: pmlview.width - categoriesView.width;
            height: pmlview.height
            interactive: true;
            clip: true
            model: pmlModel
            delegate: NewsDelegate2 {}
        }
    }
    Component {
        id: refreshButtonDelegate
        Item {
            width: categories.width; height: expandcollapsebutton.height
            Text {
                id: expandcollapsebutton
                renderType: Text.NativeRendering
                text: "Refresh"
                font { family: "Helvetica"; pointSize: 16; bold: false }
                anchors {
                    left: parent.left; leftMargin: 15
                    verticalCenter: parent.verticalCenter
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    parse.pmlList();
//                    xmlpmlModel.reload();

//                   categoriesView.width = (expandcollapsebutton.text=="<")? 50 : 220;
//                    expandcollapsebutton.text = (expandcollapsebutton.text=="<")? ">" : "<";
//                    populateCategoryModel();//xmlcategoryModel.reload();
                }
            }
        }
    }
    ScrollBar { scrollArea: list; height: list.height; width: 8; anchors.right: pmlview.right }
    Rectangle {
        x: categoriesView.width; height: pmlview.height; width: 1; color: "#cccccc" }


    ListModel { id: pmlThumbModel; }

    function focusPml(pmlid) {
        for (var i=0; i<pmlModel.count;i++) {
            if (pmlModel.get(i).pmlid == pmlid) {
//                console.log("***found***");
                list.currentIndex = i;
            }
        }
    }


    function updThumbId(refpmlid) {
        // search pmlid and increment
//        console.log("update counter for :"+refpmlid);
        for (var i=0; i< pmlThumbModel.count;i++){
            var item = pmlThumbModel.get(i);
            if (item.pmlid == refpmlid) {
//                console.log("updThumbId - Found:"+item.counter);
                pmlThumbModel.setProperty(i,"counter",item.counter+1);
//                console.log("Found, updated:"+item.counter);
                return 0;
            }
        }
        pmlThumbModel.append({pmlid: refpmlid,counter:1});
//        console.log("updThumbId - NOT Found, created:" +pmlThumbModel.count);
    }
    function getThumbId(refpmlid) {
//        console.log("getThumbId:"+refpmlid+" count:"+pmlThumbModel.count);
        for (var i=0; i< pmlThumbModel.count;i++){
            var item = pmlThumbModel.get(i);
//            console.log("getThumbId - Fetch:"+item.pmlid);
            if (item.pmlid == refpmlid) {
//                console.log("getThumbId - Found:"+item.counter);
                return item.counter;
            }
        }
        return 0;
    }


}
