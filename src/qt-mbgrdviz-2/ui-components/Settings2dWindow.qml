import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
/* ***
BackEnd singleton is registered in root context by main.cpp
See https://qml.guide/singletons/
*** */

Window {
    id: settings2dWindow
    objectName: "settings2dWindow"
    visible: true
    width: 640
    height: 480
    title: qsTr("2D Settings")

    GridLayout {
        id: gridLayout1
        columnSpacing: 5
        rowSpacing: 2
        anchors.rightMargin: -158
        anchors.bottomMargin: 10
        anchors.leftMargin: 158
        anchors.topMargin: 15
        anchors.fill: parent
        columns: 1
        rows: 6

        GroupBox {
            id: groupBox
            width: 200
            height: 200
            Layout.alignment: Qt.AlignLeft | Qt.AlignBaseline
            title: qsTr("View offsets and zoom:")

            Grid {
                id: grid
                width: 194
                anchors.leftMargin: 7
                spacing: 5
                anchors.fill: parent
                rows: 3
                columns: 2




                Text {
                    id: element1
                    height: 17
                    text: qsTr("X offset      ")
                    wrapMode: Text.NoWrap
                    textFormat: Text.AutoText
                    padding: 0
                    rightPadding: 0
                    font.pixelSize: 18
                }

                TextField {
                    id: xOffset
                    objectName: "2dXOffset"
                    text: qsTr("")
                    placeholderText: "0"
                }


                Text {
                    id: element
                    text: qsTr("Y offset")
                    font.pixelSize: 18
                }

                TextField {
                    id: yOffset
                    text: ""
                    objectName: "2dYOffset"
                    // text: qsTr("")
                    placeholderText: "0"
                    // onTextChanged: BackEnd.yOffset2d = text  // Testing...
                }

                Text {
                    id: element5
                    text: qsTr("Zoom")
                    font.pixelSize: 18
                }
                TextField {
                    id: zoom
                    objectName: "2dZoom"
                    text: qsTr("")
                    placeholderText: "1"
                }
            }
        }

        Frame {
            id: frame2
            width: 200
            height: 200

            GridLayout {
                id: gridLayout4
                anchors.fill: parent
                rows: 1
                columns: 2

                Button {
                    id: okButton
                    text: qsTr("Apply")
                    onClicked: {
                        console.log("clicked OK");
                        BackEnd.settings2dUpdated();
                    }

                }

                Button {
                    id: cancelButton
                    text: qsTr("Dismiss")
                    onClicked: { console.log("clicked Cancel"); settings2dWindow.close() }
                }
            }
        }
    }
}





































































































/*##^## Designer {
    D{i:3;anchors_height:400;anchors_width:400}D{i:11;anchors_height:100;anchors_width:100}
}
 ##^##*/
