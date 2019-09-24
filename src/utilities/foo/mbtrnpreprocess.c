/*--------------------------------------------------------------------
 *    The MB-system:  mbtrnpreprocess.c  2/19/2018
 *
 *    Copyright (c) 2018-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbtrnpreprocess
 *
 * Author:  D. W. Caress
 * Date:  February 18, 1995
 */

#if defined(__CYGWIN__)
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <float.h>
#include <arpa/inet.h>

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mbsys_ldeoih.h"

#include <string.h>
#include <poll.h>
#include "mconfig.h"
#include "mbtrn.h"
#include "r7kc.h"
/*#include "iowrap.h" */
#include "msocket.h"
#include "mtime.h"
#include "mlist.h"
#include "mlog.h"
#include "mconfig.h"
#include "medebug.h"

/* ping structure definition */
struct mbtrnpreprocess_ping_struct
  {
  int count;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sonardepth;
  double roll;
  double pitch;
  double heave;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  char *beamflag;
  char *beamflag_filter;
  double *bath;
  double *bathacrosstrack;
  double *bathalongtrack;
  double *amp;
  double *ss;
  double *ssacrosstrack;
  double *ssalongtrack;
  };

/* buffer size default */
#define MBTRNPREPROCESS_BUFFER_DEFAULT 20
#define MBTRNPREPROCESS_OUTPUT_STDOUT   0
#define MBTRNPREPROCESS_OUTPUT_TRN      1
#define MBTRNPREPROCESS_OUTPUT_FILE     2

#define MBTRNPREPROCESS_MB1_HEADER_SIZE 56  /*52 */
#define MBTRNPREPROCESS_MB1_SOUNDING_SIZE 28
#define MBTRNPREPROCESS_MB1_CHECKSUM_SIZE 4

#define MBTRNPREPROCESS_LOGFILE_TIMELENGTH 900.0

static char program_name[] = "mbtrnpreprocess";

/* mbtrn_reader test configuration */
#define RESON_SIM_HOST "localhost"
#define RESON_READER_CAPACITY_DFL (256*1024)
uint32_t reson_nsubs=11;
uint32_t reson_subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015, 1016, 7000, 7004, 7027};

mbtrn_reader_t *reson_reader;
char *reson_hostname=NULL;
int reson_port=R7K_7KCENTER_PORT;
uint32_t reader_capacity=RESON_READER_CAPACITY_DFL;

/* TRN socket output configuration */
#define TRN_HOST_DFL "localhost"
#define TRN_PORT_DFL 9999
#define TRN_MSG_CON_LEN 4
#define TRN_MAX_PEER 15
#define TRN_NPEERS (TRN_MAX_PEER+1)
#define TRN_HBTOK_DFL 50
#define MBTPP 1
typedef enum {INPUT_MODE_SOCKET=1, INPUT_MODE_FILE=2}input_mode_t;
input_mode_t input_mode;

msock_connection_t *trn_peer = NULL;
mlist_t *trn_plist = NULL;
msock_socket_t *trn_osocket = NULL;
int trn_oport=TRN_PORT_DFL;
char *trn_ohost=TRN_HOST_DFL;
int trn_hbtok = TRN_HBTOK_DFL;

#define TRN_BLOG_NAME "tbin"
#define TRN_BLOG_DESC "trn binary data"
#define TRN_MLOG_NAME "tmsg"
#define TRN_MLOG_DESC "trn message log"
#define MBR_BLOG_NAME "mbin"
#define MBR_BLOG_DESC "reader frame log"
#define TRN_LOG_EXT ".log"
#define SZ_1M (1024*1024)
#define SZ_1G (1024*1024*1024)
#define TRN_CMD_LINE_BYTES 2048
#define TRN_STATUS_INTERVAL_SEC_DFL 30

mlog_id_t trn_blog_id=MLOG_ID_INVALID;
mlog_id_t trn_mlog_id=MLOG_ID_INVALID;
mlog_id_t mbr_blog_id=MLOG_ID_INVALID;

/*mlog_id_t BLOG_ID=0x1;
   mlog_id_t MLOG_ID=0x2;
   mlog_id_t RLOG_ID=0x3;*/

mlog_config_t blog_conf =
  {
  100*SZ_1M, ML_NOLIMIT, ML_NOLIMIT, ML_OSEG|ML_LIMLEN, ML_FILE, ML_TFMT_ISO1806
  };

mlog_config_t mlog_conf =
  {
  ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806
  };

mlog_config_t mbrlog_conf =
  {
  ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806
  };

char session_date[32]={0};

char *trn_blog_path=NULL;
char *trn_mlog_path=NULL;

/*mlog_t *trn_blog=NULL;
   mlog_t *trn_mlog=NULL;*/

mfile_flags_t flags = MFILE_RDWR|MFILE_APPEND|MFILE_CREATE;
mfile_mode_t mode = MFILE_RU|MFILE_WU|MFILE_RG|MFILE_WG;
bool trn_blog_en = true;
bool trn_mlog_en = true;
char g_cmd_line[TRN_CMD_LINE_BYTES]={0};
char *g_log_dir=NULL;

char *mbr_blog_path=NULL;
bool mbr_blog_en=true;
/*mlog_t *mbr_blog=NULL; */
FILE *mbr_blogs=NULL;

typedef enum
  {
  MBTPP_EV_CYCLES=0, MBTPP_EV_EMBGETALL, MBTPP_EV_EMBFAILURE, MBTPP_EV_ESRC_SOCKET,
  MBTPP_EV_ESRC_CON, MBTPP_EV_ECLI_RXZ, MBTPP_EV_ECLI_RXE, MBTPP_EV_ECLI_TXZ, MBTPP_EV_ECLI_TXE,
  MBTPP_EV_ECLI_ACK, MBTPP_EV_ETRN_TX, MBTPP_EV_ECLIADDR_RX, MBTPP_EV_ENTOP, MBTPP_EV_SRC_CONN,
  MBTPP_EV_SRC_DISN, MBTPP_EV_CLI_CONN, MBTPP_EV_CLI_DISN, MBTPP_EV_CLI_RXN, MBTPP_EV_CLI_TXN,
  MBTPP_EV_CLI_ACKN, MBTPP_EV_TRN_PUBN, MBTPP_EV_LOG_STATN, MBTPP_EV_COUNT
  }mbtrnpp_stevent_id;

typedef enum
  {
  MBTPP_STA_CLI_LIST_LEN, MBTPP_STA_CLI_ACK_BYTES, MBTPP_STA_CLI_RX_BYTES, MBTPP_STA_TRN_TX_BYTES,
  MBTPP_STA_TRN_PUB_BYTES, MBTPP_STA_COUNT
  }mbtrnpp_ststatus_id;

typedef enum
  {
  MBTPP_CH_MBGETALL_XT=0, MBTPP_CH_MBPING_XT, MBTPP_CH_TRNRX_XT, MBTPP_CH_TRNTX_XT, MBTPP_CH_LOG_XT,
  MBTPP_CH_DTIME_XT, MBTPP_CH_MBGETFAIL_XT, MBTPP_CH_MBPOST_XT, MBTPP_CH_STATS_XT,
  MBTPP_CH_CYCLE_XT, MBTPP_CH_THRUPUT, MBTPP_CH_COUNT
  }mbtrnpp_stchan_id;

const char *mbtrnpp_stevent_labels[]=
  { \
  "cycles", "e_mbgetall", "e_mbfailure", "e_src_socket", "e_src_con", "e_cli_rx_z", "e_cli_rx_e",
  "e_cli_tx_z", "e_cli_tx_e", "e_cli_ack", "e_trn_tx", "e_cliaddr_rx", "e_ntop", "src_con",
  "src_dis", "cli_con", "cli_dis", "cli_rx", "cli_tx", "cli_ack", "trn_pub_n", "log_stat"
  };
const char *mbtrnpp_ststatus_labels[]=
  { \
  "cli_list_len", "cli_ack_bytes", "cli_rx_bytes", "trn_tx_bytes", "trn_pub_bytes"
  };

const char *mbtrnpp_stchan_labels[]=
  { \
  "mbgetall_xt", "mbping_xt", "trnrx_xt", "trntx_xt", "log_xt", "dtime_xt", "mbgetfail_xt",
  "mbpost_xt", "stats_xt", "cycle_xt", "thruput"
  };

const char **mbtrnpp_stats_labels[MBTR_LABEL_COUNT]=
  {
  mbtrnpp_stevent_labels, mbtrnpp_ststatus_labels, mbtrnpp_stchan_labels
  };

typedef struct mbtrnpp_stats_s
  {
  double session_start;
  double uptime;
  mbtr_stats_t *stats;
  }mbtrnpp_stats_t;

mbtrnpp_stats_t *app_stats=NULL;
mbtr_stats_t *reader_stats=NULL;
double trn_status_interval_sec = MBTRNPP_STAT_PERIOD_SEC;
static double stats_prev_end=0.0;
static double stats_prev_start=0.0;
static bool log_clock_res=true;

#define MBTRNPP_MEAS_MOD ((double)0.0)  /*3600.0 */

#ifdef MBTR_STATS_EN
#define MBTRNPP_UPDATE_STATS(p, l, f) (mbtrnpreprocess_update_stats(p, l, f))
#else
#define MBTRNPP_UPDATE_STATS(p, l, f)
#endif  /*MBTR_STATS_EN */

/* MBTRN_STAT_FLAGS define stats processing options
   may include
   MBTF_STATUS : status counters
   MBTF_EVENT  : event/error counters
   MBTF_ASTAT  : aggregate stats
   MBTF_PSTAT  : periodic stats
   MBTF_READER : mbtrn reader stats*/
#define MBTRNPP_STAT_FLAGS (MBTF_STATUS|MBTF_EVENT|MBTF_ASTAT)

/*--------------------------------------------------------------------*/

