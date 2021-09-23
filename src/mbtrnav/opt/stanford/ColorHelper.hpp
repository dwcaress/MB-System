
//*******************************************************************
template <class T>
void computeRGB(float* rgb, const T& index, const T& minIndex, const T& maxIndex) {
	computeRGB(rgb[0], rgb[1], rgb[2], index, minIndex, maxIndex);
}

//*******************************************************************

template <class T>
void computeRGB(float& red, float& green, float& blue, const T& index, const T& minIndex, const T& maxIndex) {
	float diff = float(maxIndex - minIndex);
	//the hue should range from 0 to 300 to get colors from red to purple.  The hue goes from 0 (red) to 360 (red) so we don't want it to wrap around.
	float hue = float(index - minIndex) / diff * 240.0;
	transformHSVtoRGB(red, green, blue, hue);
}

//*******************************************************************
