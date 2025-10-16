#include <iostream>
#include "SharedConstants.h"
#include "TopoColorMap.h"

using namespace mb_system;

const QString SharedConstants::testString_ = QString("Hello sailor!");

SharedConstants::SharedConstants() {
  
    // Load supported colormap names (defined in TopoColorMap class)
    // into vector
    std::vector<const char *> colorMapNames;
    TopoColorMap::schemeNames(&colorMapNames);

    std::cout << "ColorMaps:\n";
    // Copy colormap names to QStringList (for retrieval by QML)
    for (int i = 0; i < colorMapNames.size(); i++) {
      std::cout << colorMapNames[i] << "\n";
      // Append name to QStringList
      colorMapsList_.append(colorMapNames[i]);
    }

    // Assemble data model of supported mouse modes
    mouseModes_ += 
      new MouseMode(MousePanAndZoom, "L-drag: rotate, " \
		    "M-drag: pan, R-drag: zoom, " \
		    "wheel: zoom");
      
    mouseModes_ += new MouseMode(MouseLighting,
				 "shift-L-drag: change light pos, "  \
				 "shift-R-drag: change intensity "   \
				 "(+ basic pan and zoom)");
    mouseModes_ += new MouseMode(MouseDataSelect,
				 "'r': toggle select mode,  "	\
				 "R-drag: select data "		\
				 "(+ basic pan and zoom)");

    mouseModes_ += new MouseMode(MouseEditSites, "Tooltip goes here");
    mouseModes_ += new MouseMode(MouseEditRoutes, "Tooltip goes here");
    mouseModes_ += new MouseMode(MousePickNav, "Tooltip goes here");
    mouseModes_ += new MouseMode(MousePickNavFile, "Tooltip goes here");    
    mouseModes_ += new MouseMode(MouseTest, "TESTING");
}
