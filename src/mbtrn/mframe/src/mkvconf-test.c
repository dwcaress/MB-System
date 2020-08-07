#include "mkvconf.h"
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
#if defined(WITH_MKVCONF_TEST)
            g_mkvconf_test_quit=true;
#endif
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
    
#if defined(WITH_MKVCONF_TEST)
    retval = mkvconf_test();
#else
    fprintf(stderr,"mkvconf_test not implemented - compile using -DWITH_MKVCONF_TEST (WITH_MKVCONF_TEST=1 make...)\n");
    fprintf(stderr,"i.e. WITH_MKVCONF_TEST=1 make\n");
#endif
    return retval;
}
