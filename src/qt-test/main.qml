// DataList GUI
import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Window 2.15
// import ToDo 1.0

Window {
    visible: true
    title: 'Hi there'
    width: 600
    height: 600

    Frame {
        visible: true
        ListView {
            implicitWidth: 250
            implicitHeight: 250
            clip: true

            // model: ToDoModel {}

            delegate: RowLayout {
                width: parent.width
                CheckBox {
                    checked: model.done
                    onClicked: model.done = checked
                }
                TextField {
                    text: model.description
                    onEditingFinished: model.description = textTo
                    Layout.fillWidth: true
                }
            }

        }
    }

}

