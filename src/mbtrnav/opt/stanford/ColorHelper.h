#ifndef _COLOR_HELPER_H_
#define _COLOR_HELPER_H_

#include <math.h>


//Returns an rgb color in the range [0,1] based on linearly interpolating the hue between 0 and 300 based on the values  minIndex, maxIndex and index.
template <class T>
void computeRGB(float *rgb, const T &index, const T &minIndex, const T &maxIndex);

//Returns an rgb color in the range [0,1] based on linearly interpolating the hue between 0 and 300 based on the values  minIndex, maxIndex and index.
template <class T>
void computeRGB(float &red, float &green, float &blue, const T &index, const T &minIndex, const T &maxIndex);

//Converts HSV to RGB colorspace
void transformHSVtoRGB(float &red, float &green, float &blue, const float &hue, const float &saturation = 1, const float &value = 1);



#include "ColorHelper.hpp"


#endif
