import QtQuick 2.9
import QtQuick.Controls
import QtQuick.Window 2.2
import QtQuick.Layouts 1.14
import QtQuick.Dialogs
import QtGraphs 6.8
import "ui-components"
import Mbeditviz 1.0    // Registered in main.cpp

Window {
    id: mainWindow
    width: 900
    height: 850
    title: "mbeditviz"

    property list<vector2d> myProfile
    property string toolBarEdge: "top"   // "top" | "bottom" | "left" | "right"

    Component.onCompleted: {
        colormapMenu.currentCmap = 'Haxby'
        mouseModeMenu.currentMode = SharedConstants.mouseModes[0].name
        console.log('Running app', Qt.application.name)
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Menu bar
    // ─────────────────────────────────────────────────────────────────────────

    MenuBar {
        id: menuBar

        Menu {
            title: qsTr('File')
            Action {
                text: qsTr('Open GMT grid, swath or datalist...')
                onTriggered: datafileDialog.open()
            }
            Action {
                text: qsTr('Save settings')
                onTriggered: surfaceView.saveSettings()
            }
            Action {
                text: qsTr('Load settings')
                onTriggered: surfaceView.loadSettings()
            }
            MenuSeparator {}
            Action {
                text: qsTr('Exit')
                onTriggered: quitDialog.open()
            }
        }

        Menu {
            title: qsTr('View')

            Action {
                text: qsTr('Toolbar')
                checkable: true
                checked: toolBar.visible
                onTriggered: toolBar.visible = !toolBar.visible
            }
            MenuSeparator {}
            Action {
                text: qsTr('2D preferences')
                onTriggered: settings2d.show()
            }
            Action {
                text: qsTr('3D preferences')
                onTriggered: settings3dDialog.show()
            }
            MenuSeparator {}

            Menu {
                title: qsTr('Overlays')
                Action {
                    text: qsTr('Axes')
                    checkable: true
                    checked: surfaceView.showAxes
                    onTriggered: surfaceView.setShowAxes(checked)
                }
                Action {
                    text: qsTr('Contours')
                    checkable: true
                    checked: surfaceView.showContours
                    onTriggered: surfaceView.setContours(checked)
                }
                Action {
                    text: qsTr('Navigation');
		    checkable: true;
		    checked: surfaceView.showNavigation
                    onTriggered: {surfaceView.setShowNavTrack(checked)}
                }						
            }

            Menu {
                title: qsTr('Surface colored by')
                ActionGroup { id: coloredScalarActions; exclusive: true }
                Action {
                    text: qsTr('Elevation')
                    checkable: true
                    // Enums are declared on TopoDataItem; accessible via that
                    // registered name regardless of which subclass is in use.
                    checked: surfaceView.coloredScalar === TopoDataItem.Elevation
                    ActionGroup.group: coloredScalarActions
                    onTriggered: surfaceView.setColoredScalar(TopoDataItem.Elevation)
                }
                Action {
                    text: qsTr('Slope')
                    checkable: true
                    checked: surfaceView.coloredScalar === TopoDataItem.Slope
                    ActionGroup.group: coloredScalarActions
                    onTriggered: surfaceView.setColoredScalar(TopoDataItem.Slope)
                }
            }

            Menu {
                title: qsTr('Surface shadows')
                ActionGroup { id: shadowSourceActions; exclusive: true }
                Action {
                    text: qsTr('Illumination')
                    checkable: true
                    checked: surfaceView.shadowSource === TopoDataItem.Illumination
                    ActionGroup.group: shadowSourceActions
                    onTriggered: surfaceView.setShadowSource(TopoDataItem.Illumination)
                }
                Action {
                    text: qsTr('Local slope')
                    checkable: true
                    checked: surfaceView.shadowSource === TopoDataItem.LocalSlope
                    ActionGroup.group: shadowSourceActions
                    onTriggered: surfaceView.setShadowSource(TopoDataItem.LocalSlope)
                }
                Action {
                    text: qsTr('Local slope (GPU)')
                    checkable: true
                    checked: surfaceView.shadowSource === TopoDataItem.LocalSlopeGpu
                    ActionGroup.group: shadowSourceActions
                    onTriggered: surfaceView.setShadowSource(TopoDataItem.LocalSlopeGpu)
                }
                Action {
                    text: qsTr('No shadows')
                    checkable: true
                    checked: surfaceView.shadowSource === TopoDataItem.NoShadows
                    ActionGroup.group: shadowSourceActions
                    onTriggered: surfaceView.setShadowSource(TopoDataItem.NoShadows)
                }
            }

            Menu {
                title: qsTr('Surface render')
                ActionGroup { id: surfaceRenderActions; exclusive: true }
                Action {
                    text: qsTr('Polygons')
                    checkable: true
                    checked: surfaceView.surfaceRenderType === TopoDataItem.Polys
                    ActionGroup.group: surfaceRenderActions
                    onTriggered: surfaceView.setSurfaceRenderType(TopoDataItem.Polys)
                }
                Action {
                    text: qsTr('Point cloud')
                    checkable: true
                    checked: surfaceView.surfaceRenderType === TopoDataItem.PointCloud
                    ActionGroup.group: surfaceRenderActions
                    onTriggered: surfaceView.setSurfaceRenderType(TopoDataItem.PointCloud)
                }
                Action {
                    text: qsTr('Wireframe')
                    checkable: true
                    checked: surfaceView.surfaceRenderType === TopoDataItem.Wireframe
                    ActionGroup.group: surfaceRenderActions
                    onTriggered: surfaceView.setSurfaceRenderType(TopoDataItem.Wireframe)
                }
            }

            Menu {
                id: colormapMenu
                title: qsTr('Color map')
                property string currentCmap: ''
                Repeater {
                    model: SharedConstants.cmaps
                    MenuItem {
                        text: modelData
                        checkable: true
                        checked: colormapMenu.currentCmap === modelData
                        onTriggered: {
                            colormapMenu.currentCmap = modelData
                            surfaceView.setColormap(modelData)
                        }
                    }
                }
            }
            MenuSeparator {}
            MenuItem {
                text: qsTr('Reset camera')
                onTriggered: surfaceView.setOrthographicView()
            }
        }

	Menu {
	    id: toolsMenu
	    title: qsTr('Tools')

	    Menu {
		id: mouseModeMenu
		title: qsTr('Mouse-Action')
		property string currentMode: ''
		property var disabledModes: []
	    
		Repeater {
                    model: SharedConstants.mouseModes
                    MenuItem {
			text: modelData.name
			checkable: true
			checked: mouseModeMenu.currentMode === modelData.name
			enabled: mouseModeMenu.disabledModes.indexOf(modelData.name) == -1
			ToolTip.visible: hovered
			ToolTip.text: modelData.toolTip
			onTriggered: {
                            mouseModeMenu.currentMode = modelData.name
                            surfaceView.setMouseMode(modelData.name)
			}
                    }
		}
            }
	    Action {
		text: qsTr('Build swath grid')
		onTriggered: {
		    console.log('Show grid parameters')
		    gridParamterDialog.show()
		}
	    }
	    Action {
		text: qsTr('Options')
	    }
	}

    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Toolbar dock-position context menu (right-click on toolbar)
    // ─────────────────────────────────────────────────────────────────────────

    Menu {
        id: toolBarDockMenu
        MenuItem { text: qsTr("Dock to top");    onTriggered: mainWindow.toolBarEdge = "top"    }
        MenuItem { text: qsTr("Dock to bottom"); onTriggered: mainWindow.toolBarEdge = "bottom" }
        MenuItem { text: qsTr("Dock to left");   onTriggered: mainWindow.toolBarEdge = "left"   }
        MenuItem { text: qsTr("Dock to right");  onTriggered: mainWindow.toolBarEdge = "right"  }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Toolbar-visibility toggle — floats at top-right, always visible
    // ─────────────────────────────────────────────────────────────────────────

    ToolButton {
        id: toolBarToggle
        z: 2                              // above menu bar and toolbar
        anchors.top:   parent.top
        anchors.right: parent.right
        anchors.topMargin:   2
        anchors.rightMargin: 4
        text: "☰"
        checkable: true
        checked: toolBar.visible
        onClicked: toolBar.visible = !toolBar.visible
        ToolTip.visible: hovered
        ToolTip.text: toolBar.visible ? qsTr("Hide toolbar") : qsTr("Show toolbar")
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Mouse-mode toolbar (dockable — right-click to move to any edge)
    // ─────────────────────────────────────────────────────────────────────────

    ToolBar {
        id: toolBar

        // ── position & size ──────────────────────────────────────────────────
        readonly property bool vertical:
            mainWindow.toolBarEdge === "left" || mainWindow.toolBarEdge === "right"
        readonly property real menuBottom: menuBar.y + menuBar.height

        x:      mainWindow.toolBarEdge === "right" ? parent.width - width : 0
        y:      mainWindow.toolBarEdge === "bottom" ? parent.height - height : menuBottom
        // For vertical docking use a fixed width; a plain-child Flow does NOT
        // propagate its implicitWidth to the ToolBar (the internal contentItem
        // always reports 0), so implicitWidth-based sizing would collapse the bar.
        width:  vertical ? 100 : parent.width
        height: vertical ? (parent.height - menuBottom) : implicitHeight

        // ── button group ─────────────────────────────────────────────────────
        ButtonGroup { id: mouseToolGroup }

        // ── button flow ──────────────────────────────────────────────────────
        // Plain child (not contentItem:) so Qt Controls does not interfere with
        // the Flow's layout bootstrap.  Declared before the MouseArea so the
        // MouseArea sits on top in z-order and can catch right-clicks.
        Flow {
            anchors.fill: parent
            anchors.margins: 2
            flow:    toolBar.vertical ? Flow.TopToBottom : Flow.LeftToRight
            spacing: 2

            Repeater {
                model: SharedConstants.mouseModes

                ToolButton {
                    id: modeBtn
                    text: modelData.name
                    checkable: true
                    checked: mouseModeMenu.currentMode === modelData.name
                    enabled: mouseModeMenu.disabledModes.indexOf(modelData.name) === -1
                    ButtonGroup.group: mouseToolGroup

                    // Vertical toolbar: fill the toolbar width so the label can wrap.
                    // Horizontal toolbar: size naturally (one line, no wrap needed).
                    width: toolBar.vertical ? (toolBar.width - 4) : implicitWidth

                    // ── label ──────────────────────────────────────────────────
                    contentItem: Label {
                        text:                modeBtn.text
                        font:                modeBtn.font
                        wrapMode:            Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment:   Text.AlignVCenter
                        // White text on the blue checked background; dark otherwise.
                        color:   modeBtn.checked ? "#ffffff" : "#1a1a1a"
                        opacity: modeBtn.enabled ? 1.0 : 0.4
                    }

                    // ── background with outline + gradient shading ─────────────
                    background: Rectangle {
                        radius: 3
                        border.width: 1

                        // Border darkens on interaction; blue tint when checked.
                        border.color: !modeBtn.enabled ? "#cccccc"
                                    : modeBtn.checked  ? "#1a4a89"
                                    : modeBtn.pressed  ? "#666666"
                                    : modeBtn.hovered  ? "#888888"
                                    :                    "#aaaaaa"

                        gradient: Gradient {
                            // Top stop — lighter edge gives the "lit from above" look.
                            GradientStop {
                                position: 0.0
                                color: !modeBtn.enabled ? "#eeeeee"
                                     : modeBtn.checked  ? "#5090d9"
                                     : modeBtn.pressed  ? "#c0c0c0"
                                     : modeBtn.hovered  ? "#fefefe"
                                     :                    "#f6f6f6"
                            }
                            // Bottom stop — darker edge; reversed for pressed (sunken look).
                            GradientStop {
                                position: 1.0
                                color: !modeBtn.enabled ? "#e0e0e0"
                                     : modeBtn.checked  ? "#1a5a99"
                                     : modeBtn.pressed  ? "#e0e0e0"
                                     : modeBtn.hovered  ? "#d4d4d4"
                                     :                    "#dcdcdc"
                            }
                        }
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: modelData.toolTip

                    onClicked: {
                        mouseModeMenu.currentMode = modelData.name
                        surfaceView.setMouseMode(modelData.name)
                    }
                }
            }
        }

        // ── right-click to reposition ────────────────────────────────────────
        // Declared last → highest z → catches right-clicks on the background.
        // acceptedButtons: RightButton means left-clicks are never consumed here;
        // hoverEnabled defaults to false so hover propagates to buttons below.
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: toolBarDockMenu.popup()
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Main surface view
    // ─────────────────────────────────────────────────────────────────────────

    ColumnLayout {
        // Respond to all four possible dock edges, plus the hidden state.
        anchors.top:    (toolBar.visible && mainWindow.toolBarEdge === "top")    ? toolBar.bottom : menuBar.bottom
        anchors.bottom: (toolBar.visible && mainWindow.toolBarEdge === "bottom") ? toolBar.top    : parent.bottom
        anchors.left:   (toolBar.visible && mainWindow.toolBarEdge === "left")   ? toolBar.right  : parent.left
        anchors.right:  (toolBar.visible && mainWindow.toolBarEdge === "right")  ? toolBar.left   : parent.right
        spacing: 0

        Label {
            id: dataFilenameLabel
            text: qsTr('No file loaded')
            font.pixelSize: 13
            leftPadding: 6
            Layout.fillWidth: true
        }

        SurfaceDataItem {
            objectName: 'surfaceDataItem'
            id: surfaceView
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Edit window
    //  Opens automatically when the user completes a rubber-band selection in
    //  the main window (onEditBoundsChanged below).  setEditBounds() itself is
    //  wired in C++ (main.cpp) so it does not need to be called here.
    // ─────────────────────────────────────────────────────────────────────────

    Window {
        id: editWindow
        title: qsTr('Edit data — point cloud')
        width: 700
        height: 750
        visible: false

        onClosing: surfaceView.forceActiveFocus()

        // Ctrl+Z undoes the most recent edit gesture when the edit window
        // has focus.  Mirrors the Undo toolbar button.
        Shortcut {
            sequence: "Ctrl+Z"
            onActivated: editView.undoLastEdit()
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            // ── Edit toolbar ──────────────────────────────────────────────────
            ToolBar {
                Layout.fillWidth: true

                Flow {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 6

                    Label { text: qsTr("Flag selected point() as:") }

                    // RadioButton pair controls EditDataItem::setFlagValue()
                    RadioButton {
                        id: flagBadButton
                        text: qsTr("Bad")
                        checked: true
                        onCheckedChanged: if (checked) editView.setFlagValue(0)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Clicking a point marks it as bad data (shown in red)")
                    }
                    RadioButton {
                        id: unflagButton
                        text: qsTr("Good")
                        onCheckedChanged: if (checked) editView.setFlagValue(1)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Clicking a point restores it as good data")
                    }

                    ToolSeparator {}

                    CheckBox {
                        id: showBadCheckbox
                        text: qsTr("Show bad points")
                        checked: true
                        onCheckedChanged: editView.setShowBadPoints(checked)
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Hide or show points currently flagged as bad")
                    }

                    ToolSeparator {}

                    Button {
                        text: qsTr("Reset view")
                        flat: true
                        onClicked: editView.resetCamera()
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Reset camera to fit the current edit volume")
                    }

                    ToolSeparator {}

                    Button {
                        text: qsTr("Undo")
                        flat: true
                        onClicked: editView.undoLastEdit()
                        ToolTip.visible: hovered
                        ToolTip.text: qsTr("Undo the most recent click or alt-drag edit (Ctrl+Z)")
                    }

                    Label { text: qsTr('Select points with left-click or alt-drag-left') }		    

                    Item { Layout.fillWidth: true }   // push controls left
                }
            }

            // ── Edit point-cloud view ─────────────────────────────────────────
            EditDataItem {
                objectName: 'editDataItem'
                id: editView
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Auxiliary windows
    // ─────────────────────────────────────────────────────────────────────────

    Window {
        id: topoProfileWindow
        title: qsTr('Elevation profile')
        width: 500
        height: 500
        visible: false
        TopoProfileGraph { id: profileGraph }
        onClosing: surfaceView.forceActiveFocus()
    }

    Settings2dWindow {
        id: settings2d
        visible: false
        onClosing: surfaceView.forceActiveFocus()
    }

    Window {
	id: gridParamterDialog
	title: qsTr('Grid parameters')
	visible: false
        width:       gridParameters.implicitWidth
        height:      gridParameters.implicitHeight
        minimumWidth:  gridParameters.implicitWidth
        minimumHeight: gridParameters.implicitHeight


	   GridParameters {
               id: gridParameters
               anchors.fill: parent
               onAccepted: {
		   // read gridParameters.cellSize, .algorithm,
		   // .interpScale, ._selected
		   // and apply them, then close
		   gridParameterDialog.close()
               }
               onRejected: gridParameterDialog.close()
	   }
    }
    
    Window {
        id: settings3dDialog
        title: qsTr('3D preferences')
        visible: false
        width:       settings3D.implicitWidth
        height:      settings3D.implicitHeight
        minimumWidth:  settings3D.implicitWidth
        minimumHeight: settings3D.implicitHeight

        Settings3D {
            id: settings3D
            anchors.fill: parent
            anchors.margins: 10

            lightsEnabled.onCheckedChanged: updateLighting()
            intensity.onMoved:              updateLighting()
            lightX.onMoved:                 updateLighting()
            lightY.onMoved:                 updateLighting()
            lightZ.onMoved:                 updateLighting()

            slopeGamma.onMoved:
                surfaceView.setSlopeGamma(slopeGamma.value)
            slopeFloor.onMoved:
                surfaceView.setMinBrightness(slopeFloor.value)
            verticalExagg.onMoved:
                surfaceView.setVerticalExagg(verticalExagg.value)
            contourInterval.onMoved:
                surfaceView.setContourInterval(contourInterval.value)
            showContours.onCheckedChanged:
                surfaceView.setContours(showContours.checked)
            showContourLabels.onCheckedChanged:
                surfaceView.setShowContourLabels(showContourLabels.checked)
        }

        // Sync slider values to current item state each time the dialog opens
        onVisibilityChanged: {
            if (!visible) return
            var pos = surfaceView.getLightPosition()
            settings3D.lightX.value    = pos[0]
            settings3D.lightY.value    = pos[1]
            settings3D.lightZ.value    = pos[2]
            settings3D.intensity.value            = surfaceView.getLightIntensity()
            settings3D.slopeGamma.value           = surfaceView.getSlopeGamma()
            settings3D.slopeFloor.value           = surfaceView.getMinBrightness()
            settings3D.verticalExagg.value        = surfaceView.getVerticalExagg()
            settings3D.showContours.checked       = surfaceView.showContours
            settings3D.showContourLabels.checked  = surfaceView.showContourLabels
            settings3D.contourInterval.value      = surfaceView.getContourInterval()
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Dialogs
    // ─────────────────────────────────────────────────────────────────────────

    FileDialog {
        id: datafileDialog
        title: qsTr('Open GMT grid or swath file')
        nameFilters: ['Swath files (*.grd *.mb-1 *.mb[0-9][0-9])']
        onAccepted: {
            surfaceView.loadDatafile(selectedFile)
            surfaceView.forceActiveFocus()
        }
        onRejected: surfaceView.forceActiveFocus()
    }

    MessageDialog {
        id: quitDialog
        title: qsTr("Quit?")
        text:  qsTr("Quit application?")
        buttons: MessageDialog.Yes | MessageDialog.No
        Component.onCompleted: visible = false
        onAccepted: Qt.quit()
        onRejected: surfaceView.forceActiveFocus()
    }

    MessageDialog {
        id: errorDialog
        title: qsTr('Error')
        buttons: MessageDialog.Ok
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Signal connections
    // ─────────────────────────────────────────────────────────────────────────

    Connections {
        target: surfaceView

        // Update the filename label when a new file is loaded
        function onDataFilenameChanged(newName) {
            dataFilenameLabel.text = newName

	    if (/\.mb.*$/.test(newName)) {
		// This is a swath file; all mouse modes relevant
 		mouseModeMenu.disabledModes = []
	    }
	    else {
		// Not a swath file; edit swath data not relevant
		mouseModeMenu.disabledModes = [SharedConstants.editSwathModeName]
	    }
        }

        // Open the edit window when the user completes a rubber-band selection.
        // setEditBounds() is connected to editView in C++ (main.cpp); this
        // handler only manages window visibility and focus.
        function onEditBoundsChanged(xMin, xMax, yMin, yMax, zMin, zMax) {
            editWindow.visible = true
            editView.forceActiveFocus()
        }

        // Elevation profile graph
        function onLineDefined(profileData) {
            profileGraph.xyData.clear()
            var xmin = Infinity,  xmax = -Infinity
            var ymin = Infinity,  ymax = -Infinity
            for (var i = 0; i < profileData.length; i++) {
                var pt = profileData[i]
                if (pt.x < xmin) xmin = pt.x
                if (pt.x > xmax) xmax = pt.x
                if (pt.y < ymin) ymin = pt.y
                if (pt.y > ymax) ymax = pt.y
                profileGraph.xyData.append(pt.x, pt.y)
            }
            profileGraph.axisX.min = xmin
            profileGraph.axisX.max = xmax
            profileGraph.axisY.min = ymin
            profileGraph.axisY.max = ymax
            topoProfileWindow.show()
            surfaceView.forceActiveFocus()
        }

        function onErrorOccurred(message) {
            errorDialog.text = message
            errorDialog.open()
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    //  Helper functions
    // ─────────────────────────────────────────────────────────────────────────

    function updateLighting() {
        surfaceView.setLight(settings3D.lightsEnabled.checked,
                             settings3D.intensity.value,
                             settings3D.lightX.value,
                             settings3D.lightY.value,
                             settings3D.lightZ.value)
    }
}
