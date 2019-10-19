/*--------------------------------------------------------------------
 *    The MB-system:  mbtrnpp.c  2/19/2018
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
 * mbtrnpp - originally mbtrnpreprocess
 *
 * Author:  D. W. Caress
 * Date:  February 18, 2018
 */

#if defined(__CYGWIN__)
#include <Windows.h>
#endif

#include <arpa/inet.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mbsys_ldeoih.h"
#include "mbsys_kmbes.h"

#include "mconfig.h"
#include "r7kc.h"
#include "msocket.h"
#include "mtime.h"
#include "mlist.h"
#include "mlog.h"
#include "medebug.h"
#include "r7k-reader.h"
#include "mstats.h"
#ifdef WITH_MBTNAV
#include "trnw.h"
#endif // WITH_MBTNAV

/* ping structure definition */
struct mbtrnpp_ping_struct {
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
#define MBTRNPREPROCESS_OUTPUT_STDOUT 0
#define MBTRNPREPROCESS_OUTPUT_TRN 1
#define MBTRNPREPROCESS_OUTPUT_FILE 2

#define MBTRNPREPROCESS_MB1_HEADER_SIZE 56 // 52
#define MBTRNPREPROCESS_MB1_SOUNDING_SIZE 28
#define MBTRNPREPROCESS_MB1_CHECKSUM_SIZE 4

#define MBTRNPREPROCESS_LOGFILE_TIMELENGTH 900.0

int mbtrnpp_openlog(int verbose, mb_path log_directory, FILE **logfp, int *error);
int mbtrnpp_closelog(int verbose, FILE **logfp, int *error);
int mbtrnpp_postlog(int verbose, FILE *logfp, char *message, int *error);
int mbtrnpp_logparameters(int verbose, FILE *logfp, char *input, int format, char *output, double swath_width,
                          int n_output_soundings, int median_filter, int median_filter_n_across, int median_filter_n_along,
                          double median_filter_threshold, int n_buffer_max, int *error);
int mbtrnpp_logstatistics(int verbose, FILE *logfp, int n_pings_read, int n_soundings_read, int n_soundings_valid_read,
                          int n_soundings_flagged_read, int n_soundings_null_read, int n_soundings_trimmed,
                          int n_soundings_decimated, int n_soundings_flagged, int n_soundings_written, int *error);
int mbtrnpp_init_debug(int verbose);

int mbtrnpp_reson7kr_input_open(int verbose, void *mbio_ptr, char *definition, int *error);
int mbtrnpp_reson7kr_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
int mbtrnpp_reson7kr_input_close(int verbose, void *mbio_ptr, int *error);
int mbtrnpp_kemkmall_input_open(int verbose, void *mbio_ptr, char *definition, int *error);
int mbtrnpp_kemkmall_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
int mbtrnpp_kemkmall_input_close(int verbose, void *mbio_ptr, int *error);

static char program_name[] = "mbtrnpp";

mb_path socket_definition;

#define SONAR_SIM_HOST "localhost"

// TRN socket output configuration
#define TRN_HOST_DFL "localhost"
#define TRN_PORT_DFL 9999
#define TRN_MSG_CON_LEN 4
#define TRN_MAX_PEER 15
#define TRN_NPEERS (TRN_MAX_PEER + 1)
#define TRN_HBTOK_DFL 50
#define MBTPP 1
typedef enum { INPUT_MODE_SOCKET = 1, INPUT_MODE_FILE = 2 } input_mode_t;
input_mode_t input_mode;

msock_connection_t *trn_peer = NULL;
mlist_t *trn_plist = NULL;
msock_socket_t *trn_osocket = NULL;
int trn_oport = TRN_PORT_DFL;
char *trn_hostname = TRN_HOST_DFL;
int trn_hbtok = TRN_HBTOK_DFL;

#define TRN_BLOG_NAME "tbin"
#define TRN_BLOG_DESC "trn binary data"
#define TRN_MLOG_NAME "tmsg"
#define TRN_MLOG_DESC "trn message log"
#define MBR_BLOG_NAME "mbin"
#define MBR_BLOG_DESC "reader frame log"
#define TRN_LOG_EXT ".log"
#define SZ_1M (1024 * 1024)
#define SZ_1G (1024 * 1024 * 1024)
#define TRN_CMD_LINE_BYTES 2048
#define TRN_STATUS_INTERVAL_SEC_DFL 30

mlog_id_t trn_blog_id = MLOG_ID_INVALID;
mlog_id_t trn_mlog_id = MLOG_ID_INVALID;
mlog_id_t mbr_blog_id = MLOG_ID_INVALID;
mlog_config_t blog_conf = {100 * SZ_1M, ML_NOLIMIT, ML_NOLIMIT, ML_OSEG | ML_LIMLEN, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t mlog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t mbrlog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};
char session_date[32] = {0};
char *trn_blog_path = NULL;
char *trn_mlog_path = NULL;
mfile_flags_t flags = MFILE_RDWR | MFILE_APPEND | MFILE_CREATE;
mfile_mode_t mode = MFILE_RU | MFILE_WU | MFILE_RG | MFILE_WG;
bool trn_blog_en = true;
bool trn_mlog_en = true;
int64_t trn_pub_delay_msec = 0;
char g_cmd_line[TRN_CMD_LINE_BYTES] = {0};
char *g_log_dir = NULL;
char *mbr_blog_path = NULL;
bool mbr_blog_en = true;
FILE *mbr_blogs = NULL;

typedef enum {
  MBTPP_EV_CYCLES = 0,
  MBTPP_EV_EMBGETALL,
  MBTPP_EV_EMBFAILURE,
  MBTPP_EV_ESRC_SOCKET,
  MBTPP_EV_ESRC_CON,
  MBTPP_EV_ECLI_RXZ,
  MBTPP_EV_ECLI_RXE,
  MBTPP_EV_ECLI_TXZ,
  MBTPP_EV_ECLI_TXE,
  MBTPP_EV_ECLI_ACK,
  MBTPP_EV_ETRN_TX,
  MBTPP_EV_ECLIADDR_RX,
  MBTPP_EV_ENTOP,
  MBTPP_EV_SRC_CONN,
  MBTPP_EV_SRC_DISN,
  MBTPP_EV_CLI_CONN,
  MBTPP_EV_CLI_DISN,
  MBTPP_EV_CLI_RXN,
  MBTPP_EV_CLI_TXN,
  MBTPP_EV_CLI_ACKN,
  MBTPP_EV_TRN_PUBN,
  MBTPP_EV_LOG_STATN,
  MBTPP_EV_COUNT
} mbtrnpp_stevent_id;

typedef enum {
  MBTPP_STA_CLI_LIST_LEN,
  MBTPP_STA_CLI_ACK_BYTES,
  MBTPP_STA_CLI_RX_BYTES,
  MBTPP_STA_TRN_TX_BYTES,
  MBTPP_STA_TRN_PUB_BYTES,
  MBTPP_STA_COUNT
} mbtrnpp_ststatus_id;

typedef enum {
  MBTPP_CH_MBGETALL_XT = 0,
  MBTPP_CH_MBPING_XT,
  MBTPP_CH_TRNRX_XT,
  MBTPP_CH_TRNTX_XT,
  MBTPP_CH_LOG_XT,
  MBTPP_CH_DTIME_XT,
  MBTPP_CH_MBGETFAIL_XT,
  MBTPP_CH_MBPOST_XT,
  MBTPP_CH_STATS_XT,
  MBTPP_CH_CYCLE_XT,
  MBTPP_CH_THRUPUT,
  MBTPP_CH_COUNT
} mbtrnpp_stchan_id;

const char *mbtrnpp_stevent_labels[] = {"cycles",     "e_mbgetall", "e_mbfailure", "e_src_socket", "e_src_con", "e_cli_rx_z",
                                        "e_cli_rx_e", "e_cli_tx_z", "e_cli_tx_e",  "e_cli_ack",    "e_trn_tx",  "e_cliaddr_rx",
                                        "e_ntop",     "src_con",    "src_dis",     "cli_con",      "cli_dis",   "cli_rx",
                                        "cli_tx",     "cli_ack",    "trn_pub_n",   "log_stat"};
const char *mbtrnpp_ststatus_labels[] = {"cli_list_len", "cli_ack_bytes", "cli_rx_bytes", "trn_tx_bytes", "trn_pub_bytes"};

const char *mbtrnpp_stchan_labels[] = {"mbgetall_xt",  "mbping_xt", "trnrx_xt", "trntx_xt", "log_xt", "dtime_xt",
                                       "mbgetfail_xt", "mbpost_xt", "stats_xt", "cycle_xt", "thruput"};

const char **mbtrnpp_stats_labels[MSLABEL_COUNT] = {mbtrnpp_stevent_labels, mbtrnpp_ststatus_labels, mbtrnpp_stchan_labels};

mstats_profile_t *app_stats = NULL;
mstats_t *reader_stats = NULL;
double trn_status_interval_sec = MBTRNPP_STAT_PERIOD_SEC;
static double stats_prev_end = 0.0;
static double stats_prev_start = 0.0;
static bool log_clock_res = true;

#ifdef WITH_MBTNAV
trn_config_t *trn_cfg = NULL;
#define UTM_MONTEREY_BAY 10L
#define UTM_AXIAL 12L
#define TRN_UTM_DFL UTM_MONTEREY_BAY
#define TRN_MTYPE_DFL TRN_MAP_BO
#define TRN_FTYPE_DFL TRN_FILT_PARTICLE

bool trn_enable = false;
long int trn_utm_zone = TRN_UTM_DFL;
int trn_mtype = TRN_MTYPE_DFL;
int trn_ftype = TRN_FTYPE_DFL;
char *trn_map_file = NULL;
char *trn_cfg_file = NULL;
char *trn_particles_file = NULL;
char *trn_log_dir = NULL;
wtnav_t *tnav = NULL;
#endif // WITH_MBTNAV

#define MBTRNPP_MEAS_MOD ((double)0.0) // 3600.0

#ifdef MST_STATS_EN
#define MBTRNPP_UPDATE_STATS(p, l, f) (mbtrnpp_update_stats(p, l, f))
#else
#define MBTRNPP_UPDATE_STATS(p, l, f)
#endif // MST_STATS_EN

// MSF_STAT_FLAGS define stats processing options
// may include
// MSF_STATUS : status counters
// MSF_EVENT  : event/error counters
// MSF_ASTAT  : aggregate stats
// MSF_PSTAT  : periodic stats
// MSF_READER : r7kr reader stats
#define MBTRNPP_STAT_FLAGS (MSF_STATUS | MSF_EVENT | MSF_ASTAT)

int mbtrnpp_update_stats(mstats_profile_t *stats, mlog_id_t log_id, mstats_flags flags);

#ifdef WITH_MBTNAV

int mbtrnpp_init_trn(int verbose, trn_config_t *cfg);
int mbtrnpp_trn_process_mb1(wtnav_t *tnav, mb1_t *mb1, trn_config_t *cfg);
int mbtrnpp_trn_get_bias_estimates(wtnav_t *self, wposet_t *pt, pt_cdata_t **pt_out, pt_cdata_t **mle_out, pt_cdata_t **mse_out);
int mbtrnpp_trn_update(wtnav_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out, trn_config_t *cfg);
#endif // WITH_MBTNAV

char mRecordBuf[MBSYS_KMBES_MAX_NUM_MRZ_DGMS][64*1024];
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  char help_message[] = "mbtrnpp reads raw multibeam data, applies automated cleaning\n\t"
                        "and downsampling, and then passes the bathymetry on to a terrain relative navigation (TRN) process.\n";
  char usage_message[] = "mbtrnpp [\n"
                         "\t--verbose\n"
                         "\t--help\n"
                         "\t--log-directory=path\n"
                         "\t--input=datalist|file|socket_definition\n"
                         "\t--output=file|'socket'\n"
                         "\t--swathwidth=value\n"
                         "\t--soundings=value\n"
                         "\t--median-filter=threshold/nx/ny\n"
                         "\t--format=format\n"
                         "\t--platform-file\n"
                         "\t--platform-target-sensor\n"
                         "\t--projection=projection_id\n"
                         "\t--thost=hostname\n"
                         "\t--stats=n\n"
                         "\t--hbeat=n\n"
                         "\t--delay=n\n"
                         "\t--no-blog\n"
                         "\t--no-mlog\n"
                         "\t--no-rlog\n"
                         "\t--trn-en\n"
                         "\t--trn-utm\n"
                         "\t--trn-map\n"
                         "\t--trn-par\n"
                         "\t--trn-log\n"
                         "\t--trn-cfg\n"
                         "\t--trn-mtype\n"
                         "\t--trn-ftype\n";
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
  /* mbtrnpp
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
  static struct option options[] = {{"help", no_argument, NULL, 0},
                                    {"verbose", required_argument, NULL, 0},
                                    {"input", required_argument, NULL, 0},
                                    {"thost", required_argument, NULL, 0},
                                    {"hbeat", required_argument, NULL, 0},
                                    {"delay", required_argument, NULL, 0},
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
                                    {"trn-en", no_argument, NULL, 0},
                                    {"trn-utm", required_argument, NULL, 0},
                                    {"trn-map", required_argument, NULL, 0},
                                    {"trn-cfg", required_argument, NULL, 0},
                                    {"trn-par", required_argument, NULL, 0},
                                    {"trn-log", required_argument, NULL, 0},
                                    {"trn-mtype", required_argument, NULL, 0},
                                    {"trn-ftype", required_argument, NULL, 0},
                                    {NULL, 0, NULL, 0}};

  /* MBIO read control parameters */
  int read_datalist = false;
  int read_data = false;
  int read_socket = false;
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
  int use_platform_file = false;
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
  struct mbtrnpp_ping_struct ping[MBTRNPREPROCESS_BUFFER_DEFAULT];
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
  int median_filter = false;
  int median_filter_n_across = 1;
  int median_filter_n_along = 1;
  int median_filter_n_total = 1;
  int median_filter_n_min = 1;
  double median_filter_threshold = 0.05;
  double *median_filter_soundings = NULL;
  int n_median_filter_soundings = 0;
  double median;
  int n_output;

