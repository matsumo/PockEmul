import QtQuick 1.0

Item {
    id: container

    // Year item dimensions
    property int yearWidth: (width-2*spacing)*0.4
    property int yearHeight: height
    // Month item dimensions
    property int monthWidth: (width-2*spacing)*0.3
    property int monthHeight: height
    // Day item dimensions
    property int dayWidth: (width-2*spacing)*0.3
    property int dayHeight: height
    // Font properties
    property string fontName: 'Helvetica'
    property int fontSize: 22
    property color fontColor: "#666666"
    // Spacing between items
    property int spacing: 8

    property Component itemBackground: Component {
        BorderImage {
            border { top: 8; bottom: 8; left: 8; right: 8 }
            source: "gfx/button.png"
        }
    }
    property Component itemBackgroundPressed: Component {
        BorderImage {
            border { top: 8; bottom: 8; left: 8; right: 8 }
            source: "gfx/button_pressed.png"
        }
    }

    width: 240
    height:  60

    Component.onCompleted: {
        var date = new Date();
        month.index = date.getMonth();
        day.index = date.getDate()-1;
    }

    Component {
        id: yearDelegate
        Button {
            width: container.yearWidth
            height: container.yearHeight
            text: number
            fontColor: container.fontColor
            fontName: container.fontName
            fontSize: container.fontSize
            bg: itemBackground
            bgPressed: itemBackgroundPressed
            onClicked: { year.index = index; year.toggle() }
        }
    }

    Component {
        id: monthDelegate
        Button {
            width: container.monthWidth
            height: container.monthHeight
            text: number
            fontColor: container.fontColor
            fontName: container.fontName
            fontSize: container.fontSize
            bg: itemBackground
            bgPressed: itemBackgroundPressed
            onClicked: { month.index = index; month.toggle() }
            opacity: (index+1 < months.start) ? 0.5 : 1.0
        }
    }

    Component {
        id: dayDelegate
        Button {
            width: container.dayWidth
            height: container.dayHeight
            text: number
            fontColor: container.fontColor
            fontName: container.fontName
            fontSize: container.fontSize
            bg: itemBackground
            bgPressed: itemBackgroundPressed
            onClicked: { day.index = index; day.toggle() }
            opacity: (index+1 < days.start || index+1 > days.end) ? 0.5 : 1.0
        }
    }
    Row {
        id: reels
        spacing: container.spacing

        Reel {
            id: year
            width: container.yearWidth
            height: container.yearHeight
            model: years
            delegate: yearDelegate
            onIndexChanged: months.update()
            autoClose: false            
        }

        Reel {
            id: month
            width: container.monthWidth
            height: container.monthHeight
            model: months
            delegate: monthDelegate
            onIndexChanged: { checkIndex(); days.update() }
            autoClose: false
            function checkIndex() {
                if (index+1 < months.start) index = (index+1 < months.start-(index+1)) ? index = 11 : index = months.start-1
            }
        }

        Reel {
            id: day
            width: container.dayWidth
            height: container.dayHeight            
            onIndexChanged: checkIndex()
            model: days
            delegate:  dayDelegate
            autoClose: false
            function checkIndex() {
                if (index+1 < days.start) {
                    index = (31-days.end+index+1 < days.start-(index+1)) ? days.end-1 : days.start-1
                } else if (index + 1 > days.end) {
                    index = (index+1-days.end < 31-(index+1)+days.start) ? days.end-1 : days.start-1
                }
            }
        }
    }


    ListModel{
        id: years        
        Component.onCompleted: {
            months.update();
        }
        ListElement { number: "2011" }
        ListElement { number: "2012" }
        ListElement { number: "2013" }
        ListElement { number: "2014" }
        ListElement { number: "2015" }
        ListElement { number: "2016" }
    }

    ListModel{
        id: months
        property int start: 1        
        function update() {
            var date = new Date();
            months.start = (year.index === 0) ? date.getMonth()+1 : 1;
            month.checkIndex();
            days.update();
        }

        ListElement { number: "1"; name: "January" }
        ListElement { number: "2"; name: "February" }
        ListElement { number: "3"; name: "March" }
        ListElement { number: "4"; name: "April" }
        ListElement { number: "5"; name: "May" }
        ListElement { number: "6"; name: "June" }
        ListElement { number: "7"; name: "July" }
        ListElement { number: "8"; name: "August" }
        ListElement { number: "9"; name: "September" }
        ListElement { number: "10"; name: "October" }
        ListElement { number: "11"; name: "November" }
        ListElement { number: "12"; name: "December" }
    }

    ListModel {
        id: days
        property int start: 1
        property int end: 31

        function update() {
            var date = new Date();

            var selectedYear = date.getFullYear();
            try { selectedYear = years.get(year.index).number;
            } catch(err) {}

            days.start = 1;
            if (selectedYear == date.getFullYear() && month.index === date.getMonth()) days.start = date.getDate();

            // Determine the amount of days in month
            days.end = 32 - new Date(selectedYear, month.index, 32).getDate();

            if (day.index+1 < days.start) day.index = days.start-1;
            else if (day.index+1 > days.end ) day.index = days.end-1;
        }

        ListElement { number: "1"}
        ListElement { number: "2"}
        ListElement { number: "3"}
        ListElement { number: "4"}
        ListElement { number: "5"}
        ListElement { number: "6"}
        ListElement { number: "7"}
        ListElement { number: "8"}
        ListElement { number: "9"}
        ListElement { number: "10"}
        ListElement { number: "11"}
        ListElement { number: "12"}
        ListElement { number: "13"}
        ListElement { number: "14"}
        ListElement { number: "15"}
        ListElement { number: "16"}
        ListElement { number: "17"}
        ListElement { number: "18"}
        ListElement { number: "19"}
        ListElement { number: "20"}
        ListElement { number: "21"}
        ListElement { number: "22"}
        ListElement { number: "23"}
        ListElement { number: "24"}
        ListElement { number: "25"}
        ListElement { number: "26"}
        ListElement { number: "27"}
        ListElement { number: "28"}
        ListElement { number: "29"}
        ListElement { number: "30"}
        ListElement { number: "31"}
    }

    function getDate() {
        return "" + months.get(month.index).name + " " + days.get(day.index).number

    }
}
