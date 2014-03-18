import QtQuick 1.1
Item {
  id: itemRatingIndicator
  property int rating: 3
    anchors.fill: parent


  Row{
       spacing: 5
      Image {
        id: imageStar1
        source: "star.png"

        opacity: 0.4
        scale: 0.5
        MouseArea {
          anchors.fill: parent
          height: parent.height+20
          width: parent.width+20

          onClicked: {
              itemRatingIndicator.rating = 1
          }
        }
        states: [
        State {
          name: "star"
          when: itemRatingIndicator.rating >= 1
          PropertyChanges {
            target: imageStar1
            opacity: 1
            scale: 1

          }
        }
        ]
        transitions: [
        Transition {

            NumberAnimation { properties: "opacity,scale"; easing.type: Easing.OutBounce; duration: 1000 }
        }
        ]
      }

      Image {
        id: imageStar2
        source: "star.png"

        opacity: 0.4
        scale: 0.5
        MouseArea {
          anchors.fill: parent
          height: parent.height+20
          width: parent.width+20

          onClicked: {
                itemRatingIndicator.rating = 2
          }
        }
        states: [
        State {
          name: "star"
          when: itemRatingIndicator.rating >= 2
          PropertyChanges {
            target: imageStar2
            opacity: 1
            scale: 1

          }
        }
        ]
        transitions: [
        Transition {

            NumberAnimation { properties: "opacity,scale"; easing.type: Easing.OutBounce; duration: 1000 }
        }
        ]
      }

      Image {
        id: imageStar3
        source: "star.png"

        opacity: 0.4
        scale: 0.5
        MouseArea {
          anchors.fill: parent
          height: parent.height+20
          width: parent.width+20

          onClicked: {
              itemRatingIndicator.rating = 3
          }
        }
        states: [
        State {
          name: "star"
          when: itemRatingIndicator.rating >= 3
          PropertyChanges {
            target: imageStar3
            opacity: 1
            scale: 1

          }
        }
        ]
        transitions: [
        Transition {

            NumberAnimation { properties: "opacity,scale"; easing.type: Easing.OutBounce; duration: 1000 }
        }
        ]
      }
      Image {
        id: imageStar4
        source: "star.png"

        opacity: 0.4
        scale: 0.5
        MouseArea {
          anchors.fill: parent
          height: parent.height+20
          width: parent.width+20

          onClicked: {
              itemRatingIndicator.rating = 4
          }
        }
        states: [
        State {
          name: "star"
          when: itemRatingIndicator.rating >= 4
          PropertyChanges {
            target: imageStar4
            opacity: 1
            scale: 1

          }
        }
        ]
        transitions: [
        Transition {

            NumberAnimation { properties: "opacity,scale"; easing.type: Easing.OutBounce; duration: 1000 }
        }
        ]
      }
      Image {
        id: imageStar5
        source: "star.png"

        opacity: 0.4
        scale: 0.5
        MouseArea {
          anchors.fill: parent
          height: parent.height+20
          width: parent.width+20

          onClicked: {
              itemRatingIndicator.rating = 5
          }
        }
        states: [
        State {
          name: "star"
          when: itemRatingIndicator.rating >= 5
          PropertyChanges {
            target: imageStar5
            opacity: 1
            scale: 1

          }
        }
        ]
        transitions: [
        Transition {

            NumberAnimation { properties: "opacity,scale"; easing.type: Easing.OutBounce; duration: 1000 }
        }
        ]
      }


  }




}
