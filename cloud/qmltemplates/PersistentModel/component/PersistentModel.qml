import QtQuick 1.0

import "Store.js" as Store

ListModel {
    id: model

    ListElement {
        itemName: "Item name 1"
        attributes: [
            ListElement {
                subItemName: "Subitem name 1"
            }
        ]
    }

    Component.onCompleted: {
        // Comment the following line and run once to restore the initial state with predefined content.
        Store.restore(model);
    }

    Component.onDestruction: {
        Store.store(model)
    }

    function addItem(itemName, subItemName) {
        model.append({"itemName": itemName,
                     "attributes": [{"subItemName": subItemName}] })
    }

}
