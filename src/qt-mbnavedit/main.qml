import QtQuick 2.9
import QtQuick.Controls 2.3
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


ApplicationWindow {
    id: applicationWindow
    objectName: 'mainWindow'
    visible: true
    width: 1000
    height: 880
    title: 'qt-mbnavedit'

    // Interface to C++ Backend methods
    required property var backend
    

    menuBar: MenuBar {
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

        Rectangle {
            width: 600
            height: 600

            PixmapImage {
                id: swathPixmap
                objectName: 'swathPixmapObj'
                anchors.fill: parent
            }


            MouseArea {
                id: swathMouseArea
                objectName: 'swathMouseAreaObj'
                anchors.fill: parent
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




