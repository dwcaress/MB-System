import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Dialogs 
import QtQuick.Layouts 1.14
import QtQuick.Window 2.14
import mbsystem.MBQuickItem 1.0
import "ui-components"

/* ***
Displays bathymetry/topography as an OpenGL "underlay" in QML

BackEnd singleton must be registered in root context by main.cpp
See https://qml.guide/singletons/
*** */


ApplicationWindow {
    id: applicationWindow
    objectName: "mainWindow"
    visible: true
    width: 1000
    height: 880
    title: "mbgrdview-3"


    property var distSlider: -1
    property var elevSlider: -1
    property var azimSlider: -1
    
    property bool dragOrientation: false;
    property bool dragPan: false;
    property bool panMode: false
    property int pressedMouseX: -1;
    property int pressedMouseY: -1


    ActionGroup {
        id: mapActions
        exclusive: true
    }

    ActionGroup {
        id: topoActions
        exclusive: true
    }

    ActionGroup {
        id: shadeActions
        exclusive: true
    }

    ActionGroup {
        id: navActions
        exclusive: true
    }

    ActionGroup {
        id: colorActions
        exclusive: true
    }

    ActionGroup {
        id: mouseActions
        exclusive: true
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action { text: qsTr("Open grid") ;
                onTriggered: { console.log("show file dialog")
                    fileDialog.open()}
            }
            Action { text: qsTr("Open site") ;
                onTriggered: { console.log("open site")
                }
            }
            Action { text: qsTr("Open route") ;
                onTriggered: { console.log("open route")
                }
            }
            Action { text: qsTr("Open navigation") ;
                onTriggered: { console.log("open navigation")
                }
            }
            Action { text: qsTr("Exit") ;
                onTriggered: { console.log("exit");
                    quitDialog.open()
                }
            }

            onAboutToHide: {
                console.log("about to hide; start timer");
                syncRenderTimer.running = true;
            }
        }


        Menu {
            title: qsTr("&View")
            Menu {
                title: "Map/3D"
                Action { checkable: true; checked: true; text: qsTr("&Map"); ActionGroup.group: mapActions }
                Action { checkable: true; text: qsTr("&3D"); ActionGroup.group: mapActions }
                Action { checkable: true; checked: true; text: qsTr("&Topography"); ActionGroup.group: topoActions }
                Action { checkable: true; text: qsTr("&Topography slope"); ActionGroup.group: topoActions }

            }

            MenuSeparator {}
            Action { checkable: true; text: qsTr("&Histograms") }
            Action { checkable: true; text: qsTr("&Contours") }
            Action { checkable: true; text: qsTr("&Sites") }
            Action { checkable: true; text: qsTr("&Routes") }
            Action { checkable: true; text: qsTr("&Vector") }
            Action { checkable: true; text: qsTr("&Profile window") }
            MenuSeparator {}
            Menu {
                title: "Shading"
                Action {checkable: true; checked: true; text: qsTr("Off"); ActionGroup.group: shadeActions }
                Action {checkable: true; text: qsTr("Slope"); ActionGroup.group: shadeActions}
                Action {checkable: true; text: qsTr("Illumination"); ActionGroup.group: shadeActions }
            }
            MenuSeparator {}
            Menu {
                title: "Navigation"
                Action {checkable: true; checked: true; text: qsTr("Off"); ActionGroup.group: navActions }
                Action {checkable: true; text: qsTr("Draped"); ActionGroup.group: navActions}
                Action {checkable: true; text: qsTr("Non-draped"); ActionGroup.group: navActions }
            }
            MenuSeparator {}
            Menu {
                title: "Color table"
                Action {checkable: true; checked: true; text: qsTr("Haxby"); ActionGroup.group: colorActions
                    onTriggered: { surface3D.theme.baseGradients=[haxbyGradient] }
                }

                Action {checkable: true; text: qsTr("Bright rainbow"); ActionGroup.group: colorActions
                    onTriggered: { surface3D.theme.baseGradients=[rainbowGradient] }
                }

                Action {checkable: true; text: qsTr("Muted rainbow"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Grayscale"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Flat gray"); ActionGroup.group: colorActions }

                Action {checkable: true; text: qsTr("Sealevel-1"); ActionGroup.group: colorActions
                    onTriggered: { surface3D.theme.baseGradients=[sealevel1Gradient] }
                }

                Action {checkable: true; text: qsTr("Sealevel-2"); ActionGroup.group: colorActions }
            }
            MenuSeparator {}
            Action {text: qsTr("Re-center");
                onTriggered: {
                    console.log("set offsets");
                    camera.xOffset = 0; camera.yOffset = 0;
                    console.log("offsets have been changed");
                    // selectedFile.text = "Re-centered!";
                    // mouseControlArea.requestRepaint;
                }
            }
            onAboutToHide: {
                console.log("about to hide; start timer");
                syncRenderTimer.running = true;
            }
        }

        Menu {
            title: "&Settings"
            Action {text: qsTr("Color and contours"); onTriggered: {
                    console.log("Create Popup");
                    var component = Qt.createComponent("ui-components/Popup.qml");
                    if (component.status === Component.Ready) {
                        console.log("component is ready - call BackEnd");
                        console.log("component is ready - yOffset2d=" + BackEnd.yOffset2d);
                        var dialog = component.createObject(appWindow,{popupType: 1});
                        // dialogConnection.target = dialog
                        dialog.show();
                    }
                    else {
                        console.log("component is NOT ready");
                    }
                }
            }

            Action {text: qsTr("2D"); onTriggered: {
                    console.log("show 2d settings window");
                    settings2d.show()
                }
            }

            Action {text: qsTr("3D"); onTriggered: {
                    console.log("show 3d settings window");
                    settings3d.show()
                }
            }
            Action {text: qsTr("Shading")}
            Action {text: qsTr("Resolution")}
            Action {text: qsTr("Projections")}
            Action {text: qsTr("Site list")}
            Action {text: qsTr("Route list")}
            Action {text: qsTr("Navigation list")}

            onAboutToHide: {
                console.log("about to hide; start timer");
                syncRenderTimer.running = true;
            }
        }

        Menu {
            title: "&Mouse"
            Action {checkable: true; checked: true; text: qsTr("Pan and zoom"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Rotate model"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Rotate view"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Shading"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Pick area"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Edit sites"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Edit routes"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Pick nav"); ActionGroup.group: mouseActions }
            Action {checkable: true; text: qsTr("Pick nav file"); ActionGroup.group: mouseActions }

            onAboutToHide: {
                console.log("about to hide; start timer");
                syncRenderTimer.running = true;
            }
        }

        Menu {
            title: "Help"
            Action {text: qsTr("About"); onTriggered: {
                    console.log("show version info");
                    myMessageDialog.text = qsTr("PROTOTYPE");
                    myMessageDialog.open()
                }

            }

            onAboutToHide: {
                console.log("about to hide; start timer");
                syncRenderTimer.running = true;
            }

        }
    }

    
    ColumnLayout {
        id: columnLayout
        Layout.fillHeight: false
        width: 1000
        Text {
            id: selectedFile
            objectName: "selectedFile"
            text: "filename goes HERE"
            font.family: "Courier"
            font.pointSize: 18
            color: "black"
        }

        RowLayout {
            CheckBox {
                text: qsTr("Right mouse button pan")
                anchors.top: selectedFile.bottom
                anchors.topMargin: 0
                onClicked: {
                    if (checkState == Qt.Checked) {
                        panMode = true;
                        console.log("pan enabled")
                    }
                    else {
                        panMode = false;
                        console.log("pan disabled")
                    }
                }
            }
        }


        // Item through which C++ application makes "custom" OpenGL drawing of bathymetry in ApplicationWindow.
        // No size need be specified
        MBQuickItem {
            id: mbQuickItem
            objectName: "mbQuickItem"   // main.cpp looks up object by this name
        }

    }   // ColumnLayout


    Rectangle {   // Camera controls rectangle
        id: cameraControls
        property var camera
        border.color: "#000000"
        border.width: 2
        radius: 5
        color: "#55ffffff"

        width: 400
        height: 150
        anchors.bottom: parent.bottom

        Component.onCompleted: controlItems.createObject(cameraControls)

        Component {
            id: controlItems
            GridLayout {
                anchors.fill: parent
                anchors.margins: 5
                columns: 3

                Label { text: "Azimuth" }
                Slider {
                    id: azimuthSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 360
                    value: 180
                    Component.onCompleted: {
                        azimSlider = azimuthSlider;
                    }
                    onValueChanged: camera.azimuth = value
                }
                // Set displayed decimal places on slider label
                Label { text: camera.azimuth.toFixed(2) }

                Label { text: "Elevation" }
                Slider {
                    id: elevationSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 360
                    value: 10
                    Component.onCompleted: {
                        elevSlider = elevationSlider;
                    }
                    onValueChanged: camera.elevation = value
                }
                // Set displayed decimal places on slider label
                Label { text: camera.elevation.toFixed(2) }

                Label { text: "Distance" }
                Slider {
                    id: distanceSlider
                    objectName: "distanceSlider"
                    Layout.fillWidth: true
                    from: 1
                    to: 50
                    value: 4
                    Component.onCompleted: {
                        distSlider = distanceSlider;
                    }
                    onValueChanged: {camera.distance = value}
                }
                // Set displayed decimal places on slider label
                Label { text: camera.distance.toFixed(2) }

                /* ***
                Label { text: "xOffset" }
                Slider {
                    id: xOffsetSlider
                    objectName: "xOffsetSlider"
                    Layout.fillWidth: true
                    from: -1000
                    to: 1000
                    value: 0
                    Component.onCompleted: {
              // distSlider = distanceSlider;
            }
                    onValueChanged: {camera.xOffset = value}
                }
        *** */
                // Set displayed decimal places on slider label
                // Label { text: camera.xOffset.toFixed(2) }
            }

        }
    }

    Canvas {
        id: mouseControlArea
        height: 200
        width: 100
        // anchors.left: cameraControls.right
        anchors.bottom: cameraControls.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: columnLayout.bottom
        // color: "#800000FF"   // translucent blue
        // color: "transparent"

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            enabled: true

            onPressed: {
                if (pressedButtons & Qt.RightButton) {
                    if (panMode) {
                        dragPan = true;
                        dragOrientation = false;
                    }
                    else {
                        dragOrientation = true;
                        dragPan = false;
                    }
                    pressedMouseX = mouse.x;
                    pressedMouseY = mouse.y;
                }
            }

            onReleased: {
                dragOrientation = false;
                dragPan = false;
                console.log("DISABLE dragOrientation");
                console.log("DISABLE dragPan");
            }

            onWheel: {
                console.log("wheel turned ", wheel.angleDelta);
                console.log("wheel y change: ", wheel.angleDelta.y);
                console.log("distSlider.maximumValue: ", distSlider.maximumValue);
                console.log("distSlider.from: ", distSlider.from, ", distSlider.to: ", distSlider.to);
                var delta = 0.05 * (distSlider.to - distSlider.from);
                if (wheel.angleDelta.y > 0) {
                    camera.distance = camera.distance + delta;
                }
                else {
                    camera.distance = camera.distance - delta;
                    if (camera.distance < 0.1) { camera.distance = 0.1; }
                }
                distSlider.value = camera.distance
                // console.log("camera.distance: ", camera.distance);
            }

            onPositionChanged: {
                if (dragOrientation) {
                    // console.log("dragOrientation");
                    var delta = -0.01 * (mouse.x - pressedMouseX);
                    camera.azimuth = camera.azimuth + delta;
                    azimSlider.value = camera.azimuth;

                    delta = 0.01 * (mouse.y - pressedMouseY);
                    camera.elevation += delta;
                    elevSlider.value = camera.elevation;
                }

                if (dragPan) {
                    console.log("dragging pan");
                    var delta = (mouse.x - pressedMouseX);
                    console.log("delta: " , delta);
                    console.log("camera.xOffset: " , camera.xOffset);
                    camera.xOffset = camera.xOffset + delta;

                    delta = (mouse.y - pressedMouseY);
                    camera.yOffset = camera.yOffset + delta;
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

    MessageDialog {
        id: myMessageDialog
        objectName: "myMessageDialog"
        title: "my message dialog"
        text: "this is default text"
        Component.onCompleted: visible = false
    }

    Settings2dWindow {
        id: settings2d
        visible: false
    }

    Settings3dWindow {
        id: settings3d
        visible: false
    }


    FileDialog {
        id: fileDialog
        title: "Open file"
        nameFilters: ["Grid files (*.grd)"]
        onAccepted: {
            console.log("accepted " + selectedFile);
            mbQuickItem.setGridSurface(selectedFile)
        }
    }


    // Timer is used to force a sync-render 
    Timer {
        id: syncRenderTimer
        interval: 100; running: false; repeat: false
        onTriggered: {
            console.log("timer triggered");
            // console.log("set camera.xOffset");
            // camera.xOffset=0
            // mouseControlArea.requestRepaint;
            console.log("set camera.forceRender");
            camera.forceRender = true;
            // selectedFile.text = "timer triggered";
        }
    }

}

