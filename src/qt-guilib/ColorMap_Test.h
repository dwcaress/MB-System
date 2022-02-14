// MyTestSuite2.h
#include <cxxtest/TestSuite.h>
#include "ColorMap.h"

/**
Test suite for CXXTest
** */
class ColorMapTestSuite : public CxxTest::TestSuite
{
public:

  const int intValue_ = 123;
  
    void testBuild(void) {

      ColorMap colorMap;

      QVector4D rgbaColor;

      rgbaColor[0] = 1.0;
      rgbaColor[1] = 0.;  
      rgbaColor[2] = 0.0;
      rgbaColor[3] = 1.0;
      colorMap.addLevel(-1000., rgbaColor);
  
      rgbaColor[0] = 1.0;
      rgbaColor[1] = 0.;  
      rgbaColor[2] = 0.0;
      colorMap.addLevel(0., rgbaColor);
      
      TS_TRACE("Try to add invalid color");

      // TS_ASSERT(ret == false);  // Expect error return
      
    }
};
