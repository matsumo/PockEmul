import QtQuick 2.0

Item {
    id: root

    property string name
    property bool isSelected: listView.currentIndex === index

    width: parent ? parent.width : imageItem.width
    height: imageItem.height
    z: isSelected ? 1000 : -index
    rotation: isSelected ? 0 : -15
    scale: isSelected ? mainView.height/540 : mainView.height/1080
    opacity: 1.0 - Math.abs((listView.currentIndex - index) * 0.25)

    Behavior on rotation {
        NumberAnimation { duration: 500; easing.type: Easing.OutBack }
    }
    Behavior on scale {
        NumberAnimation { duration: 1500; easing.type: Easing.OutElastic }
    }
    Behavior on opacity {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("*****",model.brand,model.idpocket);
            var _b = model.brand;
            if (model.idpocket.substring(0, 1)==='#') {
                // drill down into brand
                var _brand = model.idpocket;
                var _res = _brand.split('#');
                console.log(_brand,_res[0],_res[1],_res[2]);

                pobjectsmodel.brandsearch.push(_res[2]);
                pobjectsmodel.reload();
            }
            else {
                sendLoadPocket(model.idpocket);
                showroom.visible =false;
            }
//            if (isSelected) {
//                detailsView.image = model.image
//                detailsView.name =  model.name
//                detailsView.year = model.year
//                detailsView.director = model.director
//                detailsView.cast = model.cast
////                detailsView.rating = model.rating
//                detailsView.overview = model.overview
//                detailsView.show();
//            } else {
//                listView.currentIndex = index;
//                if (settings.showShootingStarParticles) shootingStarBurst.burst(50);
//            }
        }
    }

    Image {
        id: imageItem
        width: 500 //parent.width / 2
        height: 300 //parent.height / 2
        anchors.horizontalCenter: parent.horizontalCenter
        fillMode: Image.PreserveAspectFit
        source: "qrc" + model.image
        visible: !settings.showLighting
    }

    ShaderEffectSource {
        id: s1
        sourceItem: imageItem
        hideSource: settings.showLighting
        visible: settings.showLighting
    }

    ShaderEffect {
        anchors.fill: imageItem
        property variant src: s1
        property variant srcNmap: coverNmapSource
        property real widthPortition: mainView.width/imageItem.width
        property real heightPortition: mainView.height/imageItem.height
        property real widthNorm: widthPortition * 0.5 - 0.5
        property real heightNorm: root.y/imageItem.height - listView.contentY / imageItem.height
        property real lightPosX: listView.globalLightPosX * widthPortition - widthNorm
        property real lightPosY: listView.globalLightPosY * heightPortition - heightNorm
        visible: settings.showLighting

        fragmentShader: "
            uniform sampler2D src;
            uniform sampler2D srcNmap;
            uniform lowp float qt_Opacity;
            varying highp vec2 qt_TexCoord0;
            uniform highp float lightPosX;
            uniform highp float lightPosY;
            void main() {
                highp vec4 pix = texture2D(src, qt_TexCoord0.st);
                highp vec4 pix2 = texture2D(srcNmap, qt_TexCoord0.st);
                highp vec3 normal = normalize(pix2.rgb * 2.0 - 1.0);
                highp vec3 light_pos = normalize(vec3(qt_TexCoord0.x - lightPosX, qt_TexCoord0.y - lightPosY, 0.8 ));
                highp float diffuse = max(dot(normal, light_pos), 0.2);

                // boost a bit
                diffuse *= 2.5;

                highp vec3 color = diffuse * pix.rgb;
                gl_FragColor = vec4(color, pix.a) * qt_Opacity;
            }
        "
    }
}
