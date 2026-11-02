import QtQuick
import QtGraphs

GraphsView {
    anchors.fill: parent
    seriesList: parent.seriesList
    axisX: parent.axisX
    axisY: parent.axisY

    ScatterSeries {
        pointDelegate: Rectangle {
            width: 20
            height: 20
            radius: 10
            color: "blue"
        }
    }
}