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
import QtQuick.Controls 1.1
import QtQuick.Extras 1.4

//    property Component mouseArea

//    property Component customizer: Item {
//        property alias currentStylePath: stylePicker.currentStylePath
//        property alias currentStyleDark: stylePicker.currentStyleDark
//    }


PieMenu {
    signal selectedOption(int option,int idpocket, point mousePt, int buttons);

    property int idpocket: 0;
    property point mousePt;
    property int buttons: 0

    id: pieMenu
    triggerMode: TriggerMode.TriggerOnClick
    width: 300; // Math.min(controlBoundsItem.width, controlBoundsItem.height) * 0.5
    height: width

    //            style: Qt.createComponent(customizerItem.currentStylePath)

    MenuItem {
        text: "Zoom In"
        onTriggered: {
            selectedOption(1,idpocket,mousePt,buttons);
        }
        iconSource: "images/zoom-in-xxl.png"
    }
    MenuItem {
        text: "Zoom Out"
        onTriggered: {
            selectedOption(2,idpocket,mousePt,buttons);
        }
        iconSource: "images/zoom-out-xxl.png"
    }
    MenuItem {
        text: "Info"
        onTriggered: {
            selectedOption(3,idpocket,mousePt,buttons);
        }
        iconSource: "images/context-menu.png"
    }
    MenuItem {
        text: "Info"
        onTriggered: {
            selectedOption(3,idpocket,mousePt,buttons);
        }
        iconSource: "images/readingblackhi.png"
    }
}
