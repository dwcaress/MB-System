/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/* For full ANSI compliance of global variable */

#include <projects.h>

C_NAMESPACE_VAR int pj_errno = 0;

/************************************************************************/
/*                          pj_get_errno_ref()                          */
/************************************************************************/

int *pj_get_errno_ref()

{
    return &pj_errno;
}

/* end */
