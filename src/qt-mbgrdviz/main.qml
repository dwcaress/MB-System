// import related modules
import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.14
import QtQuick.Dialogs

import VTK 9.3
import SharedConstants 1.1


Window {
  width: 800
  height: 800

  // Declare instance of SharedConstants
  SharedConstants {
    id: constants
  }

  // Short-hand reference to supported color maps
  property variant cmaps: constants.cmaps

  ActionGroup {
    id: colorActions
    exclusive: true
  }

  ActionGroup {
      id: exclusiveActions
      exclusive: true
  }
    
  MenuBar {
     id: menuBar
     
     Menu {
       title: qsTr('&File')

       Action { text: qsTr('Open grid...') ;
                onTriggered: {console.log('show file dialog')
                gridfileDialog.open()}
       }
       Action {
         text: qsTr('Exit')
	 onTriggered: quitDialog.open()
       }
     }

     Menu {
       title: 'View'

       Action {
         text: qsTr('Preferences')
       }

       Menu {
         title: qsTr('Overlays')
	 Action {
	   text: qsTr('&Axes'); checkable: true;
	   onTriggered: {topoDataItem.showAxes(checked)}
	 }
       }

       Menu {
         title: qsTr('Displayed surface')
	 
	 Action {
	   text: qsTr('&Topography'); checkable: true;
	   ActionGroup.group: exclusiveActions
	   onTriggered: { topoDataItem.setDisplayedSurface(TopoDataItem.Elevation) }	   
	 }
	 Action {
	   text: qsTr('&Slope'); checkable: true;
	   ActionGroup.group: exclusiveActions
	   onTriggered: { topoDataItem.setDisplayedSurface(TopoDataItem.Gradient) }
	 }
       }

       Menu {
         title: qsTr('&Color map')
	 id: colormapMenu

         // Add actions when menu is complete
	 Component.onCompleted: {
           // Insert menu items here, with number of items and
           // item names as specified in the cmaps[] array that was
           // retrieved from C++
           for (var i = 0; i < cmaps.length; i++)  {
             console.log('colormap: ', cmaps[i]);
             // Build QML string that specifies menu Action to insert
             var qmlStr = 'import QtQuick.Controls 2.3; ' +
                     'Action {id: myAction; checkable: true; ';

             // First item is checked
             if (i == 0) { qmlStr += 'checked: true; '; }

             qmlStr += 'ActionGroup.group: colorActions; ';
             qmlStr += 'text: \'' + cmaps[i] + '\'; ';

             qmlStr += 'onTriggered: { console.log(\'selected ' +
	                               cmaps[i] + '\');' +
				       'topoDataItem.setColormap(\'' +
				       cmaps[i] +
				       '\')' +
				       '}'
             qmlStr += '} '

	     console.log('qmlStr: ', qmlStr)
             // Create the menu Action
             var obj =
                 Qt.createQmlObject(qmlStr,
                                    colormapMenu,
                                   'dynamicAction');

              // Add created action to menu
	      colormapMenu.addAction(obj)
           }

	 }
	 
       }

       // Profile
       Action {
          text: qsTr('&Profile');
	  onTriggered: { console.log('show profile') }
	 }	 
       
     }
  }

  ColumnLayout {

  anchors.top: menuBar.bottom
  
  Button {
    text: qsTr('Push me!')
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
        id: gridfileDialog
        title: qsTr('Open grid or swath file')
        // nameFilters: ['Swath files (*.grd *.mb[0-9]*)']
        nameFilters: ['Swath files (*.grd *.mb[0-9][0-9])']
        onAccepted: {
            console.log('accepted ' + selectedFile);
            topoDataItem.loadGridfile(selectedFile);
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

}
