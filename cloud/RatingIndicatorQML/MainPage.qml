import QtQuick 1.1
import com.nokia.symbian 1.1


Page {
    id: mainPage
    Text {

        text: qsTr("Custom Rating Indicator !")
        color: platformStyle.colorNormalLight
        font.pixelSize: 20
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
    }


      MyRatingIndicator {
          id: myRatingIndicator
          width:parent.width

          anchors.verticalCenter: parent.verticalCenter
          anchors.left: parent.left
          anchors.leftMargin: 10


        }
        Text
        {
            id:textRating
            width: parent.width
            text: "Rating: " +myRatingIndicator.rating
            color: "white"
            font.pixelSize: 21
            anchors.top:myRatingIndicator.bottom
            anchors.topMargin: 80
            anchors.left: parent.left
            anchors.leftMargin: 140


        }


}
