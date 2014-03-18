import QtQuick 1.0
import "../component" as Comp

Item {
    id: container
    width: 360
    height: 640

    Comp.AccordionList {
        id: list
        anchors.fill: parent
    }
}
