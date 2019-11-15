#include <trn_log.h>

int main(int argc, char **argv)
{
    
    const char *logdir="./";
    // this message should be buffered
    // and written after the log file is created
    logs((TL_LOG),"starting test (msg should be buffered)\n");
    logs((TL_LOG),"creating file in [%s]  (msg should be buffered)\n",logdir);
    logs((TL_LOG),"this is a buffered message\n");
    tl_new_logfile(logdir);
    logs((TL_LOG),"this should be written only to the log, appearing after the buffered message(s)\n");
    
    logs((TL_BOTH),"this message should be sent to stderr and the log\n");
    logs((TL_SERR),"this message should only go to stderr\n");
	
    // configure this module default stderr enabled, none disabled
    tl_mconfig(TL_TEST_TRN_LOG,TL_SERR,TL_NONE);
    // log to module config defaults
    logs(TL_OMASK(TL_TEST_TRN_LOG, TL_NONE),"write to module defaults");
    // this uses log + module enabled - module disabled
    logs(TL_OMASK(TL_TEST_TRN_LOG, TL_LOG),"write to log + module enabled - module disabled");
    // disable log output
    tl_mconfig(TL_TEST_TRN_LOG,TL_NC,TL_LOG);
    // should log only to module enabled, since module disable
    logs(TL_OMASK(TL_TEST_TRN_LOG, TL_LOG),"write to log + module enabled - module disabled (log disabled)");
    
    fprintf(stderr,"disabling all output, sending 'if you see this, something's broken'\n");
    // disable log output
    tl_mconfig(TL_TEST_TRN_LOG,TL_NC,TL_ALL);
    // send message (should never see this)
    logs(TL_OMASK(TL_TEST_TRN_LOG, TL_LOG),"if you see this, something's broken");

    logs((TL_BOTH),"ending test\n");
    return 0;
}