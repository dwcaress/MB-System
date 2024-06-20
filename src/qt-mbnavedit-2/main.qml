import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Controls.Universal 2.3
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import PixmapImage 1.0

/* ***
Display and edit swath file pings/beams

NOTE: objectName values in this file MUST match names used in
findChild() C++ calls.

*** */

Window {
    id: applicationWindow
    objectName: 'mainWindow'
    visible: true
    width: 1000
    height: 880
    title: 'qt-mbnavedit-2'

    // Interface to C++ Backend methods
    required property var backend
    

    MenuBar {
        Menu {
            title: qsTr("File")
            Action { text: qsTr("Open swath") ;
                onTriggered: { console.log("show file dialog")
                    fileDialog.open()} }
            Action { text: qsTr("Available swath files") ;
                onTriggered: { console.log("available swath files") }}
            Action { text: qsTr("Exit") ;
                onTriggered: { console.log("exit"); quitDialog.open()}}

        }
    }


    ColumnLayout {
        id: columnLayout
        Layout.fillHeight: false
        width: 1000

        CheckBox {
            objectName: 'timeInt'
            checked: true
            text: qsTr('Time interval')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'lon'
            checked: true
            text: qsTr('Longitude')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'lat'
            checked: true
            text: qsTr('Latitude')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'speed'
            checked: true
            text: qsTr('Speed')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'heading'
            checked: true
            text: qsTr('Heading')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'sensorDepth'
            checked: true
            text: qsTr('Sonar depth')
            onToggled: backend.setPlot(objectName, checked)
        }
        CheckBox {
            objectName: 'attitude'
            checked: false
            text: qsTr('Roll,pitch,heave')
            onToggled: backend.setPlot(objectName, checked)
        }

        ScrollView {
            implicitWidth: Window.window.width
            implicitHeight: Window.window.height
            contentHeight: 5000
            clip: true

            ColumnLayout {

                Rectangle {
                    id: rectangle
                    implicitWidth: Window.window.width * 0.9
                    implicitHeight:Window.window.height * 5

                    PixmapImage {
                        id: swathPixmap
                        objectName: 'swathPixmapObj'
                        width: rectangle.width
                        height: 30000
                        anchors.fill: parent
                    }


                    MouseArea {
                        id: swathMouseArea
                        objectName: 'swathMouseAreaObj'
                        anchors.fill: parent
                        implicitWidth: rectangle.width
                        implicitHeight: rectangle.height
                        hoverEnabled: false
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked: (mouse)=> {
                                       /// console.log("Mouse clicked at ",
                                       //        mouse.x, ", ", mouse.y)
                                       if (mouse.button == Qt.LeftButton) {
                                           // console.log('left clicked');
                                           backend.onLeftMouseButtonClicked(mouse.x,
                                                                            mouse.y);
                                       }
                                       else {
                                           // console.log("right clicked");
                                           backend.onRightMouseButtonClicked(mouse.x,
                                                                             mouse.y);
                                       }
                                   }

                        onPressed: (mouse) => {
                                       console.log("Mouse pressed at ",
                                                   mouse.x, ", ", mouse.y);
                                       console.log('button: ', mouse.button);
                                       if (mouse.button == Qt.LeftButton) {
                                           backend.onLeftMouseButtonDown(mouse.x, mouse.y)
                                       }
                                   }

                        onReleased: (mouse) => {
                                        console.log("Mouse released at ",
                                                    mouse.x, ", ", mouse.y)
                                        if (mouse.button == Qt.LeftButton) {
                                            backend.onLeftMouseButtonUp(mouse.x, mouse.y)
                                        }
                                    }

                        onPositionChanged: (mouse) => {
                                               console.log("Mouse moved at ",
                                                           mouse.x, ", ", mouse.y);
                                               console.log('pressed: ', pressed);
                                               console.log('button: ', mouse.button);
                                               console.log('buttons: ', mouse.buttons);
                                               if (mouse.buttons == Qt.LeftButton) {
                                                   backend.onMouseMove(mouse.x, mouse.y)
                                               }
                                               else {
                                                   console.log('do not call backend')
                                               }
                                           }
                    }


                }

            }
        }
    }
    MessageDialog {
        id: quitDialog
        title: "Quit?"
        icon: StandardIcon.Question
        text: "Quit application?"
        standardButtons: StandardButton.Yes |
                         StandardButton.No
        Component.onCompleted: visible = false
        onYes: Qt.quit(0)
        onNo: console.log("did not quit")
    }


    FileDialog {
        id: fileDialog
        title: "Open swath file"
        nameFilters: ["Swath files (*.mb[0-9]*)"]
        onAccepted: {
            console.log("accepted " + fileUrl);
            backend.processSwathFile(fileUrl);
        }
    }

    MessageDialog {
        id: infoDialog
        title: "Info"
        icon: StandardIcon.Info
        text: "Text goes here"
        standardButtons: StandardButton.Ok
        Component.onCompleted: visible = false
    }


    function showInfoDialog(message) {
        console.log('showInfoDialog()', message);
        infoDialog.text = message;
        infoDialog.open();
    }
}




