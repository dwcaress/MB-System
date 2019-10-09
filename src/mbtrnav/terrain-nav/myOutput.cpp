/* File: myOutput.cpp
 * -------------------
 * Written by: Debbie Meduna
 *****************************************************************************/
#include "myOutput.h"

void output(const char* format, ...) {
	//get the arguments
	va_list args;
	va_start(args, format);
	
#ifdef USE_SYSLOG
	//print it to the sys log...
	
	char temp[1024]; //temporary variable
	
	//converts the format and the variable argument list into
	//the formatted output string.  It caps it at the max size of
	//the temporary variable as well.
	_vbprintf(temp, 1024, format, args);
	Syslog::write(temp);
	
#else
	
	//print it to the screen
	vprintf(format, args);
#endif
	
	//end the argument list
	va_end(args);
}