  /* output write control parameters */
  mb_path output;
  int output_mode = MBTRNPREPROCESS_OUTPUT_STDOUT;
  FILE *ofp = NULL;
  char *output_buffer = NULL;
  int n_output_buffer_alloc = 0;
  size_t mb1_size, index;
  unsigned int checksum;

  /* log file parameters */
  int make_logs = false;
  mb_path log_directory;
  FILE *logfp = NULL;
  mb_path log_message;
  double now_time_d;
  double log_file_open_time_d = 0.0;
  char date[32];
  struct stat logd_stat;
  int logd_status;

  /* function pointers for reading realtime sonar data using a socket */
  int (*mbtrnpp_input_open)(int verbose, void *mbio_ptr, char *definition, int *error);
  int (*mbtrnpp_input_read)(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
  int (*mbtrnpp_input_close)(int verbose, void *mbio_ptr, int *error);

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

  // make session time string to use
  // in log file names
  time_t rawtime;
  struct tm *gmt;

  time(&rawtime);
  // Get GMT time
  gmt = gmtime(&rawtime);
  // format YYYYMMDD-HHMMSS
  sprintf(session_date, "%04d%02d%02d-%02d%02d%02d", (gmt->tm_year + 1900), gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour,
          gmt->tm_min, gmt->tm_sec);

  memset(g_cmd_line, 0, TRN_CMD_LINE_BYTES);

  char *ip = g_cmd_line;
  int ilen = 0;
  for (int x = 0; x < argc; x++) {
    if ((ip + strlen(argv[x]) - g_cmd_line) > TRN_CMD_LINE_BYTES) {
      fprintf(stderr, "warning - logged cmdline truncated\n");
      break;
    }
    ilen = sprintf(ip, " %s", argv[x]);
    ip += ilen;
  }
  g_cmd_line[TRN_CMD_LINE_BYTES - 1] = '\0';
  g_log_dir = strdup("./");

  /* process argument list */
  while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
    switch (c) {
    /* long options all return c=0 */
    case 0:
      /* verbose */
      if (strcmp("verbose", options[option_index].name) == 0) {
        sscanf(optarg, "%d", &verbose); // verbose++;
      }

      /* help */
      else if (strcmp("help", options[option_index].name) == 0) {
        help = true;
      }

      /*-------------------------------------------------------
       * Define input file and format */

      /* input */
      else if (strcmp("input", options[option_index].name) == 0) {
        strcpy(input, optarg);
        if (strstr(input, "socket:")) {
          input_mode = INPUT_MODE_SOCKET;
          sscanf(input, "socket:%s", socket_definition);
fprintf(stderr, "socket_definition|%s\n", socket_definition);
        }
        else {
          input_mode = INPUT_MODE_FILE;
        }
      }
      else if (strcmp("thost", options[option_index].name) == 0) {
        char *ocopy = strdup(optarg);
        trn_hostname = strtok(ocopy, ":");
        if (trn_hostname == NULL) {
          trn_hostname = SONAR_SIM_HOST;
        }
        char *port = strtok(NULL, ":");
        if (port != NULL) {
          sscanf(ip, "%d", &trn_oport);
        }
        strcpy(output, "socket");
        output_mode = MBTRNPREPROCESS_OUTPUT_TRN;
        // don't free ocopy here
      }
      // heartbeat (pings)
      else if (strcmp("hbeat", options[option_index].name) == 0) {
        sscanf(optarg, "%d", &trn_hbtok);
      }
      // ping delay
      else if (strcmp("delay", options[option_index].name) == 0) {
        sscanf(optarg, "%lld", &trn_pub_delay_msec);
      }
      /* disable TRN binary log output */
      else if (strcmp("no-blog", options[option_index].name) == 0) {
        trn_blog_en = false;
      }
      /* disable TRN message log output */
      else if (strcmp("no-mlog", options[option_index].name) == 0) {
        trn_mlog_en = false;
      }
      /* disable TRN reader log output */
      else if (strcmp("no-rlog", options[option_index].name) == 0) {
        mbr_blog_en = false;
      }
      /* status log interval (s) */
      else if (strcmp("stats", options[option_index].name) == 0) {
        sscanf(optarg, "%lf", &trn_status_interval_sec);
      }
#ifdef WITH_MBTNAV
      /* TRN enable */
      else if (strcmp("trn-en", options[option_index].name) == 0) {
        trn_enable = true;
      }
      /* TRN UTM zone */
      else if (strcmp("trn-utm", options[option_index].name) == 0) {
        sscanf(optarg, "%ld", &trn_utm_zone);
      }
      /* TRN map type */
      else if (strcmp("trn-mtype", options[option_index].name) == 0) {
        sscanf(optarg, "%d", &trn_mtype);
      }
      /* TRN filter type */
      else if (strcmp("trn-ftype", options[option_index].name) == 0) {
        sscanf(optarg, "%d", &trn_ftype);
      }
      /* TRN map file */
      else if (strcmp("trn-map", options[option_index].name) == 0) {
        if (NULL != trn_map_file)
          free(trn_map_file);
        trn_map_file = strdup(optarg);
      }
      /* TRN config file */
      else if (strcmp("trn-cfg", options[option_index].name) == 0) {
        if (NULL != trn_cfg_file)
          free(trn_cfg_file);
        trn_cfg_file = strdup(optarg);
      }
      /* TRN particles file */
      else if (strcmp("trn-par", options[option_index].name) == 0) {
        if (NULL != trn_particles_file)
          free(trn_particles_file);
        trn_particles_file = strdup(optarg);
      }
      /* TRN log directory */
      else if (strcmp("trn-log", options[option_index].name) == 0) {
        if (NULL != trn_log_dir)
          free(trn_log_dir);
        trn_log_dir = strdup(optarg);
      }
#endif // WITH_MBTNAV

      /* format */
      else if (strcmp("format", options[option_index].name) == 0) {
        n = sscanf(optarg, "%d", &format);
      }

      /*-------------------------------------------------------
       * Set platform file */

      /* platform-file */
      else if (strcmp("platform-file", options[option_index].name) == 0) {
        n = sscanf(optarg, "%s", platform_file);
        if (n == 1)
          use_platform_file = true;
      }

      /* platform-target-sensor */
      else if (strcmp("platform-target-sensor", options[option_index].name) == 0) {
        n = sscanf(optarg, "%d", &target_sensor);
      }

      /*-------------------------------------------------------
       * Define processing parameters */

      /* output */
      else if ((strcmp("output", options[option_index].name) == 0)) {
        strcpy(output, optarg);
        if (strstr(output, "socket") != NULL) {
          output_mode = MBTRNPREPROCESS_OUTPUT_TRN;
        }
        else {
          output_mode = MBTRNPREPROCESS_OUTPUT_FILE;
        }
      }

      /* log-directory */
      else if ((strcmp("log-directory", options[option_index].name) == 0)) {
        strcpy(log_directory, optarg);
        logd_status = stat(log_directory, &logd_stat);
        if (logd_status != 0) {
          fprintf(stderr, "\nSpecified log file directory %s does not exist...\n", log_directory);
          make_logs = false;
        }
        else if ((logd_stat.st_mode & S_IFMT) != S_IFDIR) {
          fprintf(stderr, "\nSpecified log file directory %s is not a directory...\n", log_directory);
          make_logs = false;
        }
        else {
          make_logs = true;
          free(g_log_dir);
          g_log_dir = strdup(log_directory);
          fprintf(stderr, "\nusing log directory %s...\n", g_log_dir);
        }
      }

      /* swathwidth */
      else if ((strcmp("swath-width", options[option_index].name) == 0)) {
        n = sscanf(optarg, "%lf", &swath_width);
      }

      /* soundings */
      else if ((strcmp("soundings", options[option_index].name) == 0)) {
        n = sscanf(optarg, "%d", &n_output_soundings);
      }

      /* median-filter */
      else if ((strcmp("median-filter", options[option_index].name) == 0)) {
        n = sscanf(optarg, "%lf/%d/%d", &median_filter_threshold,
                            &median_filter_n_across, &median_filter_n_along);
        if (n == 3) {
          median_filter = true;
          n_buffer_max = median_filter_n_along;
        }
      }

      /*-------------------------------------------------------*/

      break;
    case '?':
      errflg++;
    }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }

  /* print starting message */
  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       help:                     %d\n", help);
    fprintf(stderr, "dbg2       pings:                    %d\n", pings);
    fprintf(stderr, "dbg2       lonflip:                  %d\n", lonflip);
    fprintf(stderr, "dbg2       bounds[0]:                %f\n", bounds[0]);
    fprintf(stderr, "dbg2       bounds[1]:                %f\n", bounds[1]);
    fprintf(stderr, "dbg2       bounds[2]:                %f\n", bounds[2]);
    fprintf(stderr, "dbg2       bounds[3]:                %f\n", bounds[3]);
    fprintf(stderr, "dbg2       btime_i[0]:               %d\n", btime_i[0]);
    fprintf(stderr, "dbg2       btime_i[1]:               %d\n", btime_i[1]);
    fprintf(stderr, "dbg2       btime_i[2]:               %d\n", btime_i[2]);
    fprintf(stderr, "dbg2       btime_i[3]:               %d\n", btime_i[3]);
    fprintf(stderr, "dbg2       btime_i[4]:               %d\n", btime_i[4]);
    fprintf(stderr, "dbg2       btime_i[5]:               %d\n", btime_i[5]);
    fprintf(stderr, "dbg2       btime_i[6]:               %d\n", btime_i[6]);
    fprintf(stderr, "dbg2       etime_i[0]:               %d\n", etime_i[0]);
    fprintf(stderr, "dbg2       etime_i[1]:               %d\n", etime_i[1]);
    fprintf(stderr, "dbg2       etime_i[2]:               %d\n", etime_i[2]);
    fprintf(stderr, "dbg2       etime_i[3]:               %d\n", etime_i[3]);
    fprintf(stderr, "dbg2       etime_i[4]:               %d\n", etime_i[4]);
    fprintf(stderr, "dbg2       etime_i[5]:               %d\n", etime_i[5]);
    fprintf(stderr, "dbg2       etime_i[6]:               %d\n", etime_i[6]);
    fprintf(stderr, "dbg2       speedmin:                 %f\n", speedmin);
    fprintf(stderr, "dbg2       timegap:                  %f\n", timegap);
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
    fprintf(stderr, "dbg2       socket_definition:        %s\n", socket_definition);
    fprintf(stderr, "dbg2       trn_hostname:             %s\n", trn_hostname);
    fprintf(stderr, "dbg2       trn_oport:                %d\n", trn_oport);
  }

  /* if help desired then print it and exit */
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error);
  }
