import QtQuick 2.3
import "../TabbedQuickApp"
import ".."
import "."

Rectangle {
    VisualItemModel {
        id: tabsModel

        Tab {name: "Messages"
            color: "blue"
//            icon: "pics/public-cloud-white.png"

//            SearchBox {
//                id: newprivateSearchItem
//                width: parent.width
//                height: 50
//                objectName: "searchFld"
//                onTextChanged: {
//                    myCloud.populate(text)
//                }
//            }
//            PmlView2   {
//                id: myCloud
//                anchors.top: newprivateSearchItem.bottom
//                anchors.bottom: parent.bottom
//                width: parent.width
//                ispublicCloud: false
//                searchText: newprivateSearchItem.text
//                cacheFileName: "newprivateCloud.xml"
////                xml: cloud.loadCache(cacheFileName)
//            }

        }

        Tab {name: "New Shared sessions"
            color: "white"
//            icon: "pics/action-white.png"
//            Actions {}
        }

        Tab {name: "News"
            icon: "pics/settings-white.png"

            color: "red"

//            Settings {
//                anchors.fill: parent
//            }
        }

//        Tab {name: ""
//            icon: "pics/back-white.png"
//            MouseArea {
//                anchors.fill: parent
//                onClicked: close();
//            }
//        }

    }

    TabbedUI {
        id: tabbedui
        vertical: true
//        tabsHeight: 72 * cloud.getValueFor("hiResRatio","1")
        tabsWidth:  144 * cloud.getValueFor("hiResRatio","1")
        tabIndex: 0
        tabsModel: tabsModel
        quitIndex: 4
//        onClose: rootCloud.close();

    }
}
