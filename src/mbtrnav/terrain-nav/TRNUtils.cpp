
#include <string.h>
#include "TRNUtils.h"

char*
TRNUtils::basename(char* pathname)
{
  // Special conditions
  //
  if (NULL == pathname) return pathname;
  if (0 == strlen(pathname)) return pathname;

  // First remove trailing '/' chars
  //
  while ('/' == pathname[strlen(pathname)-1])
    pathname[strlen(pathname)-1] = '\0';

  // Return basename
  //
  char *p = strrchr(pathname, '/');
  return p ? p+1 : pathname;
}
