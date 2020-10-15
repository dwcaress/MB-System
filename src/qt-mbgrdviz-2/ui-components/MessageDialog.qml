import QtQuick 2.0
import QtQuick.Dialogs 1.2

MessageDialog {
  id: myMessageDialog
  objectName: "myMessageDialog"
  title: "My message"
  text: "Fill in this message from C++"
  onAccepted: {
      console.log("Knew you'd see it my way!")
      // Hide the dialog
      visible = false
  }


  Component.onCompleted: visible = true
}
