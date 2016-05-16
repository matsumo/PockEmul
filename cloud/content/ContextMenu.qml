/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.5
import QtGraphicalEffects 1.0
import QtQuick.Controls 1.4
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4

//    property Component mouseArea

//    property Component customizer: Item {
//        property alias currentStylePath: stylePicker.currentStylePath
//        property alias currentStyleDark: stylePicker.currentStyleDark
//    }


PieMenu {
    signal selectedOption(string option,int idpocket, point mousePt, int buttons);

    property int idpocket: 0;
    property point mousePt;
    property int buttons: 0
    property string path: ""

    id: pieMenu
    triggerMode: TriggerMode.TriggerOnClick
    width: 300 * cloud.getValueFor("hiResRatio","1"); // Math.min(controlBoundsItem.width, controlBoundsItem.height) * 0.5
    height: width

    //            style: Qt.createComponent(customizerItem.currentStylePath)

    MenuItem {
        text: "Zoom In"
        onTriggered: {
            selectedOption(text,idpocket,mousePt,buttons);
        }
        iconSource: "images/zoom-in-xxl.png"
    }
    MenuItem {
        text: "Zoom Out"
        onTriggered: {
            selectedOption(text,idpocket,mousePt,buttons);
        }
        iconSource: "images/zoom-out-xxl.png"
    }
    MenuItem {
        text: "Pinch"
        onTriggered: {
            selectedOption(text,idpocket,mousePt,buttons);
        }
        iconSource: "images/pinch.png"
    }
//    MenuItem {
//        text: "Info"
//        onTriggered: {
//            selectedOption(text,idpocket,mousePt,buttons);
//        }
//        iconSource: "images/info.png"
//    }
    MenuItem {
        text: "Menu"
        onTriggered: {
            selectedOption(text,idpocket,mousePt,buttons);
        }
        iconSource: "images/context-menu.png"
    }

//    MenuItem {
//        text: "BookCase"
//        onTriggered: {
////            selectedOption(3,idpocket,mousePt,buttons);
//            while( menuItems.length >0) {
//                removeItem(menuItems[0]);
//            }
//            var i= addItem("TOTO");
//            i.iconSource = "images/zoom-out-xxl.png";
//            pieMenu.popup();
//        }
//        iconSource: "images/readingblackhi.png"
//    }



//    style: PieMenuStyle {
//        id:styletest
//        shadowRadius: 0

//        menuItem: Item {
//            id: item
//            rotation: -90+sectionCenterAngle(styleData.index)
//            Rectangle {
//                width: parent.height * 0.2
//                height: width
//                anchors.right: parent.right
//                anchors.verticalCenter: parent.verticalCenter
//                Column {
//                    Image {
//                        id: name
//                        source: control.menuItems[styleData.index].iconSource
//                    }
//                    Text {
//                        id: textItem
//                        text: control.menuItems[styleData.index].text
//                        anchors.centerIn: parent
//                        //                    color: control.currentIndex === styleData.index ? "red" : "white"
//                        rotation: 90
//                    }
//                }
//            }
//        }
//    }
}


