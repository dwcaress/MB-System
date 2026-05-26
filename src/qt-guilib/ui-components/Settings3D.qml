import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

Item {
    visible: true
    implicitWidth: 640
    implicitHeight: 480
    property alias intensity: intensity
    property alias lightX: lightX
    property alias lightY: lightY
    property alias lightZ: lightZ
    property alias lightsEnabled: lightsEnabled
    property alias slopeGamma: slopeGamma
    property alias slopeFloor: slopeFloor
    property alias verticalExagg: verticalExagg
    property alias contourInterval: contourInterval
    
    ColumnLayout {
        Label {
            text: qsTr('Lights enabled')
            fontSizeMode: Text.Fit
            font.pixelSize: 18
        }

        CheckBox {
            id: lightsEnabled
            checked: true
        }

        GridLayout {
            columns: 2

            Frame { ColumnLayout {
                Label {
                    text: qsTr("Intensity")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: intensity
		    Layout.preferredWidth: 250
                    Layout.fillWidth: true
                    from: 0.2
                    to: 2
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(intensity.value *100)/100 }
            }}

            Frame {ColumnLayout {
                Label {
                    text: qsTr("Light X")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: lightX
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: -1
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(lightX.value *100)/100 }
            }}

            Frame {ColumnLayout {
                Label {
                    text: qsTr("Light Y")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: lightY
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: -1
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(lightY.value *100)/100 }
            }}

            Frame {ColumnLayout {
                Label {
                    text: qsTr("Light Z")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: lightZ
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: 1
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(lightZ.value *100)/100 }
            }}

            Frame {ColumnLayout {
                MenuSeparator { }
                Label {
                    text: qsTr("Slope shade gamma")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: slopeGamma
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: 0.2
                    to: 2.0
                    value: 2.0
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(slopeGamma.value *100)/100 }
            }}

            Frame {ColumnLayout {
                Label {
                    text: qsTr("Slope shade floor")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: slopeFloor
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: 0.2
                    to: 2.0
                    value: 2.0
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(slopeFloor.value *100)/100 }
            }}

            Frame {ColumnLayout {
                Label {
                    text: qsTr("Vertical exaggeration")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: verticalExagg
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: 1.0
                    to: 5.0
                    value: 1.0
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(verticalExagg.value *100)/100 }
            }}
	    
            Frame {ColumnLayout {
                Label {
                    text: qsTr("Contour interval")
                    fontSizeMode: Text.Fit
                    font.pixelSize: 18
                }

                Slider {
                    id: contourInterval
		    Layout.preferredWidth: 250		    
                    Layout.fillWidth: true
                    from: 1
                    to: 100
                    value: 100
                }
                // Set displayed decimal places on slider label
                Label { text: Math.round(contourInterval.value *100)/100 }
            }}	    

        }
    }
}

