#include "ColorHelper.h"

//*******************************************************************

void transformHSVtoRGB(float& red, float& green, float& blue, const float& hue, const float& saturation, const float& value) {
	float c = value * saturation;
	float hue_p = hue / 60.0;
	float x = c * (1 - fabs(fmod(hue_p, 2.0) - 1));
	
	if(hue_p >= 0 && hue_p < 1) {
		red = c;
		green = x;
		blue = 0.0;
	} else if(hue_p >= 1 && hue_p < 2) {
		red = x;
		green = c;
		blue = 0.0;
	} else if(hue_p >= 2 && hue_p < 3) {
		red = 0.0;
		green = c;
		blue = x;
	} else if(hue_p >= 3 && hue_p < 4) {
		red = 0.0;
		green = x;
		blue = c;
	} else if(hue_p >= 4 && hue_p < 5) {
		red = x;
		green = 0.0;
		blue = c;
	} else if(hue_p >= 5 && hue_p <= 6) {
		red = c;
		green = 0.0;
		blue = x;
	}
	
	float m = value - c;
	red += m;
	green += m;
	blue += m;
}

//*******************************************************************
