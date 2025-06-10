import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3


Item {
    visible: true
    width: 640
    height: 480
    property alias intensity: intensity
    property alias lightX: lightX
    property alias lightY: lightY
    property alias lightZ: lightZ    

            ColumnLayout {

                Label {
                    text: qsTr("Intensity")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18                    
                }
                
                Slider {
                    id: intensity
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: "val: " + Math.round(intensity.value *100)/100 }

                MenuSeparator { }
                Label {
                    text: qsTr("Light X")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18                    
                }
                
                Slider {
                    id: lightX
                    Layout.fillWidth: true
                    from: -1
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: "lightXval: " +
		        Math.round(intensity.value *100)/100 }

                MenuSeparator { }
                Label {
                    text: qsTr("Light Y")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18                    
                }
                
                Slider {
                    id: lightY
                    Layout.fillWidth: true
                    from: -1
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: "lightYval: " +
		        Math.round(lightY.value *100)/100 }
			
                MenuSeparator { }			
                Label {
                    text: qsTr("Light Z")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18                    
                }
                
                Slider {
                    id: lightZ
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: "lightZval: " +
		        Math.round(lightZ.value *100)/100 }






	}



}

	

