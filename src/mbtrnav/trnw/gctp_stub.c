#include <stdio.h>
#include "gctp_stub.h"

#pragma message("WARN - Compiling using gctp stub (GCTP support disabled)")

void gctp(double *incoor, long *insys, long *inzone, double *inparm,
        long *inunit, long *inspheroid, long *ipr, char *efile, long *jpr,
        char *pfile, double *outcoor, long *outsys, long *outzone,
        double *outparm, long *outunit, long *outspheroid, char fn27[],
        char fn83[], long *iflg)
{
    fprintf(stderr, "%s:%d - ERR not implemented; GCTP support disabled, compile using -DGEO_GCTP_EN\n", __func__, __LINE__);
    return;
}