int mbtrnpreprocess_openlog
(
  int verbose,
  mb_path log_directory,
  FILE **logfp,
  int *error
)
{
  int status = MB_SUCCESS;

  /* time, user, host variables */
  struct timeval timeofday;
  struct timezone timezone;
  double time_d;
  int time_i[7];
  char date[32], user[128], *user_ptr;
  mb_path host;
  mb_path log_file;
  mb_path log_message;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       log_directory:      %s\n", log_directory);
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
    }

  /* close existing log file */
  if (*logfp != NULL)
    mbtrnpreprocess_closelog(verbose, logfp, error);

  /* get time and user data */
  gettimeofday(&timeofday, &timezone);
  time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  status = mb_get_date(verbose, time_d, time_i);
  sprintf(date,
    "%4.4d%2.2d%2.2d_%2.2d%2.2d%2.2d%6.6d",
    time_i[0],
    time_i[1],
    time_i[2],
    time_i[3],
    time_i[4],
    time_i[5],
    time_i[6]);
  if ((user_ptr = getenv("USER")) == NULL)
    user_ptr = getenv("LOGNAME");
  if (user_ptr != NULL)
    strcpy(user, user_ptr);
  else
    strcpy(user, "unknown");
  gethostname(host, sizeof(mb_path));

  /* open new log file */
  sprintf(log_file, "%s/%s_mbtrnpreprocess_log.txt", log_directory,   date);
  *logfp = fopen(log_file, "w");
  if (*logfp != NULL)
    {
    fprintf(*logfp, "Program %s log file\n-------------------\n", program_name);
    if (verbose > 0)
      fprintf(stderr, "Program %s log file\n-------------------\n", program_name);
    sprintf(log_message, "Opened by user %s on cpu %s", user, host);
    mbtrnpreprocess_postlog(verbose, *logfp, log_message, error);
    }
  else
    {
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, "\nUnable to open %s log file: %s\n", program_name, log_file);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_openlog */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_closelog
(
  int verbose,
  FILE **logfp,
  int *error
)
{
  int status = MB_SUCCESS;
  char *log_message = "Closing mbtrnpreprocess log file";

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
    }

  /* close log file */
  if (logfp != NULL)
    {
    mbtrnpreprocess_postlog(verbose, *logfp, log_message, error);
    fclose(*logfp);
    *logfp = NULL;
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_closelog */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_postlog
(
  int verbose,
  FILE *logfp,
  char *log_message,
  int *error
)
{
  int status = MB_SUCCESS;

  /* time, user, host variables */
  struct timeval timeofday;
  struct timezone timezone;
  double time_d;
  int time_i[7];
  char date[32];

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       log_message:        %s\n", log_message);
    }

  /* get time  */
  gettimeofday(&timeofday, &timezone);
  time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  status = mb_get_date(verbose, time_d, time_i);
  sprintf(date,
    "%4.4d%2.2d%2.2d_%2.2d%2.2d%2.2d%6.6d",
    time_i[0],
    time_i[1],
    time_i[2],
    time_i[3],
    time_i[4],
    time_i[5],
    time_i[6]);

  /* post log_message */
  if (logfp != NULL)
    fprintf(logfp,
      "<%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d>: %s\n",
      time_i[0],
      time_i[1],
      time_i[2],
      time_i[3],
      time_i[4],
      time_i[5],
      time_i[6],
      log_message);
  if (verbose > 0)
    fprintf(stderr,
      "<%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d>: %s\n",
      time_i[0],
      time_i[1],
      time_i[2],
      time_i[3],
      time_i[4],
      time_i[5],
      time_i[6],
      log_message);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_postlog */
/*--------------------------------------------------------------------*/
int mbtrnpreprocess_logparameters
(
  int verbose,
  FILE *logfp,
  char *input,
  int format,
  char *output,
  double swath_width,
  int n_output_soundings,
  int median_filter,
  int median_filter_n_across,
  int median_filter_n_along,
  double median_filter_threshold,
  int n_buffer_max,
  int *error
)
{
  int status = MB_SUCCESS;
  mb_path log_message;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
    fprintf(stderr, "dbg2       logfp:                        %p\n", logfp);
    fprintf(stderr, "dbg2       input:                        %s\n", input);
    fprintf(stderr, "dbg2       format:                       %d\n", format);
    fprintf(stderr, "dbg2       output:                       %s\n", output);
    fprintf(stderr, "dbg2       swath_width:                  %f\n", swath_width);
    fprintf(stderr, "dbg2       n_output_soundings:           %d\n", n_output_soundings);
    fprintf(stderr, "dbg2       median_filter:                %d\n", median_filter);
    fprintf(stderr, "dbg2       median_filter_n_across:       %d\n", median_filter_n_across);
    fprintf(stderr, "dbg2       median_filter_n_along:        %d\n", median_filter_n_along);
    fprintf(stderr, "dbg2       median_filter_threshold:      %f\n", median_filter_threshold);
    fprintf(stderr, "dbg2       n_buffer_max:                 %d\n", n_buffer_max);
    }

  /* post log_message */
  if (logfp != NULL)
    {
    sprintf(log_message, "       input:                    %s", input);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       format:                   %d", format);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       output:                   %s", output);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       swath_width:              %f", swath_width);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_output_soundings:       %d", n_output_soundings);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter:            %d", median_filter);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_n_across:   %d", median_filter_n_across);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_n_along:    %d", median_filter_n_along);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_threshold:  %f", median_filter_threshold);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_buffer_max:             %d", n_buffer_max);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_logparameters */
/*--------------------------------------------------------------------*/
int mbtrnpreprocess_logstatistics
(
  int verbose,
  FILE *logfp,
  int n_pings_read,
  int n_soundings_read,
  int n_soundings_valid_read,
  int n_soundings_flagged_read,
  int n_soundings_null_read,
  int n_soundings_trimmed,
  int n_soundings_decimated,
  int n_soundings_flagged,
  int n_soundings_written,
  int *error
)
{
  int status = MB_SUCCESS;
  mb_path log_message;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
    fprintf(stderr, "dbg2       logfp:                        %p\n", logfp);
    fprintf(stderr, "dbg2       n_pings_read:                 %d\n", n_pings_read);
    fprintf(stderr, "dbg2       n_soundings_read:             %d\n", n_soundings_read);
    fprintf(stderr, "dbg2       n_soundings_valid_read:       %d\n", n_soundings_valid_read);
    fprintf(stderr, "dbg2       n_soundings_flagged_read:     %d\n", n_soundings_flagged_read);
    fprintf(stderr, "dbg2       n_soundings_null_read:        %d\n", n_soundings_null_read);
    fprintf(stderr, "dbg2       n_soundings_trimmed:          %d\n", n_pings_read);
    fprintf(stderr, "dbg2       n_soundings_decimated:        %d\n", n_soundings_decimated);
    fprintf(stderr, "dbg2       n_soundings_flagged:          %d\n", n_soundings_flagged);
    fprintf(stderr, "dbg2       n_soundings_written:          %d\n", n_soundings_written);
    }

  /* post log_message */
  if (logfp != NULL)
    {
    sprintf(log_message, "Log File Statistics:");
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_pings_read:                 %d", n_pings_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_read:             %d", n_soundings_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_valid_read:       %d", n_soundings_valid_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_flagged_read:     %d", n_soundings_flagged_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_null_read:        %d", n_soundings_null_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_trimmed:          %d", n_pings_read);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_decimated:        %d", n_soundings_decimated);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_flagged:          %d", n_soundings_flagged);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_written:          %d", n_soundings_written);
    mbtrnpreprocess_postlog(verbose, logfp, log_message, error);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_logstatistics */
/*--------------------------------------------------------------------*/



int mbtrnpreprocess_log_min
(
  mbtrn_stat_chan_t *stats
)
{
  if (NULL!=stats)
    mlog_tprintf(trn_mlog_id,
      "ut[%lu] src[%d/%d] cli[%d/%d/%d] tx[%d/%d] rx[%d/%d]\n",
      app_stats->uptime,
      MBTR_COUNTER_GET(app_stats->stats->events[MBTPP_EV_SRC_CONN]),
      MBTR_COUNTER_GET(app_stats->stats->events[MBTPP_EV_SRC_DISN]),
      MBTR_COUNTER_GET(app_stats->stats->status[MBTPP_STA_CLI_LIST_LEN]),
      MBTR_COUNTER_GET(app_stats->stats->events[MBTPP_EV_CLI_CONN]),
      MBTR_COUNTER_GET(app_stats->stats->status[MBTPP_STA_CLI_LIST_LEN]),
      MBTR_COUNTER_GET(app_stats->stats->events[MBTPP_EV_TRN_PUBN]),
      MBTR_COUNTER_GET(app_stats->stats->status[MBTPP_STA_TRN_TX_BYTES]),
      MBTR_COUNTER_GET(app_stats->stats->events[MBTPP_EV_CLI_RXN]),
      MBTR_COUNTER_GET(app_stats->stats->status[MBTPP_STA_CLI_RX_BYTES]));

  return 0;
}
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_update_stats
(
  mbtrnpp_stats_t *stats,
  mlog_id_t log_id,
  mbtr_stat_flags flags
)
{
  if (NULL!=stats)
    {
    double stats_now = mtime_dtime();

    if (log_clock_res)
      {
      /* log the timing clock resolution (once) */
      struct timespec res;
      clock_getres(CLOCK_MONOTONIC, &res);
      mlog_tprintf(trn_mlog_id,
        "%.3lf,i,clkres_mono,s[%ld] ns[%ld]\n",
        stats_now,
        res.tv_sec,
        res.tv_nsec);
      log_clock_res=false;
      }

    /* we can only measure the previous stats cycle... */
    if (stats->stats->per_stats[MBTPP_CH_CYCLE_XT].n>0)
      {
      /* get the timing of the last cycle */
      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_STATS_XT], stats_prev_start);
      MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_STATS_XT], stats_prev_end);
      }
    else
      {
      /* seed the first cycle */
      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_STATS_XT], (stats_now-0.0001));
      MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_STATS_XT], stats_now);
      }

    /* end the cycle timer here
       [start at the end if this function]*/
    MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_CYCLE_XT], stats_now);

    /* measure dtime execution time (twice), while we're at it */
    MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_DTIME_XT], mtime_dtime());
    MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_DTIME_XT], mtime_dtime());
    MBTR_SW_DIV(app_stats->stats->measurements[MBTPP_CH_DTIME_XT], 2.0);

    /* update uptime */
    stats->uptime = stats_now-stats->session_start;

    /* update throughput measurement */
    stats->stats->measurements[MBTPP_CH_THRUPUT].value =
      ( stats->uptime>
      0.0 ? (double)stats->stats->status[MBTPP_STA_TRN_TX_BYTES]/stats->uptime : 0.0);

    fprintf(stderr,
      "cycle_xt: stat_now[%.4lf] start[%.4lf] stop[%.4lf] value[%.4lf]\n",
      stats_now,
      app_stats->stats->measurements[MBTPP_CH_CYCLE_XT].start,
      app_stats->stats->measurements[MBTPP_CH_CYCLE_XT].stop,
      app_stats->stats->measurements[MBTPP_CH_CYCLE_XT].value);
    /* update stats */
    mbtrn_update_stats(stats->stats, MBTPP_CH_COUNT, flags);

    fprintf(stderr,
      "cycle_xt.p: N[%lld] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
      app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].n,
      app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].sum,
      app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].min,
      app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].max,
      app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].avg);

    fprintf(stderr,
      "cycle_xt.a: N[%lld] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
      app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].n,
      app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].sum,
      app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].min,
      app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].max,
      app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].avg);

    if (flags&MBTF_READER)
      mbtrn_update_stats(reader_stats, MBTR_CH_COUNT, flags);

/*        fprintf(stderr,"stat period sec[%.3lf] start[%.3lf] now[%.3lf] elapsed[%.3lf]\n",
                  stats->stats->stat_period_sec,
                  stats->stats->stat_period_start,
                  stats_now,
                  (stats_now - stats->stats->stat_period_start)
                  );*/
    /* check stats periods, process if ready */
    if ( (stats->stats->stat_period_sec>0.0) &&
      ((stats_now - stats->stats->stat_period_start) >  stats->stats->stat_period_sec))
      {
      /* start log execution timer */
      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_LOG_XT], mtime_dtime());

      mlog_tprintf(trn_mlog_id, "%.3lf,i,uptime,%0.3lf\n", stats_now, stats->uptime);
      mbtrn_log_stats(stats->stats, stats_now, log_id, flags);

      if (flags&MBTF_READER)
        mbtrn_log_stats(reader_stats, stats_now, log_id, flags);

      /* reset period stats */
      mbtrn_reset_pstats(stats->stats, MBTPP_CH_COUNT);
      mbtrn_reset_pstats(reader_stats, MBTR_CH_COUNT);

      /* reset period timer */
      stats->stats->stat_period_start = stats_now;

      /* stop log execution timer */
      MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_LOG_XT], mtime_dtime());
      }

    /* start cycle timer */
    MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_CYCLE_XT], mtime_dtime());

    /* update stats execution time variables */
    stats_prev_start=stats_now;
    stats_prev_end=mtime_dtime();
    }
  else
    {
    fprintf(stderr, "mbtrnpreprocess_update_stats: invalid argument\n");
    }

  return 0;
} /* mbtrnpreprocess_update_stats */
/*--------------------------------------------------------------------*/

