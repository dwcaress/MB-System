#ifndef __unistd_h__
#define __unistd_h__ 1

#include <direct.h>
#include <process.h>
#include <io.h>

#ifndef PATH_MAX
# define PATH_MAX 1024
#endif

#ifdef _WIN32
#	ifndef R_OK
#		define R_OK 04
#		define W_OK 02
#		define X_OK 01
#		define F_OK 00
#	endif
#endif

#endif
