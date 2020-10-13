import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

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
            title: qsTr("Data color bounds:")

            Grid {
                id: grid
                width: 400
                height: 400
                rows: 3
                columns: 2




                Text {
                    id: element1
                    text: qsTr("Min      ")
                    font.pixelSize: 18
                }

                TextField {
                    id: viewAzimuth
                    text: qsTr("Text Field")
                }


                Text {
                    id: element
                    text: qsTr("Max")
                    font.pixelSize: 18
                }

                TextField {
                    id: viewElevation
                    text: qsTr("Text Field")
                }




            }
        }

        GroupBox {
            id: groupBox1
            width: 200
            height: 200
            title: qsTr("Model orientation and zoom:")

            GridLayout {
                id: gridLayout2
                width: 100
                height: 100
                rows: 3
                columns: 2

                Text {
                    id: element3
                    text: qsTr("Model azimuth")
                    font.pixelSize: 18
                }

                TextField {
                    id: modelAzimuth
                    text: qsTr("Text Field")
                }


                Text {
                    id: element2
                    text: qsTr("Model elevation")
                    font.pixelSize: 18

                }


                TextField {
                    id: modelElevation
                    text: qsTr("Text Field")
                }

                Text {
                    id: element6
                    text: qsTr("Model zoom")
                    font.pixelSize: 18
                }

                TextField {
                    id: modelZoom
                    text: qsTr("Text Field")
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

                Text {
                    id: element4
                    text: qsTr("Vertical exagg")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                TextField {
                    id: textField
                    text: qsTr("Text Field")
                }
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
                    text: qsTr("Text Field")
                }

                Text {
                    id: element8
                    text: qsTr("Pan Y")
                    font.pixelSize: 18
                }

                TextField {
                    id: panY
                    text: qsTr("Text Field")
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
                    text: qsTr("OK")
                    onClicked: { console.log("clicked OK"); settings3dWindow.close() }

                }

                Button {
                    id: cancelButton
                    text: qsTr("Cancel")
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
