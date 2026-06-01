// import related modules
import QtQuick 2.9
import QtQuick.Controls 
import QtQuick.Window 2.2
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 
import QtGraphs 6.8   
import "ui-components"
import VTK 9.3
import Mbgrdviz 1.0


Window {
    width: 800
    height: 800

    id: mainWindow

    // Bathymetry profile
    property list<vector2d> myProfile

    Component.onCompleted: {
	colormapMenu.currentCmap = 'Haxby'
	mouseModeMenu.currentMode = SharedConstants.mouseModes[0].name
	console.log('Running app ', Qt.application.name)
    }

    
    MenuBar {
        id: menuBar

        Menu {
            title: qsTr('File')

            Action { text: qsTr('Open GMT grid or swath...') ;
                     onTriggered: {console.log('show file dialog')
				   datafileDialog.open()}
		   }

            Action { text: qsTr('Save settings') ;
                     onTriggered: {console.log('call topoDataItem.foo()')
				   console.log('call topoDataItem.saveSettings()')
				   topoDataItem.saveSettings()
				  }
		     
		   }

	    Action { text: qsTr('Load settings') ;
                     onTriggered: {console.log('call topoDataItem.foo()')
				   console.log('call topoDataItem.loadSettings()')
				   topoDataItem.loadSettings()
				  }
		   }
	    
            Action {
                text: qsTr('Exit')
                onTriggered: quitDialog.open()
            }
        }

        Menu {
            title: 'View'

            Action {
		text: qsTr('2D preferences')
		onTriggered: {console.log('show 2D preferences');
			      settings2d.show()}
	    }

            Action {
	        text: qsTr('3D preferences')
	        onTriggered: {console.log('show 3D preferences');
			      settings3dDialog.show();
			     }
	    }



            Menu {
                title: qsTr('Overlays')
                Action {
                    text: qsTr('Axes');
		    checkable: true;
		    checked: topoDataItem.showAxes
                    onTriggered: {topoDataItem.setShowAxes(checked)}
                }
                Action {
                    text: qsTr('Contours');
		    checkable: true;
		    checked: topoDataItem.showContours
                    onTriggered: {topoDataItem.setContours(checked)}
                }		
            }

            Menu {
                title: qsTr('Surface colored by')
		ActionGroup { id: coloredScalarActions;  exclusive: true }

                Action {
                    text: qsTr('Elevation')
		    checkable: true;
		    checked: topoDataItem.coloredScalar === TopoDataItem.Elevation
                    ActionGroup.group: coloredScalarActions
                    onTriggered: {
			topoDataItem.setColoredScalar(TopoDataItem.Elevation) }
                }
                Action {
                    text: qsTr('Slope')
		    checkable: true;
		    checked: topoDataItem.coloredScalar === TopoDataItem.Slope  
                    ActionGroup.group: coloredScalarActions
                    onTriggered: {
			topoDataItem.setColoredScalar(TopoDataItem.Slope)}
                }
            }

            Menu {
                title: qsTr('Surface shadows')
		ActionGroup { id: shadowSourceActions;   exclusive: true }

                Action {
                    text: qsTr('Illumination')
		    checkable: true
		    Component.onCompleted: {
			console.log('topoDataItem.shadowSource: ',
				    topoDataItem.shadowSource)

			console.log('TopoDataItem.Illumination: ',
				    TopoDataItem.Illumination)
		    }
		    
		    checked: topoDataItem.shadowSource === TopoDataItem.Illumination
                    ActionGroup.group: shadowSourceActions
                    onTriggered: {
			topoDataItem.setShadowSource(TopoDataItem.Illumination) }
                }
                Action {
                    text: qsTr('Local slope')
		    checkable: true
		    checked: topoDataItem.shadowSource === TopoDataItem.LocalSlope
                    ActionGroup.group: shadowSourceActions
                    onTriggered: {
			topoDataItem.setShadowSource(TopoDataItem.LocalSlope)}
                }
                Action {
                    text: qsTr('Local slope (GPU)')
		    checkable: true
		    checked: topoDataItem.shadowSource === TopoDataItem.LocalSlopeGpu
                    ActionGroup.group: shadowSourceActions
                    onTriggered: {
			topoDataItem.setShadowSource(TopoDataItem.LocalSlopeGpu)}
                }		
                Action {
                    text: qsTr('No shadows')
		    checkable: true
		    checked: topoDataItem.shadowSource === TopoDataItem.NoShadows
                    ActionGroup.group: shadowSourceActions
                    onTriggered: {
			console.log('selected no shadows')
			topoDataItem.setShadowSource(TopoDataItem.NoShadows)
                    }
		}
            }	    

            Menu {
                title: qsTr('Surface render')
		ActionGroup { id: surfaceRenderActions;  exclusive: true }
    
                Action {
                    text: qsTr('Polygons')
		    checkable: true
		    checked: topoDataItem.surfaceRenderType === TopoDataItem.Polys
                    ActionGroup.group: surfaceRenderActions
                    onTriggered: {
			topoDataItem.setSurfaceRenderType(TopoDataItem.Polys)
		    }
                }
                Action {
                    text: qsTr('Point cloud')
		    checkable: true
		    checked: topoDataItem.surfaceRenderType === 
		       TopoDataItem.PointCloud
                    ActionGroup.group: surfaceRenderActions
                    onTriggered: {
			topoDataItem.setSurfaceRenderType(TopoDataItem.PointCloud)
                    }
		}
                Action {
		    text: qsTr('Wireframe')
		    checkable: true
		    checked: topoDataItem.surfaceRenderType === 
		       TopoDataItem.Wireframe
		    ActionGroup.group: surfaceRenderActions
		    onTriggered: {
			topoDataItem.setSurfaceRenderType(TopoDataItem.Wireframe)
		    }		    
		}
	    }
		

	    Menu {
		title: 'Color map'
		id: colormapMenu

		property string currentCmap
		
		Repeater {
		    // List of colormap names from SharedConstants
		    model: SharedConstants.cmaps
		    
		    visible: true
		    MenuItem {
			text: modelData; checkable: true;  // colormap name
			checked: colormapMenu.currentCmap == modelData
			onTriggered: {
		            colormapMenu.currentCmap = modelData
		            console.log('selected ', modelData);
		            topoDataItem.setColormap(modelData)
			}		
		    }
		}
	    }
	    
            MenuItem {
		text: qsTr('Reset camera')
		checkable: false
		onTriggered: {
		    /// topoDataItem.resetCamera()
		    topoDataItem.setOrthographicView()		    
		}		    
	    }
        }

	Menu {
	    title: 'Mouse'
	    id: mouseModeMenu

            property string currentMode: ''

            Repeater {
		// mouse mode names, tooltips defined in SharedConstants.h
		model: SharedConstants.mouseModes
		
		visible: true
		
		MenuItem {
	            text: modelData.name
		    checkable: true
		    checked: mouseModeMenu.currentMode === modelData.name

                    ToolTip.visible: hovered
		    ToolTip.text: modelData.toolTip
		    
                    onTriggered: {
		        mouseModeMenu.currentMode = modelData.name,
		        console.log('selected ', modelData.name);
		        topoDataItem.setMouseMode(modelData.name)
	            }		
		}
	    }
	}

    }

    ColumnLayout {

        anchors.top: menuBar.bottom

        Button {
            text: qsTr('Push me!')
            onPressed: {
		console.log('Pressed test button')
		// TESTING
		console.log('topoDataItem.setPBR()???')
		topoDataItem.setPBR(0.5, 0.)
            }
        }

        TopoDataItem {
            objectName: 'topoDataItem'
            id: topoDataItem
            x: 200
            y: 200
            width: 650
            height: 650
            focus: true
        }

    }


    FileDialog {
        id: datafileDialog
        title: qsTr('Open GMT grid or swath file')
        // nameFilters: ['Swath files (*.grd *.mb[0-9]*)']
        nameFilters: ['Swath files (*.grd *.mb[0-9][0-9])']
        onAccepted: {
            console.log('accepted ' + selectedFile);
            topoDataItem.loadDatafile(selectedFile);
	    // Known issue: must return focus to topoDataItem	    
            topoDataItem.forceActiveFocus();	    
        }
        onRejected: {
	    // Known issue: must return focus to topoDataItem			      	
            topoDataItem.forceActiveFocus();
	}



    }

    MessageDialog {
        id: quitDialog
        title: "Quit?"
        text: "Quit application?"
        buttons: MessageDialog.Yes | MessageDialog.No
        Component.onCompleted: visible = false
        onAccepted: Qt.quit()
	// Known issue: must return focus to topoDataItem      	
        onRejected: topoDataItem.forceActiveFocus();
    }

    Window {
        id: topoProfileWindow
	width: 500
	height: 500
        visible: false
        TopoProfileGraph {
            id: profileGraph
        }
	onClosing: topoDataItem.forceActiveFocus();
    }

    Settings2dWindow {
        id: settings2d
        visible: false
	onClosing: topoDataItem.forceActiveFocus();
    }




    Window {
        id: settings3dDialog
	title: '3D preferences'
	visible: false
	property list<double> intensity

	// Bind window size to content's preferred size
	width: settings3D.implicitWidth
	height: settings3D.implicitHeight

	// Prevents the user from shrinking it smaller than the content
	minimumWidth: settings3D.implicitWidth
	minimumHeight: settings3D.implicitHeight
	
        Settings3D {
	    id: settings3D
	    anchors.fill: parent
	    anchors.margins: 10

	    lightsEnabled.onCheckedChanged: {
		console.log('Lights checked: ', settings3D.checked)
		updateLighting()
	    }
	    
	    intensity.onMoved: {
		updateLighting()
	    }

            lightX.onMoved: {
		updateLighting()
	    }

            lightY.onMoved: {
		updateLighting()
	    }

            lightZ.onMoved: {
		updateLighting()
	    }

	    slopeGamma.onMoved: {
		topoDataItem.setSlopeGamma(slopeGamma.value)
	    }

	    slopeFloor.onMoved: {
		topoDataItem.setMinBrightness(slopeFloor.value)
	    }

	    verticalExagg.onMoved: {
		topoDataItem.setVerticalExagg(verticalExagg.value)
	    }

	    contourInterval.onMoved: {
		topoDataItem.setContourInterval(contourInterval.value)
	    }

	    showContours.onCheckedChanged: {
		topoDataItem.setContours(showContours.checked)
	    }

	    showContourLabels.onCheckedChanged: {
		topoDataItem.setShowContourLabels(showContourLabels.checked)
	    }
        }
	
	onVisibilityChanged: { console.log('settings3dDialog opened');
                    // Set values in settings gui to current values
                    var pos = topoDataItem.getLightPosition()
		    console.log('pos=', JSON.stringify(pos))
		    settings3D.lightX.value = pos[0]
		    settings3D.lightY.value = pos[1]
		    settings3D.lightZ.value = pos[2]
		    settings3D.intensity.value =
			       topoDataItem.getLightIntensity()

		    settings3D.slopeGamma.value = 
			       topoDataItem.getSlopeGamma()
			       
		    settings3D.slopeFloor.value = 
			       topoDataItem.getMinBrightness()

		    settings3D.verticalExagg.value =
		               topoDataItem.getVerticalExagg()
 		    console.log('showContours: ', topoDataItem.showContours)		       
	            settings3D.showContours.checked = topoDataItem.showContours
 	            settings3D.showContourLabels.checked = topoDataItem.showContourLabels
	            settings3D.contourInterval.value = topoDataItem.getContourInterval()
		  }
    }
    

    function updateLighting() {
        console.log('updateLighting()')
	console.log('settings3dDialog=', settings3dDialog)
	console.log('settings3D=', settings3D)	

	topoDataItem.setLight(settings3D.lightsEnabled.checked,
			      settings3D.intensity.value,
		              settings3D.lightX.value,
		              settings3D.lightY.value,			      		                      settings3D.lightZ.value)
    }


    MessageDialog {
	id: errorDialog
	title: 'ERROR'
	buttons: MessageDialog.Ok
    }
    
    Connections {
	ignoreUnknownSignals: true
	
	target: topoDataItem
	function onLineDefined(profileData) {
	    console.log("Line defined!")
	    var xmin = Infinity
	    var xmax = -Infinity
	    var ymin = Infinity
	    var ymax = -Infinity
	    // Find min/max values of x and y
	    for (var i = 0; i < profileData.length; i++) {
		if (profileData[i].x < xmin) { xmin = profileData[i].x }
		if (profileData[i].x > xmax) { xmax = profileData[i].x }
		if (profileData[i].y < ymin) { ymin = profileData[i].y }
		if (profileData[i].y > ymax) { ymax = profileData[i].y }
	    }
	    console.log('xmin: ', xmin, '  xmax: ', xmax);
	    console.log('ymin: ', ymin, '  ymax: ', ymax);

	    profileGraph.xyData.clear()

	    // Populate graph line-series points
	    for (var i = 0; i < profileData.length; i++) {
		profileGraph.xyData.append(profileData[i].x, profileData[i].y);
	    }
	    topoProfileWindow.show();

	    // Set graph axes ranges
	    profileGraph.axisX.min = xmin
	    profileGraph.axisX.max = xmax
	    profileGraph.axisY.min = ymin
	    profileGraph.axisY.max = ymax

	    topoDataItem.forceActiveFocus();
	}

	function onErrorOccurred(message) {
	    console.log('Received error message from C++:', message)
	    errorDialog.close()  // If somehow partially open...
	    errorDialog.text = message
	    errorDialog.open()
	}
    }
}