mbtrnpp_stats_t *mbtrnpreprocess_stats_new
(
  uint32_t ev_counters,
  uint32_t status_counters,
  uint32_t tm_channels,
  double pstart,
  double psec
)
{
  mbtrnpp_stats_t *self =(mbtrnpp_stats_t *)malloc(sizeof(mbtrnpp_stats_t));
  if (self)
    {
    self->session_start = mtime_dtime();
    self->uptime = 0.0;
    self->stats = mbtrn_stats_new(ev_counters, status_counters, tm_channels, mbtrnpp_stats_labels);

    mbtrn_stats_set_period(self->stats, pstart, psec);
    }

  return self;
}
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_init_debug
(
  int verbose
)
{
  /* Open and initialize the socket based input for reading using function
   * mbtrnpreprocess_input_read(). Allocate an internal, hidden buffer to hold data from
   * full s7k records while waiting to return bytes from those records as
   * requested by the MBIO read functions.
   * Store the relevant pointers and parameters within the
   * mb_io_struct structure *mb_io_ptr. */

  mmd_initialize();
  mconf_init(NULL, NULL);
/*    mmd_channel_set(MOD_MBTRNPP,MM_ERR|MM_DEBUG);
      mmd_channel_set(MOD_R7K,MM_ERR);
      mmd_channel_set(MOD_MBTRN,MM_ERR);
      mmd_channel_set(MOD_MSOCK,MM_ERR);*/
  fprintf(stderr,
    "%s:%d >>> MOD_MBTRNPP[id=%d]  %08X\n",
    __FUNCTION__,
    __LINE__,
    MOD_MBTRNPP,
    mmd_get_enmask(MOD_MBTRNPP, NULL));

  switch (verbose)
    {
  case 0:
    mmd_channel_set(MOD_MBTRNPP, MM_NONE);
    mmd_channel_set(MOD_R7K, MM_NONE);
    mmd_channel_set(MOD_MBTRN, MM_NONE);
    mmd_channel_set(MOD_MSOCK, MM_NONE);
    break;
  case 1:
    mmd_channel_en(MOD_MBTRNPP, MBTRNPP_V1);
    mmd_channel_en(MOD_MBTRN, MBTRN_V1);
    break;
  case 2:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG);
    mmd_channel_en(MOD_MBTRN, MM_DEBUG);
    mmd_channel_en(MOD_R7K, R7K_PARSER);
    break;
  case -1:
    mmd_channel_en(MOD_MBTRNPP, MBTRNPP_V1);
    mmd_channel_en(MOD_MBTRN, MM_DEBUG);
    break;
  case -2:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG);
    break;
  case -3:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG|MBTRNPP_V1|MBTRNPP_V2);
    mmd_channel_en(MOD_MBTRN, MM_DEBUG);
    mmd_channel_en(MOD_R7K, MM_WARN|R7K_PARSER);
    break;
  case -4:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG|MBTRNPP_V1|MBTRNPP_V2);
    mmd_channel_en(MOD_MBTRN, MM_DEBUG);
    mmd_channel_en(MOD_R7K, MM_WARN|R7K_PARSER|R7K_DRFCON);
    mmd_channel_en(MOD_MSOCK, MM_DEBUG);
    break;
  case -5:
    mmd_channel_en(MOD_MBTRNPP, MM_ALL);
    mmd_channel_en(MOD_MBTRN, MM_ALL);
    mmd_channel_en(MOD_R7K, MM_ALL);
    mmd_channel_en(MOD_MSOCK, MM_ALL);
    break;
  case -6:
    mmd_channel_set(MOD_MBTRNPP, MBTRNPP_V6);
    mmd_channel_set(MOD_R7K, MM_NONE);
    mmd_channel_set(MOD_MBTRN, MM_NONE);
    mmd_channel_set(MOD_MSOCK, MM_NONE);
    break;
  default:
    break;
    }     /* switch */
  fprintf(stderr, "%s:%d >>> MOD_MBTRNPP  %08X\n", __FUNCTION__, __LINE__,
    mmd_get_enmask(MOD_MBTRNPP, NULL));

  /* initialize reader
     create and open socket connection*/
/*    mcfg_configure(NULL,0);
      mdb_set_name(APP,"mbtrnpreprocess");*/

  /* use MDI_ALL to globally set module debug output
     may also set per module basis using module IDs
     defined in mconfig.h:
     MBTRN, R7K, MREADER, RPARSER,
     DRFCON, IOW, APP, MAX_MODULE
     valid level values are
     MDL_UNSET,MDL_NONE
     MDL_FATAL, MDL_ERROR, MDL_WARN
     MDL_INFO, MDL_DEBUG*/

/*    mdb_set(MDI_ALL,MDL_NONE);
      switch (verbose) {
          case 0:
              mdb_set(APP,MDL_NONE);
              break;
          case 1:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(MBTRN,MDL_INFO);
              break;
          case 2:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(MBTRN,MDL_DEBUG);
              mdb_set(R7K,MDL_DEBUG);
              mdb_set(RPARSER,MDL_DEBUG);
              break;
          case -1:
              mdb_set(APP,MDL_DEBUG);
              break;
          case -2:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(APP1,MDL_DEBUG);
              mdb_set(APP2,MDL_DEBUG);
              break;
          case -3:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(APP1,MDL_DEBUG);
              mdb_set(APP2,MDL_DEBUG);
              mdb_set(APP3,MDL_DEBUG);
              mdb_set(MBTRN,MDL_DEBUG);
              mdb_set(R7K,MDL_WARN);
              mdb_set(RPARSER,MDL_WARN);
              break;
          case -4:
              mdb_set(APP,MDL_DEBUG);
              //            mdb_set(APP1,MDL_DEBUG);
              //            mdb_set(APP2,MDL_DEBUG);
              //            mdb_set(APP3,MDL_DEBUG);
              mdb_set(APP4,MDL_DEBUG);
              mdb_set(MBTRN,MDL_DEBUG);
              mdb_set(R7K,MDL_DEBUG);
              mdb_set(RPARSER,MDL_DEBUG);
              mdb_set(IOW,MDL_DEBUG);
              mdb_set(DRFCON,MDL_DEBUG);
              break;
          case -5:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(APP1,MDL_DEBUG);
              mdb_set(APP2,MDL_DEBUG);
              mdb_set(APP3,MDL_DEBUG);
              mdb_set(APP4,MDL_DEBUG);
              mdb_set(APP5,MDL_DEBUG);
              mdb_set(MBTRN,MDL_DEBUG);
              mdb_set(R7K,MDL_DEBUG);
              mdb_set(RPARSER,MDL_DEBUG);
              mdb_set(IOW,MDL_DEBUG);
              mdb_set(DRFCON,MDL_DEBUG);
              break;
          default:
              mdb_set(APP,MDL_DEBUG);
              mdb_set(APP1,MDL_DEBUG);
              mdb_set(APP2,MDL_DEBUG);
              mdb_set(MBTRN,MDL_DEBUG);
              mdb_set(MREADER,MDL_DEBUG);
              mdb_set(DRFCON,MDL_DEBUG);
              mdb_set(R7K,MDL_DEBUG);
              mdb_set(RPARSER,MDL_DEBUG);
              break;
      }*/


  /* open trn data log */
  if (trn_blog_en)
    {
    trn_blog_path = (char *)malloc(512);
    sprintf(trn_blog_path, "%s//%s-%s%s", g_log_dir, TRN_BLOG_NAME, session_date, TRN_LOG_EXT);
/*        trn_blog = mlog_new(trn_blog_path,&blog_conf); */
    trn_blog_id = mlog_get_instance(trn_blog_path, &blog_conf, TRN_BLOG_NAME);
    mlog_show(trn_blog_id, true, 5);
    mlog_open(trn_blog_id, flags, mode);
/*        mlog_add(trn_blog,trn_blog_id,TRN_BLOG_DESC); */
    }
  /* open trn message log */
  if (trn_mlog_en)
    {
    trn_mlog_path = (char *)malloc(512);
    sprintf(trn_mlog_path, "%s//%s-%s%s", g_log_dir, TRN_MLOG_NAME, session_date, TRN_LOG_EXT);
/*        trn_mlog = mlog_new(trn_mlog_path,&mlog_conf); */
    trn_mlog_id = mlog_get_instance(trn_mlog_path, &mlog_conf, TRN_MLOG_NAME);
    mlog_show(trn_mlog_id, true, 5);
    mlog_open(trn_mlog_id, flags, mode);
/*        mlog_add(trn_mlog,trn_mlog_id,TRN_MLOG_DESC); */
    mlog_tprintf(trn_mlog_id, "*** mbtrn session start ***\n");
    mlog_tprintf(trn_mlog_id, "cmdline [%s]\n", g_cmd_line);
    mlog_tprintf(trn_mlog_id, "libmbtrn v[%s] build[%s]\n", mbtrn_get_version(), mbtrn_get_build());
    }
  else
    {
    fprintf(stderr, "*** mbtrn session start ***\n");
    fprintf(stderr, "cmdline [%s]\n", g_cmd_line);
    }

  app_stats = mbtrnpreprocess_stats_new(MBTPP_EV_COUNT,
    MBTPP_STA_COUNT,
    MBTPP_CH_COUNT,
    mtime_dtime(),
    trn_status_interval_sec);

  return 0;
} /* mbtrnpreprocess_init_debug */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_close_debug()
{
    /* close trn data log */
  if (trn_blog_en)
    {
    mlog_close(trn_blog_id);
    }
  /* open trn message log */
  if (trn_mlog_en)
    {
    mlog_close(trn_mlog_id);
    }

  /* close mbr data log */
  if (mbr_blog_en)
    {
    mlog_close(mbr_blog_id);
    }

  return 0;
} /* mbtrnpreprocess_close_debug */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_input_open
(
  int verbose,
  void *mbio_ptr,
  char *inputname,
  int *error
)
{
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p,%p\n", mbio_ptr, &mbio_ptr);
    fprintf(stderr, "dbg2       inputname:  %s\n", inputname);
    }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  /* Open and initialize the socket based input for reading using function
   * mbtrnpreprocess_input_read(). Allocate an internal, hidden buffer to hold data from
   * full s7k records while waiting to return bytes from those records as
   * requested by the MBIO read functions.
   * Store the relevant pointers and parameters within the
   * mb_io_struct structure *mb_io_ptr. */

  PMPRINT(MOD_MBTRNPP, MM_DEBUG,
    (stderr, "configuring mbtrn_reader using %s:%d\n", reson_hostname, reson_port));
  mb_io_ptr->mbsp = mbtrn_reader_new(reson_hostname,
    reson_port,
    reader_capacity,
    reson_subs,
    reson_nsubs);

  if ((mb_io_ptr->mbsp->state==MBS_CONNECTED) || (mb_io_ptr->mbsp->state==MBS_SUBSCRIBED))
    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_CONN]);

  /* get global reader stats reference */
  reader_stats=mbtrn_reader_get_stats(mb_io_ptr->mbsp);
  mbtrn_stats_set_period(reader_stats,
    app_stats->stats->stat_period_start,
    app_stats->stats->stat_period_sec);

  /* configure reader data log */
  if (mbr_blog_en && (NULL!=mb_io_ptr->mbsp))
    {
    /* open mbr data log */
    mbr_blog_path = (char *)malloc(512);
    sprintf(mbr_blog_path, "%s//%s-%s%s", g_log_dir, MBR_BLOG_NAME, session_date, TRN_LOG_EXT);

/*        mbr_blog = mlog_new(mbr_blog_path,&mbrlog_conf); */
    mbr_blog_id = mlog_get_instance(mbr_blog_path, &mbrlog_conf, MBR_BLOG_NAME);

    mlog_show(mbr_blog_id, true, 5);
    mlog_open(mbr_blog_id, flags, mode);


    mbtrn_reader_set_log(mb_io_ptr->mbsp, mbr_blog_id);

/*        mbr_blogs = fopen(mbr_blog_path,"a+");
          mbtrn_reader_set_logstream(mb_io_ptr->mbsp,mbr_blogs);*/
    }

  if (verbose>=1)
    mbtrn_reader_show(mb_io_ptr->mbsp, true, 5);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_input_open */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_input_read
(
  int verbose,
  void *mbio_ptr,
  size_t size,
  char *buffer,
  int *error
)
{
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       sizer:      %zu\n", size);
    fprintf(stderr, "dbg2       buffer:     %p\n", buffer);
    }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  /* Read the requested number of bytes (= size) off the input and  place
   * those bytes into the buffer.
   * This requires reading full s7k records off the socket, storing the data
   * in an internal, hidden buffer, and parceling those bytes out as requested.
   * The internal buffer should be allocated in mbtrnpreprocess_input_init() and stored
   * in the mb_io_struct structure *mb_io_ptr. */

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_input_read */
/*--------------------------------------------------------------------*/

int mbtrnpreprocess_input_close
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  /* Close the socket based input for reading using function
   * mbtrnpreprocess_input_read(). Deallocate the internal, hidden buffer and any
   * other resources that were allocated by mbtrnpreprocess_input_init(). */

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
    }

  return status;
} /* mbtrnpreprocess_input_close */
/*--------------------------------------------------------------------*/

