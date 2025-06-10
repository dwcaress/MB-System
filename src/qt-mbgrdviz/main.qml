// import related modules
import QtQuick 2.9
import QtQuick.Controls 
import QtQuick.Window 2.2
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 
import QtGraphs 6.8   
import "ui-components"

import VTK 9.3
import SharedConstants 1.1
import GuiNames 1.1


Window {
    width: 800
    height: 800

    id: mainWindow

    // Declare instance of SharedConstants
    SharedConstants {
        id: constants
    }

    GuiNames {
      id: guiNames
    }
    
    // Short-hand reference to supported color maps
    property variant cmaps: constants.cmaps

    // Short-hand reference to supported mouse modes
    property variant mouseModes: constants.mouseModes

    /// TEST TEST TEST
    property int mainTestInt: 77
    property string mainTestString: 'hello sailor!'

    // Bathymetry profile
    property list<vector2d> myProfile

    property int myVal: GuiNames.Speed

    Component.onCompleted: {

      console.log('GuiNames.Depth =  ', GuiNames.Depth)
      console.log('GuiNames.Heading =  ', GuiNames.Heading)            
      console.log('guiNames.objectName(Speed): ', guiNames.objectName(GuiNames.Speed))

    }

    ActionGroup {
        id: exclusiveActions
        exclusive: true
    }
    
    MenuBar {
        id: menuBar

        Menu {
            title: qsTr('File')

            Action { text: qsTr('Open grid or swath...') ;
                onTriggered: {console.log('show file dialog')
                    datafileDialog.open()}
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
		  text: qsTr('Lighting')
		  onTriggered: {console.log('show 3D preferences');
		  settings3dDialog.open();
		  }
		}


            Menu {
                title: qsTr('Overlays')
                Action {
                    text: qsTr('Axes'); checkable: true;
                    onTriggered: {topoDataItem.showAxes(checked)}
                }
            }

            Menu {
                title: qsTr('Displayed surface')

                Action {
                    text: qsTr('Topography'); checkable: true;
                    ActionGroup.group: exclusiveActions
                    onTriggered: { topoDataItem.setDisplayedSurface(TopoDataItem.Elevation) }
                }
                Action {
                    text: qsTr('Slope'); checkable: true;
                    ActionGroup.group: exclusiveActions
                    onTriggered: { topoDataItem.setDisplayedSurface(TopoDataItem.Gradient) }
                }
            }


	Menu {
	    title: 'Color map'
	    id: colormapMenu

            property string currentCmap
	    
            Repeater {
	      model: cmaps
	      visible: true
	      MenuItem {
	        text: modelData; checkable: true;
		checked: colormapMenu.currentCmap == modelData
                onTriggered: {
		               colormapMenu.currentCmap = modelData
		               console.log('selected ', modelData);
		               topoDataItem.setColormap(modelData)
		}		
	      }
	    }
	  }

  
            // Profile
            Action {
                text: qsTr('Profile');
                onTriggered: { console.log('show profile');
		topoProfileWindow.show() }
            }

        }

	Menu {
	    title: 'Mouse'
	    id: mouseModeMenu

            property string currentMode: ''

            Repeater {
	      model: mouseModes
	      visible: true
	      
	      MenuItem {
	        text: modelData
		checkable: true
		checked: mouseModeMenu.currentMode === modelData
		
                onTriggered: {
		               mouseModeMenu.currentMode = modelData
		               console.log('selected ', modelData);
		               topoDataItem.setMouseMode(modelData)
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
                var dummy = 0;
                myProfile = topoDataItem.getElevProfile(dummy, dummy, dummy, dummy, 500);
                console.log('profile length: ', myProfile.length);

                var xmin = 10000;
                var xmax = -xmin;
                var ymin = 100000;
                var ymax = -ymin;
                for (var i = 0; i < myProfile.length; i++) {
                    console.log('x: ', myProfile[i].x, ' y: ', myProfile[i].y);
                    if (myProfile[i].x < xmin) { xmin = myProfile[i].x }
                    if (myProfile[i].x > xmax) { xmax = myProfile[i].x }
                    if (myProfile[i].y < ymin) { ymin = myProfile[i].y }
                    if (myProfile[i].y > ymax) { ymax = myProfile[i].y }
                }
                console.log('xmin: ', xmin, '  xmax: ', xmax);
                console.log('ymin: ', ymin, '  ymax: ', ymax);

                // Set graph axes ranges
                profileGraph.xAxis.min = xmin
                profileGraph.xAxis.max = xmax
                profileGraph.yAxis.min = ymin
                profileGraph.yAxis.max = ymax

                profileGraph.xyData.clear()

                // Populate graph line-series points
                for (var i = 0; i < myProfile.length; i++) {
                    profileGraph.xyData.append(myProfile[i].x, myProfile[i].y);
                }

            }
        }

        TopoDataItem {
            objectName: 'topoDataItem'
            id: topoDataItem
            x: 200
            y: 200
            width: 600
            height: 600
            focus: true
        }

    }


    FileDialog {
        id: datafileDialog
        title: qsTr('Open grid or swath file')
        // nameFilters: ['Swath files (*.grd *.mb[0-9]*)']
        nameFilters: ['Swath files (*.grd *.mb[0-9][0-9])']
        onAccepted: {
            console.log('accepted ' + selectedFile);
            topoDataItem.loadDatafile(selectedFile);
        }
    }


    MessageDialog {
        id: quitDialog
        title: "Quit?"
        text: "Quit application?"
        buttons: MessageDialog.Yes | MessageDialog.No
        Component.onCompleted: visible = false
        onAccepted: Qt.quit()
    }

    Window {
        id: topoProfileWindow
	width: 500
	height: 500
        visible: false
        TopoProfileGraph {
            id: profileGraph
        }
    }

    Settings2dWindow {
        id: settings2d
        visible: false
    }




    Dialog {
        id: settings3dDialog
	title: '3D view preferences'
	modal: false
	property list<double> intensity
	
        contentItem: Settings3D {
	  id: settings3D
	  intensity.onPressedChanged: {
	                        console.log('intensity pressed/released')
				if (!intensity.pressed) {
				  console.log('RELEASED')
				  updateLighting()
				  }
				}

           lightX.onPressedChanged: {
	                        console.log('intensity pressed/released')
				if (!lightX.pressed) {
				  console.log('RELEASED')
				  updateLighting()
				  }
				}


           lightY.onPressedChanged: {
	                        console.log('intensity pressed/released')
				if (!lightY.pressed) {
				  console.log('RELEASED')
				  updateLighting()
				  }
				}

           lightZ.onPressedChanged: {
	                        console.log('intensity pressed/released')
				if (!lightZ.pressed) {
				  console.log('RELEASED')
				  updateLighting()
				  }
				}				
        }
	
        // Button box at the bottom
        footer: DialogButtonBox {
            standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel |
	                     DialogButtonBox.Apply

            onApplied: {
	        console.log('set intensity to ', settings3D.intensity.value);
	        topoDataItem.setLight(settings3D.intensity.value,
		                      settings3D.lightX.value,
				      settings3D.lightY.value,
				      settings3D.lightZ.value)}
			
            onAccepted: {
                console.log("OK clicked")
                settings3dDialog.accept()
            }
            
            onRejected: {
                console.log("Cancel clicked")
                settings3dDialog.reject()
            }
      }
      onOpened: { console.log('settings3dDialog opened');
                // Set values in settings gui to current values
                var pos = topoDataItem.getLightPosition()
		console.log('pos=', JSON.stringify(pos))
		settings3D.lightX.value = pos[0]
		settings3D.lightY.value = pos[1]
		settings3D.lightZ.value = pos[2]
      }
	

    }
    
    function updateLighting() {
        console.log('updateLighting()')
	console.log('settings3dDialog=', settings3dDialog)
	console.log('settings3D=', settings3D)	

	topoDataItem.setLight(settings3D.intensity.value,
		              settings3D.lightX.value,
		              settings3D.lightY.value,			      		                      settings3D.lightZ.value)
    }
    
}


