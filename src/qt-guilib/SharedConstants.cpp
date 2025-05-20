#include <iostream>
#include "SharedConstants.h"
#include "TopoColorMap.h"

using namespace mb_system;

const QString SharedConstants::testString_ = QString("Hello sailor!");

SharedConstants::SharedConstants() {
  
    // Get colormap names to be displayed by QML GUI
    std::vector<const char *> colorMapNames;

    // Load colormap names into vector
    TopoColorMap::schemeNames(&colorMapNames);

    std::cout << "ColorMaps:\n";
    // Copy colormap names to QStringList
    for (int i = 0; i < colorMapNames.size(); i++) {
      std::cout << colorMapNames[i] << "\n";
      // Append name to QStringList
      colorMapsList_.append(colorMapNames[i]);
    }

    mouseModes_ += "Pan and zoom";
    mouseModes_ += "Rotate model";
    mouseModes_ += "Rotate view";
    mouseModes_ += "Shading";
    mouseModes_ += "Pick area";
    mouseModes_ += "Edit sites";
    mouseModes_ += "Edit routes";
    mouseModes_ += "Pick nav";
    mouseModes_ += "Pick nav file";    
      
}