int main
(
  int argc,
  char **argv
)
{
  char help_message[] =
    "mbtrnpreprocess reads raw multibeam data, applies automated cleaning\n\t"
    "and downsampling, and then passes the bathymetry on to a terrain relative navigation (TRN) process.\n";
  char usage_message[] =
    "mbtrnpreprocess [\n" "\t--verbose\n" "\t--help\n" "\t--log-directory=path\n"
                                                       "\t--input=datalist|file|'socket'\n"
                                                       "\t--output=file|'socket'\n"
                                                       "\t--swathwidth=value\n"
                                                       "\t--soundings=value\n"
                                                       "\t--median-filter=threshold/nx/ny\n"
                                                       "\t--format=format\n"
                                                       "\t--platform-file\n"
                                                       "\t--platform-target-sensor\n"
                                                       "\t--projection=projection_id\n"
                                                       "\t--rhost=hostname\n"
                                                       "\t--thost=hostname\n"
                                                       "\t--stats=n\n"
                                                       "\t--hbeat=n\n"
                                                       "\t--delay=n\n"
                                                       "\t--decimate=n\n"
                                                       "\t--no-blog\n"
                                                       "\t--no-mlog\n"
                                                       "\t--no-rlog\n";
  extern char WIN_DECLSPEC *optarg;
  int option_index;
  int errflg = 0;
  int c;
  int help = 0;

  /* MBIO status variables */
  int status;
  int verbose = 0;
  int error = MB_ERROR_NO_ERROR;
  char *message;

  /* command line option definitions */
  /* mbtrnpreprocess
   *     --verbose
   *     --help
   *     --input=datalist [or file or socket id]
   *     --format=format
   *     --platform-file=file
   *     --platform-target-sensor
   *     --log-directory=path
   *     --output=file [or socket id]
   *     --projection=projection_id
   *     --swath-width=value
   *     --soundings=value
   *     --median-filter=threshold/nacrosstrack/nalongtrack
   *
   *
   */
  static struct option options[] =
      {{"help", no_argument, NULL, 0},
      {"verbose", required_argument, NULL, 0},
      {"input", required_argument, NULL, 0},
      {"rhost", required_argument, NULL, 0},
      {"thost", required_argument, NULL, 0},
      {"rcap", required_argument, NULL, 0},
      {"hbeat", required_argument, NULL, 0},
      {"delay", required_argument, NULL, 0},
      {"decimate", required_argument, NULL, 0},
      {"no-blog", no_argument, NULL, 0},
      {"no-mlog", no_argument, NULL, 0},
      {"no-rlog", no_argument, NULL, 0},
      {"stats", required_argument, NULL, 0},
      {"format", required_argument, NULL, 0},
      {"platform-file", required_argument, NULL, 0},
      {"platform-target-sensor", required_argument, NULL, 0},
      {"log-directory", required_argument, NULL, 0},
      {"output", required_argument, NULL, 0},
      {"projection", required_argument, NULL, 0},
      {"swath-width", required_argument, NULL, 0},
      {"soundings", required_argument, NULL, 0},
      {"median-filter", required_argument, NULL, 0},
      {NULL, 0, NULL, 0}};

  /* MBIO read control parameters */
  int read_datalist = MB_NO;
  int read_data = MB_NO;
  int read_socket = MB_NO;
  mb_path input;
  void *datalist;
  int look_processed = MB_DATALIST_LOOK_UNSET;
  double file_weight;
  int format;
  int system;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double btime_d;
  double etime_d;
  double speedmin;
  double timegap;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  int obeams_bath;
  int obeams_amp;
  int opixels_ss;
  mb_path ifile;
  mb_path dfile;
  void *imbio_ptr = NULL;
  unsigned int ping_number = 0;

  /* mbio read and write values */
  void *store_ptr;
  int kind;
  int ndata = 0;
  char comment[MB_COMMENT_MAXLINE];

  /* platform definition file */
  mb_path platform_file;
  int use_platform_file = MB_NO;
  struct mb_platform_struct *platform = NULL;
  struct mb_sensor_struct *sensor_bathymetry = NULL;
  struct mb_sensor_struct *sensor_backscatter = NULL;
  struct mb_sensor_struct *sensor_position = NULL;
  struct mb_sensor_struct *sensor_depth = NULL;
  struct mb_sensor_struct *sensor_heading = NULL;
  struct mb_sensor_struct *sensor_rollpitch = NULL;
  struct mb_sensor_struct *sensor_heave = NULL;
  struct mb_sensor_struct *sensor_target = NULL;
  int target_sensor = -1;

  /* buffer handling parameters */
  int n_buffer_max = 1;
  struct mbtrnpreprocess_ping_struct ping[MBTRNPREPROCESS_BUFFER_DEFAULT];
  int done;

  /* counting parameters */
  int n_pings_read = 0;
  int n_soundings_read = 0;
  int n_soundings_valid_read = 0;
  int n_soundings_flagged_read = 0;
  int n_soundings_null_read = 0;
  int n_soundings_trimmed = 0;
  int n_soundings_decimated = 0;
  int n_soundings_flagged = 0;
  int n_soundings_written = 0;
  int n_tot_pings_read = 0;
  int n_tot_soundings_read = 0;
  int n_tot_soundings_valid_read = 0;
  int n_tot_soundings_flagged_read = 0;
  int n_tot_soundings_null_read = 0;
  int n_tot_soundings_trimmed = 0;
  int n_tot_soundings_decimated = 0;
  int n_tot_soundings_flagged = 0;
  int n_tot_soundings_written = 0;

  /* processing control variables */
  double swath_width = 150.0;
  double tangent, threshold_tangent;
  int n_output_soundings = 101;
  int median_filter = MB_NO;
  int median_filter_n_across = 1;
  int median_filter_n_along = 1;
  int median_filter_n_total = 1;
  int median_filter_n_min = 1;
  double median_filter_threshold = 0.05;
  double *median_filter_soundings = NULL;
  int n_median_filter_soundings = 0;
  double median;
  int n_output;
  int64_t delay_msec=0;
  int ping_decimate = 1;


  /* output write control parameters */
  mb_path output;
  int output_mode = MBTRNPREPROCESS_OUTPUT_STDOUT;
  FILE *ofp = NULL;
  char *output_buffer = NULL;
  int n_output_buffer_alloc = 0;
  size_t mb1_size, index;
  unsigned int checksum;

  /* log file parameters */
  int make_logs = MB_NO;
  mb_path log_directory;
  FILE *logfp = NULL;
  mb_path log_message;
  double now_time_d;
  double log_file_open_time_d = 0.0;
  char date[32];
  struct stat logd_stat;
  int logd_status;

  int idataread, n_ping_process, i_ping_process;
  int beam_start, beam_end, beam_decimation;
  int i, ii, j, jj, n;
  int jj0, jj1, dj;
  int ii0, ii1, di;

  struct timeval timeofday;
  struct timezone timezone;
  double time_d;

  /* set default values */
  format = 0;
  pings = 1;
  bounds[0] = -360.;
  bounds[1] = 360.;
  bounds[2] = -90.;
  bounds[3] = 90.;
  btime_i[0] = 1962;
  btime_i[1] = 2;
  btime_i[2] = 21;
  btime_i[3] = 10;
  btime_i[4] = 30;
  btime_i[5] = 0;
  btime_i[6] = 0;
  etime_i[0] = 2062;
  etime_i[1] = 2;
  etime_i[2] = 21;
  etime_i[3] = 10;
  etime_i[4] = 30;
  etime_i[5] = 0;
  etime_i[6] = 0;
  speedmin = 0.0;
  timegap = 1000000000.0;

  /* set default input and output */
  memset(input, 0, sizeof(mb_path));
  memset(output, 0, sizeof(mb_path));
  memset(log_directory, 0, sizeof(mb_path));
  strcpy(input, "datalist.mb-1");
  strcpy(output, "stdout");

  /* make session time string to use
     in log file names*/
  time_t rawtime;
  struct tm *gmt;

  time(&rawtime);
  /* Get GMT time */
  gmt = gmtime(&rawtime);
  /* format YYYYMMDD-HHMMSS */
  sprintf(session_date,
    "%04d%02d%02d-%02d%02d%02d",
    (gmt->tm_year+1900),
    gmt->tm_mon+1,
    gmt->tm_mday,
    gmt->tm_hour,
    gmt->tm_min,
    gmt->tm_sec);

  memset(g_cmd_line, 0, TRN_CMD_LINE_BYTES);

  char *ip=g_cmd_line;
  int ilen=0;
  for (int x=0; x<argc; x++)
    {
    if ((ip+strlen(argv[x])-g_cmd_line) > TRN_CMD_LINE_BYTES)
      {
      fprintf(stderr, "warning - logged cmdline truncated\n");
      break;
      }
    ilen=sprintf(ip, " %s", argv[x]);
    ip+=ilen;
    }
  g_cmd_line[TRN_CMD_LINE_BYTES-1]='\0';
  g_log_dir=strdup("./");

  /* process argument list */
  while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
    switch (c)
      {
    /* long options all return c=0 */
    case 0:
      /* verbose */
      if (strcmp("verbose", options[option_index].name) == 0)
        {
        sscanf(optarg, "%d", &verbose);         /*verbose++; */
        }

      /* help */
      else if (strcmp("help", options[option_index].name) == 0)
        {
        help = MB_YES;
        }

      /*-------------------------------------------------------
       * Define input file and format */

      /* input */
      else if (strcmp("input", options[option_index].name) == 0)
        {
        strcpy(input, optarg);
        if (strstr(input, "socket"))
          input_mode=INPUT_MODE_SOCKET;
        else
          input_mode=INPUT_MODE_FILE;
        }
      else if (strcmp("rhost", options[option_index].name) == 0)
        {
        char *ocopy=strdup(optarg);
        reson_hostname=strtok(ocopy, ":");
        if (reson_hostname==NULL)
          reson_hostname=RESON_SIM_HOST;
        char *ip = strtok(NULL, ":");
        if (ip!=NULL)
          sscanf(ip, "%d", &reson_port);
        strcpy(input, "socket");
        input_mode=INPUT_MODE_SOCKET;
        /* don't free ocopy here */
        }
      else if (strcmp("thost", options[option_index].name) == 0)
        {
        char *ocopy=strdup(optarg);
        trn_ohost=strtok(ocopy, ":");
        if (trn_ohost==NULL)
          trn_ohost=RESON_SIM_HOST;
        char *ip = strtok(NULL, ":");
        if (ip!=NULL)
          sscanf(ip, "%d", &trn_oport);
        strcpy(output, "socket");
        output_mode = MBTRNPREPROCESS_OUTPUT_TRN;
        /* don't free ocopy here */
        }
      /* TRN reader capacity */
      else if (strcmp("rcap", options[option_index].name) == 0)
        {
        sscanf(optarg, "%u", &reader_capacity);
        }
      /* heartbeat (pings) */
      else if (strcmp("hbeat", options[option_index].name) == 0)
        {
        sscanf(optarg, "%d", &trn_hbtok);
        }
      /* ping delay */
      else if (strcmp("delay", options[option_index].name) == 0)
        {
        sscanf(optarg, "%lld", &delay_msec);
        }
      else if (strcmp("decimate", options[option_index].name) == 0)
        {
        sscanf(optarg, "%d", &ping_decimate);
        }
      /* disable TRN binary log output */
      else if (strcmp("no-blog", options[option_index].name) == 0)
        {
        trn_blog_en=false;
        }
      /* disable TRN message log output */
      else if (strcmp("no-mlog", options[option_index].name) == 0)
        {
        trn_mlog_en=false;
        }
      /* disable TRN reader log output */
      else if (strcmp("no-rlog", options[option_index].name) == 0)
        {
        mbr_blog_en=false;
        }
      /* status log interval (s) */
      else if (strcmp("stats", options[option_index].name) == 0)
        {
        sscanf(optarg, "%lf", &trn_status_interval_sec);
        }

      /* format */
      else if (strcmp("format", options[option_index].name) == 0)
        {
        n = sscanf(optarg, "%d", &format);
        }

      /*-------------------------------------------------------
       * Set platform file */

      /* platform-file */
      else if (strcmp("platform-file", options[option_index].name) == 0)
        {
        n = sscanf(optarg, "%s", platform_file);
        if (n == 1)
          use_platform_file = MB_YES;
        }

      /* platform-target-sensor */
      else if (strcmp("platform-target-sensor", options[option_index].name) == 0)
        {
        n = sscanf(optarg, "%d", &target_sensor);
        }

      /*-------------------------------------------------------
       * Define processing parameters */

      /* output */
      else if ((strcmp("output", options[option_index].name) == 0))
        {
        strcpy(output, optarg);
        if (strstr(output, "socket") != NULL)
          output_mode = MBTRNPREPROCESS_OUTPUT_TRN;
        else
          output_mode = MBTRNPREPROCESS_OUTPUT_FILE;
        fprintf(stderr, "output:%s output_mode:%d\n", output, output_mode);
        }

      /* log-directory */
      else if ((strcmp("log-directory", options[option_index].name) == 0))
        {
        strcpy(log_directory, optarg);
        logd_status = stat(log_directory, &logd_stat);
        if (logd_status != 0)
          {
          fprintf(stderr, "\nSpecified log file directory %s does not exist...\n", log_directory);
          make_logs = MB_NO;
          }
        else if ((logd_stat.st_mode & S_IFMT) != S_IFDIR)
          {
          fprintf(stderr, "\nSpecified log file directory %s is not a directory...\n",
            log_directory);
          make_logs = MB_NO;
          }
        else
          {
          make_logs = MB_YES;
          free(g_log_dir);
          g_log_dir=strdup(log_directory);
          fprintf(stderr, "\nusing log directory %s...\n", g_log_dir);
          }
        }

      /* swathwidth */
      else if ((strcmp("swath-width", options[option_index].name) == 0))
        {
        n = sscanf(optarg, "%lf", &swath_width);
        }

      /* soundings */
      else if ((strcmp("soundings", options[option_index].name) == 0))
        {
        n = sscanf(optarg, "%d", &n_output_soundings);
        }

      /* median-filter */
      else if ((strcmp("median-filter", options[option_index].name) == 0))
        {
        n = sscanf(optarg,
          "%lf/%d/%d",
          &median_filter_threshold,
          &median_filter_n_across,
          &median_filter_n_along);
        if (n == 3)
          {
          median_filter = MB_YES;
          n_buffer_max = median_filter_n_along;
          }
        }

      /*-------------------------------------------------------*/

      break;
    case '?':
      errflg++;
      }

  if(reson_hostname==NULL)
    reson_hostname=strdup(RESON_SIM_HOST);
  /* if error flagged then print it and exit */
  if (errflg)
    {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
    }

  if ((verbose == 1) || help)
    {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    }

  if (verbose >= 0)
    {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       help:           %d\n", help);
    fprintf(stderr, "dbg2       pings:          %d\n", pings);
    fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
    fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
    fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
    fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
    fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
    fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
    fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
    fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
    fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
    fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
    fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
    fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
    fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
    fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
    fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
    fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
    fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
    fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
    fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
    fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
    fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
    fprintf(stderr, "dbg2       input:                    %s\n", input);
    fprintf(stderr, "dbg2       format:                   %d\n", format);
    fprintf(stderr, "dbg2       output:                   %s\n", output);
    fprintf(stderr, "dbg2       swath_width:              %f\n", swath_width);
    fprintf(stderr, "dbg2       n_output_soundings:       %d\n", n_output_soundings);
    fprintf(stderr, "dbg2       median_filter:            %d\n", median_filter);
    fprintf(stderr, "dbg2       median_filter_n_across:   %d\n", median_filter_n_across);
    fprintf(stderr, "dbg2       median_filter_n_along:    %d\n", median_filter_n_along);
    fprintf(stderr, "dbg2       median_filter_threshold:  %f\n", median_filter_threshold);
    fprintf(stderr, "dbg2       n_buffer_max:             %d\n", n_buffer_max);
    fprintf(stderr, "dbg2       reson_hostname:           %s\n", reson_hostname);
    fprintf(stderr, "dbg2       reson_port:               %d\n", reson_port);
    fprintf(stderr, "dbg2       reader_capacity:          %u\n", reader_capacity);
    fprintf(stderr, "dbg2       trn_ohost:                %s\n", trn_ohost);
    fprintf(stderr, "dbg2       trn_oport:                %d\n", trn_oport);
    }

  /* if help desired then print it and exit */
  if (help)
    {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error);
    }
