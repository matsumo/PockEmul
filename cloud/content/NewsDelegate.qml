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
import "counter.js" as Counter

import ".."

Item {
    id: delegate
    height: column.height + 40
    width: delegate.ListView.view.width

    // Create a property to contain the visibility of the details.
    // We can bind multiple element's opacity to this one property,
    // rather than having a "PropertyChanges" line for each element we
    // want to fade.
    property real detailsOpacity : 0

    property bool ismine: (username == cloud.getValueFor("username",""))
    property bool changed: false
    property int newpublicstatus: ispublic

    // A simple rounded rectangle for the background
    Rectangle {
        id: background
        x: 2; y: 2; width: parent.width - x*2; height: parent.height - y*2
        color: "white"
        border.color: "orange"
        radius: 5
    }

    MouseArea {
        anchors.fill: parent
        onClicked: delegate.state = 'Details';
    }

    Column {
        id: column
        x: 20; y: 20
        width: parent.width - 40
        //        height: background.height
        Row {
            width: parent.width;
            spacing: 5

            Image {
                id: publicIcon
                source: "images/public.png"
                width: 40
                height: 40
                visible: (ispublic==1)
            }
            Edit {
                id: titleText
                text: (title=="")?"No title":title
                nbLine: 1
                readOnly: (detailsOpacity==0) || (!ismine)
                interactive: !readOnly

                wrapMode: Text.WordWrap
                width: parent.width - 40 - (buttonColumn.visible? buttonColumn.width:0) - 5
                //                height: 30
                font { bold: true; family: "Helvetica"; pointSize: 14 }
                textColor: changed ? "red" : "black"
                onTextChanged: {
                    checkmodif()
                }
            }
        }

        Text {
            id: usernameText
            text: username; width: parent.width; wrapMode: Text.WordWrap
            font { bold: false; family: "Helvetica"; pointSize: 14 }
        }
        Row {
            Image {
                id: pmlThumbImage
                source: serverURL+"getPMLthumb/"+pmlid+"/"+getThumbId(pmlid)
                fillMode: Image.PreserveAspectFit;

//                MouseArea {
//                    anchors.fill: parent
////                    onClicked: {
////                        root.bigposter.source = serverURL+"getPMLsnap/"+pmlid;
////                        root.bigposter.visible = true;
////                    }
//                }
            }
            TextEdit {
                id: objectsText
                text: objects
                readOnly: true
                opacity: delegate.detailsOpacity
            }
        }

        Edit {
            id: descriptionText
            width: parent.width;
//            height:background.height - y - 25
            text: (description=="")?"No description":description
            readOnly: !ismine
            wrapMode: Text.WordWrap;
            font.family: "Helvetica"; font.pointSize: 14
            textColor: changed ? "red" : "black"
            opacity: delegate.detailsOpacity
            onTextChanged: checkmodif()
        }
        Item {
            id: comments
        }
    }


    Item {
        id: details
        opacity: delegate.detailsOpacity
    }
    Column {
        id: buttonColumn
        visible: (delegate.detailsOpacity==1) || ((width+pmlThumbImage.width)<column.width)
//        width: 150
//        width: childrenRect.width
        y:40
        anchors { right: background.right; rightMargin: 10 }
        spacing: 5
        TextButton {
            id: closeButton
            opacity: delegate.detailsOpacity
            text: "Close"
            onClicked: {
                if (changed) {
                    root.sendWarning("Cancel changes before closing.");
                }
                else {

                    delegate.state = '';
                }
            }
        }
        TextButton {
            id: downloadButton
            text: "Download File"
            onClicked: cloud.getPML(pmlid);
        }
        TextButton {
            id: importButton
            visible: ispublic && !ismine
            text: "Clone to private"
            onClicked: {
//                console.log('onClicked');
                //                if (cloud.askMsg("Are you sure ?",2) == 1)
                {
                    // update data
                    var url = serverURL + "clonePML/" + currentApiKey +"/" + pmlid;
                    console.log('url:'+url);
                    requestGet(url, function (o) {
                        if (o.readyState == 4 ) {
                            if (o.status==200) {
                                tmpXmlListModel.xml = o.responseText;
                            }
                        }
                    });
                }

            }
        }
        TextButton {
            id: saveCurrentSessionButtonIn
            text: "Save current session"
            visible: ismine
            onClicked: {
                var xml = cloud.save();
                var url = cloud.getValueFor("serverURL","")+"savePML/"+ currentApiKey +"/"+pmlid;

//                console.log("ok:"+url);
                requestPost(url,xml, function(o) {
                    if (o.readyState == 4) {
                        if (o.status==200) {
                            tmpXmlListModel.xml = o.responseText;
                            // Trick to reload the thumbnail
                            //console.log("thumb url="+pmlThumbImage.source);
                            updThumbId(pmlid);
//                            pmlThumbImage.source = serverURL+"getPMLthumb/"+pmlid+"/"+getThumbId(pmlid);
                            ///console.log("thumb url="+pmlThumbImage.source);
                        }
                    }
                });

                populateCategoryModel("");


//                        privateCloud.categoryModel.reload();
//                        publicCloud.categoryModel.reload();
            }

        }
        TextButton {
            id: saveButton
            visible: changed
            text: "Save"
            onClicked: {
                console.log('onClicked');
                //                if (cloud.askMsg("Are you sure ?",2) == 1)
                {
                    // update data
                    var url = serverURL + "updPML/" + currentApiKey +"/" + pmlid;
//                    console.log('url:'+url);
                    var data = "<data>";
                    data +=     "<title>"+encodeXml(titleText.text)+"</title>";
                    data +=     "<description>"+encodeXml(descriptionText.text)+"</description>";
                    data +=     "<ispublic>"+newpublicstatus+"</ispublic>";
                    data += "</data>";
                    requestPost(url, data,function (o) {
                        if (o.readyState == 4) {
                            //                        console.log("status:"+o.status);
                            if (o.status==200) {
                                //                        cloud.askMsg("Ok, saved !",1);
                                //Reload record
                                changed = false;
                                var publicstatuschanged = false;
                                refpmlModel.setProperty(rowid,"title",titleText.text);
                                refpmlModel.setProperty(rowid,"description",descriptionText.text);
                                if (refpmlModel.get(rowid).ispublic != newpublicstatus) {
                                    refpmlModel.setProperty(rowid,"ispublic",newpublicstatus);
                                    publicstatuschanged = true;
                                }
                                refpmlModel.setProperty(rowid,"ideleted",newpublicstatus);

                                //                    console.log(o.responseText);
                                if (publicstatuschanged) {
                                    delegate.state="";
                                    if (newpublicstatus) {
                                        // copy item from private to public refpmlModel
                                        publicCloud.refpmlModel.appendPml(privateCloud.refpmlModel.get(rowid));
                                    }
                                    else  {
                                        // remove item from public refpmlModel
                                        publicCloud.refpmlModel.removePml(pmlid);
                                    }

                                    privateCloud.populateCategoryModel("");
                                    privateCloud.objid = 0;
                                    privateCloud.populatePMLModel("");
                                    publicCloud.populateCategoryModel("");
                                    publicCloud.objid = 0;
                                    publicCloud.populatePMLModel("");
                                    if (newpublicstatus) {
                                        tabbedui.tabClicked(1);
                                        publicCloud.focusPml(pmlid);
                                    }
                                    else  {
                                        tabbedui.tabClicked(0);
                                        privateCloud.focusPml(pmlid);
                                    }

                                    cloud.saveCache(privateCloud.cacheFileName,privateCloud.serializerefpmlModel());
                                    cloud.saveCache(publicCloud.cacheFileName,publicCloud.serializerefpmlModel());
                                }
                                else {
                                    delegate.state="";
                                    publicCloud.refpmlModel.removePml(pmlid);
                                    publicCloud.refpmlModel.appendPml(privateCloud.refpmlModel.get(rowid));
                                    publicCloud.populatePMLModel("");
                                    populatePMLModel("");
                                    cloud.saveCache(cacheFileName,serializerefpmlModel());
                                }

                            }
                        }
                    });
                }

            }
        }
        TextButton {
            id: revertButton
            //            y: 100
            //            anchors { right: background.right; rightMargin: 10 }
            visible: changed
            text: "Cancel changes"
            onClicked: {
                titleText.text = (title=="")?"No title":title
                descriptionText.text = (description=="")?"No description":description
                newpublicstatus = ispublic
                switchelt.state = (ispublic==1) ? "on" :"off"
                checkmodif()
            }
        }
        TextButton {
            id: undeleteButton
            text: "UnDelete"
            visible: (isdeleted ==1) && ismine
            onClicked: {
                var url = serverURL + "undelPML/" + currentApiKey +"/" + pmlid;
                requestGet(url,function (o) {
                    if (o.readyState == 4 ) {
                        if (o.status==200) {
                            refpmlModel.setProperty(rowid,"isdeleted",0);
                            cloud.saveCache(cacheFileName,serializerefpmlModel());
                            pmlview.categoryListView.currentIndex = 0;
                            pmlview.objid = 0;
                            populatePMLModel("");
                            populateCategoryModel("");
                            // Position at the just undeleted item
                            console.log("****search pmlid:"+pmlid)
                            pmlview.focusPml(pmlid);

                        }}
                });
            }
        }
        TextButton {
            id: deleteButton
            visible: ismine
            text: (isdeleted ==1) ? "Permanently delete" : "Delete"
            onClicked: {
                var url = serverURL + "delPML/" + currentApiKey +"/" + pmlid;
                requestGet(url,function (o) {
                    if (o.readyState == 4 ) {
                        if (o.status==200) {
                            delegate.state="";
                            if (isdeleted==1) {
                                refpmlModel.remove(rowid);
                                cloud.saveCache(cacheFileName,serializerefpmlModel());
                            }
                            else {

                                console.log("DELETE");
                                changed = false;
                                refpmlModel.setProperty(rowid,"isdeleted",1);
                                cloud.saveCache(cacheFileName,serializerefpmlModel());
                            }
                            populateCategoryModel();
                            pmlview.categoryListView.currentIndex = pmlview.categoryListView.count-1;
                            pmlview.objid = -1;
                            populatePMLModel();

                        }
                    }
                });
            }
        }
        Row {
            id: publicSwitch
            opacity: delegate.detailsOpacity
            visible: ismine
            anchors.horizontalCenter: parent.horizontalCenter
            Text {
                id: switchLabel
                text: qsTr("Public ")
//                font.family: "Helvetica"; font.pointSize: 14
                anchors.verticalCenter: parent.verticalCenter
            }
            Switch {
                id:switchelt
                bwidth: 50
                bheight: 25
                state: (ispublic==1) ? "on" :"off"
                anchors.verticalCenter: parent.verticalCenter
                onStateChanged: {
                    if (state != ((ispublic==1) ? "on" :"off"))
                        root.sendWarning("Do you really want to change the public status ?");
                    newpublicstatus = (state=="on") ? 1 : 0;

                    checkmodif();
                }
            }
        }
    }

    function checkmodif() {
        if ( (titleText.text == ((title=="")?"No title":title)) &&
                (descriptionText.text == ((description=="")?"No description":description)) &&
                (ispublic == newpublicstatus)) {
            changed = false;
        }
        else { changed = true; }
    }

    states: State {
        id: viewstate
        name: "Details"

        PropertyChanges { target: background; color: "white" }
        PropertyChanges { target: pmlThumbImage; width: 200; height: 200;} // Make picture bigger
        PropertyChanges { target: delegate; detailsOpacity: 1; x: 0 } // Make details visible
        PropertyChanges { target: delegate; height: list.height } // Fill the entire list area with the detailed view
        PropertyChanges { target: categoriesView; width: root.isPortrait?0:categoriesView.width }
        // Move the list so that this item is at the top.
        PropertyChanges { target: delegate.ListView.view; explicit: true; contentY: delegate.y }

        // Disallow flicking while we're in detailed view
        PropertyChanges { target: delegate.ListView.view; interactive: false }
    }

    transitions: Transition {
        // Make the state changes smooth
        ParallelAnimation {
            ColorAnimation { property: "color"; duration: 500 }
            NumberAnimation { duration: 300; properties: "detailsOpacity,x,contentY,height,width" }
        }
    }
    //    MouseArea {
    //        anchors.fill: parent
    //        onDoubleClicked: cloud.getPML(pmlid)
    //    }

    Rectangle {
        width: parent.width; height: 1; color: "#cccccc"
        anchors.bottom: parent.bottom
    }



}