#ifdef SOCKET_TIMING
  // print time message
  struct timeval stv = {0};
  gettimeofday(&stv, NULL);
  double start_sys_time = (double)stv.tv_sec + ((double)stv.tv_usec / 1000000.0) + (7 * 3600);
  fprintf(stderr, "%11.5lf systime %.4lf\n", mtime_dtime(), start_sys_time);
#endif

  mbtrnpp_init_debug(verbose);
#ifdef WITH_MBTNAV
  if (trn_enable && NULL != (trn_cfg = trncfg_new(NULL, -1, trn_utm_zone, trn_mtype, trn_ftype, trn_map_file, trn_cfg_file,
                                                  trn_particles_file, trn_log_dir))) {
    if (mbtrnpp_init_trn(verbose, trn_cfg) == 0) {
      fprintf(stderr, "TRN init OK\n");
    }
    else {
      fprintf(stderr, "TRN init failed\n");
    }
  }

  // release the config strings
  if (NULL != trn_map_file)
    free(trn_map_file);
  if (NULL != trn_cfg_file)
    free(trn_cfg_file);
  if (NULL != trn_particles_file)
    free(trn_particles_file);
  if (NULL != trn_log_dir)
    free(trn_log_dir);

  trncfg_show(trn_cfg, true, 5);