#ifdef MBTRN_TIMING
  /* print time message */
  struct timeval stv={0};
  gettimeofday(&stv, NULL);
  double start_sys_time = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0)+(7*3600);
  fprintf(stderr, "%11.5lf systime %.4lf\n", mtime_dtime(), start_sys_time);
#endif
  fprintf(stderr,
    "%s:%d: calling mbtrnpreprocess_init_debug verbose:%d\n",
    __FILE__,
    __LINE__,
    verbose);
  mbtrnpreprocess_init_debug(verbose);

  /* load platform definition if specified */
  if (use_platform_file == MB_YES)
    {
    status = mb_platform_read(verbose, platform_file, (void **)&platform, &error);
    if (status == MB_FAILURE)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open and parse platform file: %s\n", platform_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
      }

    /* get sensor structures */
    if (platform->source_bathymetry >= 0)
      sensor_bathymetry = &(platform->sensors[platform->source_bathymetry]);
    if (platform->source_backscatter >= 0)
      sensor_backscatter = &(platform->sensors[platform->source_backscatter]);
    if (platform->source_position >= 0)
      sensor_position = &(platform->sensors[platform->source_position]);
    if (platform->source_depth >= 0)
      sensor_depth = &(platform->sensors[platform->source_depth]);
    if (platform->source_heading >= 0)
      sensor_heading = &(platform->sensors[platform->source_heading]);
    if (platform->source_rollpitch >= 0)
      sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
    if (platform->source_heave >= 0)
      sensor_heave = &(platform->sensors[platform->source_heave]);
    if (target_sensor < 0)
      target_sensor = platform->source_bathymetry;
    if (target_sensor >= 0)
      sensor_target = &(platform->sensors[target_sensor]);
    }

  /* initialize output */
  if (output_mode == MBTRNPREPROCESS_OUTPUT_STDOUT)
    {
    }
  /* insert option to recognize and initialize ipc with TRN
     else open ipc to TRN*/
  else if (output_mode == MBTRNPREPROCESS_OUTPUT_TRN)
    {
/*        md_level_t olvl; */
    mmd_en_mask_t olvl;
    if (verbose!=0)
      {
      olvl = mmd_get_enmask(MOD_MBTRNPP, NULL);
      mmd_channel_en(MOD_MBTRNPP, MM_DEBUG);
/*            olvl = mdb_get(APP,NULL);
              mdb_set(APP,MDL_DEBUG);*/
      }
    trn_peer=msock_connection_new();
    trn_plist = mlist_new();
    mlist_autofree(trn_plist, msock_connection_free);
    PMPRINT(MOD_MBTRNPP, MM_DEBUG,
      (stderr, "configuring TRN host using %s:%d\n", trn_ohost, trn_oport));
    trn_osocket = msock_socket_new(trn_ohost, trn_oport, ST_UDP);
    msock_set_blocking(trn_osocket, false);
    int test=-1;
    if ( (test=msock_bind(trn_osocket))==0) {
      PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "TRN host socket bind OK [%s:%d]\n", TRN_HOST_DFL, TRN_PORT_DFL));
    }
    else {
      fprintf(stderr, "\nTRN host socket bind failed [%d] [%d %s]\n", test, errno, strerror(errno));
    }
    if (verbose!=0)
      mmd_channel_set(MOD_MBTRNPP, olvl);
