import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.1
import QtDataVisualization 1.14
import QtQuick.Layouts 1.14
import Qt3D.Extras 2.14
import "ui-components"

/* ***
Displays bathymetry/topography as a Surface3D object.
Mouse controls:
Right drag: rotate
Center wheel: zoom
Left click: Select axis to be dragged
Left drag: Translate scale on selected axis

BackEnd singleton must be registered in root context by main.cpp
See https://qml.guide/singletons/
*** */

ApplicationWindow {
    id: applicationWindow
    objectName: "mainWindow"
    visible: true
    width: 1000
    height: 880
    title: "TEST4"

    property int mySelectedAxis: -1
    property real dragSpeedModifier: 100.0
    property int currentMouseX: -1
    property int currentMouseY: -1
    property int previousMouseX: -1
    property int previousMouseY: -1
    property bool panMode: false

    Settings2dWindow {
        id: settings2d
        visible: false
    }


    Settings3dWindow {
        id: settings3d
        visible: false
    }


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
        }

        Menu {
            title: "&Settings"
            Action {text: qsTr("Color and contours"); onTriggered: {
                    console.log("Create Popup");
                    var component = Qt.createComponent("ui-components/Popup.qml");
                    if (component.status === Component.Ready) {
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

    FileDialog {
        id: fileDialog
        title: "Open file"
        nameFilters: ["Grid files (*.grd)"]
        onAccepted: {
            console.log("accepted " + fileUrl);
            BackEnd.setGridFile(fileUrl)
        }
    }

    Item {
        id: item1
        anchors.fill: parent


        Text {
            id: selectedFile
            objectName: "selectedFile"
            text: "filename goes HERE"
            anchors.top: parent.top
            anchors.topMargin: 0
            font.family: "Courier"
            font.pointSize: 18
            color: "black"
        }


        RowLayout {
            id: radioLayout
            anchors.top: selectedFile.bottom
            anchors.topMargin: 0

            CheckBox {
                text: qsTr("Left mouse pan enabled")
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
        Item {
            width: 964
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: radioLayout.bottom
            anchors.topMargin: 0
            objectName: "surface3DItem"

            Surface3D {
                // inputHandler: null
                anchors.fill: parent
                objectName: "surface3D"
                id: surface3D


                theme: Theme3D {
                    type: Theme3D.ThemeQt
                    labelBorderEnabled: false
                    font.pointSize: 35
                    labelBackgroundEnabled: true

                    //  lightStrength: 1.
                    lightStrength: 5.
                    ambientLightStrength: .3
                    colorStyle: Theme3D.ColorStyleRangeGradient

                    baseGradients: [haxbyGradient]

                }


                Surface3DSeries {
                    objectName: "surface3DSeries"
                    itemLabelFormat: "Pop density at (@xLabel N, @zLabel E): @yLabel"
                    ItemModelSurfaceDataProxy {
                        itemModel: dataModel
                        // Mapping model roles to surface series rows, columns, and values.
                        rowRole: "longitude"
                        columnRole: "latitude"
                        yPosRole: "pop_density"
                    }
                    Component.onCompleted: selectedFile.text="TEST SURFACE"

                }



                // Clicking on specific axis and then dragging on that axis
                // translates surace along that axis
                onSelectedElementChanged: {

                    if (selectedElement >= AbstractGraph3D.ElementAxisXLabel
                            && selectedElement <= AbstractGraph3D.ElementAxisZLabel) {
                        mySelectedAxis = selectedElement
                        console.log("set selected axis=", mySelectedAxis)
                    }
                    else {
                        // console.log("selected a non-axis element; set selected axis = -1")
                        if (!panMode) mySelectedAxis = -1
                    }
                }

            }


            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton
                //! [1]

                //! [3]
                onPositionChanged: {

                    // Keep track of mouse position
                    currentMouseX = mouse.x;
                    currentMouseY = mouse.y;

                    if (pressed && panMode && mySelectedAxis == -1) {
                        // console.log("call dragAxes()");
                        item1.dragAxes();
                    }
                    else if (pressed && mySelectedAxis != -1) {
                        // console.log("call dragAxis(selectedAxis)");
                        item1.dragAxis();
                    }

                    previousMouseX = currentMouseX;
                    previousMouseY = currentMouseY;
                }

                onPressed: {
                    surface3D.scene.selectionQueryPosition = Qt.point(mouse.x, mouse.y);
                }

                onReleased: {
                    // We need to clear mouse positions and selected axis, because touch devices cannot
                    // track position all the time
                    mySelectedAxis = -1
                    currentMouseX = -1
                    currentMouseY = -1
                    previousMouseX = -1
                    previousMouseY = -1
                }
            }
        }

        // Change range of selected axis to reflect mouse-drag
        function dragAxis() {
            // console.log("dragAxis()")
            // Do nothing if previous mouse position is uninitialized
            if (previousMouseX === -1) {
                // console.log("dragAxis(): previousMousX=-1 just return")
                return
            }

            // Directional drag multipliers based on rotation. Camera is locked to 45 degrees, so we
            // can use one precalculated value instead of calculating xx, xy, zx and zy individually
            var cameraMultiplier = 0.70710678; // Need to multiply angle by axis range

            // Calculate the mouse move amount
            var moveX = currentMouseX - previousMouseX
            var moveY = currentMouseY - previousMouseY
            // console.log("dragAxis(): moveX=", moveX, "moveY=", moveY)
            // Adjust axes
            switch (mySelectedAxis) {
            case AbstractGraph3D.ElementAxisXLabel:
                var distance = ((moveX - moveY) * cameraMultiplier) / dragSpeedModifier
                distance *= (surface3D.axisX.max -surface3D.axisX.min)
                // console.log("X: distance=", distance)
                // Check if we need to change min or max first to avoid invalid ranges
                if (distance > 0) {
                    surface3D.axisX.min -= distance
                    surface3D.axisX.max -= distance
                } else {
                    surface3D.axisX.max -= distance
                    surface3D.axisX.min -= distance
                }
                break
            case AbstractGraph3D.ElementAxisYLabel:
                distance = moveY / dragSpeedModifier
                distance *= (surface3D.axisY.max -surface3D.axisY.min)
                // console.log("Y: distance=", distance)
                // Check if we need to change min or max first to avoid invalid ranges
                if (distance > 0) {
                    surface3D.axisY.max += distance
                    surface3D.axisY.min += distance
                } else {
                    surface3D.axisY.min += distance
                    surface3D.axisY.max += distance
                }
                break
            case AbstractGraph3D.ElementAxisZLabel:
                distance = ((moveX + moveY) * cameraMultiplier) / dragSpeedModifier
                distance *= (surface3D.axisZ.max -surface3D.axisZ.min)
                // Check if we need to change min or max first to avoid invalid ranges
                // console.log("Z: distance=", distance)
                if (distance > 0) {
                    surface3D.axisZ.max += distance
                    surface3D.axisZ.min += distance
                } else {
                    surface3D.axisZ.min += distance
                    surface3D.axisZ.max += distance
                }
                break
            }
        }

        // Change range of selected axis to reflect mouse-drag
        function dragAxes() {
            // console.log("dragAxes()")
            // Do nothing if previous mouse position is uninitialized
            if (previousMouseX === -1) {
                // console.log("dragAxis(): previousMousX=-1 just return")
                return
            }

            // Directional drag multipliers based on rotation. Camera is locked
            // to 45 degrees, so we can use one precalculated value instead of
            // calculating xx, xy, zx and zy individually
            var cameraMultiplier = 0.70710678; // Need to multiply angle by axis range

            // Calculate the mouse move amount
            var moveX = currentMouseX - previousMouseX
            var moveY = currentMouseY - previousMouseY
            // console.log("dragAxes(): moveX=", moveX, "moveY=", moveY)
            // Adjust axes

            // Adjust x-axis
            var distance = ((moveX - moveY) * cameraMultiplier) / dragSpeedModifier
            distance *= (surface3D.axisX.max -surface3D.axisX.min)
            // console.log("X: distance=", distance)
            // Check if we need to change min or max first to avoid invalid ranges
            if (distance > 0) {
                surface3D.axisX.min -= distance
                surface3D.axisX.max -= distance
            } else {
                surface3D.axisX.max -= distance
                surface3D.axisX.min -= distance
            }

            // Adjust z-axis
            distance = ((moveX + moveY) * cameraMultiplier) / dragSpeedModifier
            distance *= (surface3D.axisZ.max -surface3D.axisZ.min)
            // Check if we need to change min or max first to avoid invalid ranges
            // console.log("Z: distance=", distance)
            if (distance > 0) {
                surface3D.axisZ.max += distance
                surface3D.axisZ.min += distance
            } else {
                surface3D.axisZ.min += distance
                surface3D.axisZ.max += distance
            }

        }
    } // Item1





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

    MessageDialog {
        id: myMessageDialog
        objectName: "myMessageDialog"
        title: "my message dialog"
        text: "this is default text"
        Component.onCompleted: visible = false
    }


    // Test surface data model
    ListModel {
        id: dataModel
        ListElement{ longitude: "20"; latitude: "10"; pop_density: "4.75"; }
        ListElement{ longitude: "21"; latitude: "10"; pop_density: "3.00"; }
        ListElement{ longitude: "22"; latitude: "10"; pop_density: "1.24"; }
        ListElement{ longitude: "23"; latitude: "10"; pop_density: "2.53"; }
        ListElement{ longitude: "20"; latitude: "11"; pop_density: "2.55"; }
        ListElement{ longitude: "21"; latitude: "11"; pop_density: "2.03"; }
        ListElement{ longitude: "22"; latitude: "11"; pop_density: "3.46"; }
        ListElement{ longitude: "23"; latitude: "11"; pop_density: "5.12"; }
        ListElement{ longitude: "20"; latitude: "12"; pop_density: "1.37"; }
        ListElement{ longitude: "21"; latitude: "12"; pop_density: "2.98"; }
        ListElement{ longitude: "22"; latitude: "12"; pop_density: "3.33"; }
        ListElement{ longitude: "23"; latitude: "12"; pop_density: "3.23"; }
        ListElement{ longitude: "20"; latitude: "13"; pop_density: "4.34"; }
        ListElement{ longitude: "21"; latitude: "13"; pop_density: "3.54"; }
        ListElement{ longitude: "22"; latitude: "13"; pop_density: "1.65"; }
        ListElement{ longitude: "23"; latitude: "13"; pop_density: "2.67"; }
    }


    ColorGradient {
        id: duskGradient
        ColorGradientStop { position: 0.0; color: "darkslategray" }
        ColorGradientStop { id: middleGradient; position: 0.5; color: "peru" }
        ColorGradientStop { position: 1.0; color: "red" }
    }

    ColorGradient {
        id: rainbowGradient
        ColorGradientStop { position: 0.2; color: "blue" }
        ColorGradientStop { position: 0.4; color: "green" }
        ColorGradientStop { position: 0.6; color: "yellow" }
        ColorGradientStop { position: 0.8; color: "orange" }
        ColorGradientStop { position: 1.0; color: "red" }
    }

    ColorGradient {
        id: haxbyGradient
        ColorGradientStop { position: 0.100; color: "#2539AF"}
        ColorGradientStop { position: 0.200; color: "#287FFB"}
        ColorGradientStop { position: 0.300; color: "#32BEFF"}
        ColorGradientStop { position: 0.400; color: "#6AEBFF"}
        ColorGradientStop { position: 0.500; color: "#8AECAE"}
        ColorGradientStop { position: 0.600; color: "#CDFFA2"}
        ColorGradientStop { position: 0.700; color: "#F0EC79"}
        ColorGradientStop { position: 0.800; color: "#FFBD57"}
        ColorGradientStop { position: 0.900; color: "#FFA144"}
        ColorGradientStop { position: 1.000; color: "#FFBA85"}
        ColorGradientStop { position: 1.100; color: "#F2F2F2"}
    }

    ColorGradient {
        id: sealevel1Gradient
        ColorGradientStop { position: 0.100; color: "#C89628"}
        ColorGradientStop { position: 0.200; color: "#CDA030"}
        ColorGradientStop { position: 0.300; color: "#D2AA38"}
        ColorGradientStop { position: 0.400; color: "#D7B440"}
        ColorGradientStop { position: 0.500; color: "#DCBE48"}
        ColorGradientStop { position: 0.600; color: "#E1C850"}
        ColorGradientStop { position: 0.700; color: "#E6D258"}
        ColorGradientStop { position: 0.800; color: "#EBDC60"}
        ColorGradientStop { position: 0.900; color: "#F0E668"}
        ColorGradientStop { position: 1.000; color: "#F5F070"}
        ColorGradientStop { position: 1.100; color: "#FAFA78"}
    }
}

