import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.3
import QtQuick.Controls.Material 2.2
import QtVTK 1.0

// 'canvasHandler' is C++ CanvasHandler backend

ApplicationWindow {
    id: root
    minimumWidth: 1024
    minimumHeight: 700
    visible: true
    title: "QtVTK"

    Material.primary: Material.Indigo
    Material.accent: Material.LightBlue
    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action { text: qsTr("Open grid") ;
	    }
	    }
	    }
	    
    Rectangle {
        id: screenCanvasUI
        anchors.fill: parent

        VtkFboItem {
            id: vtkFboItem
            objectName: "vtkFboItem"
            anchors.fill: parent

            MouseArea {
                acceptedButtons: Qt.LeftButton
                anchors.fill: parent

                onPositionChanged: {
                    canvasHandler.mouseMoveEvent(pressedButtons, mouseX, mouseY);
                }
                onPressed: {
                    canvasHandler.mousePressEvent(pressedButtons, mouseX, mouseY);
                }
                onReleased: {
                    canvasHandler.mouseReleaseEvent(pressedButtons, mouseX, mouseY);
                }
            }
        }

        Button {
            id: openFileButton
            text: "Open file"
            highlighted: true
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 50
            onClicked: canvasHandler.showFileDialog = true;

            ToolTip.visible: hovered
            ToolTip.delay: 1000
            ToolTip.text: "Open a 3D model into the canvas"
        }

        ComboBox {
            id: representationCombobox
            visible: canvasHandler.isModelSelected
            width: 200
            model: ["Points", "Wireframe", "Surface"]
            currentIndex: 2
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: 40
            anchors.topMargin: 30

            onActivated: canvasHandler.setMdelsRepresentation(currentIndex);
        }

        Slider {
            id: opacitySlider
            visible: canvasHandler.isModelSelected
            width: 200
            value: 1
            from: 0.1
            to: 1
            stepSize: 0.01
            anchors.left: parent.left
            anchors.top: representationCombobox.bottom
            anchors.leftMargin: 40
            anchors.topMargin: 30

            onValueChanged: canvasHandler.setModelsOpacity(value);
        }

        Switch {
            id: gouraudInterpolationSwitch
            visible: canvasHandler.isModelSelected
            text: "Gouraud interpolation"
            anchors.left: parent.left
            anchors.top: opacitySlider.bottom
            anchors.leftMargin: 40
            anchors.topMargin: 30

            onCheckedChanged: canvasHandler.setGouraudInterpolation(checked);
        }

        SpinBox {
            id: modelColorR
            visible: canvasHandler.isModelSelected
            value: 3
            from: 0
            to: 255
            onValueChanged: canvasHandler.setModelColorR(value);
            anchors.left: parent.left
            anchors.top: gouraudInterpolationSwitch.bottom
            anchors.leftMargin: 40
            anchors.topMargin: 30
        }

        SpinBox {
            id: modelColorG
            visible: canvasHandler.isModelSelected
            value: 169
            from: 0
            to: 255
            onValueChanged: canvasHandler.setModelColorG(value);
            anchors.left: parent.left
            anchors.top: modelColorR.bottom
            anchors.leftMargin: 40
            anchors.topMargin: 25
        }

        SpinBox {
            id: modelColorB
            visible: canvasHandler.isModelSelected
            value: 244
            from: 0
            to: 255
            onValueChanged: canvasHandler.setModelColorB(value);
            anchors.left: parent.left
            anchors.top: modelColorG.bottom
            anchors.leftMargin: 40
            anchors.topMargin: 25
        }

        Label {
            id: positionLabelX
            visible: canvasHandler.isModelSelected
            text: "X: " + canvasHandler.modelPositionX
            font.pixelSize: 12
            anchors.bottom: positionLabelY.top
            anchors.left: parent.left
            anchors.margins: 40
        }

        Label {
            id: positionLabelY
            visible: canvasHandler.isModelSelected
            text: "Y: " + canvasHandler.modelPositionY
            font.pixelSize: 12
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.margins: 40
        }
    }

    FileDialog {
        id: openModelsFileDialog
        visible: canvasHandler.showFileDialog
        title: "Import model"
        folder: shortcuts.documents
        nameFilters: ["Model files" + "(*.stl *.STL *.obj *.OBJ *.grd *.GRD)", "All files" + "(*)"]

        onAccepted: {
            canvasHandler.showFileDialog = false;
            canvasHandler.openModel(fileUrl);
        }
        onRejected: {
            canvasHandler.showFileDialog = false;
        }
    }
}