/*        mdb_set(APP,olvl); */
    }
  /* else open output file in which the binary data otherwise communicated
   * to TRN will be saved */
  else
    {
    ofp = fopen(output, "w");
    }

  /* get number of ping records to hold */
  if (median_filter == MB_YES)
    {
    median_filter_n_total = median_filter_n_across * median_filter_n_along;
    median_filter_n_min = median_filter_n_total / 2;

    /* allocate memory for median filter */
    if (error == MB_ERROR_NO_ERROR)
      {
      status =
        mb_mallocd(verbose,
        __FILE__,
        __LINE__,
        median_filter_n_total * sizeof(double),
        (void **)&median_filter_soundings,
        &error);
      if (error != MB_ERROR_NO_ERROR)
        {
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
        }
      }
    }

  /* get format if required */
  if (format == 0)
    mb_get_format(verbose, input, NULL, &format, &error);

  /* determine whether to read one file or a list of files */
  if (format < 0)
    read_datalist = MB_YES;

  /* open file list */
  if (read_datalist == MB_YES)
    {
    if ((status =mb_datalist_open(verbose, &datalist, input, look_processed, &error)) != MB_SUCCESS)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open data list file: %s\n", input);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
      }
    if ((status =
      mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight,
        &error)) == MB_SUCCESS)
      read_data = MB_YES;
    else
      read_data = MB_NO;
    }
  /* else copy single filename to be read */
  else
    {
    strcpy(ifile, input);
    read_data = MB_YES;
    }
  /* kick off the first cycle here
     future cycles start and end in the stats update*/
  MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_CYCLE_XT], mtime_dtime());
  MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_STATS_XT], mtime_dtime());
  /* loop over all files to be read */
  while (read_data == MB_YES)
    {
    /* open log file if specified */
    if (make_logs == MB_YES)
      {
      gettimeofday(&timeofday, &timezone);
      now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
/*fprintf(stderr,"CHECKING AT TOP OF LOOP: logfp:%p log_file_open_time_d:%.6ff now_time_d:%.6f\n",
   logfp, log_file_open_time_d, now_time_d); */
      if ((logfp == NULL) ||
        ((now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) )
        {
        if (logfp != NULL)
          {
          status = mbtrnpreprocess_logstatistics(verbose,
            logfp,
            n_pings_read,
            n_soundings_read,
            n_soundings_valid_read,
            n_soundings_flagged_read,
            n_soundings_null_read,
            n_soundings_trimmed,
            n_soundings_decimated,
            n_soundings_flagged,
            n_soundings_written,
            &error);
          n_tot_pings_read += n_pings_read;
          n_tot_soundings_read += n_soundings_read;
          n_tot_soundings_valid_read += n_soundings_valid_read;
          n_tot_soundings_flagged_read += n_soundings_flagged_read;
          n_tot_soundings_null_read += n_soundings_null_read;
          n_tot_soundings_trimmed += n_soundings_trimmed;
          n_tot_soundings_decimated += n_soundings_decimated;
          n_tot_soundings_flagged += n_soundings_flagged;
          n_tot_soundings_written += n_soundings_written;
          n_pings_read = 0;
          n_soundings_read = 0;
          n_soundings_valid_read = 0;
          n_soundings_flagged_read = 0;
          n_soundings_null_read = 0;
          n_soundings_trimmed = 0;
          n_soundings_decimated = 0;
          n_soundings_flagged = 0;
          n_soundings_written = 0;

          status = mbtrnpreprocess_closelog(verbose, &logfp, &error);
          }

        status = mbtrnpreprocess_openlog(verbose, log_directory, &logfp, &error);
        if (status == MB_SUCCESS)
          {
          gettimeofday(&timeofday, &timezone);
          log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
          status = mbtrnpreprocess_logparameters(verbose,
            logfp,
            input,
            format,
            output,
            swath_width,
            n_output_soundings,
            median_filter,
            median_filter_n_across,
            median_filter_n_along,
            median_filter_threshold,
            n_buffer_max,
            &error);
          }
        else
          {
          fprintf(stderr, "\nLog file could not be opened in directory %s...\n", log_directory);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
          }
        }
      }

    /* check for format with amplitude or sidescan data */
    status = mb_format_system(verbose, &format, &system, &error);
    status =mb_format_dimensions(verbose, &format, &beams_bath, &beams_amp, &pixels_ss, &error);

    /* initialize reading the input swath data over a socket interface
     * using functions defined in this code block and passed into the
     * init function as function pointers */
    if (strncmp(input, "socket", 6) == 0)
      {
      if ((status =
        mb_input_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin,
          timegap, &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
          &mbtrnpreprocess_input_open, &mbtrnpreprocess_input_read, &mbtrnpreprocess_input_close,
          &error) != MB_SUCCESS))
        {
        sprintf(log_message, "MBIO Error returned from function <mb_input_init>");
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(verbose, error, &message);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Multibeam data socket <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        exit(error);
        }
      else
        {
        sprintf(log_message, "Multibeam data socket <%s> initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "MBIO format id: %d", format);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "%s\n", log_message);
        }
      }

    /* otherwised open swath data files as is normal for MB-System programs */
    else
      {
      if ((status =
        mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin,
          timegap, &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss,
          &error)) != MB_SUCCESS)
        {
        sprintf(log_message, "MBIO Error returned from function <mb_read_init>");
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(verbose, error, &message);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Multibeam File <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        exit(error);
        }
      else
        {
        sprintf(log_message, "Multibeam File <%s> initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "MBIO format id: %d", format);
        if (logfp != NULL)
          mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "%s\n", log_message);
        }
      }

    /* allocate memory for data arrays */
    memset(ping, 0, MBTRNPREPROCESS_BUFFER_DEFAULT * sizeof(struct mbtrnpreprocess_ping_struct));
    for (i = 0; i < n_buffer_max; i++)
      {
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_BATHYMETRY,
          sizeof(char),
          (void **)&ping[i].beamflag,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_BATHYMETRY,
          sizeof(char),
          (void **)&ping[i].beamflag_filter,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_BATHYMETRY,
          sizeof(double),
          (void **)&ping[i].bath,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_AMPLITUDE,
          sizeof(double),
          (void **)&ping[i].amp,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_BATHYMETRY,
          sizeof(double),
          (void **)&ping[i].bathacrosstrack,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_BATHYMETRY,
          sizeof(double),
          (void **)&ping[i].bathalongtrack,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_SIDESCAN,
          sizeof(double),
          (void **)&ping[i].ss,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_SIDESCAN,
          sizeof(double),
          (void **)&ping[i].ssacrosstrack,
          &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
          mb_register_array(verbose,
          imbio_ptr,
          MB_MEM_TYPE_SIDESCAN,
          sizeof(double),
          (void **)&ping[i].ssalongtrack,
          &error);
      }

    /* plan on storing enough pings for median filter */
    n_buffer_max = median_filter_n_along;
    n_ping_process = n_buffer_max / 2;
    int n_ping_count = 0;

    /* loop over reading data */
    done = MB_NO;
    idataread = 0;

    while (!done)
      {
      /* open new log file if it is time */
      if (make_logs == MB_YES)
        {
        gettimeofday(&timeofday, &timezone);
        now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
/*fprintf(stderr,"CHECKING AT MIDDLE OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n",
   logfp, log_file_open_time_d, now_time_d); */
        if ((logfp == NULL) ||
          ((now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) )
          {
          if (logfp != NULL)
            {
            status = mbtrnpreprocess_logstatistics(verbose,
              logfp,
              n_pings_read,
              n_soundings_read,
              n_soundings_valid_read,
              n_soundings_flagged_read,
              n_soundings_null_read,
              n_soundings_trimmed,
              n_soundings_decimated,
              n_soundings_flagged,
              n_soundings_written,
              &error);
            n_tot_pings_read += n_pings_read;
            n_tot_soundings_read += n_soundings_read;
            n_tot_soundings_valid_read += n_soundings_valid_read;
            n_tot_soundings_flagged_read += n_soundings_flagged_read;
            n_tot_soundings_null_read += n_soundings_null_read;
            n_tot_soundings_trimmed += n_soundings_trimmed;
            n_tot_soundings_decimated += n_soundings_decimated;
            n_tot_soundings_flagged += n_soundings_flagged;
            n_tot_soundings_written += n_soundings_written;
            n_pings_read = 0;
            n_soundings_read = 0;
            n_soundings_valid_read = 0;
            n_soundings_flagged_read = 0;
            n_soundings_null_read = 0;
            n_soundings_trimmed = 0;
            n_soundings_decimated = 0;
            n_soundings_flagged = 0;
            n_soundings_written = 0;

            status = mbtrnpreprocess_closelog(verbose, &logfp, &error);
            }

          status = mbtrnpreprocess_openlog(verbose, log_directory, &logfp, &error);
          if (status == MB_SUCCESS)
            {
            gettimeofday(&timeofday, &timezone);
            log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
            status = mbtrnpreprocess_logparameters(verbose,
              logfp,
              input,
              format,
              output,
              swath_width,
              n_output_soundings,
              median_filter,
              median_filter_n_across,
              median_filter_n_along,
              median_filter_threshold,
              n_buffer_max,
              &error);
            }
          else
            {
            fprintf(stderr, "\nLog file could not be opened in directory %s...\n", log_directory);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
            }
          }
        }

      /* read the next data */
      error = MB_ERROR_NO_ERROR;

      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_MBGETALL_XT], mtime_dtime());
//fprintf(stderr, "%s:%d:%s \n", __FILE__, __LINE__, __FUNCTION__);
      status =
        mb_get_all(verbose,
                    imbio_ptr,
                    &store_ptr,
                    &kind,
                    ping[idataread].time_i,
                    &ping[idataread].time_d,
                    &ping[idataread].navlon,
                    &ping[idataread].navlat,
                    &ping[idataread].speed,
                    &ping[idataread].heading,
                    &ping[idataread].distance,
                    &ping[idataread].altitude,
                    &ping[idataread].sonardepth,
                    &ping[idataread].beams_bath,
                    &ping[idataread].beams_amp,
                    &ping[idataread].pixels_ss,
                    ping[idataread].beamflag,
                    ping[idataread].bath,
                    ping[idataread].amp,
                    ping[idataread].bathacrosstrack,
                    ping[idataread].bathalongtrack,
                    ping[idataread].ss,
                    ping[idataread].ssacrosstrack,
                    ping[idataread].ssalongtrack,
                    comment,
                    &error);
//fprintf(stderr, "%s:%d:%s \n", __FILE__, __LINE__, __FUNCTION__);

      /*PMPRINT(MOD_MBTRNPP,MBTRNPP_V4,(stderr,"mb_get_all - status[%d] kind[%d]
         err[%d]\n",status, kind, error)); */
      MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_MBGETALL_XT], mtime_dtime());
      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_MBPING_XT], mtime_dtime());

      if ((status == MB_SUCCESS) && (kind == MB_DATA_DATA))
        {
        ping[idataread].count = ndata;
        ndata++;
        n_pings_read++;
        n_soundings_read += ping[idataread].beams_bath;
        for (i=0; i<ping[idataread].beams_bath; i++)
          {
          ping[idataread].beamflag_filter[i] = ping[idataread].beamflag[i];
          if (mb_beam_ok(ping[idataread].beamflag[i]))
            n_soundings_valid_read++;
          else if (ping[idataread].beamflag[i] == MB_FLAG_NULL)
            n_soundings_null_read++;
          else
            n_soundings_flagged_read++;
          }

        status =
          mb_extract_nav(verbose,
          imbio_ptr,
          store_ptr,
          &kind,
          ping[idataread].time_i,
          &ping[idataread].time_d,
          &ping[idataread].navlon,
          &ping[idataread].navlat,
          &ping[idataread].speed,
          &ping[idataread].heading,
          &ping[idataread].sonardepth,
          &ping[idataread].roll,
          &ping[idataread].pitch,
          &ping[idataread].heave,
          &error);
        status = mb_extract_altitude(verbose,
          imbio_ptr,
          store_ptr,
          &kind,
          &ping[idataread].sonardepth,
          &ping[idataread].altitude,
          &error);

        /* only process and output if enough data have been read */
        if (ndata == n_buffer_max)
          {
          for (i = 0; i < n_buffer_max; i++)
            if (ping[i].count == n_ping_process)
              i_ping_process = i;
/*fprintf(stdout, "\nProcess some data: ndata:%d counts: ", ndata);
   for (i = 0; i < n_buffer_max; i++) {
      fprintf(stdout,"%d ", ping[i].count);
   }
   fprintf(stdout," : process %d\n", i_ping_process);*/

          /* apply swath width */
          threshold_tangent = tan(DTR * 0.5 * swath_width);
          beam_start = ping[i_ping_process].beams_bath - 1;
          beam_end = 0;
          for (j = 0; j < ping[i_ping_process].beams_bath; j++)
            if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
              {
              tangent = ping[i_ping_process].bathacrosstrack[j]/
                (ping[i_ping_process].bath[j]- ping[i_ping_process].sonardepth);
              if ((fabs(tangent) > threshold_tangent) &&
                mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
                {
                ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG +MB_FLAG_FILTER;
                n_soundings_trimmed++;
                }
              else
                {
                beam_start = MIN(beam_start, j);
                beam_end = MAX(beam_end, j);
                }
              }

          /* apply decimation - only consider outputting decimated soundings */
          beam_decimation = ((beam_end - beam_start + 1) / n_output_soundings) + 1;
          dj = median_filter_n_across / 2;
          di = median_filter_n_along / 2;
          n_output = 0;
          for (j = beam_start; j <= beam_end; j++)
            {
            if ((j - beam_start) % beam_decimation == 0)
              {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
                {
                /* apply median filtering to this sounding */
                if (median_filter_n_total > 1)
                  {
                  /* accumulate soundings for median filter */
                  n_median_filter_soundings = 0;
                  jj0 = MAX(beam_start, j - dj);
                  jj1 = MIN(beam_end, j + dj);
                  for (ii = 0; ii < n_buffer_max; ii++)
                    for (jj = jj0; jj <= jj1; jj++)
                      if (mb_beam_ok(ping[ii].beamflag[jj]))
                        {
                        median_filter_soundings[n_median_filter_soundings] =ping[ii].bath[jj];
                        n_median_filter_soundings++;
                        }

                  /* run qsort */
                  qsort((char *)median_filter_soundings,
                    n_median_filter_soundings,
                    sizeof(double),
                    (void *)mb_double_compare);
                  median = median_filter_soundings[n_median_filter_soundings / 2];
/*fprintf(stdout, "Beam %3d of %d:%d bath:%.3f n:%3d:%3d median:%.3f ", j, beam_start, beam_end,
   ping[i_ping_process].bath[j], n_median_filter_soundings, median_filter_n_min, median);*/

                  /* apply median filter - also flag soundings that don't have
                     enough neighbors to filter */
                  if ((n_median_filter_soundings < median_filter_n_min) ||
                    (fabs(ping[i_ping_process].bath[j] - median) >
                    median_filter_threshold * median) )
                    {
                    ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG +MB_FLAG_FILTER;
                    n_soundings_flagged++;

/*fprintf(stdout, "**filtered**"); */
                    }
/*fprintf(stdout, "\n"); */
                  }
                if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
                  n_output++;
                }
              }
            else if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
              {
              ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
              n_soundings_decimated++;
              }
            }

/*fprintf(stderr, "Mbtrnpreprocess: Ping read: %7d %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d
   %2.2d:%2.2d\n", */
/*ping_number,
   ping[i_ping_process].time_i[0], ping[i_ping_process].time_i[1], ping[i_ping_process].time_i[2],
   ping[i_ping_process].time_i[3], ping[i_ping_process].time_i[4], ping[i_ping_process].time_i[5],
   ping[i_ping_process].time_i[6],
   ping[i_ping_process].beams_bath, n_output);*/

          /* decimate output records */
          n_ping_count++;
//fprintf(stderr,"Decimation calculation: ping_decimate:%d n_ping_count:%d output?:%d\n",
//ping_decimate, n_ping_count, (n_ping_count % ping_decimate == 0));
          if (n_ping_count % ping_decimate == 0)
            {
            n_ping_count = 0;

            /* write out results to stdout as text */
            if (output_mode == MBTRNPREPROCESS_OUTPUT_STDOUT)
              {
              fprintf(stdout,
                "Ping: %.9f %.7f %.7f %.3f %.3f %4d\n",
                ping[i_ping_process].time_d,
                ping[i_ping_process].navlat,
                ping[i_ping_process].navlon,
                ping[i_ping_process].sonardepth,
                (double)(DTR * ping[i_ping_process].heading),
                n_output);
              for (j = 0; j < ping[i_ping_process].beams_bath; j++)
                if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
                  {
                  fprintf(stdout,
                    "%3.3d starboard:%.3f forward:%.3f down:%.3f\n",
                    j,
                    ping[i_ping_process].bathacrosstrack[j],
                    ping[i_ping_process].bathalongtrack[j],
                    ping[i_ping_process].bath[j] -ping[i_ping_process].sonardepth);
                  n_soundings_written++;
                  }
              }

            /* pack the data into a TRN MB1 packet and either send it to TRN or write it
               to a file */
            else
              {
              n_soundings_written++;

              /* make sure buffer is large enough to hold the packet */
              mb1_size = MBTRNPREPROCESS_MB1_HEADER_SIZE+ n_output *
                MBTRNPREPROCESS_MB1_SOUNDING_SIZE+MBTRNPREPROCESS_MB1_CHECKSUM_SIZE;
              if (n_output_buffer_alloc < mb1_size)
                {
                if ((status =
                  mb_reallocd(verbose, __FILE__, __LINE__, mb1_size, (void **)&output_buffer,
                    &error))== MB_SUCCESS)
                  {
                  n_output_buffer_alloc = mb1_size;
                  }
                else
                  {
                  mb_error(verbose, error, &message);
                  fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
                  fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
                  exit(error);
                  }
                }

              /* get ping number */
              mb_pingnumber(verbose, imbio_ptr, &ping_number, &error);

              /* now pack the data into the packet buffer */
              index = 0;
              output_buffer[index] = 'M'; index++;
              output_buffer[index] = 'B'; index++;
              output_buffer[index] = '1'; index++;
              output_buffer[index] = 0; index++;
              mb_put_binary_int(MB_YES, mb1_size, &output_buffer[index]); index += 4;

              mb_put_binary_double(MB_YES, ping[i_ping_process].time_d, &output_buffer[index]);
              index += 8;
              mb_put_binary_double(MB_YES, ping[i_ping_process].navlat, &output_buffer[index]);
              index += 8;
              mb_put_binary_double(MB_YES, ping[i_ping_process].navlon, &output_buffer[index]);
              index += 8;
              mb_put_binary_double(MB_YES, ping[i_ping_process].sonardepth, &output_buffer[index]);
              index += 8;
              mb_put_binary_double(MB_YES,
                (double)(DTR * ping[i_ping_process].heading),
                &output_buffer[index]); index += 8;

              mb_put_binary_int(MB_YES, ping_number, &output_buffer[index]);
              index += 4;

              mb_put_binary_int(MB_YES, n_output, &output_buffer[index]); index += 4;

              PMPRINT(MOD_MBTRNPP, MM_DEBUG,
                (stderr,
                "\nts[%.3lf] beams[%03d] ping[%06u]\nlat[%.4lf] lon[%.4lf] hdg[%6.2lf] sd[%7.2lf]\nv[%+6.2lf] p/r/y[%.3lf / %.3lf / %.3lf]\n",
                ping[i_ping_process].time_d, n_output, ping_number, ping[i_ping_process].navlat,
                ping[i_ping_process].navlon, (double)(DTR * ping[i_ping_process].heading),
                ping[i_ping_process].sonardepth, ping[i_ping_process].speed,
                ping[i_ping_process].pitch, ping[i_ping_process].roll, ping[i_ping_process].heave));

              for (j = 0; j < ping[i_ping_process].beams_bath; j++)
                if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j]))
                  {
                  mb_put_binary_int(MB_YES, j, &output_buffer[index]);
                  index += 4;
                  mb_put_binary_double(MB_YES,
                    ping[i_ping_process].bathalongtrack[j],
                    &output_buffer[index]); index += 8;
                  mb_put_binary_double(MB_YES,
                    ping[i_ping_process].bathacrosstrack[j],
                    &output_buffer[index]); index += 8;
/*                                mb_put_binary_double(MB_YES, ping[i_ping_process].bath[j],
   &output_buffer[index]); index += 8; */
                  /* subtract sonar depth from vehicle bathy; changed 12jul18
                     cruises */
                  mb_put_binary_double(MB_YES,
                    (ping[i_ping_process].bath[j] -ping[i_ping_process].sonardepth),
                    &output_buffer[index]);
                  index += 8;

                  PMPRINT(MOD_MBTRNPP, MBTRNPP_V1,
                    (stderr, "n[%03d] atrk/X[%+10.3lf] ctrk/Y[%+10.3lf] dpth/Z[%+10.3lf]\n", j,
                    ping[i_ping_process].bathalongtrack[j], ping[i_ping_process].bathacrosstrack[j],
                    (ping[i_ping_process].bath[j] -ping[i_ping_process].sonardepth)));
                  }

              /* add the checksum */
              checksum = 0;
              unsigned char *cp = (unsigned char *)output_buffer;
              for (j = 0; j < index; j++)
/*                            checksum += (unsigned int) output_buffer[j]; */
                checksum += (unsigned int) (*cp++);
              PMPRINT(MOD_MBTRNPP, MBTRNPP_V1,
                (stderr, "before put : chk[%08X/%u] idx[%zu] mb1sz[%zu]\n", checksum, checksum,
                index, mb1_size));
              mb_put_binary_int(MB_YES, checksum, &output_buffer[index]); index += 4;
              PMPRINT(MOD_MBTRNPP, MBTRNPP_V1,
                (stderr, "after put : chk[%08X] idx[%zu] mb1sz[%zu]\n", checksum, index, mb1_size));

              MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_MBPING_XT], mtime_dtime());

              /* send the packet to TRN */
              if (output_mode == MBTRNPREPROCESS_OUTPUT_TRN)
                {
                MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CYCLES]);

