import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Dialogs 1.1
import QtDataVisualization 1.14
import "ui-components"
/* ***
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
                Action {checkable: true; checked: true; text: qsTr("Haxby"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Bright rainbow"); ActionGroup.group: colorActions}
                Action {checkable: true; text: qsTr("Muted rainbow"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Grayscale"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Flat gray"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Sealevel1"); ActionGroup.group: colorActions }
                Action {checkable: true; text: qsTr("Sealevel2"); ActionGroup.group: colorActions }
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
            text: "filename goes here"
            anchors.top: parent.top
            anchors.topMargin: 0
            font.family: "Helvetica"
            font.pointSize: 24
            color: "red"
        }

        Item {
            width: 964
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 0
            anchors.top: selectedFile.bottom
            anchors.topMargin: -36
            objectName: "surface3DItem"

            Surface3D {
                anchors.fill: parent
                objectName: "surface3D"


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
                }

            }
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

        }
    }

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


}
