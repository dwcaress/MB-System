#include <stdio.h>
#include "ColorMap.h"


int main(int argc, char **argv) {

  ColorMap colorMap;

  QVector4D rgbaColor;

  rgbaColor[0] = 1.0;
  rgbaColor[1] = 0.;  
  rgbaColor[2] = 0.0;
  rgbaColor[3] = 1.0;
  fprintf(stderr, "add level 1\n");
  colorMap.addLevel(-1000., rgbaColor);

  rgbaColor[0] = 1.0;
  rgbaColor[1] = 0.;  
  rgbaColor[2] = 0.0;
  fprintf(stderr, "add level 2\n");  
  colorMap.addLevel(0., rgbaColor);

  
  fprintf(stderr, "try to add duplicate value\n");
  colorMap.addLevel(-1000., rgbaColor);  

  rgbaColor[0] = 1.0;
  rgbaColor[1] = 0.;  
  rgbaColor[2] = 1.1;
  fprintf(stderr, "try to add invalid color\n");
  colorMap.addLevel(-2000., rgbaColor);  
  
  return 0;
}
