import QtQuick 2.0
import QtQuick.XmlListModel 2.0

Rectangle {
    width: 240; height: 200

    anchors.fill: parent
    id: main


    XmlListModel {
        id: xmlpmlModel
        source: "qrc:/pockemul/config.xml"
        query: "/demolauncher/demos/example"

        XmlRole { name: "name"; query: "@name/string()"; }
        XmlRole { name: "image"; query: "@image/string()" }
    }

    Component {
        id: delegate

        Item {
            id:myitem
            width: 200; height: 190
            z: PathView.view.zLevel(index)
            scale: PathView.iconScale
//            opacity: PathView.imageOpacity
//            property int angle: PathView.iconRotate
//            property int ind:index
//            transform:  Rotation {
//                          axis {x: 0; y: 1; z: 0}
//                          angle: (myitem.z==10 ? 0 : 70) * ((PathView.currentIndex - ind )>0 ? 1:-1)
//                        }

            Column {
                id: wrapper
                Image {
                    anchors.horizontalCenter: nameText.horizontalCenter
                    width: 200; height: 150
                    source: "qrc"+image
                    fillMode: Image.PreserveAspectFit;
                }
                Text {
                    id: nameText
                    text: name+"("+myitem.z+")"
                    font.pointSize: 16
                    color: wrapper.PathView.isCurrentItem ? "red" : "black"
                }
            }
        }
    }

    PathView {
        id: horizontalPathView
        anchors.fill: parent
        model: xmlpmlModel
        delegate: delegate
        pathItemCount: 11
           preferredHighlightBegin: 0.5; preferredHighlightEnd: 0.5
           path: Path {
               startX: -400; startY: main.height/2
               PathAttribute { name: "iconScale"; value: 0.5 }
               PathAttribute { name: "iconRotate"; value: 60 }
//               PathLine {x:main.width/2
//                   y: main/2
//               }
               PathQuad {
                   x: horizontalPathView.width/2
                   y: horizontalPathView.height/2-20
                   controlX: horizontalPathView.width/4
                   controlY: horizontalPathView.height/2-45
               }
               PathAttribute { name: "iconScale"; value: 1.0 }
               PathAttribute { name: "iconRotate"; value: 0 }
               PathQuad {
                   x: horizontalPathView.width+400
                   y: horizontalPathView.height/2-70
                   controlX: 3*horizontalPathView.width/4
                   controlY: horizontalPathView.height/2-45
               }
//               PathLine {x:main.width+400
//                   y: main/2
//               }
               PathAttribute { name: "iconScale"; value: 0.5 }
               PathAttribute { name: "iconRotate"; value: -60 }

           }

        Keys.onLeftPressed: decrementCurrentIndex()
        Keys.onRightPressed: incrementCurrentIndex()
        function zLevel(index) {
            var dist = Math.abs((currentIndex - index) % count)
            if (dist > (pathItemCount/2.0 + 1))
                dist = count - dist
            return Math.floor(pathItemCount/2.0) - dist
        }
    }
}
