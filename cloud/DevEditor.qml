import QtQuick 2.0
import "TabbedQuickApp"

TabbedUI {
    id: editor
    visible: false
    z: parent.z+2
    tabsHeight: 72 * cloud.getValueFor("hiResRatio","1")
    tabIndex: 0
    tabsModel: model
    quitIndex: 2
    onClose: visible=false;

    onVisibleChanged: {
        if (visible) {
            scene.parent = sceneTab;
        }
        else {
            scene.parent = renderArea;
        }
    }

    VisualItemModel {
        id: model

        Tab {
            id: sceneTab
            name: "PockEmul Desktop"
            icon: "qrc:/core/pocket.png"
        }
        Tab {
            name: "Code Editor"
            icon: "qrc:/core/dev.png"
            Rectangle {
                property rect rect: Qt.rect(x, y, width, height)
                id: codeEditor
                anchors.fill: parent
                onRectChanged: {
                    console.log("change size",rect,mapFromItem(null,x, y, width, height));
                    main.changeGeo(x, y, width, height);
                }
            }
        }
        Tab {name: "Back"
            icon: "pics/back-white.png"
            MouseArea {
                anchors.fill: parent
                onClicked: close();
            }
        }
    }

}
