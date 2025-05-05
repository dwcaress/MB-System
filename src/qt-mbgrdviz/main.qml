// import related modules
import QtQuick 2.9
import QtQuick.Controls 2.9
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

    /// TEST TEST TEST
    property int mainTestInt: 77
    property string mainTestString: 'hello sailor!'

    // Bathymetry profile
    property list<vector2d> myProfile

    onMyProfileChanged: {
        console.log('Hey, myProfile changed to: ', myProfile)
        mainWindow.profileSig(999, 'profileSig()')
    }

    property int myVal: GuiNames.Speed

    Component.onCompleted: {

      console.log('GuiNames.Depth =  ', GuiNames.Depth)
      console.log('GuiNames.Heading =  ', GuiNames.Heading)            
      console.log('guiNames.objectName(Speed): ', guiNames.objectName(GuiNames.Speed))

    }

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
                onTriggered: { console.log('show profile'); topoProfileWindow.show() }
            }

        }

        Menu {
            title: 'Mouse mode'
            Action {
                text: qsTr('&Pan and zoom'); checkable: true;
                ActionGroup.group: exclusiveActions
                onTriggered: { }
            }
            Action {
                text: qsTr('&Rotate model'); checkable: true;
                ActionGroup.group: exclusiveActions
                onTriggered: { }
            }

            Action {
                text: qsTr('&Rotate view'); checkable: true;
                ActionGroup.group: exclusiveActions
                onTriggered: { }
            }

            Action {
                text: qsTr('&Lighting'); checkable: true;
                ActionGroup.group: exclusiveActions
                onTriggered: { }
            }

            Action {
                text: qsTr('&Ruler'); checkable: true;
                ActionGroup.group: exclusiveActions
                onTriggered: { topoProfileWindow.show() }
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

}

