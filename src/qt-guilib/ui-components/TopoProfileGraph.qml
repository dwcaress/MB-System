// TopoProfileGraph.qml
import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphs

/** Displays 2d vertical 'slice' between specified endpoints */
GraphsView {
    id: graphsView
    /// TEST TEST TEST
    property int topoTestInt: 999

    /// type:ValueAxis profile x-axis
    property alias xAxis: graphsView.axisX
    /// type:ValueAxis profile y-axis
    property alias yAxis: graphsView.axisY
    /// type:LineSeries profile x-y data
    property alias xyData: lineSeries

    anchors.fill: parent
    anchors.margins: 16
    theme: GraphsTheme {
        readonly property color c1: "#DBEB00"
        readonly property color c2: "#373F26"
        readonly property color c3: Qt.lighter(c2, 1.5)
        colorScheme: GraphsTheme.ColorScheme.Dark
        seriesColors: ["#2CDE85", "#DBEB00"]
        grid.mainColor: c3
        grid.subColor: c2
        axisX.mainColor: c3
        axisY.mainColor: c3
        axisX.subColor: c2
        axisY.subColor: c2
        axisX.labelTextColor: c1
        axisY.labelTextColor: c1
    }
    axisX: ValueAxis {
        max: 5
        labelDecimals: 1
    }

    axisY: ValueAxis {
        max: 10
        labelDecimals: 1
    }

    //! [linemarker]
    component Marker: Rectangle {
        width: 16
        height: 16
        color: "#ffffff"
        radius: width * 0.5
        border.width: 4
        border.color: "#000000"
    }

    //! [linemarker]
    LineSeries {
        id: lineSeries
        width: 4
        pointDelegate: Marker {}
        /* ***
        XYPoint {
            x: 0
            y: 0
        }
        XYPoint {
            x: 1
            y: 2.1
        }
        XYPoint {
            x: 2
            y: 3.3
        }
        XYPoint {
            x: 3
            y: 2.1
        }
        XYPoint {
            x: 4
            y: 4.9
        }
        XYPoint {
            x: 5
            y: 3.0
        }
        *** */
    }
}



/*##^## Designer {
    D{i:3;anchors_height:400;anchors_width:400}D{i:11;anchors_height:100;anchors_width:100}
}
 ##^##*/