/*                            struct timeval stv={0};
                              gettimeofday(&stv,NULL);
                              double stime = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0);
                              double ptime=ping[i_ping_process].time_d;*/
/*                            fprintf(stderr,"mbtx : ptime[%.3lf] stime[%.3lf]
   (s-p)[%+6.3lf]**\n",ptime,stime,(stime-ptime)); */
/*                            fprintf(stderr,"mbtx : (s-p)[%+6.3lf]**\n",(stime-ptime)); */

                /* log current TRN message */
                if(trn_blog_en)
                  mlog_write(trn_blog_id, (byte *)output_buffer, mb1_size);

                /* send output to TRN clients */
                MBTR_COUNTER_SET(app_stats->stats->status[MBTPP_STA_CLI_LIST_LEN],
                  mlist_size(trn_plist));
                MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_TRNTX_XT], mtime_dtime());

                int iobytes = 0;
                int test = -1;
                int idx=-1;
                msock_connection_t *psub = (msock_connection_t *)mlist_first(trn_plist);
                idx=0;
                while (psub != NULL) {
                  psub->heartbeat--;

                  iobytes =
                    msock_sendto(trn_osocket, psub->addr, (byte *)output_buffer, mb1_size, 0 );

                  if (  iobytes > 0) {
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_TXN]);
                    MBTR_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_TRN_TX_BYTES], iobytes);
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_TRN_PUBN]);
                    MBTR_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_TRN_PUB_BYTES], iobytes);

                    PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                      (stderr, "tx TRN [%5d]b cli[%d/%s:%s] hb[%d]\n", iobytes, idx, psub->chost,
                      psub->service, psub->heartbeat));
                  }
                  else {
                    PEPRINT((stderr, "err - sendto ret[%d] cli[%d] [%d/%s]\n", iobytes, idx, errno,
                      strerror(errno)));
                    mlog_tprintf(trn_mlog_id,
                      "err - sendto ret[%d] cli[%d] [%d/%s]\n",
                      iobytes,
                      idx,
                      errno,
                      strerror(errno));
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ETRN_TX]);
                  }

                  /* check heartbeat, remove expired peers */
                  if ((NULL!=psub) && (psub->heartbeat==0)) {
                    PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                      (stderr, "hbeat=0 cli[%d/%d] - removed\n", idx, psub->id));
                    mlog_tprintf(trn_mlog_id, "hbeat=0 cli[%d/%d] - removed\n", idx, psub->id);
                    mlist_remove(trn_plist, psub);
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_DISN]);
                    MBTR_COUNTER_SET(app_stats->stats->status[
                        MBTPP_STA_CLI_LIST_LEN], mlist_size(trn_plist));
                  }
                  psub=(msock_connection_t *)mlist_next(trn_plist);
                  idx++;
                }                  /* while psub */
                if (iobytes > 0) {
                  fprintf(stderr,
                    "Mbtrnpreprocess: Ping xmtd %d bytes: %7d %.6f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %2.2d:%2.2d\n",
                    iobytes, ping_number,
                    ping[i_ping_process].time_d,
                    ping[i_ping_process].time_i[0],
                    ping[i_ping_process].time_i[1],
                    ping[i_ping_process].time_i[2],
                    ping[i_ping_process].time_i[3],
                    ping[i_ping_process].time_i[4],
                    ping[i_ping_process].time_i[5],
                    ping[i_ping_process].time_i[6],
                    ping[i_ping_process].beams_bath,
                    n_output);
                }
                MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_TRNTX_XT], mtime_dtime());

                /* check trn socket for client messages (connection, heartbeat) */
                byte cmsg[TRN_MSG_CON_LEN];
                int svc=0;

                PMPRINT(MOD_MBTRNPP, MBTRNPP_V4, (stderr, "checking trn host socket\n"));
                bool trn_recv_pending=true;
                do
                  {
                  MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_TRNRX_XT], mtime_dtime());

                  iobytes = msock_recvfrom(trn_osocket, trn_peer->addr, cmsg, TRN_MSG_CON_LEN, 0);

                  MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_TRNRX_XT], mtime_dtime());

                  switch (iobytes)
                    {
                  case 0:
                    /* no bytes returned (socket closed) */
                    PMPRINT(MOD_MBTRNPP, MM_DEBUG,
                      (stderr, "err - recvfrom ret 0 (socket closed) removing cli[%d]\n",
                      trn_peer->id));
                    mlog_tprintf(trn_mlog_id,
                      "recvfrom ret 0 (socket closed) removing cli[%d]\n",
                      trn_peer->id);
                    /* remove from list */
                    if(sscanf(trn_peer->service, "%d", &svc)==1)
                      {
                      msock_connection_t *peer =(msock_connection_t *)mlist_vlookup(trn_plist,
                        &svc,
                        mbtrn_peer_vcmp);
                      if (peer!=NULL)
                        mlist_remove(trn_plist, peer);
                      }
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLI_RXZ]);
                    trn_recv_pending=false;
                    break;
                  case -1:
                    /* error (usually nothing to read) */
                    if ((errno!=EAGAIN) && (errno!=EWOULDBLOCK))
                      PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                        (stderr, "err - recvfrom cli[%d] ret -1 [%d/%s]\n", trn_peer->id, errno,
                        strerror(errno)));
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLI_RXE]);
                    trn_recv_pending=false;
                    break;

                  default:
                    /* bytes received */
                    MBTR_COUNTER_ADD(app_stats->stats->status[
                        MBTPP_STA_CLI_RX_BYTES], iobytes);
                    MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_RXN]);

                    const char *ctest=NULL;
                    uint16_t port=0xFFFF;
                    struct sockaddr_in *psin = NULL;
                    if ((NULL != trn_peer->addr) && (NULL != trn_peer->addr->ainfo) &&
                      (NULL != trn_peer->addr->ainfo->ai_addr) )
                      {
                      psin =(struct sockaddr_in *)trn_peer->addr->ainfo->ai_addr;
                      ctest = inet_ntop(AF_INET, &psin->sin_addr, trn_peer->chost, MSOCK_ADDR_LEN);
                      if (NULL!=ctest)
                        {
                        port = ntohs(psin->sin_port);

                        svc = port;
                        snprintf(trn_peer->service, NI_MAXSERV, "%d", svc);

                        msock_connection_t *pclient=NULL;
                        pclient = (msock_connection_t *)mlist_vlookup(trn_plist,
                          &svc,
                          mbtrn_peer_vcmp);
                        if (pclient!=NULL)
                          {
                          /*
                             PMPRINT(MOD_MBTRNPP,MM_DEBUG,(stderr,"updating
                             hbeat id[%d] plist[%p/%d]\n",svc,pp,pp->id));
                           */
                          /* client exists, update heartbeat tokens
                             [could make additive, i.e. +=]*/
                          pclient->heartbeat = trn_hbtok;
                          }
                        else
                          {
                          PMPRINT(MOD_MBTRNPP, MBTRNPP_V3,
                            (stderr, "adding to client list id[%d] addr[%p]\n", svc, trn_peer));
                          /* client doesn't exist
                             initialze and add to list*/
                          trn_peer->id = svc;
                          trn_peer->heartbeat = trn_hbtok;
                          trn_peer->next=NULL;
                          mlist_add(trn_plist, (void *)trn_peer);
                          /* save pointer to finish up (send ACK, update
                             hbeat) */
                          pclient=trn_peer;
                          /* create a new peer for next read */
                          trn_peer = msock_connection_new();

                          mlog_tprintf(trn_mlog_id,
                            "client connected id[%d] addr[%p]\n",
                            svc,
                            trn_peer);
                          MBTR_COUNTER_INC(app_stats->stats->events[
                              MBTPP_EV_CLI_CONN]);
                          }

                        PMPRINT(MOD_MBTRNPP, MBTRNPP_V2,
                          (stderr, "rx [%d]b cli[%d/%s:%s]\n", iobytes, svc, trn_peer->chost,
                          trn_peer->service));
                        /* mlog_tprintf(trn_mlog_id,"rx [%zd]b
                           cli[%d/%s:%s]\n",iobytes, svc, trn_peer->chost,
                           trn_peer->service); */

                        /* send ACK to client */
                        iobytes =msock_sendto(trn_osocket, pclient->addr, (byte *)"ACK", 4, 0 );
                        if ( (NULL!=pclient) && ( iobytes > 0) )
                          {
                          PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                            (stderr, "tx ACK [%d]b cli[%d/%s:%s]\n", iobytes, svc, pclient->chost,
                            pclient->service));
                          /* mlog_tprintf(trn_mlog_id,"tx ACK [%zd]b
                             cli[%d/%s:%s]\n",iobytes, svc,
                             pclient->chost, pclient->service); */
                          MBTR_COUNTER_INC(app_stats->stats->events[
                              MBTPP_EV_CLI_ACKN]);
                          MBTR_COUNTER_ADD(app_stats->stats->status[
                              MBTPP_STA_CLI_ACK_BYTES], iobytes);
                          }
                        else
                          {
                          /* ACK failed */
                          mlog_tprintf(trn_mlog_id,
                            "tx cli[%d] failed pclient[%p] iobytes[%zd] [%d/%s]\n",
                            svc,
                            pclient,
                            iobytes,
                            errno,
                            strerror(errno));
                          MBTR_COUNTER_INC(app_stats->stats->events[
                              MBTPP_EV_ECLI_ACK]);
                          }
                        }
                      else
                        {
                        mlog_tprintf(trn_mlog_id,
                          "err - inet_ntop failed [%d/%s]\n",
                          errno,
                          strerror(errno));
                        MBTR_COUNTER_INC(app_stats->stats->events[
                            MBTPP_EV_ENTOP]);
                        }
                      }
                    else
                      {
                      /* invalid sockaddr */
                      PMPRINT(MOD_MBTRNPP, MBTRNPP_V2,
                        (stderr, "err - NULL cliaddr(rx) cli[%d]\n", trn_peer->id));
                      mlog_tprintf(trn_mlog_id, "err - NULL cliaddr(rx) cli[%d]\n", trn_peer->id);
                      MBTR_COUNTER_INC(app_stats->stats->events[
                          MBTPP_EV_ECLIADDR_RX]);
                      }

                    break;
                    }                    /* switch iobytes */
                  }
                while(trn_recv_pending);

                if (delay_msec>0)
                  {
                  struct timespec delay={0};
                  struct timespec rem={0};
                  delay.tv_sec = delay_msec/1000;
                  delay.tv_nsec = (delay_msec%1000)*1000000L;
                  PMPRINT(MOD_MBTRNPP, MBTRNPP_V5,
                    (stderr, "delaying msec[%lld] sec:nsec[%ld:%ld]\n", delay_msec, delay.tv_sec,
                    delay.tv_nsec));
                  while (nanosleep(&delay, &rem)<0)
                    {
                    PMPRINT(MOD_MBTRNPP, MBTRNPP_V5, (stderr, "sleep interrupted\n"));
                    delay.tv_sec=rem.tv_sec;
                    delay.tv_nsec=rem.tv_nsec;
                    }
                  }

                MBTRNPP_UPDATE_STATS(app_stats, trn_mlog_id, MBTRNPP_STAT_FLAGS);
                }                /* end MBTRNPREPROCESS_OUTPUT_TRN */

              /* write the packet to a file */
              else if (output_mode == MBTRNPREPROCESS_OUTPUT_FILE)
                {
                fwrite(output_buffer, mb1_size, 1, ofp);
/*fprintf(stderr, "WRITE SIZE: %zu %zu %zu\n", mb1_size, index, index - mb1_size); */
                }
              }
            }
          }

        /* move data in buffer */
        if (ndata >= n_buffer_max)
          {
          ndata--;
          for (i = 0; i < n_buffer_max; i++)
            {
            ping[i].count--;
            if (ping[i].count < 0)
              idataread = i;
            }
          }
        else
          {
          idataread++;
          if (idataread >= n_buffer_max)
            idataread = 0;
          }
        }
      else
        {
        MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_MBGETFAIL_XT], mtime_dtime());
        PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
          (stderr, "mb_get_all failed: status[%d] kind[%d] err[%d]\n", status, kind, error));

        if ( (status == MB_FAILURE) && (error==MB_ERROR_EOF) && (input_mode == INPUT_MODE_SOCKET))
          {
          MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBGETALL]);

          fprintf(stderr, "EOF (input socket) - clear status/error\n");
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;

          /* check connection status
             only reconnect if disconnected*/
          mbtrn_reader_t *reader = ((struct mb_io_struct *)imbio_ptr)->mbsp;
          if (((NULL!=reader) && (reader->state==MBS_INITIALIZED)) || (me_errno==ME_ESOCK) ||
            (me_errno==ME_EOF)  )
            {
            MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ESRC_SOCKET]);

            /* empty the reader's record frame container */
            mbtrn_reader_purge(reader);
            fprintf(stderr, "mbtrnpp: input socket disconnected status[%s]\n",
              mbtrn_strstate(reader->state));
            mlog_tprintf(trn_mlog_id,
              "mbtrnpp: input socket disconnected status[%s]\n",
              mbtrn_strstate(reader->state));
            MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_DISN]);
            if (mbtrn_reader_connect(reader, true)==0)
              {
              fprintf(stderr, "mbtrnpp: input socket connected status[%s]\n",
                mbtrn_strstate(reader->state));
              mlog_tprintf(trn_mlog_id,
                "mbtrnpp: input socket connected status[%s]\n",
                mbtrn_strstate(reader->state));
              MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_CONN]);
              }
            else
              {
              fprintf(stderr, "mbtrnpp: input socket reconnect failed status[%s]\n",
                mbtrn_strstate(reader->state));
              mlog_tprintf(trn_mlog_id,
                "mbtrnpp: input socket reconnect failed status[%s]\n",
                mbtrn_strstate(reader->state));
              MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ESRC_CON]);

              struct timespec twait={0}, trem={0};
              twait.tv_sec=5;
              nanosleep(&twait, &trem);
              }
            }
          }
        MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_MBGETFAIL_XT], mtime_dtime());
        }

      MBTR_SW_START(app_stats->stats->measurements[MBTPP_CH_MBPOST_XT], mtime_dtime());

      if (status == MB_FAILURE)
        {
        /* if libmbtrn ESOCK error, then keep going */
        if ( (me_errno==ME_ESOCK) || (me_errno==ME_EOF) || (me_errno==ME_ERECV))
          {
          fprintf(stderr,
            "mbtrnpp: MB_FAILURE - me_error[%d/%s] (input socket may be disconnected)\n",
            me_errno,
            me_strerror(me_errno));
          /* try to reconnect to server */
          mbtrn_reader_t *reader = ((struct mb_io_struct *)imbio_ptr)->mbsp;
          /* empty the reader's record frame container */
          mbtrn_reader_purge(reader);

          fprintf(stderr,
            "mbtrnpp: MB_FAILURE - input socket disconnected status[%s]\n",
            mbtrn_strstate(reader->state));
          mlog_tprintf(trn_mlog_id, "input socket disconnected status[%s]\n",
            mbtrn_strstate(reader->state));

          MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_DISN]);

          if (mbtrn_reader_connect(reader, true)==0)
            {
            fprintf(stderr,
              "mbtrnpp: MB_FAILURE - input socket connected status[%s]\n",
              mbtrn_strstate(reader->state));
            mlog_tprintf(trn_mlog_id,
              "mbtrnpp: input socket connected status[%s]\n",
              mbtrn_strstate(reader->state));
            MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_CONN]);
            }
          else
            {
            fprintf(stderr,
              "mbtrnpp: MB_FAILURE - input socket reconnect failed status[%s]\n",
              mbtrn_strstate(reader->state));
            mlog_tprintf(trn_mlog_id,
              "mbtrnpp: MB_FAILURE - input socket reconnect failed status[%s]\n",
              mbtrn_strstate(reader->state));
            MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ESRC_CON]);

            struct timespec twait={0}, trem={0};
            twait.tv_sec=5;
            nanosleep(&twait, &trem);
            }
          }
        else if (error > 0)
          {
          fprintf(stderr, "mbtrnpp: MB_FAILURE - error>0 : setting done flag\n");
          done = MB_YES;
          MBTR_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBFAILURE]);
          }
        }
      MBTR_SW_LAP(app_stats->stats->measurements[MBTPP_CH_MBPOST_XT], mtime_dtime());
      }

    /* close the files */
    if (input_mode==INPUT_MODE_SOCKET)
      {
      fprintf(stderr, "socket input mode - continue (probably shouldn't be here)\n");
      read_data = MB_YES;
      }
    else
      {
      fprintf(stderr, "file input mode - file cleanup\n");
      status = mb_close(verbose, &imbio_ptr, &error);

      if (logfp != NULL)
        sprintf(log_message, "Multibeam File <%s> closed", ifile);

      mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);
      if (verbose != 0)
        fprintf(stderr, "\n%s\n", log_message);

      sprintf(log_message, "MBIO format id: %d", format);
      if (logfp != NULL)
        mbtrnpreprocess_postlog(verbose, logfp, log_message, &error);

      if (verbose > 0)
        fprintf(stderr, "%s\n", log_message);

      fflush(logfp);

      /* give the statistics
         figure out whether and what to read next*/
      if (read_datalist == MB_YES)
        {
        if ((status =
          mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight,
            &error)) == MB_SUCCESS)
          {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG,
            (stderr, "read_datalist status[%d] - continuing\n", status));
          read_data = MB_YES;
          }
        else
          {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "read_datalist status[%d] - done\n", status));
          read_data = MB_NO;
          }
        }
      else
        {
        PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "read_datalist == NO\n"));
        read_data = MB_NO;
        }
      }
    /* end loop over files in list */
    }

  fprintf(stderr, "exit loop\n");
  if (read_datalist == MB_YES)
    mb_datalist_close(verbose, &datalist, &error);

  /* close log file */
  gettimeofday(&timeofday, &timezone);
  now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
