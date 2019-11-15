#include <stdio.h>
#include <stdlib.h>
#include "medebug.h"


#if defined(__QNX__)
int medebug_test()
{
    fprintf(stderr,"%s not implemented for QNX\n",__FUNCTION__);
return -1;
}
#else

/// @fn int medebug_test()
/// @brief debug unit test. throws assertions on failure
/// @return 0 on success, -1 otherwise
int medebug_test()
{
#ifdef WITHOUT_MEDEBUG_REQUIRED
    fprintf(stderr,"\n\ncompiled with -DWITHOUT_MEDEBUG_REQUIRED\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITHOUT_MEDEBUG_REQUIRED\r\n");
#endif
    // required group variants
    // basic message types: msg, print, write
    MSG("wmsg");
    PRINT("wprint %d/1",1);
    WRITE("wwrite %d/2 ",1);
    WRITE("wwrite %d/2\n",2);
    // warnings
    WMSG("wmsg");
    WPRINT("wprint %d/1",1);
    WWRITE("wwrite %d/2 ",1);
    WWRITE("wwrite %d/2\n",2);
    WTWRITE("wtwrite %d/2 ",1);
    WTWRITE("wtwrite %d/2\n",2);
    WTPRINT("wtprint %d/1",1);
    // error
    EMSG("emsg");
    EPRINT("eprint %d/1",1);
    EWRITE("ewrite %d/2 ",1);
    EWRITE("ewrite %d/2\n",2);
    ETWRITE("etwrite %d/2",1);
    ETWRITE("etwrite %d/2\n",2);
    ETPRINT("etprint %d/1",1);
    
#ifdef WITH_MEDEBUG_OPTIONAL
    fprintf(stderr,"\n\ncompiled with -DWITH_MEDEBUG_OPTIONAL\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MEDEBUG_OPTIONAL\r\n");
#endif
    // optional group variants
    OMSG("omsg");
    OPRINT("oprint %d/1",1);
    OWRITE("owrite %d/2 ", 1 );
    OWRITE("owrite %d/2\n", 2 );
    OEMSG("oemsg");
    OEPRINT("oeprint %d/1", 1 );
    OEWRITE("oewrite %d/2 ", 1 );
    OEWRITE("oewrite %d/2\n", 2 );
    OETPRINT("oetprint %d/1",1 );
    OETWRITE("oetwrite %d/2 ", 1 );
    OETWRITE("oetwrite %d/2\n", 2 );
    OWMSG("owmsg");
    OWPRINT("owprint %d/1", 1 );
    OWWRITE("owwrite %d/2 ", 1 );
    OWWRITE("owwrite %d/2\n", 2 );
    OWTPRINT("owtprint %d/1", 1 );
    OWTWRITE("owtwrite %d/2 ", 1 );
    OWTWRITE("owtwrite %d/2\n", 2 );
    
#ifdef WITH_MEDEBUG_DEBUG
    fprintf(stderr,"\n\ncompiled with -DWITH_MEDEBUG_DEBUG\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_MEDEBUG_DEBUG\r\n");
#endif
    // debug group variants
    // trace
    TRACE();
    TMSG("tmsg");
    TPRINT("tprint %d/1",1);
    TWRITE("twrite %d/2 ",1);
    TWRITE("twrite %d/2\n",2);
    // debug
    DMSG("dmsg");
    DPRINT("dprint %d/1",1);
    DWRITE("dwrite %d/2 ",1);
    DWRITE("dwrite %d/2\n",2);
    
#ifdef WITH_PDEBUG
    fprintf(stderr,"\n\ncompiled with -DWITH_PDEBUG\r\n");
#else
    fprintf(stderr,"\n\ncompiled without -DWITH_PDEBUG\r\n");
#endif
	fprintf(stderr,"PTRACE:\r\n");
	PTRACE();
    PDPRINT((stderr,"PDPRINT\r\n"));
    PVPRINT((stderr,"PVPRINT\r\n"));
    PWPRINT((stderr,"PWPRINT\r\n"));
    PEPRINT((stderr,"PEPRINT\r\n"));
    return 0;
}
// End function mdb_test

#endif // not __QNX__


int main(int argc, char **argv)
{
int retval=-1;
#if defined(WITH_MEDEBUG_TEST)
    retval = medebug_test();
#else
    fprintf(stderr,"medebug_test not implemented - compile using -DWITH_MEDEBUG_TEST (WITH_MEDEBUG_TEST=1 make...)\r\n");
 #endif

    return retval;
}
