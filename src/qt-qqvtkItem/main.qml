// import related modules
import QtQuick 2.9
import QtQuick.Controls 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.14

import VTK 9.3

Window {
  width: 400
  height: 400

  MenuBar {
     id: menuBar
     
     Menu {
       title: 'File'
     
       Action {
         text: 'Exit'
       }
     }

     Menu {
       title: 'View'

       Action {
         text: 'Preferences'
       }
     }
  }

  ColumnLayout {

  anchors.top: menuBar.bottom
  
  // a rectangle in the middle of the content area
  Rectangle {
    width: 100
    height: 100
    color: "blue"
    border.color: "red"
    border.width: 5
    radius: 10
  }

  Button {
    text: 'Push me!'
  }
  
  MyVtkItem {
    objectName: "ConeView"
    x: 200
    y: 200
    width: 200
    height: 200
    focus: true
  }
  }
}
