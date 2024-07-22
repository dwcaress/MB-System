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


Window {
    id: applicationWindow
    objectName: 'mainWindow'
    visible: true
    width: 1000
    height: 880
    title: 'qt-mbedit'

    // Interface to C++ Backend methods
    required property var backend
    
    ActionGroup {
        id: ancillaryData
        exclusive: true
    }

    ActionGroup {
        id: slice
        exclusive: true
    }

    ActionGroup {
        id: colorCoding
        exclusive: true
    }

    MenuBar {
        id: menuBar

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

        Menu {
            title: qsTr("&View")
            Action { checkable: true; text: qsTr("&Show flagged soundings") }
            Action { checkable: true; text: qsTr("&Show flagged profiles") }
            MenuSeparator {}
            Menu {
                title: "Ancillary data"

                Action {objectName: "none"; checkable: true; checked: true;
                    text: qsTr("None");
                    ActionGroup.group: ancillaryData;
                    onTriggered: backend.onAncillDataChanged(objectName)
                }

                Action {objectName: "time"; checkable: true; text: qsTr("Time");
                    ActionGroup.group: ancillaryData;
                    onTriggered: backend.onAncillDataChanged(objectName)
                }

                Action {objectName: "interval"; checkable: true;
                    text: qsTr("Interval");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }

                Action {objectName: "latitude"; checkable: true;
                    text: qsTr("Latitude");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }

                Action {objectName: "longitude"; checkable: true;
                    text: qsTr("Longitude");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "heading"; checkable: true;
                    text: qsTr("Heading");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "speed"; checkable: true;
                    text: qsTr("Speed");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "depth"; checkable: true;
                    text: qsTr("Depth");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "altitude"; checkable: true;
                    text: qsTr("Altitude");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "sensorDepth"; checkable: true;
                    text: qsTr("Sensor depth");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "roll"; checkable: true;
                    text: qsTr("Roll");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "pitch"; checkable: true;
                    text: qsTr("Pitch");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }
                Action {objectName: "heave"; checkable: true;
                    text: qsTr("Heave");
                    ActionGroup.group: ancillaryData
                    onTriggered: backend.onAncillDataChanged(objectName)
                }

            }
            MenuSeparator {}
            Menu {
                title: "Slice"
                Action {objectName: "waterfall"; checkable: true; checked: true;
                    text: qsTr("Waterfall"); ActionGroup.group: slice
                    onTriggered: backend.onSliceChanged(objectName)
                }
                Action {objectName: "alongTrack"; checkable: true;
                    text: qsTr("Along-track"); ActionGroup.group: slice
                    onTriggered: backend.onSliceChanged(objectName)
                }
                Action {objectName: "crossTrack"; checkable: true;
                    text: qsTr("Cross-track"); ActionGroup.group: slice
                    onTriggered: backend.onSliceChanged(objectName)
                }
            }
            MenuSeparator {}
            Menu {
                title: "Color-coding"
                Action {objectName: 'bottomDetect';
                    checkable: true; checked: true;
                    text: qsTr("Bottom-detect algorithm");
                    ActionGroup.group: colorCoding;
                    onTriggered: backend.onColorCodeChanged(objectName)}
                Action {objectName: 'pulseSource';
                    checkable: true;
                    text: qsTr("Pulse source");
                    ActionGroup.group: colorCoding;
                    onTriggered: backend.onColorCodeChanged(objectName)}
                Action {objectName: 'flagState'; checkable: true;
                    text: qsTr("Flag state");
                    ActionGroup.group: colorCoding;
                    onTriggered: backend.onColorCodeChanged(objectName)}
            }

        }
        Menu {
            title: "&Control"
            Action {text: qsTr("Go to specified time...")}
            Action {text: qsTr("Buffer controls...")}
            Action {text: qsTr("Annotate...")}
            Action {text: qsTr("Filters...")}
        }


        Menu {
            title: "Help"
            Action {text: qsTr("About"); onTriggered: {
                    console.log("show version info");
                    myMessageDialog.text = qsTr("PROTOTYPE");
                    myMessageDialog.open()
                }
            }
        }
    }

    ColumnLayout {
        id: columnLayout
        anchors.top: menuBar.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: parent.bottom

        Flow {
            Label { text: "Xrack width (m) " + xtrackSlider.value.toFixed(0) }
            Layout.fillWidth: true
            Layout.fillHeight: true

            Slider {
                id: xtrackSlider
                objectName: "xTrackSliderObj"
                Layout.fillWidth: true
                Layout.fillHeight: true
                from: 1
                to: 300
                value: 150
                live: false  // only update value when button released
                Component.onCompleted: { backend.onXtrackChanged(value) }
                onValueChanged: { console.log('xTrackSlider moved');
                    backend.onXtrackChanged(value) }
            }

            Label { text: "Pings shown " + pingsShownSlider.value.toFixed(0) }
            Slider {
                id: pingsShownSlider
                objectName: "pingsShownSliderObj"
                Layout.fillWidth: true
                Layout.fillHeight: true
                from: 1
                to: 20
                value: 10
                live: false  // only update value when button released
                Component.onCompleted: { backend.onPingsShownChanged(value) }
                onValueChanged: { console.log('pingsShownSlider moved: ', value);
                    backend.onPingsShownChanged(value) }
            }
        }


        Flow {
            Label { text: "VERTICAL exaggeration " +
                          (verticalExaggSlider.value * 100).toFixed(1)}
            Layout.fillWidth: true
            Layout.fillHeight: true

            Slider {
                id: verticalExaggSlider
                objectName: "verticalExaggSliderObj"
                Layout.fillWidth: true
                Layout.fillHeight: true
                from: 0.01
                to: 20
                live: false
                value: 10
                Component.onCompleted: { backend.onVerticalExaggChanged(value) }
                onValueChanged: { backend.onVerticalExaggChanged(value) }
            }
            Label { text: "Ping step" }
            Slider {
                id: pingStepSlider
                objectName: "pingStepSliderObj"

                Layout.fillWidth: true
                Layout.fillHeight: true
                from: 1
                to: 20
                live: false
                Component.onCompleted: { backend.onPingStepChanged(value) }
                onValueChanged: { console.log('pingStepSlider moved');
                    backend.onPingStepChanged(value) }
            }
        }



        ButtonGroup {
            id: editModes
            objectName: "editModesObj"
        }

        Flow {
            id: buttonRow
            Layout.fillWidth: true
            Layout.fillHeight: true


            RadioButton {
                objectName: "toggleEdit"
                /// checked: true
                text: qsTr("Toggle")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }

            RadioButton {
                objectName: "pickEdit"
                text: qsTr("Pick")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }

            RadioButton {
                objectName: "eraseEdit"
                text: qsTr("Erase")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }

            RadioButton {
                objectName: "restoreEdit"
                text: qsTr("Restore")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }

            RadioButton {
                objectName: "grabEdit"
                text: qsTr("Grab")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }

            RadioButton {
                objectName: "infoEdit"
                checked: true
                Component.onCompleted: { backend.onEditModeChanged(objectName) }
                text: qsTr("Info")
                ButtonGroup.group: editModes
                onToggled: { backend.onEditModeChanged(objectName); }
            }
        }


        ScrollView {

            implicitHeight: Window.window.height * 0.5
            implicitWidth: Window.window.width
            anchors.left: parent.left
            anchors.top: buttonRow.bottom
            contentHeight: 5000
            clip: true


            ColumnLayout {
                Rectangle {
                    // Layout.fillWidth: true
                    // Grab selection matches when w=600
                    id: swathRectangle

                    width: Window.window.width
                    height: Window.window.height

                    PixmapImage {
                        id: swathPixmap
                        objectName: "swathPixmapObj"
                        anchors.fill: parent
                    }

                    MouseArea {
                        id: swathMouseArea
                        objectName: 'swathMouseAreaObj'
                        // Fit exactly on PixmapImage to ensure proper scaling
                        // and mapping of mouse events
                        anchors.fill: swathPixmap
                        onWidthChanged: { backend.onPixmapImageResize(swathMouseArea.width, swathMouseArea.height) }
                        onHeightChanged: { backend.onPixmapImageResize(swathMouseArea.width, swathMouseArea.height) }


                        preventStealing: true
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



