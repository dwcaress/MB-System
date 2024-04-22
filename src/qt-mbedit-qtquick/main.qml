import QtQuick 2.9
import QtQuick.Controls 2.3
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
    objectName: "mainWindow"
    visible: true
    width: 1000
    height: 880
    title: "qt-mbedit"

    /// Emit when ancillary data option is selected
    signal ancillDataSignal(msg: string)

    /// Emit when slice option is selected
    signal sliceSignal(msg: string)
    
    /// Emit when edit mode changes
    signal editModeSignal(msg: string)


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
		    onTriggered: ancillDataSignal(objectName)
		    }

                Action {objectName: "time"; checkable: true; text: qsTr("Time");
                    ActionGroup.group: ancillaryData;
		    onTriggered: ancillDataSignal(objectName)		    
		    }

                Action {objectName: "interval"; checkable: true;
                    text: qsTr("Interval");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)	
		    }
		    
                Action {objectName: "latitude"; checkable: true;
                    text: qsTr("Latitude");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }

                Action {objectName: "longitude"; checkable: true;
                    text: qsTr("Longitude");		    
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "heading"; checkable: true;
                    text: qsTr("Heading");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)		    
		    }
                Action {objectName: "speed"; checkable: true;
                    text: qsTr("Speed");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "depth"; checkable: true;
                    text: qsTr("Depth");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "altitude"; checkable: true;
                    text: qsTr("Altitude");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "sensorDepth"; checkable: true;
                    text: qsTr("Sensor depth");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "roll"; checkable: true;
                    text: qsTr("Roll");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)
		    }
                Action {objectName: "pitch"; checkable: true;
                    text: qsTr("Pitch");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)		    
		    }
                Action {objectName: "heave"; checkable: true;
                    text: qsTr("Heave");
                    ActionGroup.group: ancillaryData
		    onTriggered: ancillDataSignal(objectName)		    
		    }

            }
            MenuSeparator {}
            Menu {
                title: "Slice"
                Action {objectName: "waterfall"; checkable: true; checked: true;
                    text: qsTr("Waterfall"); ActionGroup.group: slice
		    onTriggered: sliceSignal(objectName)		    
		    }
                Action {objectName: "alongTrack"; checkable: true;
                    text: qsTr("Along-track"); ActionGroup.group: slice
		    onTriggered: sliceSignal(objectName)
		    }
                Action {objectName: "crossTrack"; checkable: true;
                    text: qsTr("Cross-track"); ActionGroup.group: slice
		    onTriggered: sliceSignal(objectName)		    
		    }
            }
            MenuSeparator {}
            Menu {
                title: "Color-coding"
                Action {checkable: true; checked: true;
                    text: qsTr("Bottom-detect algorithm");
                    ActionGroup.group: colorCoding}
                Action {checkable: true;
                    text: qsTr("Pulse source");
                    ActionGroup.group: colorCoding}
                Action {checkable: true;
                    text: qsTr("Flag state");
                    ActionGroup.group: colorCoding}
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
        Layout.fillHeight: false
        width: 1000

        Label { text: "XTrack width" }
        Slider {
            id: xtrackSlider
            objectName: "xTrackSliderObj"
            Layout.fillWidth: true
            from: 1
            to: 300
            live: false  // only update value when button released

        }
        Label { text: "Pings shown" }
        Slider {
            id: pingsShownSlider
            objectName: "pingsShownSliderObj"
            Layout.fillWidth: true
            from: 1
            to: 20
            live: false  // only update value when button released
        }
        Label { text: "Vertical exaggeration" }
        Slider {
            id: verticalExaggSlider
            objectName: "verticalExaggSliderObj"
            Layout.fillWidth: true
            from: 0.01
            to: 20
            live: false
        }

        Label { text: "Ping step" }
        Slider {
            id: pingStepSlider
            objectName: "pingStepSliderObj"

            Layout.fillWidth: true
            from: 1
            to: 20
            live: false
        }


        ButtonGroup {
            id: editModes
            objectName: "editModesObj"
        }

        Row {
            id: buttonRow

            RadioButton {
                objectName: "toggleEditObj"
                checked: true
                text: qsTr("Toggle")
                ButtonGroup.group: editModes
                onToggled: { editModeSignal(objectName); }
            }

            RadioButton {
                objectName: "pickEditObj"
                text: qsTr("Pick")
                ButtonGroup.group: editModes
                onToggled: { editModeSignal(objectName); }
            }

            RadioButton {
                objectName: "eraseEditObj"
                text: qsTr("Erase")
                ButtonGroup.group: editModes
                onToggled: { editModeSignal(objectName); }
            }

            RadioButton {
                objectName: "restoreEditObj"
                text: qsTr("Restore")
                ButtonGroup.group: editModes
                onToggled: { editModeSignal(objectName); }
            }

            RadioButton {
                objectName: "grabEditObj"
                text: qsTr("Grab")
                ButtonGroup.group: editModes
                onToggled: { editModeSignal(objectName); }
            }
        }

        PixmapImage {
            id: swathPixmap
            objectName: "swathPixmapObj"
            anchors.top: buttonRow.bottom
            /// anchors.topMargin: 5
            /// anchors.bottom: parent.bottom
            Layout.fillWidth: true
            /// width: 600
            height: 600
            /// anchors.fill: parent
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
        nameFilters: ["Swath files (*.mb*)"]
        onAccepted: {
            console.log("accepted " + fileUrl);
        }
    }
}



