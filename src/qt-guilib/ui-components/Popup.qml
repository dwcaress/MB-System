import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0  

Window {
    id: mypopDialog
    title: "MyPopup"
    width: 300
    height: 100
    flags: Qt.Dialog
    modality: Qt.WindowModal
    property int popupType: 1
    property string returnValue: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20
            Loader {
                id: loader
                Layout.alignment: Qt.AlignRight
                Layout.fillWidth: true
                sourceComponent: popupType == 1 ? comboboxComponent : editboxComponent
                property string myvalue : popupType == 1 ? item.currentText : item.text
                Component {
                    id: comboboxComponent

                    ComboBox {
                        id: comboBox

                        model: ListModel {
                            ListElement { text: "Banana" }
                            ListElement { text: "Apple" }
                            ListElement { text: "Coconut" }
                        }
                    }
                }
                Component {
                    id: editboxComponent
                    TextEdit {
                        id: textEdit
                    }
                }
            }
        }

        Rectangle {
            height: 30
            Layout.fillWidth: true
            Button {
                text: "Ok"
                anchors.centerIn: parent
                onClicked: {
                    returnValue = loader.myvalue;
                    mypopDialog.close();
                }
            }
        }
    }
}
