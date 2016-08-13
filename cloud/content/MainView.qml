import QtQuick 2.0

Item {
    id: mainView

    property bool exitOnBack: true
    property int iconsize: 48 * cloud.getValueFor("hiResRatio","1");

    // Set this to blur the mainView when showing something on top of it
    property real blurAmount: 0

    // Updates the blur shader source, best called right before adding blurAmount
    function scheduleUpdate() {
//        mainContentSource.scheduleUpdate();
    }

    anchors.fill: parent

    Component.onCompleted: {
        console.log("MainView.qml: Completed",new Date());
    }

    // Update blur shader source when width/height changes
    onHeightChanged: {
        mainView.scheduleUpdate();
    }
    onWidthChanged: {
        mainView.scheduleUpdate();
    }

    Item {
        id: mainViewArea
        anchors.fill: parent

        BackGround {
            id: background
        }




        ListView {
            id: listView
            focus: visible
            interactive: true

//            onActiveFocusChanged: { console.log("FocusScope activeFocusChanged", listView.activeFocus) }
//            onFocusChanged: { console.log("FocusScope focusChanged", listView.focus) }

            Keys.onPressed: {
                    if (event.key === Qt.Key_Return) {
                        console.log('Key Return was pressed');
                        currentItem.select();
                        event.accepted = true;
                    }
                    if ((event.key === Qt.Key_Escape) ||
                        (event.key === Qt.Key_Backspace) ) {
                        console.log('Key Escape was pressed');
                        backIcon.select();
                        event.accepted = true;
                    }


                }

//            property real globalLightPosX: lightImage.x / mainView.width
//            property real globalLightPosY: lightImage.y / mainView.height

            // Normal-mapped cover shared among delegates
            ShaderEffectSource {
                id: coverNmapSource
                sourceItem: Image {
                    source: ""//images/cover_nmap.png"
                }
                hideSource: true
                visible: false
            }

            anchors.fill: parent
            spacing: -60
            model: sortedModel
            delegate: DelegateItem {
                name: model.name
            }
            highlightFollowsCurrentItem: true
            highlightRangeMode: ListView.StrictlyEnforceRange
            highlightMoveDuration: 400
            preferredHighlightBegin: mainView.height * 0.5 - 140
            preferredHighlightEnd: mainView.height * 0.5 - 140
            cacheBuffer: 4000
        }


        Text {
            id: titleText
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -40
            anchors.horizontalCenter: parent.horizontalCenter
            width: 180 + parent.width * 0.25
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            text:
//                listView.activeFocus ? "I have active focus!" : "I do not have active focus";
                (listView.currentIndex >=0) ? (listView.currentIndex+1) + ". " + listView.currentItem.name : ""
            color: "#ffffff"
            style: Text.Outline
            styleColor: "#b0a030"
            font.pixelSize: settings.fontL
            Behavior on text {
                SequentialAnimation {
                    ParallelAnimation {
                        NumberAnimation { target: titleText; property: "opacity"; duration: 100; to: 0; easing.type: Easing.InOutQuad }
                        NumberAnimation { target: titleText; property: "scale"; duration: 100; to: 0.6; easing.type: Easing.InOutQuad }
                    }
                    PropertyAction { target: titleText; property: "text" }
                    ParallelAnimation {
                        NumberAnimation { target: titleText; property: "opacity"; duration: 100; to: 1; easing.type: Easing.InOutQuad }
                        NumberAnimation { target: titleText; property: "scale"; duration: 100; to: 1; easing.type: Easing.InOutQuad }
                    }
                }
            }
        }

        Image {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 32
            anchors.horizontalCenter: parent.horizontalCenter
            source: "" //images/qt_logo.png"
            opacity: listView.atYEnd
            Behavior on opacity {
                NumberAnimation { duration: 500; easing.type: Easing.InOutQuad }
            }
        }



//        SettingsView {
//            id: settingsView
//        }

        Image {
            id: backIcon
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.top: parent.top
            anchors.bottomMargin: 4
            source: "qrc:/core/back-white.png"
            width: mainView.iconsize
            height: mainView.iconsize
//            opacity: backgroundItem.opacity + 0.4
            MouseArea {
                anchors.fill: parent
                anchors.margins: -20
                onClicked: backIcon.select();
            }

            function select() {

                if (pobjectsmodel.brandsearch[pobjectsmodel.brandsearch.length-1]==='BRAND')
                {
                    if (exitOnBack) showRoom.visible = false;
                }
                else {
                    console.log("OK***:",pobjectsmodel.brandsearch);
                    pobjectsmodel.brandsearch.pop();
                    console.log("OK***:",pobjectsmodel.brandsearch);
                    pobjectsmodel.reload();
                }
            }
        }
    }
}