#endif // WITH_MBTNAV

  /* load platform definition if specified */
  if (use_platform_file == true) {
    status = mb_platform_read(verbose, platform_file, (void **)&platform, &error);
    if (status == MB_FAILURE) {
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
  if (output_mode == MBTRNPREPROCESS_OUTPUT_STDOUT) {
  }
  /* insert option to recognize and initialize ipc with TRN */
  /* else open ipc to TRN */
  else if (output_mode == MBTRNPREPROCESS_OUTPUT_TRN) {
    //        md_level_t olvl;
    mmd_en_mask_t olvl;
    if (verbose != 0) {
      olvl = mmd_get_enmask(MOD_MBTRNPP, NULL);
      mmd_channel_en(MOD_MBTRNPP, MM_DEBUG);
      //            olvl = mdb_get(APP,NULL);
      //            mdb_set(APP,MDL_DEBUG);
    }
    trn_peer = msock_connection_new();
    trn_plist = mlist_new();
    mlist_autofree(trn_plist, msock_connection_free);
    PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "configuring TRN host using %s:%d\n", trn_hostname, trn_oport));
    trn_osocket = msock_socket_new(trn_hostname, trn_oport, ST_UDP);
    msock_set_blocking(trn_osocket, false);
    int test = -1;
    if ((test = msock_bind(trn_osocket)) == 0) {
      PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "TRN host socket bind OK [%s:%d]\n", TRN_HOST_DFL, TRN_PORT_DFL));
    }
    else {
      fprintf(stderr, "\nTRN host socket bind failed [%d] [%d %s]\n", test, errno, strerror(errno));
    }
    if (verbose != 0) {
      mmd_channel_set(MOD_MBTRNPP, olvl);
      //        mdb_set(APP,olvl);
    }
  }
  /* else open output file in which the binary data otherwise communicated
   * to TRN will be saved */
  else {
    ofp = fopen(output, "w");
  }

  /* get number of ping records to hold */
  if (median_filter == true) {
    median_filter_n_total = median_filter_n_across * median_filter_n_along;
    median_filter_n_min = median_filter_n_total / 2;

    /* allocate memory for median filter */
    if (error == MB_ERROR_NO_ERROR) {
      status = mb_mallocd(verbose, __FILE__, __LINE__, median_filter_n_total * sizeof(double),
                          (void **)&median_filter_soundings, &error);
      if (error != MB_ERROR_NO_ERROR) {
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
    read_datalist = true;

  /* open file list */
  if (read_datalist == true) {
    if ((status = mb_datalist_open(verbose, &datalist, input, look_processed, &error)) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open data list file: %s\n", input);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }
    if ((status = mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
      read_data = true;
    else
      read_data = false;
  }
  /* else copy single filename to be read */
  else {
    strcpy(ifile, input);
    read_data = true;
  }
  // kick off the first cycle here
  // future cycles start and end in the stats update
  MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_CYCLE_XT], mtime_dtime());
  MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_STATS_XT], mtime_dtime());
  /* loop over all files to be read */
  while (read_data == true) {

    /* open log file if specified */
    if (make_logs == true) {
      gettimeofday(&timeofday, &timezone);
      now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
      // fprintf(stderr,"CHECKING AT TOP OF LOOP: logfp:%p log_file_open_time_d:%.6ff now_time_d:%.6f\n", logfp,
      // log_file_open_time_d, now_time_d);
      if (logfp == NULL || (now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) {
        if (logfp != NULL) {
          status = mbtrnpp_logstatistics(verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                         n_soundings_flagged_read, n_soundings_null_read, n_soundings_trimmed,
                                         n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
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

          status = mbtrnpp_closelog(verbose, &logfp, &error);
        }

        status = mbtrnpp_openlog(verbose, log_directory, &logfp, &error);
        if (status == MB_SUCCESS) {
          gettimeofday(&timeofday, &timezone);
          log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
          status = mbtrnpp_logparameters(verbose, logfp, input, format, output, swath_width, n_output_soundings,
                                         median_filter, median_filter_n_across, median_filter_n_along,
                                         median_filter_threshold, n_buffer_max, &error);
        }
        else {
          fprintf(stderr, "\nLog file could not be opened in directory %s...\n", log_directory);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }
      }
    }

    /* check for format with amplitude or sidescan data */
    status = mb_format_system(verbose, &format, &system, &error);
    status = mb_format_dimensions(verbose, &format, &beams_bath, &beams_amp, &pixels_ss, &error);

    /* initialize reading the input swath data over a socket interface
     * using functions defined in this code block and passed into the
     * init function as function pointers */
    if (strncmp(input, "socket", 6) == 0) {
      if (format == MBF_RESON7KR) {
        mbtrnpp_input_open = &mbtrnpp_reson7kr_input_open;
        mbtrnpp_input_read = &mbtrnpp_reson7kr_input_read;
        mbtrnpp_input_close = &mbtrnpp_reson7kr_input_close;
      }
      else if (format == MBF_KEMKMALL) {
        mbtrnpp_input_open = &mbtrnpp_kemkmall_input_open;
        mbtrnpp_input_read = &mbtrnpp_kemkmall_input_read;
        mbtrnpp_input_close = &mbtrnpp_kemkmall_input_close;
      }
      if ((status = mb_input_init(verbose, socket_definition, format, pings, lonflip, bounds,
                                  btime_i, etime_i, speedmin, timegap,
                                  &imbio_ptr, &btime_d, &etime_d,
                                  &beams_bath, &beams_amp, &pixels_ss,
                                  mbtrnpp_input_open, mbtrnpp_input_read, mbtrnpp_input_close,
                                  &error) != MB_SUCCESS)) {

        sprintf(log_message, "MBIO Error returned from function <mb_input_init>");
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(verbose, error, &message);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Sonar data socket <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        exit(error);
      }
      else {

        sprintf(log_message, "Sonar data socket <%s> initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "MBIO format id: %d", format);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "%s\n", log_message);
      }
    }

    /* otherwised open swath data files as is normal for MB-System programs */
    else {

      if ((status = mb_read_init(verbose, ifile, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                                 &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) !=
          MB_SUCCESS) {

        sprintf(log_message, "MBIO Error returned from function <mb_read_init>");
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(verbose, error, &message);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Sonar File <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        exit(error);
      }
      else {
        sprintf(log_message, "Sonar File <%s> initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "MBIO format id: %d", format);
        if (logfp != NULL)
          mbtrnpp_postlog(verbose, logfp, log_message, &error);
        if (verbose > 0)
          fprintf(stderr, "%s\n", log_message);
      }
    }

    /* allocate memory for data arrays */
    memset(ping, 0, MBTRNPREPROCESS_BUFFER_DEFAULT * sizeof(struct mbtrnpp_ping_struct));
    for (i = 0; i < n_buffer_max; i++) {
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&ping[i].beamflag,
                                   &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                   (void **)&ping[i].beamflag_filter, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bath, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].amp, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&ping[i].bathacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&ping[i].bathalongtrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ss, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                   (void **)&ping[i].ssacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                   (void **)&ping[i].ssalongtrack, &error);
    }

    /* plan on storing enough pings for median filter */
    n_buffer_max = median_filter_n_along;
    n_ping_process = n_buffer_max / 2;

    /* loop over reading data */
    done = false;
    idataread = 0;

    while (!done) {
      /* open new log file if it is time */
      if (make_logs == true) {

        gettimeofday(&timeofday, &timezone);
        now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
        // fprintf(stderr,"CHECKING AT MIDDLE OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n", logfp,
        // log_file_open_time_d, now_time_d);
        if (logfp == NULL || (now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) {
          if (logfp != NULL) {
            status = mbtrnpp_logstatistics(verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                           n_soundings_flagged_read, n_soundings_null_read, n_soundings_trimmed,
                                           n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
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

            status = mbtrnpp_closelog(verbose, &logfp, &error);
          }

          status = mbtrnpp_openlog(verbose, log_directory, &logfp, &error);
          if (status == MB_SUCCESS) {
            gettimeofday(&timeofday, &timezone);
            log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
            status = mbtrnpp_logparameters(verbose, logfp, input, format, output, swath_width, n_output_soundings,
                                           median_filter, median_filter_n_across, median_filter_n_along,
                                           median_filter_threshold, n_buffer_max, &error);
          }
          else {
            fprintf(stderr, "\nLog file could not be opened in directory %s...\n", log_directory);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }
      }

      /* read the next data */
      error = MB_ERROR_NO_ERROR;

      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MBGETALL_XT], mtime_dtime());
      status = mb_get_all(verbose, imbio_ptr, &store_ptr, &kind, ping[idataread].time_i, &ping[idataread].time_d,
                          &ping[idataread].navlon, &ping[idataread].navlat, &ping[idataread].speed,
                          &ping[idataread].heading, &ping[idataread].distance, &ping[idataread].altitude,
                          &ping[idataread].sonardepth, &ping[idataread].beams_bath, &ping[idataread].beams_amp,
                          &ping[idataread].pixels_ss, ping[idataread].beamflag, ping[idataread].bath, ping[idataread].amp,
                          ping[idataread].bathacrosstrack, ping[idataread].bathalongtrack, ping[idataread].ss,
                          ping[idataread].ssacrosstrack, ping[idataread].ssalongtrack, comment, &error);

      //            PMPRINT(MOD_MBTRNPP,MBTRNPP_V4,(stderr,"mb_get_all - status[%d] kind[%d] err[%d]\n",status, kind,
      //            error));
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MBGETALL_XT], mtime_dtime());
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MBPING_XT], mtime_dtime());

      if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
        ping[idataread].count = ndata;
        ndata++;
        n_pings_read++;
        n_soundings_read += ping[idataread].beams_bath;
        for (i = 0; i < ping[idataread].beams_bath; i++) {
          ping[idataread].beamflag_filter[i] = ping[idataread].beamflag[i];
          if (mb_beam_ok(ping[idataread].beamflag[i])) {
            n_soundings_valid_read++;
          }
          else if (ping[idataread].beamflag[i] == MB_FLAG_NULL) {
            n_soundings_null_read++;
          }
          else {
            n_soundings_flagged_read++;
          }
        }

        status = mb_extract_nav(verbose, imbio_ptr, store_ptr, &kind, ping[idataread].time_i, &ping[idataread].time_d,
                                &ping[idataread].navlon, &ping[idataread].navlat, &ping[idataread].speed,
                                &ping[idataread].heading, &ping[idataread].sonardepth, &ping[idataread].roll,
                                &ping[idataread].pitch, &ping[idataread].heave, &error);
        status = mb_extract_altitude(verbose, imbio_ptr, store_ptr, &kind, &ping[idataread].sonardepth,
                                     &ping[idataread].altitude, &error);

        /* only process and output if enough data have been read */
        if (ndata == n_buffer_max) {
          for (i = 0; i < n_buffer_max; i++) {
            if (ping[i].count == n_ping_process)
              i_ping_process = i;
          }
          // fprintf(stdout, "\nProcess some data: ndata:%d counts: ", ndata);
          // for (i = 0; i < n_buffer_max; i++) {
          //    fprintf(stdout,"%d ", ping[i].count);
          //}
          // fprintf(stdout," : process %d\n", i_ping_process);

          /* apply swath width */
          threshold_tangent = tan(DTR * 0.5 * swath_width);
          beam_start = ping[i_ping_process].beams_bath - 1;
          beam_end = 0;
          for (j = 0; j < ping[i_ping_process].beams_bath; j++) {
            if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
              tangent = ping[i_ping_process].bathacrosstrack[j] /
                        (ping[i_ping_process].bath[j] - ping[i_ping_process].sonardepth);
              if (fabs(tangent) > threshold_tangent && mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                n_soundings_trimmed++;
              }
              else {
                beam_start = MIN(beam_start, j);
                beam_end = MAX(beam_end, j);
              }
            }
          }

          /* apply decimation - only consider outputting decimated soundings */
          beam_decimation = ((beam_end - beam_start + 1) / n_output_soundings) + 1;
          dj = median_filter_n_across / 2;
          di = median_filter_n_along / 2;
          n_output = 0;
          for (j = beam_start; j <= beam_end; j++) {
            if ((j - beam_start) % beam_decimation == 0) {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                /* apply median filtering to this sounding */
                if (median_filter_n_total > 1) {
                  /* accumulate soundings for median filter */
                  n_median_filter_soundings = 0;
                  jj0 = MAX(beam_start, j - dj);
                  jj1 = MIN(beam_end, j + dj);
                  for (ii = 0; ii < n_buffer_max; ii++) {
                    for (jj = jj0; jj <= jj1; jj++) {
                      if (mb_beam_ok(ping[ii].beamflag[jj])) {
                        median_filter_soundings[n_median_filter_soundings] = ping[ii].bath[jj];
                        n_median_filter_soundings++;
                      }
                    }
                  }

                  /* run qsort */
                  qsort((char *)median_filter_soundings, n_median_filter_soundings, sizeof(double),
                        (void *)mb_double_compare);
                  median = median_filter_soundings[n_median_filter_soundings / 2];
                  // fprintf(stdout, "Beam %3d of %d:%d bath:%.3f n:%3d:%3d median:%.3f ", j, beam_start,
                  // beam_end, ping[i_ping_process].bath[j], n_median_filter_soundings, median_filter_n_min,
                  // median);

                  /* apply median filter - also flag soundings that don't have enough neighbors to filter */
                  if (n_median_filter_soundings < median_filter_n_min ||
                      fabs(ping[i_ping_process].bath[j] - median) > median_filter_threshold * median) {
                    ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    n_soundings_flagged++;

                    // fprintf(stdout, "**filtered**");
                  }
                  // fprintf(stdout, "\n");
                }
                if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                  n_output++;
                }
              }
            }
            else if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
              ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
              n_soundings_decimated++;
            }
          }

          /* write out results to stdout as text */
          if (output_mode == MBTRNPREPROCESS_OUTPUT_STDOUT) {
            fprintf(stdout, "Ping: %.9f %.7f %.7f %.3f %.3f %4d\n", ping[i_ping_process].time_d,
                    ping[i_ping_process].navlat, ping[i_ping_process].navlon, ping[i_ping_process].sonardepth,
                    (double)(DTR * ping[i_ping_process].heading), n_output);
            for (j = 0; j < ping[i_ping_process].beams_bath; j++) {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                fprintf(stdout, "%3.3d starboard:%.3f forward:%.3f down:%.3f\n", j,
                        ping[i_ping_process].bathacrosstrack[j], ping[i_ping_process].bathalongtrack[j],
                        ping[i_ping_process].bath[j] - ping[i_ping_process].sonardepth);
                n_soundings_written++;
              }
            }
          }
          /* pack the data into a TRN MB1 packet and either send it to TRN or write it to a file */
          else {
            n_soundings_written++;

            /* make sure buffer is large enough to hold the packet */
            mb1_size = MBTRNPREPROCESS_MB1_HEADER_SIZE + n_output * MBTRNPREPROCESS_MB1_SOUNDING_SIZE +
                       MBTRNPREPROCESS_MB1_CHECKSUM_SIZE;
            if (n_output_buffer_alloc < mb1_size) {
              if ((status = mb_reallocd(verbose, __FILE__, __LINE__, mb1_size, (void **)&output_buffer, &error)) ==
                  MB_SUCCESS) {
                n_output_buffer_alloc = mb1_size;
              }
              else {
                mb_error(verbose, error, &message);
                fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
                fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
                exit(error);
              }
            }

            // get ping number
            mb_pingnumber(verbose, imbio_ptr, &ping_number, &error);

            /* now pack the data into the packet buffer */
            index = 0;
            output_buffer[index] = 'M';
            index++;
            output_buffer[index] = 'B';
            index++;
            output_buffer[index] = '1';
            index++;
            output_buffer[index] = 0;
            index++;
            mb_put_binary_int(true, mb1_size, &output_buffer[index]);
            index += 4;

            mb_put_binary_double(true, ping[i_ping_process].time_d, &output_buffer[index]);
            index += 8;
            mb_put_binary_double(true, ping[i_ping_process].navlat, &output_buffer[index]);
            index += 8;
            mb_put_binary_double(true, ping[i_ping_process].navlon, &output_buffer[index]);
            index += 8;
            mb_put_binary_double(true, ping[i_ping_process].sonardepth, &output_buffer[index]);
            index += 8;
            mb_put_binary_double(true, (double)(DTR * ping[i_ping_process].heading), &output_buffer[index]);
            index += 8;

            mb_put_binary_int(true, ping_number, &output_buffer[index]);
            index += 4;

            mb_put_binary_int(true, n_output, &output_buffer[index]);
            index += 4;

            PMPRINT(MOD_MBTRNPP, MBTRNPP_V1,
                    (stderr,
                     "\nts[%.3lf] beams[%03d] ping[%06u]\nlat[%.4lf] lon[%.4lf] hdg[%6.2lf] sd[%7.2lf]\nv[%+6.2lf] "
                     "p/r/y[%.3lf / %.3lf / %.3lf]\n",
                     ping[i_ping_process].time_d, n_output, ping_number, ping[i_ping_process].navlat,
                     ping[i_ping_process].navlon, (double)(DTR * ping[i_ping_process].heading),
                     ping[i_ping_process].sonardepth, ping[i_ping_process].speed, ping[i_ping_process].pitch,
                     ping[i_ping_process].roll, ping[i_ping_process].heave));

            for (j = 0; j < ping[i_ping_process].beams_bath; j++) {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {

                mb_put_binary_int(true, j, &output_buffer[index]);
                index += 4;
                mb_put_binary_double(true, ping[i_ping_process].bathalongtrack[j], &output_buffer[index]);
                index += 8;
                mb_put_binary_double(true, ping[i_ping_process].bathacrosstrack[j], &output_buffer[index]);
                index += 8;
                //                                mb_put_binary_double(true, ping[i_ping_process].bath[j],
                //                                &output_buffer[index]); index += 8;
                // subtract sonar depth from vehicle bathy; changed 12jul18 cruises
                mb_put_binary_double(true, (ping[i_ping_process].bath[j] - ping[i_ping_process].sonardepth),
                                     &output_buffer[index]);
                index += 8;

                PMPRINT(MOD_MBTRNPP, MBTRNPP_V2,
                        (stderr, "n[%03d] atrk/X[%+10.3lf] ctrk/Y[%+10.3lf] dpth/Z[%+10.3lf]\n", j,
                         ping[i_ping_process].bathalongtrack[j], ping[i_ping_process].bathacrosstrack[j],
                         (ping[i_ping_process].bath[j] - ping[i_ping_process].sonardepth)));
              }
            }

            /* add the checksum */
            checksum = 0;
            unsigned char *cp = (unsigned char *)output_buffer;
            for (j = 0; j < index; j++) {
              //                            checksum += (unsigned int) output_buffer[j];
              checksum += (unsigned int)(*cp++);
            }

            mb_put_binary_int(true, checksum, &output_buffer[index]);
            index += 4;
            PMPRINT(MOD_MBTRNPP, MBTRNPP_V3, (stderr, "chk[%08X] idx[%zu] mb1sz[%zu]\n", checksum, index, mb1_size));
#ifdef WITH_MBTNAV

            // update TRN [TODO: don't send every sounding]
            if (trn_enable) {
              mb1_t *mb1 = (mb1_t *)output_buffer;
              mbtrnpp_trn_process_mb1(tnav, mb1, trn_cfg);
            }
#endif // WITH_MBTNAV

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MBPING_XT], mtime_dtime());

            /* send the packet to TRN */
            if (output_mode == MBTRNPREPROCESS_OUTPUT_TRN) {

              MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CYCLES]);

              //                            struct timeval stv={0};
              //                            gettimeofday(&stv,NULL);
              //                            double stime = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0);
              //                            double ptime=ping[i_ping_process].time_d;
              //                            fprintf(stderr,"mbtx : ptime[%.3lf] stime[%.3lf]
              //                            (s-p)[%+6.3lf]**\n",ptime,stime,(stime-ptime)); fprintf(stderr,"mbtx :
              //                            (s-p)[%+6.3lf]**\n",(stime-ptime));

              // log current TRN message
              if (trn_blog_en) {
                mlog_write(trn_blog_id, (byte *)output_buffer, mb1_size);
              }

              // send output to TRN clients
              MST_COUNTER_SET(app_stats->stats->status[MBTPP_STA_CLI_LIST_LEN], mlist_size(trn_plist));
              MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRNTX_XT], mtime_dtime());

              int iobytes = 0;
              int test = -1;
              int idx = -1;
              msock_connection_t *psub = (msock_connection_t *)mlist_first(trn_plist);
              idx = 0;
              while (psub != NULL) {

                psub->heartbeat--;

                iobytes = msock_sendto(trn_osocket, psub->addr, (byte *)output_buffer, mb1_size, 0);

                if (iobytes > 0) {
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_TXN]);
                  MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_TRN_TX_BYTES], iobytes);
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_TRN_PUBN]);
                  MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_TRN_PUB_BYTES], iobytes);

                  PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                          (stderr, "tx TRN [%5d]b cli[%d/%s:%s] hb[%d]\n", iobytes, idx, psub->chost,
                           psub->service, psub->heartbeat));
                }
                else {
                  PEPRINT(
                      (stderr, "err - sendto ret[%d] cli[%d] [%d/%s]\n", iobytes, idx, errno, strerror(errno)));
                  mlog_tprintf(trn_mlog_id, "err - sendto ret[%d] cli[%d] [%d/%s]\n", iobytes, idx, errno,
                               strerror(errno));
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ETRN_TX]);
                }

                // check heartbeat, remove expired peers
                if (NULL != psub && psub->heartbeat == 0) {
                  PMPRINT(MOD_MBTRNPP, MBTRNPP_V4, (stderr, "hbeat=0 cli[%d/%d] - removed\n", idx, psub->id));
                  mlog_tprintf(trn_mlog_id, "hbeat=0 cli[%d/%d] - removed\n", idx, psub->id);
                  mlist_remove(trn_plist, psub);
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_DISN]);
                  MST_COUNTER_SET(app_stats->stats->status[MBTPP_STA_CLI_LIST_LEN], mlist_size(trn_plist));
                }
                psub = (msock_connection_t *)mlist_next(trn_plist);
                idx++;
              } // while psub

              MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRNTX_XT], mtime_dtime());

              // check trn socket for client messages (connection, heartbeat)
              byte cmsg[TRN_MSG_CON_LEN];
              int svc = 0;

              PMPRINT(MOD_MBTRNPP, MBTRNPP_V4, (stderr, "checking trn host socket\n"));
              bool trn_recv_pending = true;
              do {
                MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRNRX_XT], mtime_dtime());

                iobytes = msock_recvfrom(trn_osocket, trn_peer->addr, cmsg, TRN_MSG_CON_LEN, 0);

                MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRNRX_XT], mtime_dtime());

                switch (iobytes) {
                case 0:
                  // no bytes returned (socket closed)
                  PMPRINT(MOD_MBTRNPP, MM_DEBUG,
                          (stderr, "err - recvfrom ret 0 (socket closed) removing cli[%d]\n", trn_peer->id));
                  mlog_tprintf(trn_mlog_id, "recvfrom ret 0 (socket closed) removing cli[%d]\n", trn_peer->id);
                  // remove from list
                  if (sscanf(trn_peer->service, "%d", &svc) == 1) {
                    msock_connection_t *peer =
                        (msock_connection_t *)mlist_vlookup(trn_plist, &svc, r7kr_peer_vcmp);
                    if (peer != NULL) {
                      mlist_remove(trn_plist, peer);
                    }
                  }
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLI_RXZ]);
                  trn_recv_pending = false;
                  break;
                case -1:
                  // error (usually nothing to read)
                  if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                            (stderr, "err - recvfrom cli[%d] ret -1 [%d/%s]\n", trn_peer->id, errno,
                             strerror(errno)));
                  }
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLI_RXE]);
                  trn_recv_pending = false;
                  break;

                default:
                  // bytes received
                  MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_CLI_RX_BYTES], iobytes);
                  MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_RXN]);

                  const char *ctest = NULL;
                  uint16_t port = 0xFFFF;
                  struct sockaddr_in *psin = NULL;
                  if (NULL != trn_peer->addr && NULL != trn_peer->addr->ainfo &&
                      NULL != trn_peer->addr->ainfo->ai_addr) {

                    psin = (struct sockaddr_in *)trn_peer->addr->ainfo->ai_addr;
                    ctest = inet_ntop(AF_INET, &psin->sin_addr, trn_peer->chost, MSOCK_ADDR_LEN);
                    if (NULL != ctest) {

                      port = ntohs(psin->sin_port);

                      svc = port;
                      snprintf(trn_peer->service, NI_MAXSERV, "%d", svc);

                      msock_connection_t *pclient = NULL;
                      pclient = (msock_connection_t *)mlist_vlookup(trn_plist, &svc, r7kr_peer_vcmp);
                      if (pclient != NULL) {
                        // PMPRINT(MOD_MBTRNPP,MM_DEBUG,(stderr,"updating hbeat id[%d]
                        // plist[%p/%d]\n",svc,pp,pp->id)); client exists, update heartbeat tokens [could
                        // make additive, i.e. +=]
                        pclient->heartbeat = trn_hbtok;
                      }
                      else {
                        PMPRINT(MOD_MBTRNPP, MBTRNPP_V3,
                                (stderr, "adding to client list id[%d] addr[%p]\n", svc, trn_peer));
                        // client doesn't exist
                        // initialze and add to list
                        trn_peer->id = svc;
                        trn_peer->heartbeat = trn_hbtok;
                        trn_peer->next = NULL;
                        mlist_add(trn_plist, (void *)trn_peer);
                        // save pointer to finish up (send ACK, update hbeat)
                        pclient = trn_peer;
                        // create a new peer for next read
                        trn_peer = msock_connection_new();

                        mlog_tprintf(trn_mlog_id, "client connected id[%d] addr[%p]\n", svc, trn_peer);
                        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_CONN]);
                      }

                      PMPRINT(MOD_MBTRNPP, MBTRNPP_V2,
                              (stderr, "rx [%d]b cli[%d/%s:%s]\n", iobytes, svc, trn_peer->chost,
                               trn_peer->service));
                      // mlog_tprintf(trn_mlog_id,"rx [%zd]b cli[%d/%s:%s]\n",iobytes, svc, trn_peer->chost,
                      // trn_peer->service);

                      // send ACK to client
                      iobytes = msock_sendto(trn_osocket, pclient->addr, (byte *)"ACK", 4, 0);
                      if ((NULL != pclient) && (iobytes > 0)) {

                        PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                                (stderr, "tx ACK [%d]b cli[%d/%s:%s]\n", iobytes, svc, pclient->chost,
                                 pclient->service));
                        // mlog_tprintf(trn_mlog_id,"tx ACK [%zd]b cli[%d/%s:%s]\n",iobytes, svc,
                        // pclient->chost, pclient->service);
                        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_CLI_ACKN]);
                        MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_CLI_ACK_BYTES], iobytes);
                      }
                      else {
                        // ACK failed
                        mlog_tprintf(trn_mlog_id, "tx cli[%d] failed pclient[%p] iobytes[%zd] [%d/%s]\n",
                                     svc, pclient, iobytes, errno, strerror(errno));
                        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLI_ACK]);
                      }
                    }
                    else {
                      mlog_tprintf(trn_mlog_id, "err - inet_ntop failed [%d/%s]\n", errno, strerror(errno));
                      MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ENTOP]);
                    }
                  }
                  else {
                    // invalid sockaddr
                    PMPRINT(MOD_MBTRNPP, MBTRNPP_V2,
                            (stderr, "err - NULL cliaddr(rx) cli[%d]\n", trn_peer->id));
                    mlog_tprintf(trn_mlog_id, "err - NULL cliaddr(rx) cli[%d]\n", trn_peer->id);
                    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ECLIADDR_RX]);
                  }

                  break;
                } // switch iobytes
              } while (trn_recv_pending);

              if (trn_pub_delay_msec > 0) {
                struct timespec delay = {0};
                struct timespec rem = {0};
                delay.tv_sec = trn_pub_delay_msec / 1000;
                delay.tv_nsec = (trn_pub_delay_msec % 1000) * 1000000L;
                PMPRINT(MOD_MBTRNPP, MBTRNPP_V5,
                        (stderr, "delaying msec[%lld] sec:nsec[%ld:%ld]\n", trn_pub_delay_msec, delay.tv_sec,
                         delay.tv_nsec));
                while (nanosleep(&delay, &rem) < 0) {
                  PMPRINT(MOD_MBTRNPP, MBTRNPP_V5, (stderr, "sleep interrupted\n"));
                  delay.tv_sec = rem.tv_sec;
                  delay.tv_nsec = rem.tv_nsec;
                }
              }

              MBTRNPP_UPDATE_STATS(app_stats, trn_mlog_id, MBTRNPP_STAT_FLAGS);

            } // end MBTRNPREPROCESS_OUTPUT_TRN

            /* write the packet to a file */
            else if (output_mode == MBTRNPREPROCESS_OUTPUT_FILE) {
              fwrite(output_buffer, mb1_size, 1, ofp);
              // fprintf(stderr, "WRITE SIZE: %zu %zu %zu\n", mb1_size, index, index - mb1_size);
            }
          }
        }

        /* move data in buffer */
        if (ndata >= n_buffer_max) {
          ndata--;
          for (i = 0; i < n_buffer_max; i++) {
            ping[i].count--;
            if (ping[i].count < 0) {
              idataread = i;
            }
          }
        }
        else {
          idataread++;
          if (idataread >= n_buffer_max)
            idataread = 0;
        }
      }
      else {

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MBGETFAIL_XT], mtime_dtime());
        PMPRINT(MOD_MBTRNPP, MBTRNPP_V4,
                (stderr, "mb_get_all failed: status[%d] kind[%d] err[%d]\n", status, kind, error));

        if ((status == MB_FAILURE) && (error == MB_ERROR_EOF) && (input_mode == INPUT_MODE_SOCKET)) {

          MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBGETALL]);

          fprintf(stderr, "EOF (input socket) - clear status/error\n");
          status = MB_SUCCESS;
          error = MB_ERROR_NO_ERROR;

        }
        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MBGETFAIL_XT], mtime_dtime());
      }

      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MBPOST_XT], mtime_dtime());

      if (status == MB_FAILURE && error > 0) {
        fprintf(stderr, "mbtrnpp: MB_FAILURE - error>0 : setting done flag\n");
        done = true;
        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBFAILURE]);
      }
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MBPOST_XT], mtime_dtime());
    }

    /* close the files */
    if (input_mode == INPUT_MODE_SOCKET) {
      fprintf(stderr, "socket input mode - continue (probably shouldn't be here)\n");
      read_data = true;
    }
    else {
      fprintf(stderr, "file input mode - file cleanup\n");
      status = mb_close(verbose, &imbio_ptr, &error);

      if (logfp != NULL) {
        sprintf(log_message, "Multibeam File <%s> closed", ifile);
      }

      mbtrnpp_postlog(verbose, logfp, log_message, &error);
      if (verbose != 0) {
        fprintf(stderr, "\n%s\n", log_message);
      }

      sprintf(log_message, "MBIO format id: %d", format);
      if (logfp != NULL) {
        mbtrnpp_postlog(verbose, logfp, log_message, &error);
      }

      if (verbose > 0) {
        fprintf(stderr, "%s\n", log_message);
      }

      fflush(logfp);

      /* give the statistics */
      /* figure out whether and what to read next */
      if (read_datalist == true) {
        if ((status = mb_datalist_read(verbose, datalist, ifile, dfile, &format, &file_weight, &error)) == MB_SUCCESS) {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "read_datalist status[%d] - continuing\n", status));
          read_data = true;
        }
        else {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "read_datalist status[%d] - done\n", status));
          read_data = false;
        }
      }
      else {
        PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "read_datalist == NO\n"));
        read_data = false;
      }
    }
    /* end loop over files in list */
  }

  fprintf(stderr, "exit loop\n");
  if (read_datalist == true)
    mb_datalist_close(verbose, &datalist, &error);

  /* close log file */
  gettimeofday(&timeofday, &timezone);
  now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  // fprintf(stderr,"CHECKING AT BOTTOM OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n", logfp,
  // log_file_open_time_d, now_time_d);
  if (logfp != NULL) {
    status = mbtrnpp_logstatistics(verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                   n_soundings_flagged_read, n_soundings_null_read, n_soundings_trimmed,
                                   n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
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

    status = mbtrnpp_closelog(verbose, &logfp, &error);
  }

  /* close output */
  if (output_mode == MBTRNPREPROCESS_OUTPUT_FILE) {
    fclose(ofp);
  }

  /* check memory */
  if (verbose >= 4)
    status = mb_memory_list(verbose, &error);

  /* give the statistics */
  if (verbose >= 1) {
  }

  fprintf(stderr, "exit app [%d]\n", error);

  /* end it all */
  exit(error);
}
/*--------------------------------------------------------------------*/

