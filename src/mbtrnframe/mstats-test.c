#include "mstats.h"
#include "mframe.h"

/// @fn void s_termination_handler (int signum)
/// @brief termination signal handler.
/// @param[in] signum signal number
/// @return none
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            fprintf(stderr,"\nsig received[%d]\n",signum);
            g_mstat_test_quit=true;
            break;
        default:
            fprintf(stderr,"\ns_termination_handler: sig not handled[%d]\n",signum);
            break;
    }
}
// End function termination_handler

int main(int argc, char **argv)
{
    int retval = -1;
    // configure signal handling
    // for main thread
    struct sigaction saStruct;
    
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);
    
#if defined(WITH_MSTATS_TEST)
    retval = mstats_test();
#else
    fprintf(stderr,"mstats_test not implemented - compile using -DWITH_MSTATS_TEST (WITH_MSTATS_TEST=1 make...)\n");
    fprintf(stderr,"i.e. WITH_MSTATS_TEST=1 make\n");

#endif
    return retval;
}
