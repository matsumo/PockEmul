import QtQuick 2.0
import QtQuick.XmlListModel 2.0

//<example brand="SHARP_PC12XX"
//idpocket="PC-1211"
//filename="./"
//name="Sharp pc-1211 SIMULATOR!!!  in progress..."
//image=":/pc1211/pc-1211.png"/>

XmlListModel {
    property var brandsearch;
    property var connectorsearch;
    property var brandPath;

    id: xmlModel
    source: "qrc:/pockemul/config.xml"
    query: "/demolauncher/demos/example"

    XmlRole { name: "brand"; query: "@brand/string()" }
    XmlRole { name: "idpocket"; query: "@idpocket/string()" }
    XmlRole { name: "filename"; query: "@filenamde/string()" }
    XmlRole { name: "name"; query: "@name/string()" }
    XmlRole { name: "image"; query: "@image/string()" }
    XmlRole { name: "connectortype"; query: "@connectortype/string()" }

    onStatusChanged: {
        if (status == XmlListModel.Ready) {
            console.log("xmlModel onStatusChanged: START found rows:"+count,brandsearch);
            sortedModel.clear();
            var _brand = brandsearch[brandsearch.length-1];
            for (var i=0; i<count; i++) {
                var item = get(i)
                //                        console.log(item.objects)
                if ( ( settings.groupByCategory && (item.brand === _brand)) ||
                     ( !settings.groupByCategory && !(item.idpocket.substring(0,1) === '#')) ||
                     ( !(item.connectortype==='') && ( item.connectortype === connectorsearch ))
                    )  {
                    sortedModel.append({rowid : i,
                                           brand: item.brand,
                                           idpocket: (item.idpocket),
                                           name: (item.name),
                                           image: (item.image),
                                           connectortype: (item.connectortype)});
                }
            }
        }
    }

}