int mbtrnpp_openlog(int verbose, mb_path log_directory, FILE **logfp, int *error) {

  /* local variables */
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

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       log_directory:      %s\n", log_directory);
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
  }

  /* close existing log file */
  if (*logfp != NULL) {
    mbtrnpp_closelog(verbose, logfp, error);
  }

  /* get time and user data */
  gettimeofday(&timeofday, &timezone);
  time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  status = mb_get_date(verbose, time_d, time_i);
  sprintf(date, "%4.4d%2.2d%2.2d_%2.2d%2.2d%2.2d%6.6d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
          time_i[6]);
  if ((user_ptr = getenv("USER")) == NULL)
    user_ptr = getenv("LOGNAME");
  if (user_ptr != NULL)
    strcpy(user, user_ptr);
  else
    strcpy(user, "unknown");
  gethostname(host, sizeof(mb_path));

  /* open new log file */
  sprintf(log_file, "%s/%s_mbtrnpp_log.txt", log_directory, date);
  *logfp = fopen(log_file, "w");
  if (*logfp != NULL) {
    fprintf(*logfp, "Program %s log file\n-------------------\n", program_name);
    if (verbose > 0) {
      fprintf(stderr, "Program %s log file\n-------------------\n", program_name);
    }
    sprintf(log_message, "Opened by user %s on cpu %s", user, host);
    mbtrnpp_postlog(verbose, *logfp, log_message, error);
  }
  else {
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, "\nUnable to open %s log file: %s\n", program_name, log_file);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(*error);
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_closelog(int verbose, FILE **logfp, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  char *log_message = "Closing mbtrnpp log file";

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       *logfp:             %p\n", *logfp);
  }

  /* close log file */
  if (logfp != NULL) {
    mbtrnpp_postlog(verbose, *logfp, log_message, error);
    fclose(*logfp);
    *logfp = NULL;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       logfp:              %p\n", logfp);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_postlog(int verbose, FILE *logfp, char *log_message, int *error) {

  /* local variables */
  int status = MB_SUCCESS;

  /* time, user, host variables */
  struct timeval timeofday;
  struct timezone timezone;
  double time_d;
  int time_i[7];
  char date[32];

  /* print input debug statements */
  if (verbose >= 2) {
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
  sprintf(date, "%4.4d%2.2d%2.2d_%2.2d%2.2d%2.2d%6.6d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
          time_i[6]);

  /* post log_message */
  if (logfp != NULL) {
    fprintf(logfp, "<%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d>: %s\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
            time_i[5], time_i[6], log_message);
  }
  if (verbose > 0) {
    fprintf(stderr, "<%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d>: %s\n", time_i[0], time_i[1], time_i[2], time_i[3],
            time_i[4], time_i[5], time_i[6], log_message);
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbtrnpp_logparameters(int verbose, FILE *logfp, char *input, int format, char *output, double swath_width,
                          int n_output_soundings, int median_filter, int median_filter_n_across, int median_filter_n_along,
                          double median_filter_threshold, int n_buffer_max, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  mb_path log_message;

  /* print input debug statements */
  if (verbose >= 2) {
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
  if (logfp != NULL) {
    sprintf(log_message, "       input:                    %s", input);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       format:                   %d", format);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       output:                   %s", output);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       swath_width:              %f", swath_width);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_output_soundings:       %d", n_output_soundings);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter:            %d", median_filter);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_n_across:   %d", median_filter_n_across);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_n_along:    %d", median_filter_n_along);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       median_filter_threshold:  %f", median_filter_threshold);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_buffer_max:             %d", n_buffer_max);
    mbtrnpp_postlog(verbose, logfp, log_message, error);
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbtrnpp_logstatistics(int verbose, FILE *logfp, int n_pings_read, int n_soundings_read, int n_soundings_valid_read,
                          int n_soundings_flagged_read, int n_soundings_null_read, int n_soundings_trimmed,
                          int n_soundings_decimated, int n_soundings_flagged, int n_soundings_written, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  mb_path log_message;

  /* print input debug statements */
  if (verbose >= 2) {
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
  if (logfp != NULL) {
    sprintf(log_message, "Log File Statistics:");
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_pings_read:                 %d", n_pings_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_read:             %d", n_soundings_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_valid_read:       %d", n_soundings_valid_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_flagged_read:     %d", n_soundings_flagged_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_null_read:        %d", n_soundings_null_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_trimmed:          %d", n_pings_read);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_decimated:        %d", n_soundings_decimated);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_flagged:          %d", n_soundings_flagged);
    mbtrnpp_postlog(verbose, logfp, log_message, error);

    sprintf(log_message, "       n_soundings_written:          %d", n_soundings_written);
    mbtrnpp_postlog(verbose, logfp, log_message, error);
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_update_stats(mstats_profile_t *stats, mlog_id_t log_id, mstats_flags flags) {

  if (NULL != stats) {
    double stats_now = mtime_dtime();

    if (log_clock_res) {
      // log the timing clock resolution (once)
      struct timespec res;
      clock_getres(CLOCK_MONOTONIC, &res);
      mlog_tprintf(trn_mlog_id, "%.3lf,i,clkres_mono,s[%ld] ns[%ld]\n", stats_now, res.tv_sec, res.tv_nsec);
      log_clock_res = false;
    }

    // we can only measure the previous stats cycle...
    if (stats->stats->per_stats[MBTPP_CH_CYCLE_XT].n > 0) {
      // get the timing of the last cycle
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_STATS_XT], stats_prev_start);
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_STATS_XT], stats_prev_end);
    }
    else {
      // seed the first cycle
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_STATS_XT], (stats_now - 0.0001));
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_STATS_XT], stats_now);
    }

    // end the cycle timer here
    // [start at the end if this function]
    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_CYCLE_XT], stats_now);

    // measure dtime execution time (twice), while we're at it
    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_DTIME_XT], mtime_dtime());
    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_DTIME_XT], mtime_dtime());
    MST_METRIC_DIV(app_stats->stats->metrics[MBTPP_CH_DTIME_XT], 2.0);

    // update uptime
    stats->uptime = stats_now - stats->session_start;

    // update throughput measurement
    stats->stats->metrics[MBTPP_CH_THRUPUT].value =
        (stats->uptime > 0.0 ? (double)stats->stats->status[MBTPP_STA_TRN_TX_BYTES] / stats->uptime : 0.0);

    PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V3,
            (stderr, "cycle_xt: stat_now[%.4lf] start[%.4lf] stop[%.4lf] value[%.4lf]\n", stats_now,
             app_stats->stats->metrics[MBTPP_CH_CYCLE_XT].start, app_stats->stats->metrics[MBTPP_CH_CYCLE_XT].stop,
             app_stats->stats->metrics[MBTPP_CH_CYCLE_XT].value));

    // update stats
    mstats_update_stats(stats->stats, MBTPP_CH_COUNT, flags);

    PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V3,
            (stderr, "cycle_xt.p: N[%lld] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
             app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].n, app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].sum,
             app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].min, app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].max,
             app_stats->stats->per_stats[MBTPP_CH_CYCLE_XT].avg));

    PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V3,
            (stderr, "cycle_xt.a: N[%lld] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
             app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].n, app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].sum,
             app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].min, app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].max,
             app_stats->stats->agg_stats[MBTPP_CH_CYCLE_XT].avg));

    if (flags & MSF_READER) {
      mstats_update_stats(reader_stats, R7KR_MET_COUNT, flags);
    }

    //        fprintf(stderr,"stat period sec[%.3lf] start[%.3lf] now[%.3lf] elapsed[%.3lf]\n",
    //                stats->stats->stat_period_sec,
    //                stats->stats->stat_period_start,
    //                stats_now,
    //                (stats_now - stats->stats->stat_period_start)
    //                );
    // check stats periods, process if ready
    if ((stats->stats->stat_period_sec > 0.0) &&
        ((stats_now - stats->stats->stat_period_start) > stats->stats->stat_period_sec)) {

      // start log execution timer
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_LOG_XT], mtime_dtime());

      mlog_tprintf(trn_mlog_id, "%.3lf,i,uptime,%0.3lf\n", stats_now, stats->uptime);
      mstats_log_stats(stats->stats, stats_now, log_id, flags);

      if (flags & MSF_READER) {
        mstats_log_stats(reader_stats, stats_now, log_id, flags);
      }

      // reset period stats
      mstats_reset_pstats(stats->stats, MBTPP_CH_COUNT);
      mstats_reset_pstats(reader_stats, R7KR_MET_COUNT);

      // reset period timer
      stats->stats->stat_period_start = stats_now;

      // stop log execution timer
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_LOG_XT], mtime_dtime());
    }

    // start cycle timer
    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_CYCLE_XT], mtime_dtime());

    // update stats execution time variables
    stats_prev_start = stats_now;
    stats_prev_end = mtime_dtime();
  }
  else {
    fprintf(stderr, "mbtrnpp_update_stats: invalid argument\n");
  }
  return 0;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_init_debug(int verbose) {

  /* Open and initialize the socket based input for reading using function
   * mbtrnpp_input_read(). Allocate an internal, hidden buffer to hold data from
   * full s7k records while waiting to return bytes from those records as
   * requested by the MBIO read functions.
   * Store the relevant pointers and parameters within the
   * mb_io_struct structure *mb_io_ptr. */

  mmd_initialize();
  mconf_init(NULL, NULL);

  fprintf(stderr, "%s:%d >>> MOD_MBTRNPP[id=%d]  %08X\n", __FUNCTION__, __LINE__, MOD_MBTRNPP,
          mmd_get_enmask(MOD_MBTRNPP, NULL));

  switch (verbose) {
  case 0:
    mmd_channel_set(MOD_MBTRNPP, MM_NONE);
    mmd_channel_set(MOD_R7K, MM_NONE);
    mmd_channel_set(MOD_R7KR, MM_NONE);
    mmd_channel_set(MOD_MSOCK, MM_NONE);
    break;
  case 1:
    mmd_channel_en(MOD_MBTRNPP, MBTRNPP_V1);
    mmd_channel_en(MOD_R7KR, R7KR_V1);
    break;
  case 2:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG);
    mmd_channel_en(MOD_R7KR, MM_DEBUG);
    mmd_channel_en(MOD_R7K, R7K_PARSER);
    break;
  case -1:
    mmd_channel_en(MOD_MBTRNPP, MBTRNPP_V1);
    mmd_channel_en(MOD_R7KR, MM_DEBUG);
    break;
  case -2:
    mmd_channel_en(MOD_MBTRNPP, MBTRNPP_V1 | MBTRNPP_V2);
    break;
  case -3:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1 | MBTRNPP_V2 | MBTRNPP_V3);
    mmd_channel_en(MOD_R7KR, MM_DEBUG);
    mmd_channel_en(MOD_R7K, MM_WARN | R7K_PARSER);
    // this enables messages from msock_recv (e.g. resource temporarily unavailable)
    msock_set_debug(1);
    break;
  case -4:
    mmd_channel_en(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1 | MBTRNPP_V2 | MBTRNPP_V3 | MBTRNPP_V4);
    mmd_channel_en(MOD_R7KR, MM_DEBUG);
    mmd_channel_en(MOD_R7K, MM_WARN | R7K_PARSER | R7K_DRFCON);
    mmd_channel_en(MOD_MSOCK, MM_DEBUG);
    msock_set_debug(1);
    break;
  case -5:
    mmd_channel_en(MOD_MBTRNPP, MM_ALL);
    mmd_channel_en(MOD_R7KR, MM_ALL);
    mmd_channel_en(MOD_R7K, MM_ALL);
    mmd_channel_en(MOD_MSOCK, MM_ALL);
    msock_set_debug(1);
    break;
  default:
    break;
  }
  fprintf(stderr, "%s:%d >>> MOD_MBTRNPP  %08X\n", __FUNCTION__, __LINE__, mmd_get_enmask(MOD_MBTRNPP, NULL));

  // open trn data log
  if (trn_blog_en) {
    trn_blog_path = (char *)malloc(512);
    sprintf(trn_blog_path, "%s//%s-%s%s", g_log_dir, TRN_BLOG_NAME, session_date, TRN_LOG_EXT);
    //        trn_blog = mlog_new(trn_blog_path,&blog_conf);
    trn_blog_id = mlog_get_instance(trn_blog_path, &blog_conf, TRN_BLOG_NAME);
    mlog_show(trn_blog_id, true, 5);
    mlog_open(trn_blog_id, flags, mode);
    //        mlog_add(trn_blog,trn_blog_id,TRN_BLOG_DESC);
  }
  // open trn message log
  if (trn_mlog_en) {
    trn_mlog_path = (char *)malloc(512);
    sprintf(trn_mlog_path, "%s//%s-%s%s", g_log_dir, TRN_MLOG_NAME, session_date, TRN_LOG_EXT);
    //        trn_mlog = mlog_new(trn_mlog_path,&mlog_conf);
    trn_mlog_id = mlog_get_instance(trn_mlog_path, &mlog_conf, TRN_MLOG_NAME);
    mlog_show(trn_mlog_id, true, 5);
    mlog_open(trn_mlog_id, flags, mode);
    //        mlog_add(trn_mlog,trn_mlog_id,TRN_MLOG_DESC);
    mlog_tprintf(trn_mlog_id, "*** mbtrn session start ***\n");
    mlog_tprintf(trn_mlog_id, "cmdline [%s]\n", g_cmd_line);
    mlog_tprintf(trn_mlog_id, "r7kr v[%s] build[%s]\n", R7KR_VERSION_STR, LIBMFRAME_BUILD);
  }
  else {
    fprintf(stderr, "*** mbtrn session start ***\n");
    fprintf(stderr, "cmdline [%s]\n", g_cmd_line);
  }

  //    app_stats = mbtrnpp_stats_new(MBTPP_EV_COUNT,
  //                                          MBTPP_STA_COUNT,
  //                                          MBTPP_CH_COUNT,
  //                                          mtime_dtime(),
  //                                          trn_status_interval_sec);
  app_stats = mstats_profile_new(MBTPP_EV_COUNT, MBTPP_STA_COUNT, MBTPP_CH_COUNT, mbtrnpp_stats_labels, mtime_dtime(),
                                 trn_status_interval_sec);

  return 0;
}

