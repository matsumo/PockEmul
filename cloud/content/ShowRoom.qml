import QtQuick 2.0

Item {
    id: showRoom

    width: 1920
    height: 1080

    QtObject {
        id: settings
        // These are used to scale fonts according to screen size
        property real _scaler: 300 + showRoom.width * showRoom.height * 0.00015
        property int fontXS: _scaler * 0.032
        property int fontS: _scaler * 0.040
        property int fontM: _scaler * 0.046
        property int fontMM: _scaler * 0.064
        property int fontL: _scaler * 0.100
        // Settings
        property bool groupByCategory: true
        property bool showFogParticles: false
        property bool showShootingStarParticles: false
        property bool showLighting: false
        property bool showColors: false
    }

    MainView {
        id: mainView
    }

//    InfoView {
//        id: infoView
//    }

    DetailsView {
        id: detailsView
    }

    ListModel {
        id: sortedModel

    }

    PObjectsModel {
        id: pobjectsmodel
        brandsearch: settings.groupByCategory ? ['BRAND'] : []
    }

//    FpsItem {
//        anchors.top: parent.top
//        anchors.topMargin: 8
//        anchors.left: parent.left
//        anchors.leftMargin: 8
//    }
}
