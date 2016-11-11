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

Item {
    id: delegate
    property int tempX: 0

    width: delegate.ListView.view.width;
    height: textlabel.height *1.5

    Text {
        id:textlabel
        renderType: Text.NativeRendering
        text: keyword+" ("+counter+")"
        color: delegate.ListView.isCurrentItem ? "white" : "black"
        font { family: "Helvetica"; pointSize: 16; bold: false }
        anchors {
//            left: parent.left; leftMargin: 15
            verticalCenter: parent.verticalCenter
        }
    }

    Timer {
        id:timer
        interval: 40; running: false; repeat: true
        onTriggered:{


            if( tempX + delegate.width > textlabel.width ) {
                timer.running = false
                pauseTimer.restart()
                tempX = 0
//                textlabel.x = 0
            }
            else {
                tempX = tempX + 1
                textlabel.x = -tempX;
            }
        }
    }

    Timer {
        id:pauseTimer
        interval: 2000; running: false; repeat: false
        onTriggered: {
            if (textlabel.x < 0) {
                textlabel.x = 0
                pauseTimer.restart()
            }
            else {
                timer.running = true;
            }
        }
    }

    BusyIndicator {
        scale: 0.6
        on: delegate.ListView.isCurrentItem && pmlview.loading
        anchors { right: parent.right; rightMargin: 10; verticalCenter: parent.verticalCenter }
    }

    Rectangle {
        width: delegate.width; height: 1; color: "#cccccc"
        anchors.bottom: delegate.bottom
        visible: delegate.ListView.isCurrentItem ? false : true
    }
    Rectangle {
        width: delegate.width; height: 1; color: "white"
        visible: delegate.ListView.isCurrentItem ? false : true
    }

    MouseArea {
        anchors.fill: delegate
        onClicked: {
            delegate.ListView.view.currentIndex = index
            pmlview.keyword = keyword
        }
    }

//    MouseArea {
//        id:mouseArea
//        anchors.fill: parent
//        onClicked: {
//            tempX = 0;
//            timer.running = true;
//        }
//    }

    Component.onCompleted: {
        tempX = 0;
        pauseTimer.restart();
    }
}