/*fprintf(stderr,"CHECKING AT BOTTOM OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n",
   logfp, log_file_open_time_d, now_time_d); */
  if (logfp != NULL)
    {
    status = mbtrnpreprocess_logstatistics(verbose,
      logfp,
      n_pings_read,
      n_soundings_read,
      n_soundings_valid_read,
      n_soundings_flagged_read,
      n_soundings_null_read,
      n_soundings_trimmed,
      n_soundings_decimated,
      n_soundings_flagged,
      n_soundings_written,
      &error);
    n_tot_pings_read += n_pings_read;
    n_tot_soundings_read += n_soundings_read;
    n_tot_soundings_valid_read += n_soundings_valid_read;
    n_tot_soundings_flagged_read += n_soundings_flagged_read;
    n_tot_soundings_null_read += n_soundings_null_read;
    n_tot_soundings_trimmed += n_soundings_trimmed;
    n_tot_soundings_decimated += n_soundings_decimated;
    n_tot_soundings_flagged += n_soundings_flagged;
    n_tot_soundings_written += n_soundings_written;
    n_pings_read = 0;
    n_soundings_read = 0;
    n_soundings_valid_read = 0;
    n_soundings_flagged_read = 0;
    n_soundings_null_read = 0;
    n_soundings_trimmed = 0;
    n_soundings_decimated = 0;
    n_soundings_flagged = 0;
    n_soundings_written = 0;

    status = mbtrnpreprocess_closelog(verbose, &logfp, &error);
    }

  /* close debug logs */
  mbtrnpreprocess_close_debug();

  /* close output */
  if (output_mode == MBTRNPREPROCESS_OUTPUT_FILE)
    fclose(ofp);

  /* check memory */
  if (verbose >= 4)
    status = mb_memory_list(verbose, &error);

  /* give the statistics */
  if (verbose >= 1)
    {
    }

  fprintf(stderr, "exit app [%d]\n", error);

  exit(error);
} /* main */
/*--------------------------------------------------------------------*/
