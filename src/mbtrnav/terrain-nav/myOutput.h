/* File: myOutput.h
 * -------------------
 * myOutput defines a function for outputting formatted strings to the screen.
 * Allows for the use of Syslog when enabled. 
 *
 * Written by: Debbie Meduna
 ******************************************************************************/

#ifndef _MYOUTPUT_H
#define _MYOUTPUT_H

#include <stdio.h>
#include <stdarg.h>

//If you want to use SysLog for outputting, uncomment the following line.
//If the following line is commented, output commands will display to 
//the terminal window only

//#define USE_SYSLOG 

#ifdef USE_SYSLOG
#include "Syslog.h"
#endif

// variable argument output function
/*! wrapper around syslog::write and outputting to the screen
 *  depending on the flag USE_SYSLOG
 */
void output(const char* format, ...);

#endif