/*--------------------------------------------------------------------*/

#ifdef WITH_MBTNAV

int mbtrnpp_init_trn(int verbose, trn_config_t *cfg) {
  int retval = -1;
  // TODO: command line config for trn (map_file, cfg_file, particles_file, log_dir, mtype, ftype)
  // map       "./maps/PortTiles"
  // cfg       "./config/mappingAUV_specs.cfg"
  // particles "./config/particles.cfg"
  // logdir    "logs"
  if (NULL != cfg) {
    if ((tnav = wtnav_new(cfg)) != NULL) {

      if (wtnav_initialized(tnav)) {
        retval = 0;
        fprintf(stderr, "TNAV intialize - OK\n");
      }
      else {
        fprintf(stderr, "TNAV intialize - ERR\n");
      }
    }
    else {
      fprintf(stderr, "TNAV new failed\n");
    }
  }
  else {
    fprintf(stderr, "TNAV config NULL\n");
  }

  return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_get_bias_estimates(wtnav_t *self, wposet_t *pt, pt_cdata_t **pt_out, pt_cdata_t **mle_out, pt_cdata_t **mse_out) {
  int retval = -1;
  uint32_t uret = 0;
  bool meas_valid = false;
  wposet_t *mle = wposet_dnew();
  wposet_t *mse = wposet_dnew();

  if (NULL != self && NULL != pt && NULL != mle_out && NULL != mse_out) {

    wtnav_estimate_pose(self, mle, 1);
    wtnav_estimate_pose(self, mse, 2);

    //        fprintf(stderr,"%s:%d MLE,MSE\n",__FUNCTION__,__LINE__);
    //        wposet_show(mle,true,5);
    //        fprintf(stderr,"\n");
    //        wposet_show(mse,true,5);

    if (wtnav_last_meas_successful(self)) {
      wposet_pose_to_cdata(pt_out, pt);
      wposet_pose_to_cdata(mle_out, mle);
      wposet_pose_to_cdata(mse_out, mse);
      retval = 0;
    }
    else {
      PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "Last Meas Invalid\n"));
    }
    wposet_destroy(mle);
    wposet_destroy(mse);
  }

  return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_update(wtnav_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out, trn_config_t *cfg) {
  int retval = -1;
  int test = -1;
  uint32_t uret = 0;

  if (NULL != self && NULL != src && NULL != pt_out && NULL != mt_out) {

    if ((test = wmeast_mb1_to_meas(mt_out, src, cfg->utm_zone)) == 0) {

      if ((test = wposet_mb1_to_pose(pt_out, src, cfg->utm_zone)) == 0) {
        // must do motion update first if pt time <= mt time
        wtnav_motion_update(self, *pt_out);
        wtnav_meas_update(self, *mt_out, TRN_SENSOR_MB);
        //                fprintf(stderr,"%s:%d DONE [PT, MT]\n",__FUNCTION__,__LINE__);
        //                wposet_show(*pt_out,true,5);
        //                wmeast_show(*mt_out,true,5);
        retval = 0;
      }
      else {
        PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "wposet_mb1_to_pose failed [%d]\n", test));
      }
    }
    else {
      PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "wmeast_mb1_to_meas failed [%d]\n", test));
    }
  }

  return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_process_mb1(wtnav_t *tnav, mb1_t *mb1, trn_config_t *cfg) {
  int retval = 0;

  int test = -1;

  wmeast_t *mt = NULL;
  wposet_t *pt = NULL;
  pt_cdata_t *pt_dat = NULL;
  pt_cdata_t *mle_dat = NULL;
  pt_cdata_t *mse_dat = NULL;

  if (NULL != tnav && NULL != mb1) {
    if ((test = mbtrnpp_trn_update(tnav, mb1, &pt, &mt, cfg)) == 0) {
      if ((test = mbtrnpp_trn_get_bias_estimates(tnav, pt, &pt_dat, &mle_dat, &mse_dat)) == 0) {
        if (NULL != pt_dat && NULL != mle_dat && NULL != mse_dat) {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1, (stderr, "\n\tBias Estimates:\n"));
          PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1,
                  (stderr, "\tMLE: %.2lf,%.4lf,%.4lf,%.4lf\n", mle_dat->time, (mle_dat->x - pt_dat->x),
                   (mle_dat->y - pt_dat->y), (mle_dat->z - pt_dat->z)));
          PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1,
                  (stderr, "\tMSE: %.2lf,%.4lf,%.4lf,%.4lf\n", mse_dat->time, (mse_dat->x - pt_dat->x),
                   (mse_dat->y - pt_dat->y), (mse_dat->z - pt_dat->z)));
          PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V1,
                  (stderr, "\tCOV:[%.2lf,%.2lf,%.2lf\n\n", sqrt(mse_dat->covariance[0]), sqrt(mse_dat->covariance[2]),
                   sqrt(mse_dat->covariance[5])));

          mlog_tprintf(trn_mlog_id, "\n\tBias Estimates:\n");
          mlog_tprintf(trn_mlog_id, "MLE,%.2lf,%.4lf,%.4lf,%.4lf\n", mle_dat->time, (mle_dat->x - pt_dat->x),
                       (mle_dat->y - pt_dat->y), (mle_dat->z - pt_dat->z));
          mlog_tprintf(trn_mlog_id, "MSE,%.2lf,%.4lf,%.4lf,%.4lf\n", mse_dat->time, (mse_dat->x - pt_dat->x),
                       (mse_dat->y - pt_dat->y), (mse_dat->z - pt_dat->z));
          mlog_tprintf(trn_mlog_id, "COV,%.2lf,%.2lf,%.2lf\n", sqrt(mse_dat->covariance[0]),
                       sqrt(mse_dat->covariance[2]), sqrt(mse_dat->covariance[5]));

          retval = 0;
        }
        else {
          PMPRINT(MOD_MBTRNPP, MM_DEBUG,
                  (stderr, "ERR: pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat));
          mlog_tprintf(trn_mlog_id, "ERR: pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n", pt, pt_dat, mle_dat, mse_dat);
          mlog_tprintf(trn_mlog_id, "ERR: ts[%.3lf] beams[%u] ping[%d] \n", mb1->sounding.ts, mb1->sounding.nbeams,
                       mb1->sounding.ping_number);
          mlog_tprintf(trn_mlog_id, "ERR: lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n\n", mb1->sounding.lat,
                       mb1->sounding.lon, mb1->sounding.hdg, mb1->sounding.depth);
        }
      }
      else {
        PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V3, (stderr, "ERR: trn_get_bias_estimates failed [%d]\n", test));
        //                mlog_tprintf(trn_mlog_id,"ERR: trncli_get_bias_estimates failed [%d]\n",test);
      }
    }
    else {
      PMPRINT(MOD_MBTRNPP, MM_DEBUG | MBTRNPP_V3, (stderr, "ERR: trn_send_update failed [%d]\n", test));
      //            mlog_tprintf(trn_mlog_id,"ERR: trncli_send_update failed [%d]\n",test);
    }
    wmeast_destroy(mt);
    wposet_destroy(pt);
    if (NULL != pt_dat)
      free(pt_dat);
    if (NULL != mse_dat)
      free(mse_dat);
    if (NULL != mle_dat)
      free(mle_dat);
  }

  return retval;
}
#endif // WITH_MBTNAV

