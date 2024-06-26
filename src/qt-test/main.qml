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
    id: window
    objectName: 'mainWindow'
    visible: true
    width: 1000
    height: 880
    title: 'qt-mbnavedit-2'

    
    ColumnLayout {
        id: columnLayout
        anchors.top: parent.top
        Layout.fillHeight: false
        width: 1000

        Row {

            CheckBox {
                objectName: 'timeInt'
                checked: true
                text: qsTr('Time interval')
            }
            CheckBox {
                objectName: 'lon'
                checked: true
                text: qsTr('Longitude')
            }
            CheckBox {
                objectName: 'lat'
                checked: true
                text: qsTr('Latitude')
            }
            CheckBox {
                objectName: 'speed'
                checked: true
                text: qsTr('Speed')
            }
            CheckBox {
                objectName: 'heading'
                checked: true
                text: qsTr('Heading')
            }
            CheckBox {
                objectName: 'sensorDepth'
                checked: true
                text: qsTr('Sonar depth')
            }
            CheckBox {
                objectName: 'attitude'
                checked: false
                text: qsTr('Roll,pitch,heave')
            }
        }


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
            }

            RadioButton {
                objectName: 'selectMode'
                text: qsTr('Select')
                ButtonGroup.group: editModes
            }

            RadioButton {
                objectName: 'deselectMode'
                text: qsTr('De-select')
                ButtonGroup.group: editModes
            }

            RadioButton {
                objectName: 'selectAllMode'
                text: qsTr('Select all')
                ButtonGroup.group: editModes
            }

            RadioButton {
                objectName: 'deselectAllMode'
                text: qsTr('De-select all')
                ButtonGroup.group: editModes
            }

            RadioButton {
                objectName: 'defineIntervalMode'
                text: qsTr('Define interval')
                ButtonGroup.group: editModes
            }
        }

        Row {
            Button {
                id: swathStart
                text: 'Start'
            }

            Button {
                id: swathForward
                text: 'FWD'
            }

            Button {
                id: swathBack
                text: 'REW'
            }

            Button {
                id: swathEnd
                text: 'End'
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
                    implicitWidth: 800
                    implicitHeight:800
		    color: 'red'
		    
                    PixmapImage {
                        id: swathPixmap
                        objectName: 'swathPixmapObj'

			width: rectangle.width
			height: rectangle.height
			
			// width: rectangle.width
			// height: rectangle.height
                    }

		    MouseArea {
		        id: mouseArea
			anchors.top: swathPixmap.top
			anchors.bottom: swathPixmap.bottom
			anchors.left: swathPixmap.left
			anchors.right: swathPixmap.right

                        hoverEnabled: false
                        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                        onClicked: (mouse)=> {
                             if (mouse.button == Qt.LeftButton) {
                                  console.log('left ', mouse.x, ', ', mouse.y)
                              }
                              else if (mouse.button == Qt.RightButton) {
                                  console.log('right')
                              }
                              else if (mouse.button == Qt.MiddleButton) {
                                  console.log('center')
                              }
                         }
		    }

                }
            }
        }
    }
}




