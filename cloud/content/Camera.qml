import QtQuick 2.0
import QtMultimedia 5.5
import QZXing 2.3

Item {
    anchors.fill: parent

    Camera {
        id: camera
        imageCapture {
            onImageCaptured: {
                // Show the preview in an Image
                photoPreview.source = preview; //"http://ereimer.net/rants/qrcode-EreimerDotNet-zxing.png";
                console.log(preview);
//                decoder.decodeImageQML(preview);

            }
        }
    }
    Column {
        VideoOutput {
            source: camera
            focus: visible // To receive focus and capture key events when visible
            width: 320; height: 240
            MouseArea {
                anchors.fill: parent
                onClicked: main.test(); //camera.imageCapture.capture()
            }
        }
        Text {
            id: text
            height: 15
            text: qsTr("Last Captured Image (none)")
        }
        Image {
            id: photoPreview
//            source: "http://ereimer.net/rants/qrcode-EreimerDotNet-zxing.png"
//            source: "http://www.podiocom.com/img/vehicules/bus_car_vip/bus_double_etage/slideshow/bus_double_etage04.jpg"
            width: 300; height: 300
            cache: false
            asynchronous: false
            onStatusChanged: {
                if (photoPreview.status == Image.Ready) {
                    console.log('Loaded');
                    photoPreview.grabToImage(function(result) {
                        decoder.decodeImageQML(result);
                    },
                    Qt.size(250, 250));
                }
            }
        }
    }

    QZXing{
        id: decoder

        enabledDecoders: QZXing.DecoderFormat_QR_CODE

        onDecodingStarted: console.log("Decoding of image started...")

        onTagFound: console.log( "Barcode data: " + tag)

        onDecodingFinished: console.log("Decoding finished " + (succeeded==true ? "successfully" :    "unsuccessfully") )
    }
}