/*--------------------------------------------------------------------*/

int mbtrnpp_reson7kr_input_open(int verbose, void *mbio_ptr, char *definition, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  uint32_t reson_nsubs = 11;
  uint32_t reson_subs[] = {1003, 1006, 1008, 1010, 1012, 1013, 1015, 1016, 7000, 7004, 7027};

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p,%p\n", mbio_ptr, &mbio_ptr);
    fprintf(stderr, "dbg2       hostname:   %s\n", definition);
  }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  /* Open and initialize the socket based input for reading using function
   * mbtrnpp_reson7kr_input_read(). Allocate an internal, hidden buffer to hold data from
   * full s7k records while waiting to return bytes from those records as
   * requested by the MBIO read functions.
   * Store the relevant pointers and parameters within the
   * mb_io_struct structure *mb_io_ptr. */

#define SONAR_READER_CAPACITY_DFL (256 * 1024)
  mb_path hostname;
  int port = 0;
  size_t size = 0;
  sscanf(definition, "%s:%d:%zd", hostname, &port, &size);
  if (strlen(hostname) == 0)
    strcpy(hostname, "localhost");
  if (port == 0)
    port = R7K_7KCENTER_PORT;
  if (size <= 0)
    size = SONAR_READER_CAPACITY_DFL;

  PMPRINT(MOD_MBTRNPP, MM_DEBUG, (stderr, "configuring r7kr_reader using %s:%d\n", hostname, port));
  r7kr_reader_t *reader = r7kr_reader_new(hostname, port, size, reson_subs, reson_nsubs);

  if (NULL != mb_io_ptr && NULL != reader) {

    // set r7k_reader
    mb_io_ptr->mbsp = (void *) reader;

    if (reader->state == R7KR_CONNECTED || reader->state == R7KR_SUBSCRIBED) {
      // update application performance profile
      MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_SRC_CONN]);
    }

    // get global 7K reader performance profile
    reader_stats = r7kr_reader_get_stats(reader);
    mstats_set_period(reader_stats, app_stats->stats->stat_period_start, app_stats->stats->stat_period_sec);

    // configure reader data log
    if (mbr_blog_en) {
      // open mbr data log
      mbr_blog_path = (char *)malloc(512);
      sprintf(mbr_blog_path, "%s//%s-%s%s", g_log_dir, MBR_BLOG_NAME, session_date, TRN_LOG_EXT);

      mbr_blog_id = mlog_get_instance(mbr_blog_path, &mbrlog_conf, MBR_BLOG_NAME);

      mlog_show(mbr_blog_id, true, 5);
      mlog_open(mbr_blog_id, flags, mode);

      r7kr_reader_set_log(reader, mbr_blog_id);
    }

    if (verbose >= 1) {
      r7kr_reader_show(reader, true, 5);
    }
  }
  else {
    fprintf(stderr, "ERR - r7kr_reader_new failed (NULL) [%d:%s]\n", errno, strerror(errno));
    status = MB_FAILURE;
    *error = MB_ERROR_INIT_FAIL;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_reson7kr_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;
  r7kr_reader_t *mbsp;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
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
   * The internal buffer should be allocated in mbtrnpp_reson7kr_input_init() and stored
   * in the mb_io_struct structure *mb_io_ptr. */

  // use the socket reader
  // read and return single frame
  uint32_t sync_bytes=0;
  int64_t rbytes=-1;
  r7kr_reader_t *reader = (r7kr_reader_t *)mb_io_ptr->mbsp;
  if ( (rbytes = r7kr_read_stripped_frame(reader, (byte *) buffer,
                                          R7K_MAX_FRAME_BYTES, R7KR_NET_STREAM,
                                          0.0, R7KR_READ_TMOUT_MSEC,
                                          &sync_bytes)) < 0) {
    status   = MB_FAILURE;
    *error   = MB_ERROR_EOF;
    *size    = (size_t)rbytes;
    if (me_errno==ME_ESOCK) {
        fprintf(stderr,"r7kr_reader server connection closed.\n");
    } else if (me_errno==ME_EOF) {
        fprintf(stderr,"r7kr_reader end of file (server connection closed).\n");
    } else{
        fprintf(stderr,"r7kr_read_stripped_frame me_errno %d/%s\n",me_errno,me_strerror(me_errno));
    }
  } else {
    *error = MB_ERROR_NO_ERROR;
    *size    = (size_t)rbytes;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_reson7kr_input_close(int verbose, void *mbio_ptr, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  /* print input debug statements */
  if (verbose >= 2) {
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
   * mbtrnpp_reson7kr_input_read(). Deallocate the internal, hidden buffer and any
   * other resources that were allocated by mbtrnpp_reson7kr_input_init(). */
  r7kr_reader_t *reader = (r7kr_reader_t *)mb_io_ptr->mbsp;
  r7kr_reader_destroy(&reader);
  mb_io_ptr->mbsp = NULL;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_kemkmall_input_open(int verbose, void *mbio_ptr, char *definition, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p,%p\n", mbio_ptr, &mbio_ptr);
    fprintf(stderr, "dbg2       definition: %s\n", definition);
  }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  // Open and initialize the socket based input for reading using function
  // mbtrnpp_kemkmall_input_read().
  // - use mb_io_ptr->mbsp to hold pointer to socket i/o structure
  // - the socket definition = "hostInterface:broadcastGroup:port"
  int port;
  mb_path bcastGrp;
  mb_path hostInterface;
  struct sockaddr_in localSock;
  struct ip_mreq group;
  char *token;
  if ((token = strsep(&definition, ":")) != NULL) {
    strncpy(hostInterface, token, sizeof(mb_path));
  }
  if ((token = strsep(&definition, ":")) != NULL) {
    strncpy(bcastGrp, token, sizeof(mb_path));
  }
  if ((token = strsep(&definition, ":")) != NULL) {
    sscanf(token, "%d", &port);
  }

  //sscanf(definition, "%s:%s:%d", hostInterface, bcastGrp, &port);
  fprintf(stderr, "Attempting to open socket to Kongsberg sonar multicast at:\n");
  fprintf(stderr, "  Definition: %s\n", definition);
  fprintf(stderr, "  hostInterface: %s\n  bcastGrp: %s\n  port: %d\n",
          hostInterface, bcastGrp, port);

  /* Create a datagram socket on which to receive. */
  int sd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sd < 0)
  {
      perror("Opening datagram socket error");
      exit(1);
  }

  /* Enable SO_REUSEADDR to allow multiple instances of this */
  /* application to receive copies of the multicast datagrams. */
  int reuse = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
      perror("Setting SO_REUSEADDR error");
      close(sd);
      exit(1);
    }

  /* Bind to the proper port number with the IP address */
  /* specified as INADDR_ANY. */
  memset((char *) &localSock, 0, sizeof(localSock));
  localSock.sin_family = AF_INET;
  localSock.sin_port = htons(port);
  localSock.sin_addr.s_addr = INADDR_ANY;
  if (bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))) {
      perror("Binding datagram socket error");
      close(sd);
      exit(1);
  }

  /* Join the multicast group on the specified */
  /* interface. Note that this IP_ADD_MEMBERSHIP option must be */
  /* called for each local interface over which the multicast */
  /* datagrams are to be received. */
  group.imr_multiaddr.s_addr = inet_addr(bcastGrp);
  group.imr_interface.s_addr = inet_addr(hostInterface);

  if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
    (char *)&group, sizeof(group)) < 0) {
    perror("Adding multicast group error");
    close(sd);
    exit(1);
  }

  // save the socket within the mb_io structure
  int *sd_ptr = NULL;
  status &= mb_mallocd(verbose, __FILE__, __LINE__, sizeof(sd), (void **)&sd_ptr, error);
  *sd_ptr = sd;
  mb_io_ptr->mbsp = (void *) sd_ptr;

  /*initialize buffer for fragmented MWZ and MRC datagrams*/
  memset(mRecordBuf, 0, sizeof(mRecordBuf));

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_kemkmall_rd_hdr(int verbose, char *buffer, void *header_ptr, void *emdgm_type_ptr, int *error) {
  struct mbsys_kmbes_header *header = NULL;
  mbsys_kmbes_emdgm_type *emdgm_type = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       header_ptr:     %p\n", (void *)header_ptr);
    fprintf(stderr, "dbg2       emdgm_type_ptr: %p\n", (void *)emdgm_type_ptr);
  }

  /* get pointer to header structure */
  header = (struct mbsys_kmbes_header *)header_ptr;
  emdgm_type = (mbsys_kmbes_emdgm_type *)emdgm_type_ptr;

  /* extract the data */
  index = 0;
  mb_get_binary_int(true, &buffer[index], &(header->numBytesDgm));
  index += 4;
  memcpy(&(header->dgmType), &buffer[index], sizeof(header->dgmType));
  index += 4;
  header->dgmVersion = buffer[index];
  index++;
  header->systemID = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(header->echoSounderID));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(header->time_sec));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(header->time_nanosec));

  /* identify the datagram type */
  if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_INSTALLATION_PARAM, 4) == 0 ) {
    *emdgm_type = IIP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_OP_RUNTIME, 4) == 0) {
    *emdgm_type = IOP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_POSITION, 4) == 0) {
    *emdgm_type = SPO;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_KM_BINARY, 4) == 0) {
    *emdgm_type = SKM;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_SOUND_VELOCITY_PROFILE, 4) == 0) {
    *emdgm_type = SVP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_SOUND_VELOCITY_TRANSDUCER, 4) == 0) {
    *emdgm_type = SVT;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_CLOCK, 4) == 0) {
    *emdgm_type = SCL;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_DEPTH, 4) == 0) {
    *emdgm_type = SDE;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_HEIGHT, 4) == 0) {
    *emdgm_type = SHI;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_HEADING, 4) == 0) {
    *emdgm_type = SHA;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_M_RANGE_AND_DEPTH, 4) == 0) {
    *emdgm_type = MRZ;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_M_WATER_COLUMN, 4) == 0) {
    *emdgm_type = MWC;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_C_POSITION, 4) == 0) {
    *emdgm_type = CPO;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_C_HEAVE, 4) == 0) {
    *emdgm_type = CHE;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_MBSYSTEM, 4) == 0) {
    *emdgm_type = XMB;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_COMMENT, 4) == 0) {
    *emdgm_type = XMC;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_PSEUDOSIDESCAN, 4) == 0) {
    *emdgm_type = XMS;
  }
  else {
    *emdgm_type = UNKNOWN;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", header->numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %.4s\n", header->dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", header->dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", header->systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", header->echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", header->time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", header->time_nanosec);
  }

  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       dgmType:    %.4s\n", header->dgmType);
    fprintf(stderr, "dbg2       emdgm_type: %d\n", *emdgm_type);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbtrnpp_kemkmall_input_read(int verbose, void *mbio_ptr, size_t *size,
                                char *buffer, int *error) {

  /* local variables */
  int status = MB_SUCCESS;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
    fprintf(stderr, "dbg2       buffer:     %p\n", buffer);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  // Read from the socket.
  int *sd_ptr = (int *)mb_io_ptr->mbsp;
  struct mbsys_kmbes_header header;
  unsigned int num_bytes_dgm_end;
  mbsys_kmbes_emdgm_type emdgm_type;
  memset(buffer, 0, *size);
  int readlen = read(*sd_ptr, buffer, *size);
  if (readlen <= 0) {
    status = MB_FAILURE;
    *error = MB_ERROR_EOF;
  }

  if (status == MB_SUCCESS) {
    status = mbtrnpp_kemkmall_rd_hdr(verbose, buffer, (void *)&header, (void *)&emdgm_type, error);

    if (status == MB_SUCCESS && emdgm_type != UNKNOWN && header.numBytesDgm <= *size) {
      mb_get_binary_int(true, &buffer[header.numBytesDgm-4], &num_bytes_dgm_end);
      if (num_bytes_dgm_end != header.numBytesDgm) {
        status = MB_FAILURE;
        *error = MB_ERROR_UNINTELLIGIBLE;
      }
    } else {
        status = MB_FAILURE;
        *error = MB_ERROR_UNINTELLIGIBLE;
    }
  }

  if (status == MB_SUCCESS) {
    *size = header.numBytesDgm;
  }
  else {
    *size = 0;
  }

  /*handle multi-packet MRZ and MWC records*/
  static int totalDgms, dgmsReceived=0;
  static unsigned int pingSecs, pingNanoSecs;
  unsigned short numOfDgms;
  unsigned short dgmNum;
  int totalSize;
  if (emdgm_type == MRZ || emdgm_type == MWC) {
    mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE], &numOfDgms);
    mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE+2], &dgmNum);
    if (numOfDgms > 1) {

      /* if we get a M record of a multi-packet sequence, and its numOfDgms
          or ping time don't match the ping we are looking for, flush the
          current read and start over with this packet */
      if (header.time_sec != pingSecs
          || header.time_nanosec != pingNanoSecs
          || numOfDgms != totalDgms) {
        dgmsReceived = 0;
      }

      if (!dgmsReceived){
        pingSecs = header.time_sec;
        pingNanoSecs = header.time_nanosec;
        totalDgms = numOfDgms;
        dgmsReceived = 1;
      }
      else {
        dgmsReceived++;
      }

      memcpy(mRecordBuf[dgmNum-1], buffer, header.numBytesDgm);

      if (dgmsReceived == totalDgms) {
fprintf(stderr, "%s:%4.4d Handling %d datagrams\n", __FILE__, __LINE__, totalDgms);
        totalSize = sizeof(struct mbsys_kmbes_m_partition)
                    + sizeof(struct mbsys_kmbes_header) + 4;
        int rsize = 0;
        for (int dgm = 0; dgm < totalDgms; dgm++) {
          mb_get_binary_int(true, mRecordBuf[dgm], &rsize);
          totalSize += rsize - sizeof(struct mbsys_kmbes_m_partition)
                      - sizeof(struct mbsys_kmbes_header) - 4;
        }

        /*copy data into new buffer*/
        if (status == MB_SUCCESS) {
          int index = 0;
          status = mbtrnpp_kemkmall_rd_hdr(verbose, mRecordBuf[0], (void *)&header, (void *)&emdgm_type, error);
          memcpy(buffer, mRecordBuf[0], header.numBytesDgm);
          index = header.numBytesDgm - 4;
          void *ptr;
          for (int dgm=1; dgm < totalDgms; dgm++) {
            status = mbtrnpp_kemkmall_rd_hdr(verbose, mRecordBuf[dgm], (void *)&header, (void *)&emdgm_type, error);
            int copy_len = header.numBytesDgm - sizeof(struct mbsys_kmbes_m_partition)
                                  - sizeof(struct mbsys_kmbes_header) - 4;
            ptr = (void *)(mRecordBuf[dgm]+
                     sizeof(struct mbsys_kmbes_m_partition)+
                     sizeof(struct mbsys_kmbes_header));
            memcpy(&buffer[index], ptr, copy_len);
            index += copy_len;
          }
          mb_put_binary_int(true, totalSize, &buffer[0]);
          mb_put_binary_short(true, 1, &buffer[sizeof(struct mbsys_kmbes_header)]);
          mb_put_binary_short(true, 1, &buffer[sizeof(struct mbsys_kmbes_header)+2]);
          mb_put_binary_int(true, totalSize, &buffer[index]);
	    dgmsReceived = 0; /*reset received counter back to 0*/
        }
      }
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbtrnpp_kemkmall_input_close(int verbose, void *mbio_ptr, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set initial status */
  status = MB_SUCCESS;

  // Close the socket based input
  int *sd_ptr = (int *)mb_io_ptr->mbsp;
  close(*sd_ptr);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sd_ptr, error);

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/
