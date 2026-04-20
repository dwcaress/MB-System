import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Controls.Universal 2.3
import QtQuick.Dialogs
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

	    Action {checkable: true; text: qsTr("&Show flagged soundings") }
	    Action {checkable: true; text: qsTr("&Show flagged profiles") }

            MenuSeparator {}
            Menu {
                title: "Ancillary data"

                Action {checkable: true;
		        checked: true
                        text: qsTr("None");
                        ActionGroup.group: ancillaryData;
                        onToggled:  {
			    checked && backend.displayNoAncillData() }
			
			Component.onCompleted: {
			    checked && backend.displayNoAncillData()}		
                       }

                Action {checkable: true; text: qsTr("Time");
			ActionGroup.group: ancillaryData;
			onToggled:  {
			    checked &&  backend.displayTime()}
                       }

                Action {checkable: true;
			text: qsTr("Interval");
			ActionGroup.group: ancillaryData
			onToggled:  {
			    checked && backend.displayInterval()}
                       }

                Action {checkable: true;
			text: qsTr("Latitude");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayLatitude()}
                       }

                Action {checkable: true;
			text: qsTr("Longitude");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayLongitude()}
                       }
		
                Action {checkable: true;
			text: qsTr("Heading");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayHeading()}
                       }
		
                Action {checkable: true;
			text: qsTr("Speed");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displaySpeed()}
                       }
		
                Action {checkable: true;
			text: qsTr("Depth");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayDepth()}
                       }
                Action {checkable: true;
			text: qsTr("Altitude");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayAltitude()}
                       }
                Action {checkable: true;
			text: qsTr("Sensor depth");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displaySensorDepth()}
                       }
                Action {checkable: true;
			text: qsTr("Roll");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayRoll()}
                       }
                Action {checkable: true;
			text: qsTr("Pitch");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayPitch()}
                       }
                Action {checkable: true;
			text: qsTr("Heave");
			ActionGroup.group: ancillaryData
			onToggled:  {checked &&  backend.displayPitch()}
                       }

            }
            MenuSeparator {}
            Menu {
                title: "Slice"
                Action {checkable: true
			checked: true
			text: qsTr("Waterfall"); ActionGroup.group: slice
			onToggled:  {checked &&  backend.setWaterfallDisplay()}
			Component.onCompleted: {
			    checked && backend.setWaterfallDisplay()}		
                       }
                Action {checkable: true
			text: qsTr("Along-track"); ActionGroup.group: slice
			onToggled:  { checked &&  backend.setAlongTrackDisplay()}
                       }
                Action {checkable: true
			text: qsTr("Cross-track"); ActionGroup.group: slice
			onToggled:  {
			    checked &&  backend.setAcrossTrackDisplay()}
                       }
            }
            MenuSeparator {}
            Menu {
                title: "Color-coding"
                Action {checkable: true
		    checked: true
		    text: qsTr("Bottom-detect algorithm")
		    ActionGroup.group: colorCoding
		    onToggled: {
			checked && backend.setBottomDetectColorCode()}
			Component.onCompleted: {
			    checked && backend.setBottomDetectColorCode()}	
		}

                Action {checkable: true
		    text: qsTr("Pulse source")
		    ActionGroup.group: colorCoding
		    onToggled:  {checked &&  backend.setPulseColorCode()}
		}

                Action {checkable: true
		    text: qsTr("Flag state")
		    ActionGroup.group: colorCoding
		    onToggled:  {checked &&  backend.setFlagStateColorCode()}
		}
            }
	    
	    MenuSeparator {}

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

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 20

            Label { text: 'Xrack width (m)' }

            Rectangle {
                color: 'red'
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

                Label {
                    id: xtrackValue
                    text: xtrackSlider.value.toFixed(0)
                    x: xtrackSlider.handle.x + xtrackSlider.handle.width/2 - width/2
                    anchors.top: xtrackSlider.bottom
                }
            }

            Label {
                text: "Pings shown " + pingsShownSlider.value.toFixed(0)
            }
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

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 20

            Label { text: "VERTICAL exaggeration " +
                    (verticalExaggSlider.value * 100).toFixed(1)}
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
	}

        Flow {
            id: buttonRow
            Layout.fillWidth: true
            Layout.fillHeight: true
            height: 20

	    RadioButton {
		text: qsTr("Toggle")
		checked: true
		ButtonGroup.group: editModes
		onToggled: {checked && backend.setToggleMode()}
		Component.onCompleted: {checked && backend.setToggleMode()}		
	    }

            RadioButton {
		text: qsTr("Pick")
		ButtonGroup.group: editModes
		onToggled: {checked && backend.setPickMode()}
            }

            RadioButton {
		text: qsTr("Erase")
		ButtonGroup.group: editModes
		onToggled: {checked && backend.setEraseMode()}
            }

            RadioButton {
		text: qsTr("Restore")
		ButtonGroup.group: editModes
		onToggled: {checked && backend.setRestoreMode()}
            }

            RadioButton {
		text: qsTr("Grab")
		ButtonGroup.group: editModes
		onToggled: {checked && backend.setGrabMode()}
            }

            RadioButton {
		text: qsTr("Info")
		ButtonGroup.group: editModes
		onCheckedChanged: {checked && backend.setInfoMode()}
            }
	}


	ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
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
			objectName: backend.swathPixmapObj
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
	text: "Quit application?"
	buttons: MessageDialog.Yes |
            MessageDialog.No
	Component.onCompleted: visible = false
	onAccepted: Qt.quit(0)

    }


    FileDialog {
	id: fileDialog
	title: "Open swath file"
	nameFilters: ["Swath files (*.mb[0-9][0-9])"]
	onAccepted: {
            console.log("accepted " + selectedFile);
            backend.processSwathFile(selectedFile);
	}
    }

    MessageDialog {
	id: infoDialog
	title: "Info"
	text: "Text goes here"
	buttons: MessageDialog.Ok
	Component.onCompleted: visible = false
    }


    function showInfoDialog(message) {
	console.log('showInfoDialog()', message);
	infoDialog.text = message;
	infoDialog.open();
    }
}



