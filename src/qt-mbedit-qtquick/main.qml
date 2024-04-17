import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import bogus 1.0

/* ***
Display and edit swath file pings/beams

BackEnd singleton must be registered in root context by main.cpp
See https://qml.guide/singletons/
*** */


ApplicationWindow {
    id: applicationWindow
    objectName: "mainWindow"
    visible: true
    width: 1000
    height: 880
    title: "qt-mbedit"
    
    signal qmlSignal(msg: string)
		    
    Component.onCompleted: { console.log("onCompleted");
                             console.log("xTrackSlider: ", xTrackSliderObj) }

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
                Action {objectName: "None"; checkable: true; checked: true;
		   text: qsTr("None");
		   ActionGroup.group: ancillaryData }
                Action {objectName: "Time"; checkable: true; text: qsTr("Time");
		   ActionGroup.group: ancillaryData}
                Action {checkable: true; text: qsTr("Interval");
		   ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Latitude");
		   ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Longitude");
		   ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Heading");
		   ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Speed");
		   ActionGroup.group: ancillaryData }
	   	Action {checkable: true; text: qsTr("Depth");
		   ActionGroup.group: ancillaryData }
		Action {checkable: true; text: qsTr("Altitude");
		   ActionGroup.group: ancillaryData }
	   	Action {checkable: true; text: qsTr("Sensor depth");
		   ActionGroup.group: ancillaryData }
		   Action {checkable: true; text: qsTr("Roll");
		   ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Pitch");
	           ActionGroup.group: ancillaryData }
                Action {checkable: true; text: qsTr("Heave");
	           ActionGroup.group: ancillaryData }		   
            }
            MenuSeparator {}
            Menu {
                title: "Slice"
                Action {checkable: true; checked: true;
		   text: qsTr("Waterfall"); ActionGroup.group: slice }
                Action {checkable: true;
		   text: qsTr("Along-track"); ActionGroup.group: slice }
                Action {checkable: true;
		   text: qsTr("Cross-track"); ActionGroup.group: slice }		   
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
		    objectName: xTrackSliderObj
                    Layout.fillWidth: true
                    from: 1
                    to: 300
		    live: false  // only update value when button released

                }
                Label { text: "Pings shown" }
                Slider {
                    id: pingsShownSlider
		    objectName: pingsShownSliderObj
                    Layout.fillWidth: true
                    from: 1
                    to: 20
		    live: false  // only update value when button released		    
                }
                Label { text: "Vertical exaggeration" }
                Slider {
                    id: verticalExaggSlider
		    objectName: verticalExaggSliderObj
                    Layout.fillWidth: true
                    from: 0.01
                    to: 20
		    live: false
                }

                Label { text: "Ping step" }
                Slider {
                    id: pingStepSlider
		    objectName: pingStepSliderObj

                    Layout.fillWidth: true
                    from: 1
                    to: 20
		    live: false
                }


    ButtonGroup {
    	buttons: mode.children
    }

    Row {
        id: mode

	RadioButton {
	  checked: true
	  text: qsTr("Toggle")
	 }

	RadioButton {
	  text: qsTr("Pick")
	}

	RadioButton {
	  text: qsTr("Erase")
	}

	RadioButton {
	  text: qsTr("Restore")
	}

	RadioButton {
	  text: qsTr("Grab")
	}
    }

    PixmapImage {
      id: swathImage
      objectName: swathImageObj
      anchors.fill: parent
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
}



