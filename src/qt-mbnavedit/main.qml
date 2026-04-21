import QtQuick.Controls 2.15
import QtQuick.Window 2.15
import QtQuick.Controls.Universal 2.3
import QtQuick.Dialogs
import QtQuick.Layouts 1.14
import QtQuick.Controls 2.15
import QtQuick.Window 2.14
import PixmapImage 1.0

/* ***
   Display and edit swath file navigation metadata

   NOTE: objectName values in this file MUST match object names used by C++ backend

   *** */

Window {
    id: appWindow
    objectName: 'mainWindow'
    visible: true
    width: 1350
    height: 880
    title: 'qt-mbnavedit'
    color: 'lightgray'

    // Interface to C++ Backend methods
    required property var backend
    

    Component.onCompleted: {

        // Synchronize GUI input control state with C++ backend?
        // Difficult to do here because each GUI button/checkbox has a
	// different associated backend function to invoke. So instead we
	// put that synchronization in the definition
        // section for each button/checkbox.

    }

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
                onTriggered: { console.log('show swath list')
                    dataList.open()} }
            Action { text: qsTr('Exit') ;
                onTriggered: { console.log('exit'); quitDialog.open()}}
        }
    }

    ColumnLayout {
        id: columnLayout
        anchors.top: menuBar.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom


        Flow {
            id: buttonRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 1

            ButtonGroup {
                id: editModes
                exclusive: true
            }

            RadioButton {

                text: qsTr('Pick')

                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setPickMode();
                    resetInterval.enabled = false }
            }

            RadioButton {
                text: qsTr('Select')
                checked: true
                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setSelectMode();
                    resetInterval.enabled = false }

                Component.onCompleted: {
                    if (checked) backend.setSelectMode();
                    resetInterval.enabled = false }
            }

            RadioButton {
                text: qsTr('De-select')
                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setDeselectMode();
                    resetInterval.enabled = false }
            }

            RadioButton {
                text: qsTr('Select all')
                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setSelectAllMode();
                    resetInterval.enabled = false }
            }

            RadioButton {
                text: qsTr('De-select all')
                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setDeselectAllMode();
                    resetInterval.enabled = false}
            }

            RadioButton {
                text: qsTr('Define interval')
                ButtonGroup.group: editModes
                onCheckedChanged: { if (checked) backend.setDefineIntervalMode();
                    resetInterval.enabled = true}
            }

            Button {
                id: resetInterval
                text: 'Reset interval'
                onClicked: { backend.onResetInterval() }
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredHeight: 1

            Flow {
                Layout.fillHeight: true
                Layout.fillWidth: true

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

            Flow {
                Layout.fillHeight: true
                Layout.fillWidth: true

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
        }



        Rectangle {
            id: testRectangle
            // width: 300
            height: 20
            color: "red"
            border.color: "black"
            border.width: 5
            radius: 10
            Layout.fillWidth: true
            Layout.rightMargin: 20
        }


        RowLayout {
            Layout.preferredHeight: 4

            Flow {
                id: plotSelectColumn
                // anchors.top: parent.top
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                flow: Flow.TopToBottom

                Label {
                    bottomPadding: 10
                    text: qsTr('PLOTS')
                }

                ButtonGroup {
                    id: plotButtons
                    exclusive: false
                }

                CheckBox {
                    checked: true
                    text: qsTr('Time interval')
                    onToggled: backend.setTimeIntPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    text: qsTr('Original data')
                    checked: true
                    leftPadding: indicator.width
                    onToggled: backend.setTimeIntOrigPlot(checked)
                    ButtonGroup.group: plotButtons

                }

                CheckBox {
                    checked: true
                    text: qsTr('Longitude')
                    onToggled: backend.setLonPlot(checked)
                    ButtonGroup.group: plotButtons

                }

                CheckBox {
                    checked: true
                    text: qsTr('Original')
                    leftPadding: indicator.width
                    onToggled: backend.setLonOrigPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Latitude')
                    onToggled: backend.setLatPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Original')
                    leftPadding: indicator.width
                    onToggled: backend.setLatOrigPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Speed')
                    onToggled: backend.setSpeedPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Original')
                    leftPadding: indicator.width
                    onToggled: backend.setSpeedOrigPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    text: qsTr('Speed made good')
                    leftPadding: indicator.width
                    onToggled: backend.setSpeedMadeGoodPlot(checked)
                    Component.onCompleted: { checked = true }
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Heading')
                    onToggled: backend.setHeadingPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Original')
                    leftPadding: indicator.width
                    onToggled: backend.setHeadingOrig(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    text: qsTr('Course made good')
                    leftPadding: indicator.width
                    onToggled: backend.setHeadingMadeGoodPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Sonar depth')
                    onToggled: backend.setSonarDepthPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: true
                    text: qsTr('Original')
                    leftPadding: indicator.width
                    onToggled: backend.setSonarDepthOrigPlot(checked)
                    ButtonGroup.group: plotButtons
                }

                CheckBox {
                    checked: false
                    text: qsTr('Roll,pitch,heave')
                    onToggled: backend.setAttitudePlot(checked)
                    ButtonGroup.group: plotButtons
                }
            }


            Rectangle {
                color: white
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredWidth: 3

                ScrollView {
                    anchors.fill: parent
                    contentHeight: 5000
                    clip: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
                    ScrollBar.vertical.policy: ScrollBBar.AlwaysOn

                    Rectangle {
                        id: rectangle
                        // Layout.fillWidth: true
                        // Layout.fillHeight: true
                        anchors.fill: parent

                        border.width: 1
                        border.color: 'black'

                        PixmapImage {
                            id: swathPixmap
                            objectName: backend.swathPixmapObj
                            width: rectangle.width
                            height: 30000
                            anchors.fill: parent
                            anchors.margins: 3
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

                            // mouseArea and pixmapImage are aligned, so notify backend of mouseArea
                            // size change, so backend maintains proper scaling/mouse-event mapping
                            onWidthChanged: { console.log('mouseArea width = ', swathMouseArea.width);
                                backend.onPixmapImageResize(swathMouseArea.width, swathMouseArea.height) }
                            onHeightChanged: { console.log('mouseArea height: ', swathMouseArea.height)
                                backend.onPixmapImageResize(swathMouseArea.width, swathMouseArea.height)}

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
                                                   /* ****
                                   console.log('Mouse moved at ',
                                   mouse.x, ', ', mouse.y);
                                   console.log('pressed: ', pressed);
                                   console.log('button: ', mouse.button);
                                   console.log('buttons: ', mouse.buttons);
                                   **** */
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
            // icon: StandardIcon.Question
            text: 'Quit application?'
            buttons: MessageDialog.Yes |
                     MessageDialog.No
            Component.onCompleted: visible = false
            onAccepted: Qt.quit(0)
            // onNoClicked: console.log('did not quit')
        }


        FileDialog {
            id: fileDialog
            title: 'Open swath file'
            nameFilters: ['Swath files (*.mb[0-9][0-9])']
            onAccepted: {
                console.log('accepted ' + selectedFile);
                backend.processSwathFile(selectedFile);
            }
        }

        MessageDialog {
            id: infoDialog
            title: 'Info'
            text: 'Text goes here'
            buttons: MessageDialog.Ok
            Component.onCompleted: visible = false
        }


        // List swath files that have been opened or specified in datalist file
        MessageDialog {
            id: dataList
            title: 'Swath file list'
            buttons: MessageDialog.Ok
        }


        function showInfoDialog(message) {
            console.log('showInfoDialog()', message);
            infoDialog.text = message;
            infoDialog.open();
        }
    }
}




