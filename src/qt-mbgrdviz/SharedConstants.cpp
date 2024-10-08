#include <iostream>
#include "SharedConstants.h"
#include "TopoColorMap.h"

using namespace mb_system;
using namespace sharedQmlCpp;

const QString Const::testString_ = QString("Hello sailor!");

Const::Const() {
  
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
}
