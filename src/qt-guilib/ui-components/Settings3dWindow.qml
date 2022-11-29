import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
/* ***
BackEnd singleton is registered in root context by main.cpp
See https://qml.guide/singletons/
*** */

Window {
    id: settings3dWindow
    visible: true
    width: 640
    height: 480
    property alias element8: element8
    title: qsTr("3D settings")

    GridLayout {
        id: gridLayout1
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
            title: qsTr("Camera:")

            Grid {
                id: grid
                width: 400
                height: 400
                rows: 3
                columns: 2

                Text {
                    id: element1
                    text: qsTr("Azimuth")
                    font.pixelSize: 18
                }

                TextField {
                    id: viewAzimuth
                    objectName: "3dViewAzimuth"
                    placeholderText:  qsTr("0")
               }


                Text {
                    id: element
                    text: qsTr("Pitch")
                    font.pixelSize: 18
                }

                TextField {
                    id: viewElevation
                    objectName: "3dViewElevation"
                    placeholderText:  qsTr("90")
                }


                Text {
                    id: element5
                    text: qsTr("Zoom")
                   font.pixelSize: 18
                }
                TextField {
                    id: viewZoom
                    objectName: "3dViewZoom"
                    placeholderText:  qsTr("1")

                }
            }
        }

        GroupBox {
            id: groupBox1
            width: 200
            height: 200
            title: qsTr("Model:")

            GridLayout {
                id: gridLayout2
                width: 100
                height: 100
                rows: 3
                columns: 2

                Text {
                    id: element3
                    text: qsTr("Azimuth")
                    font.pixelSize: 18
                }

                TextField {
                    id: modelAzimuth
                    objectName: "3dModelAzimuth"
                    placeholderText:  qsTr("0")
                }


                Text {
                    id: element2
                    text: qsTr("Pitch")
                    font.pixelSize: 18

                }


                TextField {
                    id: modelElevation
                    objectName: "3dModelElevation"
                    placeholderText:  qsTr("90")

                }

                Text {
                    id: element6
                    text: qsTr("Zoom")
                    font.pixelSize: 18
                }

                TextField {
                    id: modelZoom
                    objectName: "3dModelZoom"
                    placeholderText:  qsTr("1")
                }
            }
        }

        Frame {
            id: frame
            width: 200
            height: 200

            GridLayout {
                id: gridLayout
                anchors.fill: parent
                rows: 1
                columns: 2

                /* ***
                Text {
                    id: element4
                    text: qsTr("Vertical exagg")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }
                *** */
                Label {
                    text: qsTr("Vertical exagg")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18                    
                }
                
                Slider {
                    id: zScale
                    Layout.fillWidth: true
                    from: 1
                    to: 10
                    value: 1
                    onValueChanged: mainWindow.qmlSignal("verticalExagg " +
                           value)
                }
                // Set displayed decimal places on slider label
                Label { text: "val: " + Math.round(zScale.value *100)/100 }                
            }
        }

        Frame {
            id: frame1
            width: 200
            height: 200

            GridLayout {
                id: gridLayout3
                anchors.fill: parent
                rows: 2
                columns: 2

                Text {
                    id: element7
                    text: qsTr("Pan X")
                    font.pixelSize: 18
                }

                TextField {
                    id: panX
                    objectName: "3dPanX"
                    placeholderText: qsTr("0")
                }

                Text {
                    id: element8
                    text: qsTr("Pan Y")
                    font.pixelSize: 18
                }

                TextField {
                    id: panY
                    objectName: "3dPanY"
                    placeholderText:  qsTr("0")
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
                        mainWindow.qmlSignal("settings3dUpdated")          
                    }
                }

                Button {
                    id: cancelButton
                    text: qsTr("Dismiss")
                    onClicked: { console.log("clicked Cancel"); settings3dWindow.close() }
                }
            }
        }
    }
}























































































/*##^## Designer {
    D{i:19;anchors_height:100;anchors_width:100}D{i:23;anchors_height:100;anchors_width:100}
D{i:29;anchors_height:100;anchors_width:100}
}
 ##^##*/
