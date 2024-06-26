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
    
    ActionGroup {
        id: viewActions
        exclusive: true
    }


    MenuBar {
        id: menuBar

        Menu {
            title: qsTr('File')
            Action { text: qsTr('Open swath') ;
                onTriggered: { console.log('show file dialog')
                    fileDialog.open()} }
            Action { text: qsTr('Available swath files') ;
                onTriggered: { console.log('available swath files') }}
            Action { text: qsTr('Exit') ;
                onTriggered: { console.log('exit'); quitDialog.open()}}
        }

        Menu {
            title: qsTr('View')
            Action { text: qsTr('Zoom in') ; checkable: true; ActionGroup.group: viewActions;
                onTriggered: { swathMouseArea.cursorShape=Qt.SizeFDiagCursor }}
            Action { text: qsTr('Zoom out') ; checkable: true; ActionGroup.group: viewActions ;
                onTriggered: { swathMouseArea.cursorShape= Qt.SizeBDiagCursor }}
            Action { text: qsTr('Point') ; checkable: true; ActionGroup.group: viewActions;
                onTriggered: { swathMouseArea.cursorShape=Qt.ArrowCursor }}

        }
    }


    ColumnLayout {
        id: columnLayout
        anchors.top: menuBar.bottom
        Layout.fillHeight: false
        width: 1000

        Row {
            id: buttonRow

            ButtonGroup {
                id: editModes
            }

            RadioButton {
                objectName: 'pickMode'
                checked: true
                text: qsTr('Pick')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = false }
            }

            RadioButton {
                objectName: 'selectMode'
                text: qsTr('Select')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = false }
            }

            RadioButton {
                objectName: 'deselectMode'
                text: qsTr('De-select')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = false }
            }

            RadioButton {
                objectName: 'selectAllMode'
                text: qsTr('Select all')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = false }
            }

            RadioButton {
                objectName: 'deselectAllMode'
                text: qsTr('De-select all')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = false}
            }

            RadioButton {
                objectName: 'defineIntervalMode'
                text: qsTr('Define interval')
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); resetInterval.enabled = true}
            }

            Button {
                id: resetInterval
                text: 'Reset interval'
                onClicked: { backend.onResetInterval() }
            }
        }

        Row {
            Button {
                id: interpolate
                text: 'Interp'
                onClicked: { backend.onInterpolate() }
            }

            Button {
                id: interpolateRep
                text: 'Interp Rep'
                onClicked: { backend.onInterpolateRepeat() }
            }	
	}
	
        Row {
            Button {
                id: swathStart
                text: 'Start'
                onClicked: { backend.onGoStart() }
            }

            Button {
                id: swathForward
                text: 'FWD'
                onClicked: {backend.onGoForward() }
            }

            Button {
                id: swathBack
                text: 'REW'
                onClicked: { backend.onGoBack() }
            }

            Button {
                id: swathEnd
                text: 'End'
                onClicked: { backend.onGoEnd() }
            }


        }
        Row {

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
                        // NOTE: QML-defined MouseArea must be EXACTLY
			// fitted on PixmapImage to ensure correct
			// mapping and scaling of mouse events			
                        anchors.fill: swathPixmap

                        hoverEnabled: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
			preventStealing: true
			
                        onClicked: (mouse)=> {
                                       if (mouse.button == Qt.LeftButton) {
                                           // console.log('left clicked');
                                           backend.onLeftButtonClicked(mouse.x,
                                                                       mouse.y);
                                       }
                                       else if (mouse.button == Qt.RightButton) {
                                           // console.log('right clicked');
                                           backend.onRightButtonClicked(mouse.x,
                                                                        mouse.y);
                                       }
                                       else if (mouse.button == Qt.MiddleButton) {
                                           backend.onMiddleButtonClicked(mouse.x, mouse.y)
                                       }
                                   }

                        onPressed: (mouse) => {
                                       // console.log('Mouse pressed at ',
                                          //         mouse.x, ', ', mouse.y);
                                       console.log('button: ', mouse.button);
                                       if (mouse.button == Qt.LeftButton) {
                                           //   backend.onLeftMouseButtonDown(mouse.x, mouse.y)
                                       }
                                   }

                        onReleased: (mouse) => {
                                        // console.log('Mouse released at ',
                                           //         mouse.x, ', ', mouse.y)
                                        if (mouse.button == Qt.LeftButton) {
                                            // backend.onLeftMouseButtonUp(mouse.x, mouse.y)
                                        }
                                    }

                        onPositionChanged: (mouse) => {

                                               console.log('Mouse moved at ',
                                                          mouse.x, ', ', mouse.y);
                                                console.log('pressed: ', pressed);
                                               console.log('button: ', mouse.button);
                                               console.log('buttons: ', mouse.buttons);

                                               if (mouse.buttons == Qt.LeftButton) {
                                                   backend.onMouseMoved(mouse.x, mouse.y)
                                               }
                                           }
                    }


                }

            }
        }
    }
    MessageDialog {
        id: quitDialog
        title: 'Quit?'
        icon: StandardIcon.Question
        text: 'Quit application?'
        standardButtons: StandardButton.Yes |
                         StandardButton.No
        Component.onCompleted: visible = false
        onYes: Qt.quit(0)
        onNo: console.log('did not quit')
    }


    FileDialog {
        id: fileDialog
        title: 'Open swath file'
        nameFilters: ['Swath files (*.mb[0-9]*)']
        onAccepted: {
            console.log('accepted ' + fileUrl);
            backend.processSwathFile(fileUrl);
        }
    }

    MessageDialog {
        id: infoDialog
        title: 'Info'
        icon: StandardIcon.Info
        text: 'Text goes here'
        standardButtons: StandardButton.Ok
        Component.onCompleted: visible = false
    }


    function showInfoDialog(message) {
        console.log('showInfoDialog()', message);
        infoDialog.text = message;
        infoDialog.open();
    }
}




