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


Window {
    width: 800
    height: 800

    id: mainWindow

    // Declare instance of SharedConstants, which will populate
    // QStringList objects needed by QML
    SharedConstants {
        id: constants
    }

    // Bathymetry profile
    property list<vector2d> myProfile

    Component.onCompleted: {
      console.log('SharedConstants.EditState.ViewOnly  ',
                  SharedConstants.EditState.ViewOnly)
      console.log('SharedConstants.EditState.EditRoute  ',
                  SharedConstants.EditState.EditRoute)     
      console.log('SharedConstants.EditState.EditOverlay  ',
                  SharedConstants.EditState.EditOverlay)
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
	      model: constants.cmaps  // List of colormap names
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

  
            // Profile
            Action {
                text: qsTr('Elev profile');
                onTriggered: { console.log('show profile');
		topoProfileWindow.show();
                topoDataItem.forceActiveFocus();
	      }
            }

        }

	Menu {
	    title: 'Mouse'
	    id: mouseModeMenu

            property string currentMode: ''

            Repeater {
	      model: constants.mouseModes // mouse mode names, tooltips
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
	      console.log('Pressed buttonn')
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




    Dialog {
        id: settings3dDialog
	title: 'Lighting preferences'
	modal: false
	width: 300
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
				      settings3D.lightZ.value);
		// Known issue: must return focus to topoDataItem
                topoDataItem.forceActiveFocus();
             }
			
            onAccepted: {
                console.log("OK clicked")
                settings3dDialog.accept()
		// Known issue: must return focus to topoDataItemr		
                topoDataItem.forceActiveFocus();		
            }
            
            onRejected: {
                console.log("Cancel clicked")
                settings3dDialog.reject()
		// Known issue: must return focus to topoDataItem
                topoDataItem.forceActiveFocus();		
            }
      }
      onOpened: { console.log('settings3dDialog opened');
                // Set values in settings gui to current values
                var pos = topoDataItem.getLightPosition()
		console.log('pos=', JSON.stringify(pos))
		settings3D.lightX.value = pos[0]
		settings3D.lightY.value = pos[1]
		settings3D.lightZ.value = pos[2]
		settings3D.intensity.value = topoDataItem.getLightIntensity()
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

  Connections {
    ignoreUnknownSignals: true
    
    target: topoDataItem
    onLineDefined: function (profileData) {
      console.log("Line defined!")
      var xmin = 100000;
      var xmax = -xmin;
      var ymin = 100000;
      var ymax = -ymin;
      for (var i = 0; i < profileData.length; i++) {
        console.log('x: ', profileData[i].x, ' y: ', profileData[i].y);
        if (profileData[i].x < xmin) { xmin = profileData[i].x }
        if (profileData[i].x > xmax) { xmax = profileData[i].x }
        if (profileData[i].y < ymin) { ymin = profileData[i].y }
        if (profileData[i].y > ymax) { ymax = profileData[i].y }
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
      for (var i = 0; i < profileData.length; i++) {
        profileGraph.xyData.append(profileData[i].x, profileData[i].y);
       }
       topoProfileWindow.show();
       topoDataItem.forceActiveFocus();
     }
  }
}




