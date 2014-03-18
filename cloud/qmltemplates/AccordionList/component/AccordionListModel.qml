import QtQuick 1.0

ListModel {
    id: model

    ListElement {
        itemTitle: "Item title 1"
        attributes: [
            ListElement { subItemTitle: "Subitem title 1/1" },
            ListElement { subItemTitle: "Subitem title 2/1" }
        ]
    }
    ListElement {
        itemTitle: "Item title 2"
        attributes: [
            ListElement { subItemTitle: "Subitem title 1/2" },
            ListElement { subItemTitle: "Subitem title 2/2 long text long text long text long text long text long text long text" }
        ]
    }
    ListElement {
        itemTitle: "Item title 3"
        attributes: [
            ListElement { subItemTitle: "Subitem title 1/3" },
            ListElement { subItemTitle: "Subitem title 2/3" }
        ]
    }
}

