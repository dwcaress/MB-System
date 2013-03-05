/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/* <<< Release Notice for library >>> */

#include <projects.h>

char const pj_release[]="Rel. 4.8.0, 6 March 2012";

const char *pj_get_release()

{
    return pj_release;
}
