/*--------------------------------------------------------------------
 *    The MB-system:  mbtrnpp.c  2/19/2018
 *
 *    Copyright (c) 2018-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbtrnpp - originally mbtrnpreprocess
 *
 * Authors:  D. W. Caress and Kent Headley
 * Date:  Begun February 18, 2018
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

#include "mframe.h"
#include "merror.h"
#include "msocket.h"
#include "mtime.h"
#include "mlist.h"
#include "mlog.h"
#include "mbbuf.h"
#include "mstats.h"
#include "mkvconf.h"
#include "mxdebug.h"
#include "mxd_app.h"
#include "r7kc.h"
#include "r7k-reader.h"
#ifdef WITH_MBTNAV
#include "trnw.h"
#include "netif.h"
#include "trnif_proto.h"
#include "trn_msg.h"
#include "mb1_msg.h"
#include "mb1-reader.h"
#endif // WITH_MBTNAV

// Features
#ifndef WITHOUT_MB1_READER
#define WITH_MB1_READER
#endif

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
  double sensordepth;
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

typedef enum {
    INPUT_MODE_SOCKET = 1,
    INPUT_MODE_FILE = 2
} input_mode_t;

typedef enum{
    OUTPUT_NONE        =0x0000, OUTPUT_MB1_FILE_EN =0x0001, OUTPUT_MB1_SVR_EN  =0x0002,
    OUTPUT_TRN_SVR_EN  =0x0004, OUTPUT_TRNU_SVR_EN =0x0008, OUTPUT_MB1_BIN     =0x0010,
    OUTPUT_RESON_BIN   =0x0020, OUTPUT_TRNU_ASC    =0x0040, OUTPUT_TRNU_SOUT   =0x0080,
    OUTPUT_TRNU_SERR   =0x0100, OUTPUT_TRNU_DEBUG  =0x0200, OUTPUT_TRNU_BIN    =0x0400,
    OUTPUT_MBTRNPP_MSG =0x0800, OUTPUT_MBSYS_STDOUT=0x1000, OUTPUT_TRNUM_SVR_EN=0x2000, OUTPUT_MB1R_BIN=0x4000,
    OUTPUT_ALL         =0x7FFF
}output_mode_t;


// mbtrnpp_opts_s only encapsulates options.
// Options representing numeric primatives and booleans are parsed.
// Options with aggregate types are stored as strings.
// Parsing (into configuration variables) is done separately,
// enabling layered overrides clearer parsing logic (cmdline->config file->compilation default)
typedef struct mbtrnpp_opts_s{
    // opt "verbose"
    int verbose;

    // opt "input"
    char *input;

    // opt "format"
    int format;

    // opt "platform-file"
    char *platform_file;

    // opt "platform-target-sensor"
    int platform_target_sensor;

    // opt "tide-model"
    char *tide_model;

    // opt "log-directory"
    char *log_directory;

    // opt "output"
    char *output;

    // opt "projection"
    int projection;

    // opt "swath-width"
    double swath_width;

    // opt "soundings"
    int soundings;

    // opt "median-filter"
    char *median_filter;

    // opt "mbhbn"
    int mbhbn;

    // opt "mbhbt"
    double mbhbt;

    // opt "trnhbt"
    double trnhbt;

    // opt "trnuhbt"
    double trnuhbt;

    // opt "trnumttl"
    int trnumttl;

    // opt "delay"
    int64_t delay;

    // opt "statsec"
    double statsec;

    // opt "statflags"
    char *statflags_str;
    mstats_flags statflags;

    // opt "trn-en"
    bool trn_en;

    // opt "trn-utm"
    long int trn_utm;

    // opt "trn-map"
    char *trn_map;

    // opt "trn-cfg"
    char *trn_cfg;

    // opt "trn-par"
    char *trn_par;

    // opt "trn-mid"
    char *trn_mid;

    // opt "trn-mtype"
    int trn_mtype;

    // opt "trn-sensor-type"
    int trn_sensor_type;

    // opt "trn-ftype"
    int trn_ftype;

    // opt "trn-fgrade"
    int trn_fgrade;

    // opt "trn-freinit"
    int trn_freinit;

    // opt "trn-mweight"
    int trn_mweight;

    // opt "trn-ncov"
    double trn_ncov;

    // opt "trn-nerr"
    double trn_nerr;

    // opt "trn-ecov"
    double trn_ecov;

    // opt "trn-eerr"
    double trn_eerr;

    // opt "mb-out"
    char *mb_out;

    // opt "trn-out"
    char *trn_out;

    // opt "trn-decn"
    unsigned int trn_decn;

    // opt "trn-decs"
    double trn_decs;

    // opt "covariance-magnitude-max"
    double covariance_magnitude_max;

    // opt "convergence-repeat-min"
    int convergence_repeat_min;

    // opt "reinit-search"
    double reinit_search_xy;
    double reinit_search_z;

    // opt "reinit-gain"
    bool reinit_gain_enable;

    // opt "reinit-file"
    bool reinit_file_enable;

    // opt "reinit-xyoffset"
    bool reinit_xyoffset_enable;
    double reinit_xyoffset_max;

    // opt "reinit-offset_z"
    bool reinit_zoffset_enable;
    double reinit_zoffset_min;
    double reinit_zoffset_max;

    // opt "random-offset"
    bool random_offset_enable;

    // opt "trn-dev"
    int trn_dev;

    // opt "help"
    bool help;

}mbtrnpp_opts_t;

typedef struct mbtrnpp_cfg_s{

    // verbose output
    // <0 special trn module-specific debug channels
    // >0 mb-system debug (>2 very verbose)
    int verbose;

    // input mode selector
    input_mode_t input_mode;

    // socket input specifier
    mb_path socket_definition;

    // output mb1 file name
    mb_path output_mb1_file;

    // output trn results file name
    mb_path output_trn_file;

    // input specifier
    mb_path input;

    // data format
    int format;

    // platform file name
    mb_path platform_file;

    // platform file enable
    bool use_platform_file;

    // target sensor ID
    int target_sensor;

    // tide model
    mb_path tide_model;

    // tide model enable
    bool use_tide_model;

    // log directory
    mb_path log_directory;

    // log enable
    bool make_logs;

    // trn_log_directory
    // uses log_directory, defaults to . on failure
    char *trn_log_dir;

    // swath width (dec deg)
    double swath_width;

    // sonar output soundings
    int n_output_soundings;

    // median filter threshold
    double median_filter_threshold;

    // median filter cross-track
    int median_filter_n_across;

    // median filter along-track
    int median_filter_n_along;

    // median filter enable
    bool median_filter_en;

    // median filter buffer depth
    int n_buffer_max;

    // MB1 server hostname
    char *mb1svr_host;

    // MB1 server port
    int mb1svr_port;

    // TRN server hostname
    char *trnsvr_host;

    // TRN server port
    int trnsvr_port;

    // TRN UDP server hostname
    char *trnusvr_host;//TRNU_HOST_DFL

    // TRN UDP server port
    int trnusvr_port;

    // TRN UDP MCAST server group
    char *trnumsvr_group;//TRNU_GROUP_DFL

    // TRN UDP MCAST server port
    int trnumsvr_port;

    // TRN UDP MCAST server TTL (time to live)
    int trnumsvr_ttl;

    // TRN output flags
    output_mode_t output_flags;

    // MB1 server heartbeat counts
    int mbsvr_hbtok;

    // MB1 server heartbeat timeout (s)
    double mbsvr_hbto;

    // TRN server heartbeat timeout (s)
    double trnsvr_hbto;

    // TRN UDP server heartbeat timeout
    double trnusvr_hbto;

    // TRN processing loop delay (msec)
    int64_t mbtrnpp_loop_delay_msec;

    // profiling interval (s)
    double trn_status_interval_sec;

    // profiling configuration flags
    mstats_flags mbtrnpp_stat_flags;

    // TRN processing enable
    bool trn_enable;

    // TRN UTM zone
    long int trn_utm_zone;

    // TRN map type
    int trn_mtype;

    // TRN sensor type
    int trn_sensor_type;

    // TRN filter type
    int trn_ftype;

    // TRN filter grade
    int trn_fgrade;

    // TRN allow filter reinit
    int trn_freinit;

    // TRN modified weighting
    int trn_mweight;

    // TRN convergence northing covariance limit
    double trn_max_ncov;

    // TRN convergence northing error limit
    double trn_max_nerr;

    // TRN convergence easing covariance limit
    double trn_max_ecov;

    // TRN convergence easting error limit
    double trn_max_eerr;

    // TRN map file
    char *trn_map_file;

    // TRN config file
    char *trn_cfg_file;

    // TRN particles file
    char *trn_particles_file;

    // TRN mission ID
    char *trn_mission_id;

    // TRN process gating modulus
    unsigned int trn_decn;

    // TRN process gating timeout
    double trn_decs;

    // --------------------------
    // mbtrnpp convergence use criteria

    // covariance magnitude maximum
    double covariance_magnitude_max;

    // converged repeat streak minimum
    int convergence_repeat_min;

    // --------------------------
    // mbtrnpp reinit search sizes

    // opt "reinit-search"
    double reinit_search_xy;
    double reinit_search_z;

    // --------------------------
    // TRN reinit triggers

    // TRN reinit gain enable
    bool reinit_gain_enable;

    // TRN reinit file enable
    bool reinit_file_enable;

    // TRN reinit xyoffset enable
    bool reinit_xyoffset_enable;

    // TRN reinit xyoffset max
    double reinit_xyoffset_max;

    // TRN reinit offset_z enable
    bool reinit_zoffset_enable;

    // TRN reinit offset_z max
    double reinit_zoffset_min;

    // TRN reinit offset_z max
    double reinit_zoffset_max;

    // TRN "random-offset"
    bool random_offset_enable;

    // TRN device enum
    int trn_dev;

}mbtrnpp_cfg_t;

// ping buffer size default
#define MBTRNPREPROCESS_BUFFER_DEFAULT 20
#define MBTRNPREPROCESS_OUTPUT_STDOUT 0
#define MBTRNPREPROCESS_OUTPUT_TRN 1
#define MBTRNPREPROCESS_OUTPUT_FILE 2

#define MBTRNPREPROCESS_MB1_HEADER_SIZE 56
#define MBTRNPREPROCESS_MB1_SOUNDING_SIZE 28
#define MBTRNPREPROCESS_MB1_CHECKSUM_SIZE 4

#define MBTRNPREPROCESS_LOGFILE_TIMELENGTH 900.0

#define CHK_STRDUP(s) ( (NULL!=s) ? strdup((NULL!=s?s:"")) : NULL )
#define MEM_CHKFREE(s) if(NULL!=s)free(s)
#define MEM_CHKINVALIDATE(s) do{\
if(NULL!=s)free(s);\
s=NULL;\
}while(0)

#define BOOL2YNC(v) ( v ? 'Y' : 'N' )
#define BOOL2YNS(v) ( v ? "Y" : "N" )
#define BOOL2TF(v) ( v ? "true" : "false" )
#define BOOL2IC(v) ( v ? '1' : '0' )
#define BOOL2II(v) ( v ? 1 : 0 )

#define MBTRNPP_CONF_DEL "="

#define CFG_INPUT_DFL             "datalist.mb-1"
#define CFG_FORMAT_DFL            -1
#define CFG_OUTPUT_FILE_DFL       "stdout"
#define CFG_LOG_DIRECTORY_DFL     "."
#define CFG_SOCKET_DEFINITION_DFL "socket:TRN_SOURCE_HOST:7000:0"
//#define CFG_INPUT_DFL          "socket:localhost:7000:0"
#define CFG_MNEM_SESSION       "SESSION"
#define CFG_MNEM_TRN_SOURCE_HOST "TRN_SOURCE_HOST"
#define CFG_MNEM_TRN_HOST      "TRN_HOST"
#define CFG_MNEM_TRN_SESSION   "TRN_SESSION"
#define CFG_MNEM_TRN_LOGFILES  "TRN_LOGFILES"
#define CFG_MNEM_TRN_MAPFILES  "TRN_MAPFILES"
#define CFG_MNEM_TRN_DATAFILES "TRN_DATAFILES"
#define CFG_MNEM_TRN_CFGFILES  "TRN_CFGFILES"
#define CFG_MNEM_TRN_GROUP     "TRN_GROUP"
#define CFG_TRN_LOG_DIR_DFL    "."
#define CFG_TRN_DEV_DFL        R7KC_DEV_T50

#define OPT_VERBOSE_DFL                   0
#define OPT_INPUT_DFL                     CFG_INPUT_DFL
#define OPT_FORMAT_DFL                    CFG_FORMAT_DFL
#define OPT_PLATFORM_FILE_DFL             NULL
#define OPT_PLATFORM_TARGET_SENSOR_DFL    0
#define OPT_TIDE_MODEL_DFL                NULL
#define OPT_LOG_DIRECTORY_DFL             "."
#define OPT_OUTPUT_DFL                    NULL
#define OPT_PROJECTION_DFL                0
#define OPT_SWATH_WIDTH_DFL               90
#define OPT_SOUNDINGS_DFL                 11
#define OPT_MEDIAN_FILTER_DFL             NULL
#define OPT_MBHBN_DFL                     MB1SVR_HBTOK_DFL
#define OPT_MBHBT_DFL                     MB1SVR_HBTO_DFL
#define OPT_TRNHBT_DFL                    TRNSVR_HBTO_DFL
#define OPT_TRNUHBT_DFL                   TRNUSVR_HBTO_DFL
#define OPT_TRNUMTTL_DFL                  TRNUMSVR_TTL_DFL
#define OPT_DELAY_DFL                     0
#define OPT_STATSEC_DFL                   MBTRNPP_STAT_PERIOD_SEC
#define OPT_STATFLAGS_DFL                 MBTRNPP_STAT_FLAGS_DFL
#define OPT_STATFLAG_STR_DFL              "MSF_STATUS|MSF_EVENT|MSF_ASTAT|MSF_PSTAT"
#define OPT_TRN_EN_DFL                    true
#define OPT_TRN_UTM_DFL                   TRN_UTM_DFL
#define OPT_MAP_DFL                       NULL
#define OPT_CFG_DFL                       NULL
#define OPT_PAR_DFL                       NULL
#define OPT_TRN_MDIR_DFL                  "mb"
#define OPT_TRN_MTYPE_DFL                 TRN_MTYPE_DFL
#define OPT_TRN_SENSOR_TYPE_DFL           TRN_SENSOR_TYPE_DFL
#define OPT_TRN_FTYPE_DFL                 TRN_FTYPE_DFL
#define OPT_TRN_FGRADE_DFL                TRN_FGRADE_DFL
#define OPT_TRN_FREINIT_DFL               TRN_FREINIT_DFL
#define OPT_TRN_MWEIGHT_DFL               TRN_MWEIGHT_DFL
#define OPT_TRN_NCOV_DFL                  TRN_MAX_NCOV_DFL    // 49.
#define OPT_TRN_NERR_DFL                  TRN_MAX_NERR_DFL    // 49.
#define OPT_TRN_ECOV_DFL                  TRN_MAX_ECOV_DFL    // 50.
#define OPT_TRN_EERR_DFL                  TRN_MAX_EERR_DFL    // 50.
#define OPT_MB_OUT_DFL                    NULL
#define OPT_TRN_OUT_DFL                   NULL
#define OPT_TRN_DECN_DFL                  0
#define OPT_TRN_DECS_DFL                  0.0
#define OPT_COVARIANCE_MAGNITUDE_MAX_DFL  5.0
#define OPT_CONVERGENCE_REPEAT_MIN        200
#define OPT_REINIT_SEARCH_XY              60.0
#define OPT_REINIT_SEARCH_Z               5.0
#define OPT_REINIT_GAIN_ENABLE_DFL        false
#define OPT_REINIT_FILE_ENABLE_DFL        false
#define OPT_REINIT_XYOFFSET_ENABLE_DFL    false
#define OPT_REINIT_XYOFFSET_MAX_DFL       0.0
#define OPT_REINIT_ZOFFSET_ENABLE_DFL     false
#define OPT_REINIT_ZOFFSET_MIN_DFL        0.0
#define OPT_REINIT_ZOFFSET_MAX_DFL        0.0
#define OPT_RANDOM_OFFSET_ENABLE_DFL      false
#define OPT_HELP_DFL                      false
#define OPT_TRN_DEV_DFL                   R7KC_DEV_T50

#define MNEM_MAX_LEN 64
#define HOSTNAME_BUF_LEN 256
#define MB_PATH_SIZE 1024
#define LOG_MSG_BUF_SZ 2048
#define MBOUT_OPT_N 16
#define MBSYSOUT_OPT_N 8
#define TRNOUT_OPT_N 16
#define SONAR_READER_CAPACITY_DFL (256 * 1024)
#define SESSION_BUF_LEN 32
#define TRNSESSION_BUF_LEN 9

#define SONAR_SIM_HOST "localhost"

#define MBTPP 1
#define OUTPUT_FLAG_SET(m)  ((m&mbtrn_cfg->output_flags)==0 ? false : true)
#define OUTPUT_FLAG_CLR(m)  ((m&mbtrn_cfg->output_flags)==0 ? true : false)
#define OUTPUT_FLAGS_ZERO() ((mbtrn_cfg->output_flags==0) ? true : false)

#define MBTRN_CFG_NAME    "mbtrn.cfg"
#define MBTRN_CFG_PATH    "."

#define MB1_BLOG_NAME     "mb1"
#define MB1_BLOG_DESC     "mb1 binary data"
#define MBTRNPP_MLOG_NAME "mbtrnpp"
#define MBTRNPP_MLOG_DESC "mbtrnpp message log"
#define RESON_BLOG_NAME   "r7kbin"
#define RESON_BLOG_DESC   "reson 7k frame log"
#define TRNU_ALOG_NAME    "trnu"
#define TRNU_ALOG_DESC    "trnu log"
#define TRNU_BLOG_NAME    "trnub"
#define TRNU_BLOG_DESC    "trnu log (binary)"
#define TRNUM_ALOG_NAME   "trnum"
#define TRNUM_ALOG_DESC   "trnum log"
#define TRNUM_BLOG_NAME   "trnumb"
#define TRNUM_BLOG_DESC   "trnum log (binary)"
#define MB1R_BLOG_NAME    "mb1rbin"
#define MB1R_BLOG_DESC    "mb1r log (binary)"
#define MBTRNPP_LOG_EXT   ".log"
#ifdef WITH_MBTNAV
#define UTM_MONTEREY_BAY 10L
#define UTM_AXIAL        12L
#define TRN_UTM_DFL      UTM_MONTEREY_BAY
#define TRN_MTYPE_DFL    TRN_MAP_BO
#define TRN_SENSOR_TYPE_DFL TRN_SENSOR_MB
#define TRN_FTYPE_DFL    TRN_FILT_PARTICLE
#define TRN_FGRADE_DFL   TRN_FILT_HIGH
#define TRN_FREINIT_DFL  TRN_FILT_REINIT_EN
#define TRN_MWEIGHT_DFL  TRN_MWEIGHT_SUBCLOUD_NISON
#define TRN_OUT_DFL      (TRNW_ODEBUG|TRNW_OLOG)
#define TRNU_HOST_DFL    "localhost"
#define TRNU_PORT_DFL    8000
//#define TRNUM_HOST_DFL   "localhost"
#define TRNUM_GROUP_DFL  "239.255.0.16"
#define TRNUM_PORT_DFL   29000
#define TRNUM_TTL_DFL    32
#define TRNSVR_HOST_DFL  "localhost"
#define TRNSVR_PORT_DFL  28000
#define TRN_XMIT_GAIN_RESON7K_DFL 200.0
#define TRN_XMIT_GAIN_KMALL_DFL -20.0
#ifdef WITH_MB1_READER
#define TRN_XMIT_GAIN_MB1_DFL 0.0
#endif // WITH_MB1_READER

#endif //WITH_MBTNAV
#define SZ_1M (1024 * 1024)
#define SZ_1G (1024 * 1024 * 1024)
#define MBTRNPP_CMD_LINE_BYTES 2048

// MB1 socket output configuration
#define MB1SVR_HOST_DFL "localhost"
#define MB1SVR_PORT_DFL 27000
#define MB1SVR_MSG_CON_LEN 4
#define MB1SVR_HBTOK_DFL 50
#define MB1SVR_HBTO_DFL  0.0
#define TRNSVR_HBTO_DFL  0.0
#define TRNUSVR_HBTO_DFL 0.0
#define TRNUMSVR_TTL_DFL 64

// MSF_STAT_FLAGS define stats processing options
// may include
// MSF_STATUS : status counters
// MSF_EVENT  : event/error counters
// MSF_ASTAT  : aggregate stats
// MSF_PSTAT  : periodic stats
// MSF_READER : r7kr reader stats
#define MBTRNPP_STAT_FLAGS_DFL (MSF_STATUS | MSF_EVENT | MSF_ASTAT | MSF_PSTAT)
/// @def MBTRNPP_STAT_PERIOD_SEC
#define MBTRNPP_STAT_PERIOD_SEC ((double)20.0)


mbtrnpp_opts_t mbtrn_opts_s, *mbtrn_opts=&mbtrn_opts_s;
mbtrnpp_cfg_t mbtrn_cfg_s, *mbtrn_cfg=&mbtrn_cfg_s;

static char program_name[] = "mbtrnpp";

char *mbtrn_cfg_path=NULL;

mlog_id_t mb1_blog_id = MLOG_ID_INVALID;
mlog_id_t mbtrnpp_mlog_id = MLOG_ID_INVALID;
mlog_id_t reson_blog_id = MLOG_ID_INVALID;
mlog_id_t trnu_alog_id = MLOG_ID_INVALID;
mlog_id_t trnu_blog_id = MLOG_ID_INVALID;
mlog_id_t mb1r_blog_id = MLOG_ID_INVALID;

mlog_config_t mb1_blog_conf = {100 * SZ_1M, ML_NOLIMIT, ML_NOLIMIT, ML_OSEG | ML_LIMLEN, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t mbtrnpp_mlog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t reson_blog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t trnu_alog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t trnu_blog_conf = {100 * SZ_1M, ML_NOLIMIT, ML_NOLIMIT, ML_OSEG | ML_LIMLEN, ML_FILE, ML_TFMT_ISO1806};
mlog_config_t mb1r_blog_conf = {ML_NOLIMIT, ML_NOLIMIT, ML_NOLIMIT, ML_MONO, ML_FILE, ML_TFMT_ISO1806};

char *mb1_blog_path = NULL;
char *mbtrnpp_mlog_path = NULL;
char *reson_blog_path = NULL;
char *trnu_alog_path = NULL;
char *trnu_blog_path = NULL;
char *mb1r_blog_path = NULL;

mfile_flags_t flags = MFILE_RDWR | MFILE_APPEND | MFILE_CREATE;
mfile_mode_t mode = MFILE_RU | MFILE_WU | MFILE_RG | MFILE_WG;

netif_t *mb1svr=NULL;

#ifdef WITH_MBTNAV
trn_config_t *trn_cfg = NULL;
unsigned int trn_dec_cycles=0;
double trn_dec_time=0.0;
wtnav_t *trn_instance = NULL;
trnw_oflags_t trn_oflags=TRN_OUT_DFL;
netif_t *trnsvr=NULL;
netif_t *trnusvr=NULL;
netif_t *trnumsvr=NULL;
int s_mbtrnpp_trnu_reset_callback();
trnuif_res_t rr_resources={0},*g_trnu_res=&rr_resources;
FILE *output_trn_fp = NULL;

#endif // WITH_MBTNAV

typedef enum{RF_NONE=0,RF_FORCE_UPDATE=0x1,RF_RELEASE=0x2}mb_resource_flag_t;

// profiling - event channels
typedef enum {
    MBTPP_EV_MB_CYCLES = 0,
    MBTPP_EV_MB_CONN,
    MBTPP_EV_MB_DISN,
    MBTPP_EV_MB_PUBN,
    MBTPP_EV_MB_REINIT,

    MBTPP_EV_MB_GAIN_LO,
    MBTPP_EV_MB_FILE,
    MBTPP_EV_MB_xyoffset,
    MBTPP_EV_MB_offset_z,
    MBTPP_EV_MB_TRNUCLI_RESET,

    MBTPP_EV_MB_EOF,
    MBTPP_EV_MB_NONSURVEY,
    MBTPP_EV_EMBGETALL,
    MBTPP_EV_EMBFAILURE,
    MBTPP_EV_EMBFRAMERD,

    MBTPP_EV_EMBLOGWR,
    MBTPP_EV_EMBSOCKET,
    MBTPP_EV_EMBCON,
    MBTPP_EV_EMBPUB,
#ifdef WITH_MBTNAV
    MBTPP_EV_TRN_PROCN,

    MBTPP_EV_TRNU_PUBN,
    MBTPP_EV_TRNU_PUBEMPTYN,
    MBTPP_EV_ETRNUPUB,
    MBTPP_EV_ETRNUPUBEMPTY,
#endif
    MBTPP_EV_COUNT
} mbtrnpp_stevent_id;

// profiling - status channels
typedef enum {
    MBTPP_STA_MB_FWRITE_BYTES=0,
    MBTPP_STA_MB_SYNC_BYTES,
    MBTPP_STA_COUNT
} mbtrnpp_ststatus_id;

// profiling - measurement channels
typedef enum {
  MBTPP_CH_MB_GETALL_XT = 0,
  MBTPP_CH_MB_PING_XT,
  MBTPP_CH_MB_LOG_XT,
  MBTPP_CH_MB_DTIME_XT,
  MBTPP_CH_MB_GETFAIL_XT,
  MBTPP_CH_MB_POST_XT,
  MBTPP_CH_MB_STATS_XT,
  MBTPP_CH_MB_CYCLE_XT,
  MBTPP_CH_MB_FWRITE_XT,
  MBTPP_CH_MB_PROC_MB1_XT,
#ifdef WITH_MBTNAV
    MBTPP_CH_TRN_UPDATE_XT,
    MBTPP_CH_TRN_BIASEST_XT,
    MBTPP_CH_TRN_NREINITS_XT,
    MBTPP_CH_TRN_TRNU_PUB_XT,
    MBTPP_CH_TRN_TRNUM_PUB_XT,
    MBTPP_CH_TRN_TRNU_LOG_XT,
    MBTPP_CH_TRN_TRNU_BLOG_XT,
    MBTPP_CH_TRN_PROC_XT,
    MBTPP_CH_TRN_TRNSVR_XT,
    MBTPP_CH_TRN_TRNUSVR_XT,
    MBTPP_CH_TRN_TRNUMSVR_XT,
    MBTPP_CH_TRN_PROC_TRN_XT,
#endif
    MBTPP_CH_COUNT
} mbtrnpp_stchan_id;

// profiling - event channel labels
const char *mbtrnpp_stevent_labels[] = {
    "mb_cycles", "mb_con", "mb_dis", "mb_pub_n", "mb_reinit", "mb_gain_lo", "mb_file",
    "mb_xyoffset", "mb_offset_z", "mb_trnucli_reset", "mb_eof", "mb_nonsurvey", "e_mbgetall", "e_mbfailure",
    "e_mb_frame_rd", "e_mb_log_wr", "e_mbsocket", "e_mbcon", "e_mbpub"
#ifdef WITH_MBTNAV
    ,"trn_proc_n","trnu_pub_n","trnu_pubempty_n","e_trnu_pub","e_trnu_pubempty"
#endif
};

// profiling - status channel labels
const char *mbtrnpp_ststatus_labels[] = {
    "mb_fwrite_bytes",
    "mb_sync_bytes"
};

// profiling - measurement channel labels
const char *mbtrnpp_stchan_labels[] = {
    "mb_getall_xt",  "mb_ping_xt", "mb_log_xt", "mb_dtime_xt",
    "mb_getfail_xt", "mb_post_xt", "mb_stats_xt", "mb_cycle_xt", "mb_fwrite_xt",
    "mb_proc_mb1_xt"
#ifdef WITH_MBTNAV
    , "trn_update_xt", "trn_biasest_xt", "trn_nreinits_xt",
    "trn_trnu_pub_xt", "trn_trnums_pub_xt", "trn_trnu_log_xt", "trn_trnu_blog_xt", "trn_proc_xt",
    "trn_trnsvr_xt", "trn_trnusvr_xt", "trn_trnumsvr_xt", "trn_proc_trn_xt"
#endif
};

const char **mbtrnpp_stats_labels[MSLABEL_COUNT] = {mbtrnpp_stevent_labels, mbtrnpp_ststatus_labels, mbtrnpp_stchan_labels};
mstats_profile_t *app_stats = NULL;
mstats_t *reader_stats = NULL;
// stats interval end
static double stats_prev_end = 0.0;
// stats interval start
static double stats_prev_start = 0.0;
// system clock resolution logging enable/disable
static bool log_clock_res = true;

#ifdef MST_STATS_EN
#define MBTRNPP_UPDATE_STATS(p, l, f) (mbtrnpp_update_stats(p, l, f))
#else
#define MBTRNPP_UPDATE_STATS(p, l, f)
#endif // MST_STATS_EN

//mstats_flags mbtrnpp_stat_flags = MBTRNPP_STAT_FLAGS_DFL;

int mbtrnpp_openlog(int verbose, mb_path log_directory, FILE **logfp, int *error);
int mbtrnpp_closelog(int verbose, FILE **logfp, int *error);
int mbtrnpp_postlog(int verbose, FILE *logfp, char *message, int *error);
int mbtrnpp_logparameters(int verbose, FILE *logfp, char *input, int format, char *output, double swath_width,
                          int n_output_soundings, bool median_filter, int median_filter_n_across, int median_filter_n_along,
                          double median_filter_threshold, int n_buffer_max, int *error);
int mbtrnpp_logstatistics(int verbose, FILE *logfp, int n_pings_read, int n_soundings_read, int n_soundings_valid_read,
                          int n_soundings_flagged_read, int n_soundings_null_read, int n_pings_written, int n_soundings_trimmed,
                          int n_soundings_decimated, int n_soundings_flagged, int n_soundings_written, int *error);
int mbtrnpp_init_debug(int verbose);

int mbtrnpp_reson7kr_input_open(int verbose, void *mbio_ptr, char *definition, int *error);
int mbtrnpp_reson7kr_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
int mbtrnpp_reson7kr_input_close(int verbose, void *mbio_ptr, int *error);
int mbtrnpp_kemkmall_input_open(int verbose, void *mbio_ptr, char *definition, int *error);
int mbtrnpp_kemkmall_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
int mbtrnpp_kemkmall_input_close(int verbose, void *mbio_ptr, int *error);
#ifdef WITH_MB1_READER
int mbtrnpp_mb1r_input_open(int verbose, void *mbio_ptr, char *definition, int *error);
int mbtrnpp_mb1r_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
int mbtrnpp_mb1r_input_close(int verbose, void *mbio_ptr, int *error);
#endif // WITH_MB1_READER

// Configuration helper functions

// get TRN session string (YYYY-DDD used in TRN log directories)
static char *s_mbtrnpp_trnsession_str(char **pdest, size_t len, mb_resource_flag_t flags);
// get mbtrnpp session string (YYYYMMDD-hhmmss used in log names)
static char *s_mbtrnpp_session_str(char **pdest, size_t len, mb_resource_flag_t flags);
// get mbtrnpp command line string (used in logs, debugging)
static char *s_mbtrnpp_cmdline_str(char **pdest, size_t len, int argc, char **argv, mb_resource_flag_t flags);
// show mbtrnpp config struct contents
static int s_mbtrnpp_show_cfg(FILE * fpout, mbtrnpp_cfg_t *self, bool hashstart, int indent);
// show mbtrnpp option struct contents
static int s_mbtrnpp_show_opts(FILE *, mbtrnpp_opts_t *opts, bool hashstart, int indent);
// write configuration to string (w/ various formatting parameters)
static int s_mbtrnpp_cfgstr(char **pdest, size_t olen, mbtrnpp_cfg_t *self,
                            const char *prefix, const char *kvsep,
                            const char *delim, int indent, int wkey, int wval);
// write options to string (w/ various formatting parameters)
static int s_mbtrnpp_optstr(char **pdest, size_t olen, mbtrnpp_opts_t *self,
                            const char *prefix, const char *kvsep,
                            const char *delim, int indent, int wkey, int wval);
// get config mnemonic value
char *s_mnem_value(char **pdest, size_t len, const char *key);
// substitute mnemonic value into string
// (dest must be dynamically allocated, caller must free)
char *s_sub_mnem(char **pdest, size_t len, char *src,const char *pkey,const char *pval);
#ifdef WITH_TEST_MNEM_SUB
// test mnemonic substitution
static int s_test_mnem();
#endif
// initialize mbtrnpp config struct
static int s_mbtrnpp_init_cfg(mbtrnpp_cfg_t *cfg);
// initialize mbtrnpp options struct
static int s_mbtrnpp_init_opts(mbtrnpp_opts_t *opts);
// release dynamically allocated resources in options struct
static void s_mbtrnpp_free_opts(mbtrnpp_opts_t **pself);
// release dynamically allocated resources in config struct
static void s_mbtrnpp_free_cfg(mbtrnpp_cfg_t **pself);
// parse option: --output
static int s_parse_opt_output(mbtrnpp_cfg_t *cfg, char *opt_str);
// parse option: --mbout
static int s_parse_opt_mbout(mbtrnpp_cfg_t *cfg, char *opt_str);
// parse option: --trnout
static int s_parse_opt_trnout(mbtrnpp_cfg_t *cfg, char *opt_str);
// parse option: --logdir
static int s_parse_opt_logdir(mbtrnpp_cfg_t *cfg, char *opt_str);
// parse option: --input
static int s_parse_opt_input(mbtrnpp_cfg_t *cfg, char *opt_str);
// get --config option from cmdline, if provided
static char *s_mbtrnpp_peek_opt_cfg(int argc, char **argv, char **buf, size_t len);

// Primary configuration functions

// key/value parsing function (mkvc_parser_fn)
static int s_mbtrnpp_kvparse_fn(char *key, char *val, void *opts);
// load configuration file (override run-time defaults)
static int s_mbtrnpp_load_config(char *config_path, mbtrnpp_opts_t *opts);
// load command line options (override run-time/config file defaults)
static int s_mbtrnpp_process_cmdline(int argc, char **argv, mbtrnpp_opts_t *opts);
// parse options to configuration values
static int s_mbtrnpp_configure(mbtrnpp_cfg_t *cfg, mbtrnpp_opts_t *opts);
// validate configuration
static int s_mbtrnpp_validate_config(mbtrnpp_cfg_t *cfg);

int mbtrnpp_update_stats(mstats_profile_t *stats, mlog_id_t log_id, mstats_flags flags);
int mbtrnpp_process_mb1(char *mb1, size_t len, trn_config_t *cfg);

#ifdef WITH_MBTNAV
int mbtrnpp_init_trn(wtnav_t **pdest, int verbose, trn_config_t *cfg);
int mbtrnpp_init_trnsvr(netif_t **psvr, wtnav_t *trn, char *host, int port, bool verbose);
int mbtrnpp_init_mb1svr(netif_t **psvr, char *host, int port, bool verbose);
int mbtrnpp_init_trnusvr(netif_t **psvr, char *host, int port, wtnav_t *trn, bool verbose);
int mbtrnpp_init_trnumsvr(netif_t **psvr, char *host, int port, wtnav_t *trn, bool verbose);
int mbtrnpp_trn_process_mb1(wtnav_t *tnav, mb1_t *mb1, trn_config_t *cfg);
int mbtrnpp_trn_update(wtnav_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out, trn_config_t *cfg);
int mbtrnpp_trn_get_bias_estimates(wtnav_t *self, wposet_t *pt, trn_update_t *pstate);
int mbtrnpp_trn_publish(trn_update_t *pstate, trn_config_t *cfg);
int mbtrnpp_check_reinit(trn_update_t *pstate, trn_config_t *cfg);
int mbtrnpp_trn_pub_ostream(trn_update_t *update, FILE *stream);
int mbtrnpp_trn_pub_odebug(trn_update_t *update);
int mbtrnpp_trn_pub_olog(trn_update_t *update, mlog_id_t log_id);
//int mbtrnpp_trnu_pub_osocket(trn_update_t *update, msock_socket_t *pub_sock);
int mbtrnpp_trnu_pub_osocket(trn_update_t *update, netif_t *netif);
int mbtrnpp_trnu_pubempty_osocket(double time, double lat, double lon, double depth, netif_t *netif);
char *mbtrnpp_trn_updatestr(char *dest, int len, trn_update_t *update, int indent);
#endif // WITH_MBTNAV

// TRN reinit flag - forces reinitializing the TRN filter
bool reinit_flag=true;

/* counting convergence or lack of it */
int n_converged_streak = 0;
int n_unconverged_streak = 0;
int n_converged_tot = 0;
int n_unconverged_tot = 0;
int n_reinit = 0;
int n_reinit_since_use = 10;
double reinit_time = 0.0;
bool converged = false;
bool reinitialized = true;
bool use_trn_offset = false;
double use_offset_time = 0.0;
double use_offset_e = 0.0;
double use_offset_n = 0.0;
double use_offset_z = 0.0;
double use_covariance[4] = {0.0, 0.0, 0.0, 0.0};

char mRecordBuf[MBSYS_KMBES_MAX_NUM_MRZ_DGMS][64*1024];
/*--------------------------------------------------------------------*/

static char *s_mbtrnpp_trnsession_str(char **pdest, size_t len, mb_resource_flag_t flags)
{
    static bool initialized=false;
    static char session_date[TRNSESSION_BUF_LEN] = {0};
    char *retval=session_date;

    // lazy initialize session time string to use
    // in log file names
    time_t rawtime;
    struct tm *gmt;

    time(&rawtime);
    // Get GMT time
    gmt = gmtime(&rawtime);

    if(!initialized || ((flags&RF_FORCE_UPDATE)!=0)){
        initialized=true;
        // format YYYY.DDD
        strftime(session_date,TRNSESSION_BUF_LEN,"%Y.%j",gmt);
    }

    if(NULL!=pdest){
        // return requested
        if(NULL==*pdest){
            *pdest=strdup(session_date);
            retval=*pdest;
        } else {
            if(len>=TRNSESSION_BUF_LEN){
                sprintf(*pdest,"%s",session_date);
                retval=*pdest;
            } else {
                fprintf(stderr,"ERR - dest buffer too small");
            }
        }
    }
    return retval;
}
static char *s_mbtrnpp_session_str(char **pdest, size_t len, mb_resource_flag_t flags)
{
    static bool initialized=false;
    static char session_date[SESSION_BUF_LEN] = {0};
    char *retval=session_date;

    // lazy initialize session time string to use
    // in log file names
    time_t rawtime;
    struct tm *gmt;

    time(&rawtime);
    // Get GMT time
    gmt = gmtime(&rawtime);

    if(!initialized || ((flags&RF_FORCE_UPDATE)!=0)){
        initialized=true;
        // format YYYYMMDD-HHMMSS
        snprintf(session_date, SESSION_BUF_LEN, "%04d%02d%02d-%02d%02d%02d", (gmt->tm_year + 1900), gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour,
                gmt->tm_min, gmt->tm_sec);
    }

    if(NULL!=pdest){
        // return requested
        if(NULL==*pdest){
            *pdest=strdup(session_date);
            retval=*pdest;
        } else {
            if(len>=SESSION_BUF_LEN){
                sprintf(*pdest,"%s",session_date);
                retval=*pdest;
            } else {
                fprintf(stderr,"ERR - dest buffer too small");
            }
        }
    }
    return retval;
}

static char *s_mbtrnpp_cmdline_str(char **pdest, size_t len, int argc, char **argv, mb_resource_flag_t flags)
{
    static char *cmd_line=NULL;
    char *retval=cmd_line;

    if(argc>0 && NULL!=argv){
        static bool initialized=false;
        static size_t slen=0;
        // lazy initialize command line string
        if(!initialized || ((flags&RF_FORCE_UPDATE)!=0)){
            initialized=true;
            MEM_CHKINVALIDATE(cmd_line);
            // calculate buffer len (+1 for NULL)
            len=1;
            for(int i = 0; i < argc; i++){
                // arg len + space
                slen+=strlen(argv[i])+1;
            }
            // allocate buffer
            cmd_line=(char *)malloc(slen*sizeof(char));
            if(NULL!=cmd_line){
                memset(cmd_line,0,len);
                char *ip=cmd_line;
                for(int i = 0; i < argc; i++){
                    sprintf(ip,"%s%s",(i==0?"":" "),argv[i]);
                    ip=cmd_line+strlen(cmd_line);
                }
            } else {
                len=0;
                initialized=false;
            }
        }
        if((flags&RF_RELEASE)!=0){
            MEM_CHKINVALIDATE(cmd_line);
        }
        if(NULL!=pdest){
            // return requested
            if(NULL==*pdest){
                *pdest=CHK_STRDUP(cmd_line);
                retval=*pdest;
            } else {
                if(len>=slen){
                    sprintf(*pdest,"%s",cmd_line);
                    retval=*pdest;
                } else {
                    fprintf(stderr,"ERR - dest buffer too small");
                }
            }
        }
    }
    return retval;
}

char *s_mnem_value(char **pdest, size_t len, const char *key)
{
    char *retval=NULL;
    if( NULL!=key){

        char *val=NULL;
        char *alt=NULL;

        if(strcmp(key,CFG_MNEM_TRN_SOURCE_HOST)==0){
            val=CHK_STRDUP(getenv(key));
            if(NULL==val){
                // if unset, use local IP
                char host[HOSTNAME_BUF_LEN]={0};
                if(gethostname(host, HOSTNAME_BUF_LEN)==0 && strlen(host)>0){
                    struct hostent *host_entry;

                    if( (host_entry = gethostbyname(host))!=NULL){
                        //Convert into IP string
                        char *s =inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
                        val = CHK_STRDUP(s);
                    } //find host information
                }
                if(NULL==val){
                    val=strdup("localhost");
                }
            }
        } else if(strcmp(key,CFG_MNEM_SESSION)==0){

            val=CHK_STRDUP(s_mbtrnpp_session_str(NULL,0,RF_NONE));

        } else if(strcmp(key,CFG_MNEM_TRN_SESSION)==0){

            val=CHK_STRDUP(s_mbtrnpp_trnsession_str(NULL,0,RF_NONE));

        } else if(strcmp(key,CFG_MNEM_TRN_HOST)==0){

            // try env
            val=CHK_STRDUP(getenv(key));

            if(NULL==val){
                // if unset, use local IP
                char host[HOSTNAME_BUF_LEN]={0};
                if(gethostname(host, HOSTNAME_BUF_LEN)==0 && strlen(host)>0){
                    struct hostent *host_entry;

                    if( (host_entry = gethostbyname(host))!=NULL){
                        //Convert into IP string
                        char *s =inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
                        val = CHK_STRDUP(s);
                    } //find host information
                }
                if(NULL==val){
                    val=strdup("localhost");
                }
            }
        } else if(strcmp(key,CFG_MNEM_TRN_GROUP)==0){

            // try env
            val=CHK_STRDUP(getenv(key));

            if(NULL==val){
                // if unset, use default
                val=strdup(TRNUM_GROUP_DFL);
            }
        } else if(strcmp(key,CFG_MNEM_TRN_LOGFILES)==0 ||
                 strcmp(key,CFG_MNEM_TRN_MAPFILES)==0 ||
                 strcmp(key,CFG_MNEM_TRN_CFGFILES)==0 ||
                 strcmp(key,CFG_MNEM_TRN_DATAFILES)==0){
            // try env
            val=CHK_STRDUP(getenv(key));
            alt=".";
       }// else unsupported option

        if(NULL!=val || NULL!=alt){
            char *dest=NULL;
            size_t dlen = (NULL!=val ? strlen(val)+1 : strlen(alt)+1);
            if(NULL==pdest){
                dest=(char *)malloc(dlen);
                memset(dest,0,dlen);
                retval=dest;
            } else {
                if(NULL!=*pdest){
                    if(len>0){
                        // must fit
                        if(len<=dlen){
                            // OK
                            dest=*pdest;
                        } else {
                            fprintf(stderr,"%s - dest buffer too small\n",__func__);
                        }
                    } else {
                        // OK to realloc it
                        *pdest = (char *)realloc(*pdest,dlen);
                        dest=*pdest;
                        retval=*pdest;
                    }
                } else {
                    *pdest=(char *)malloc(dlen);
                    memset(*pdest,0,dlen);
                    retval=*pdest;
                    dest=*pdest;
                }
            }
            if(NULL!=dest){
                sprintf(dest,"%s",(NULL!=val ? val : alt));
            } else {MX_TRACE();}
//            fprintf(stderr,"%s:%d - dest[%p/%s] pdest[%p/%s] retval[%s]\n",__func__,__LINE__,dest,dest,*pdest,*pdest,retval);

        } else {MX_TRACE();}

        MEM_CHKINVALIDATE(val);
    }// else invalid arg
    return retval;
}

char *s_sub_mnem(char **pdest, size_t len, char *src,const char *pkey,const char *pval)
{

    char *retval=NULL;
    if(NULL!=src && NULL!=pkey && strlen(pkey)>0 && NULL!=pval){
        char *result;
        int i, cnt = 0;
        int vlen = strlen(pval);
        int klen = strlen(pkey);

        // Counting the number of times old word
        // occur in the string
        for (i = 0; src[i] != '\0'; i++){
            if (strstr(&src[i], pkey) == &src[i]){
                cnt++;
                // Jumping to index after the old word.
                i += klen - 1;
            }
        }

        if(cnt > 0){
           size_t new_size=(i + cnt * (vlen - klen) + 1);
            // Making new string of enough length
            result = (char *)malloc(new_size);

            i = 0;
            char *cur=src;
            while (*cur){
                // compare the substring with the result
                if (strstr(cur, pkey) == cur){
                    strcpy(&result[i], pval);
                    i += vlen;
                    cur += klen;
                } else {
                    result[i++] = *cur++;
                }
            }

            result[i] = '\0';

            if(NULL==pdest){
                // return (caller must free)
                retval=result;
            } else {
                if(NULL==*pdest){
                    *pdest=result;
                } else if(*pdest==src){
                    if(len==0){
                        // OK to realloc
                        *pdest=(char *)realloc(*pdest,new_size);
                        sprintf(*pdest,"%s",result);
                    } else if(len>0 && strlen(result)<=len){
                        sprintf(*pdest,"%s",result);
                    } else {
                        fprintf(stderr,"ERR - dest buffer too small [%zu/%zu]\n",len,new_size);
                    }
                    // not returning result, free
                    MEM_CHKFREE(result);
                } else {
                    if(len>0 && strlen(result)<=len){
                        sprintf(*pdest,"%s",result);
                    } else {
                        fprintf(stderr,"ERR - dest buffer too small [%zu/%zu]\n",len,new_size);
                    }
                    // not returning result, free
                    MEM_CHKFREE(result);
                }
                retval=*pdest;
            }
        }
    }
//    fprintf(stderr,"%d - ret dest[%s]\n",__LINE__,*pdest);
    return retval;
}
#ifdef WITH_TEST_MNEM_SUB
static int s_test_mnem()
{
    char *opt_session = strdup("test_session-SESSION--");
    char *opt_trnsrchost=strdup("test_trnsrchost-TRN_SOURCE_HOST--");
    char *opt_trnhost=strdup("test_trnhost-TRN_HOST--");
    char *opt_trnsession = strdup("test_trnsession-TRN_SESSION--");
    char *opt_trnlog = strdup("test_trnlog-TRN_LOGFILES--");
    char *opt_trnmap = strdup("test_trnmap-TRN_MAPFILES--");
    char *opt_trndata = strdup("test_trndata-TRN_DATAFILES--");
    char *opt_trncfg = strdup("test_trncfg-TRN_CFGFILES--");
    char *opt_trngroup=strdup("test_trngroup-TRN_GROUP--");

    char *val=NULL;
    s_sub_mnem(&opt_session,0,opt_session,CFG_MNEM_SESSION,s_mnem_value(&val,0,CFG_MNEM_SESSION));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trnsrchost,0,opt_trnsrchost,CFG_MNEM_TRN_SOURCE_HOST,s_mnem_value(&val,0,CFG_MNEM_TRN_SOURCE_HOST));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trnhost,0,opt_trnhost,CFG_MNEM_TRN_HOST,s_mnem_value(&val,0,CFG_MNEM_TRN_HOST));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trngroup,0,opt_trngroup,CFG_MNEM_TRN_GROUP,s_mnem_value(&val,0,CFG_MNEM_TRN_GROUP));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trnsession,0,opt_trnsession,CFG_MNEM_TRN_SESSION,s_mnem_value(&val,0,CFG_MNEM_TRN_SESSION));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trnlog,0,opt_trnlog,CFG_MNEM_TRN_LOGFILES,s_mnem_value(&val,0,CFG_MNEM_TRN_LOGFILES));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trnmap,0,opt_trnmap,CFG_MNEM_TRN_MAPFILES,s_mnem_value(&val,0,CFG_MNEM_TRN_MAPFILES));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trndata,0,opt_trndata,CFG_MNEM_TRN_DATAFILES,s_mnem_value(&val,0,CFG_MNEM_TRN_DATAFILES));
    MEM_CHKINVALIDATE(val);
    s_sub_mnem(&opt_trncfg,0,opt_trncfg,CFG_MNEM_TRN_CFGFILES,s_mnem_value(&val,0,CFG_MNEM_TRN_CFGFILES));
    MEM_CHKINVALIDATE(val);

    fprintf(stderr,"%s:%d - opt_session    [%s]\n",__func__,__LINE__,opt_session);
    fprintf(stderr,"%s:%d - opt_trnsrchost [%s]\n",__func__,__LINE__,opt_trnsrchost);
    fprintf(stderr,"%s:%d - opt_trnhost    [%s]\n",__func__,__LINE__,opt_trnhost);
    fprintf(stderr,"%s:%d - opt_trnsession [%s]\n",__func__,__LINE__,opt_trnsession);
    fprintf(stderr,"%s:%d - opt_trnlog     [%s]\n",__func__,__LINE__,opt_trnlog);
    fprintf(stderr,"%s:%d - opt_trnmap     [%s]\n",__func__,__LINE__,opt_trnmap);
    fprintf(stderr,"%s:%d - opt_trndata    [%s]\n",__func__,__LINE__,opt_trndata);
    fprintf(stderr,"%s:%d - opt_trncfg     [%s]\n",__func__,__LINE__,opt_trncfg);

    MEM_CHKFREE(opt_session);
    MEM_CHKFREE(opt_trnsrchost);
    MEM_CHKFREE(opt_trnhost);
    MEM_CHKFREE(opt_trnsession);
    MEM_CHKFREE(opt_trnlog);
    MEM_CHKFREE(opt_trnmap);
    MEM_CHKFREE(opt_trncfg);
    MEM_CHKFREE(opt_trndata);

    return 0;
}
#endif
static int s_mbtrnpp_init_cfg(mbtrnpp_cfg_t *cfg)
{
    int retval =-1;
    if(NULL!=cfg){
        cfg->verbose=0;
        cfg->input_mode=INPUT_MODE_FILE;
        memset(cfg->socket_definition,0,MB_PATH_SIZE);
        sprintf(cfg->socket_definition,"%s",CFG_SOCKET_DEFINITION_DFL);
        memset(cfg->output_mb1_file,0,MB_PATH_SIZE);
        sprintf(cfg->output_mb1_file,"%s",CFG_OUTPUT_FILE_DFL);
        memset(cfg->output_trn_file,0,MB_PATH_SIZE);
        sprintf(cfg->output_trn_file,"%s",CFG_OUTPUT_FILE_DFL);
        memset(cfg->input,0,MB_PATH_SIZE);
        sprintf(cfg->input,"%s",CFG_INPUT_DFL);
        cfg->format=0;
        memset(cfg->platform_file,0,MB_PATH_SIZE);
        cfg->use_platform_file=false;
        cfg->target_sensor=-1;
        memset(cfg->tide_model,0,MB_PATH_SIZE);
        cfg->use_tide_model=false;
        memset(cfg->log_directory,0,MB_PATH_SIZE);
        sprintf(cfg->log_directory,"%s",CFG_LOG_DIRECTORY_DFL);
        cfg->make_logs=false;
        cfg->trn_log_dir=strdup(CFG_TRN_LOG_DIR_DFL);
        cfg->swath_width=150;
        cfg->n_output_soundings=101;
        cfg->median_filter_threshold=0.5;
        cfg->median_filter_n_across=1;
        cfg->median_filter_n_along=1;
        cfg->median_filter_en=false;
        cfg->n_buffer_max=1;

        cfg->mb1svr_host=strdup(MB1SVR_HOST_DFL);
        cfg->mb1svr_port=MB1SVR_PORT_DFL;
        cfg->trnsvr_port=TRNSVR_PORT_DFL;
        cfg->trnsvr_host=strdup(TRNSVR_HOST_DFL);
        cfg->trnusvr_port=TRNU_PORT_DFL;
        cfg->trnusvr_host=strdup(TRNU_HOST_DFL);
        cfg->trnumsvr_port=TRNUM_PORT_DFL;
        cfg->trnumsvr_group=strdup(TRNUM_GROUP_DFL);
        cfg->output_flags=OUTPUT_MBTRNPP_MSG;
        cfg->mbsvr_hbtok=MB1SVR_HBTOK_DFL;
        cfg->mbsvr_hbto=MB1SVR_HBTO_DFL;
        cfg->trnsvr_hbto=TRNSVR_HBTO_DFL;
        cfg->trnusvr_hbto=TRNUSVR_HBTO_DFL;
        cfg->trnumsvr_ttl=TRNUMSVR_TTL_DFL;
        cfg->mbtrnpp_loop_delay_msec=0;
        cfg->trn_status_interval_sec=MBTRNPP_STAT_PERIOD_SEC;
        cfg->mbtrnpp_stat_flags=MBTRNPP_STAT_FLAGS_DFL;
        cfg->trn_enable=false;
        cfg->trn_utm_zone=TRN_UTM_DFL;
        cfg->trn_mtype=TRN_MTYPE_DFL;
        cfg->trn_sensor_type=TRN_SENSOR_TYPE_DFL;
        cfg->trn_ftype=TRN_FTYPE_DFL;
        cfg->trn_fgrade=TRN_FGRADE_DFL;
        cfg->trn_freinit=TRN_FREINIT_DFL;
        cfg->trn_mweight=TRN_MWEIGHT_DFL;
        cfg->trn_max_ncov=TRN_MAX_NCOV_DFL;
        cfg->trn_max_nerr=TRN_MAX_NERR_DFL;
        cfg->trn_max_ecov=TRN_MAX_ECOV_DFL;
        cfg->trn_max_eerr=TRN_MAX_EERR_DFL;
        cfg->trn_map_file=NULL;
        cfg->trn_cfg_file=NULL;
        cfg->trn_particles_file=NULL;
        cfg->trn_mission_id=NULL;
        cfg->trn_decn=0;
        cfg->trn_decs=0.0;
        cfg->covariance_magnitude_max = OPT_COVARIANCE_MAGNITUDE_MAX_DFL;
        cfg->convergence_repeat_min = OPT_CONVERGENCE_REPEAT_MIN;
        cfg->reinit_search_xy = OPT_REINIT_SEARCH_XY;
        cfg->reinit_search_z = OPT_REINIT_SEARCH_Z;
        cfg->reinit_gain_enable = false;
        cfg->reinit_file_enable = false;
        cfg->reinit_xyoffset_enable = false;
        cfg->reinit_xyoffset_max = 0.0;
        cfg->reinit_zoffset_enable = false;
        cfg->reinit_zoffset_min = 0.0;
        cfg->reinit_zoffset_max = 0.0;
        cfg->random_offset_enable = false;
        cfg->trn_dev = CFG_TRN_DEV_DFL;
        retval=0;
    }
    return retval;
}
static int s_mbtrnpp_init_opts(mbtrnpp_opts_t *opts)
{
    int retval =-1;
    if(NULL!=opts){
        opts->verbose=OPT_VERBOSE_DFL;
        opts->input=strdup(OPT_INPUT_DFL);
        opts->format=OPT_FORMAT_DFL;
        opts->platform_file=CHK_STRDUP(OPT_PLATFORM_FILE_DFL);
        opts->platform_target_sensor=OPT_PLATFORM_TARGET_SENSOR_DFL;
        opts->tide_model=OPT_TIDE_MODEL_DFL;
        opts->log_directory=strdup(OPT_LOG_DIRECTORY_DFL);
        opts->output=CHK_STRDUP(OPT_OUTPUT_DFL);
        opts->projection=OPT_PROJECTION_DFL;
        opts->swath_width=OPT_SWATH_WIDTH_DFL;
        opts->soundings=OPT_SOUNDINGS_DFL;
        opts->median_filter=CHK_STRDUP(OPT_MEDIAN_FILTER_DFL);
        opts->mbhbn=OPT_MBHBN_DFL;
        opts->mbhbt=OPT_MBHBT_DFL;
        opts->trnhbt=OPT_TRNHBT_DFL;
        opts->trnuhbt=OPT_TRNUHBT_DFL;
        opts->trnumttl=OPT_TRNUMTTL_DFL;
        opts->delay=OPT_DELAY_DFL;
        opts->statsec=OPT_STATSEC_DFL;
        opts->statflags_str=strdup(OPT_STATFLAG_STR_DFL);
        opts->statflags=OPT_STATFLAGS_DFL;
        opts->trn_en=OPT_TRN_EN_DFL;
        opts->trn_utm=OPT_TRN_UTM_DFL;
        opts->trn_map=CHK_STRDUP(OPT_MAP_DFL);
        opts->trn_cfg=CHK_STRDUP(OPT_CFG_DFL);
        opts->trn_par=CHK_STRDUP(OPT_PAR_DFL);
        opts->trn_mid=strdup(OPT_TRN_MDIR_DFL);
        opts->trn_mtype=OPT_TRN_MTYPE_DFL;
        opts->trn_sensor_type=OPT_TRN_SENSOR_TYPE_DFL;
        opts->trn_ftype=OPT_TRN_FTYPE_DFL;
        opts->trn_fgrade=OPT_TRN_FGRADE_DFL;
        opts->trn_freinit=OPT_TRN_FREINIT_DFL;
        opts->trn_mweight=OPT_TRN_MWEIGHT_DFL;
        opts->trn_ncov=OPT_TRN_NCOV_DFL;
        opts->trn_nerr=OPT_TRN_NERR_DFL;
        opts->trn_ecov=OPT_TRN_ECOV_DFL;
        opts->trn_eerr=OPT_TRN_EERR_DFL;
        opts->mb_out=CHK_STRDUP(OPT_MB_OUT_DFL);
        opts->trn_out=CHK_STRDUP(OPT_TRN_OUT_DFL);
        opts->trn_decn=OPT_TRN_DECN_DFL;
        opts->trn_decs=OPT_TRN_DECS_DFL;
        opts->covariance_magnitude_max = OPT_COVARIANCE_MAGNITUDE_MAX_DFL;
        opts->convergence_repeat_min = OPT_CONVERGENCE_REPEAT_MIN;
        opts->reinit_search_xy = OPT_REINIT_SEARCH_XY;
        opts->reinit_search_z = OPT_REINIT_SEARCH_Z;
        opts->reinit_gain_enable = OPT_REINIT_GAIN_ENABLE_DFL;
        opts->reinit_file_enable = OPT_REINIT_FILE_ENABLE_DFL;
        opts->reinit_xyoffset_enable = OPT_REINIT_XYOFFSET_ENABLE_DFL;
        opts->reinit_xyoffset_max = OPT_REINIT_XYOFFSET_MAX_DFL;
        opts->reinit_zoffset_enable = OPT_REINIT_ZOFFSET_ENABLE_DFL;
        opts->reinit_zoffset_min = OPT_REINIT_ZOFFSET_MIN_DFL;
        opts->reinit_zoffset_max = OPT_REINIT_ZOFFSET_MAX_DFL;
        opts->random_offset_enable = OPT_RANDOM_OFFSET_ENABLE_DFL;
        opts->trn_dev = OPT_TRN_DEV_DFL;
        opts->help=OPT_HELP_DFL;
        retval=0;
    }
    return retval;
}

static void s_mbtrnpp_free_opts(mbtrnpp_opts_t **pself)
{
    if(NULL!=pself && NULL!=*pself){
        mbtrnpp_opts_t *self=*pself;
        MEM_CHKFREE(self->input);
        MEM_CHKFREE(self->platform_file);
        MEM_CHKFREE(self->tide_model);
        MEM_CHKFREE(self->log_directory);
        MEM_CHKFREE(self->output);
        MEM_CHKFREE(self->median_filter);
        MEM_CHKFREE(self->statflags_str);
        MEM_CHKFREE(self->trn_map);
        MEM_CHKFREE(self->trn_cfg);
        MEM_CHKFREE(self->trn_par);
        MEM_CHKFREE(self->trn_mid);
        MEM_CHKFREE(self->mb_out);
        MEM_CHKFREE(self->trn_out);
    }
}

static void s_mbtrnpp_free_cfg(mbtrnpp_cfg_t **pself)
{
    if(NULL!=pself && NULL!=*pself){
        mbtrnpp_cfg_t *self=*pself;
        MEM_CHKFREE(self->trn_log_dir);
        MEM_CHKFREE(self->mb1svr_host);
        MEM_CHKFREE(self->trnsvr_host);
        MEM_CHKFREE(self->trnusvr_host);
        MEM_CHKFREE(self->trn_map_file);
        MEM_CHKFREE(self->trn_cfg_file);
        MEM_CHKFREE(self->trn_particles_file);
        MEM_CHKFREE(self->trn_mission_id);
    }
}
static int s_mbtrnpp_cfgstr(char **pdest, size_t olen, mbtrnpp_cfg_t *self, const char *prefix, const char *kvsep, const char *delim, int indent, int wkey, int wval)
{
    int retval=-1;
    const char *pre = (prefix ? prefix : "");
    const char *sep = (kvsep ? kvsep : "");
    const char *del = (delim ? delim : "\n");
    // mbbuf : mframe dynamically sized byte buffer (mbbuf.h)
    // auto-resizes on printf/write
    // mbbuf_t must be released using mbb_destroy()
    // buffers created by mbb_read() must be released using free()
    mbbuf_t *optr = mbb_new(1024*5, NULL, 0);
    mbb_printf(optr, "%s%*s%*s%s%*p%s", pre, indent, (indent>0?" ":""), wkey, "self", sep, wval, self, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "verbose", sep, wval, self->verbose, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "input_mode", sep, wval, self->input_mode, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "input", sep, wval, self->input, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "socket_definition", sep, wval, self->socket_definition, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "output_mb1_file", sep, wval, self->output_mb1_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "output_trn_file", sep, wval, self->output_trn_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "format", sep, wval, self->format, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "platform-file", sep, wval, self->platform_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "use_platform_file", sep, wval, BOOL2YNC(self->use_platform_file), del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "platform-target-sensor", sep, wval, self->target_sensor, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "tide-model", sep, wval, self->tide_model, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "use_tide_model", sep, wval, BOOL2YNC(self->use_tide_model), del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "log-directory", sep, wval, self->log_directory, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn_log_dir", sep, wval, self->trn_log_dir, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "make_logs", sep, wval, BOOL2YNC(self->make_logs), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "platform-file", sep, wval, BOOL2YNC(self->make_logs), del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "swath-width", sep, wval, self->swath_width, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "n_output_soundings", sep, wval, self->n_output_soundings, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "median_filter_threshold", sep, wval, self->median_filter_threshold, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "median_filter_n_across", sep, wval, self->median_filter_n_across, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "median_filter_n_along", sep, wval, self->median_filter_n_along, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "median_filter_en", sep, wval, BOOL2YNC(self->median_filter_en), del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "n_buffer_max", sep, wval, self->n_buffer_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "mb1svr_host", sep, wval, self->mb1svr_host, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "mb1svr_port", sep, wval, self->mb1svr_port, del);

    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trnsvr_host", sep, wval, self->trnsvr_host, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trnsvr_port", sep, wval, self->trnsvr_port, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trnusvr_host", sep, wval, self->trnusvr_host, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trnusvr_port", sep, wval, self->trnusvr_port, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trnumsvr_group", sep, wval, self->trnumsvr_group, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trnumsvr_port", sep, wval, self->trnumsvr_port, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trnumsvr_ttl", sep, wval, self->trnumsvr_ttl, del);
    mbb_printf(optr, "%s%*s%*s%s%*X%s", pre, indent, (indent>0?" ":""), wkey, "output_flags", sep, wval, self->output_flags, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "mbsvr_hbtok", sep, wval, self->mbsvr_hbtok, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "mbsvr_hbto", sep, wval, self->mbsvr_hbto, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trnsvr_hbto", sep, wval, self->trnsvr_hbto, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trnusvr_hbto", sep, wval, self->trnusvr_hbto, del);
    mbb_printf(optr, "%s%*s%*s%s%*"PRId64"%s", pre, indent, (indent>0?" ":""), wkey, "mbtrnpp_loop_delay_msec", sep, wval, self->mbtrnpp_loop_delay_msec, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_status_interval_sec", sep, wval, self->trn_status_interval_sec, del);
    mbb_printf(optr, "%s%*s%*s%s%*X%s", pre, indent, (indent>0?" ":""), wkey, "mbtrnpp_stat_flags", sep, wval, self->mbtrnpp_stat_flags, del);
    mbb_printf(optr, "%s%*s%*s%s%*s/%d%s", pre, indent, (indent>0?" ":""), wkey, "trn_dev", sep, wval, r7k_devidstr(self->trn_dev), self->trn_dev, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "trn_enable", sep, wval, BOOL2YNC(self->trn_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*ld%s", pre, indent, (indent>0?" ":""), wkey, "trn_utm_zone", sep, wval, self->trn_utm_zone, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_mtype", sep, wval, self->trn_mtype, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_sensor_type", sep, wval, self->trn_sensor_type, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_ftype", sep, wval, self->trn_ftype, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_fgrade", sep, wval, self->trn_fgrade, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_freinit", sep, wval, self->trn_freinit, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn_mweight", sep, wval, self->trn_mweight, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_max_ncov", sep, wval, self->trn_max_ncov, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_max_nerr", sep, wval, self->trn_max_nerr, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_max_ecov", sep, wval, self->trn_max_ecov, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_max_eerr", sep, wval, self->trn_max_eerr, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn_map_file", sep, wval, self->trn_map_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn_cfg_file", sep, wval, self->trn_cfg_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn_particles_file", sep, wval, self->trn_particles_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn_mission_dir", sep, wval, self->trn_mission_id, del);
    mbb_printf(optr, "%s%*s%*s%s%*u%s", pre, indent, (indent>0?" ":""), wkey, "trn_decn", sep, wval, self->trn_decn, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn_decs", sep, wval, self->trn_decs, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "covariance_magnitude_max", sep, wval, self->covariance_magnitude_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "convergence_repeat_min", sep, wval, self->convergence_repeat_min, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_search_xy", sep, wval, self->reinit_search_xy, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_search_z", sep, wval, self->reinit_search_z, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_gain_enable", sep, wval, BOOL2YNC(self->reinit_gain_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_file_enable", sep, wval, BOOL2YNC(self->reinit_file_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_xyoffset_enable", sep, wval, BOOL2YNC(self->reinit_xyoffset_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_xyoffset_max", sep, wval, self->reinit_xyoffset_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_enable", sep, wval, BOOL2YNC(self->reinit_zoffset_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_min", sep, wval, self->reinit_zoffset_min, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_max", sep, wval, self->reinit_zoffset_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "random_offset_enable", sep, wval, BOOL2YNC(self->random_offset_enable), del);
    size_t slen = mbb_length(optr);
    if(NULL == *pdest){
        // set dest buffer (malloc'd, caller must free)
        *pdest = (char *)mbb_read(optr, slen);
        retval = slen;
    } else {
        if(olen >= slen){
            // copy buffer contents to caller buffer
            memcpy(*pdest, mbb_head(optr), slen);
            retval = slen;
        } else {
            // buffer provided too small
            // best to fail so it gets noticed
            fprintf(stderr, "%s:%d - ERR destination buffer too small (%zu/%zu)\n", __func__, __LINE__, olen, slen);
        }
    }
    // release byte buffer
    mbb_destroy(&optr);
    return retval;
}
static int s_mbtrnpp_optstr(char **pdest, size_t olen, mbtrnpp_opts_t *self, const char *prefix, const char *kvsep, const char *delim, int indent, int wkey, int wval)
{
    int retval=-1;
    const char *pre = (prefix ? prefix : "");
    const char *sep = (kvsep ? kvsep : "");
    const char *del = (delim ? delim : "\n");
    // mbbuf : mframe dynamically sized byte buffer (mbbuf.h)
    // auto-resizes on printf/write
    // mbbuf_t must be released using mbb_destroy()
    // buffers created by mbb_read() must be released using free()
    mbbuf_t *optr = mbb_new(1024*5,NULL,0);
    mbb_printf(optr, "%s%*s%*s%s%*p%s", pre, indent, (indent>0?" ":""), wkey, "self", sep, wval, self, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "verbose", sep, wval, self->verbose, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "input", sep, wval, self->input, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "format", sep, wval, self->format, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "platform-file", sep, wval, self->platform_file, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "platform-target-sensor", sep, wval, self->platform_target_sensor, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "log-directory", sep, wval, self->log_directory, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "tide-model", sep, wval, self->tide_model, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "output", sep, wval, self->output, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "projection", sep, wval, self->projection, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "swath-width", sep, wval, self->swath_width, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "soundings", sep, wval, self->soundings, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "median-filter", sep, wval, self->median_filter, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "mbhbn", sep, wval, self->mbhbn, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "mbhbt", sep, wval, self->mbhbt, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trnhbt", sep, wval, self->trnhbt, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trnuhbt", sep, wval, self->trnuhbt, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trnumttl", sep, wval, self->trnumttl, del);
    mbb_printf(optr, "%s%*s%*s%s%*"PRId64"%s", pre, indent, (indent>0?" ":""), wkey, "delay", sep, wval, self->delay, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "statsec", sep, wval, self->statsec, del);
    mbb_printf(optr, "%s%*s%*s%s%*X/%s%s", pre, indent, (indent>0?" ":""), wkey, "statflags", sep, wval, self->statflags, self->statflags_str, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "trn-en", sep, wval, BOOL2YNC(self->trn_en), del);
    mbb_printf(optr, "%s%*s%*s%s%*s/%d%s", pre, indent, (indent>0?" ":""), wkey, "trn-dev", sep, wval, r7k_devidstr(self->trn_dev), self->trn_dev, del);
    mbb_printf(optr, "%s%*s%*s%s%*ld%s", pre, indent, (indent>0?" ":""), wkey, "trn-utm", sep, wval, self->trn_utm, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn-map", sep, wval, self->trn_map, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn-cfg", sep, wval, self->trn_cfg, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn-par", sep, wval, self->trn_par, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn-mid", sep, wval, self->trn_mid, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-mtype", sep, wval, self->trn_mtype, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-sensor-type", sep, wval, self->trn_sensor_type, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-ftype", sep, wval, self->trn_ftype, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-fgrade", sep, wval, self->trn_fgrade, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-freinit", sep, wval, self->trn_freinit, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "trn-mweight", sep, wval, self->trn_mweight, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn-ncov", sep, wval, self->trn_ncov, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn-nerr", sep, wval, self->trn_nerr, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn-ecov", sep, wval, self->trn_ecov, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn-eerr", sep, wval, self->trn_eerr, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "mb-out", sep, wval, self->mb_out, del);
    mbb_printf(optr, "%s%*s%*s%s%*s%s", pre, indent, (indent>0?" ":""), wkey, "trn-out", sep, wval, self->trn_out, del);
    mbb_printf(optr, "%s%*s%*s%s%*u%s", pre, indent, (indent>0?" ":""), wkey, "trn-decn", sep, wval, self->trn_decn, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "trn-decs", sep, wval, self->trn_decs, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "covariance-magnitude-max", sep, wval, self->covariance_magnitude_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*d%s", pre, indent, (indent>0?" ":""), wkey, "convergence-repeat-min", sep, wval, self->convergence_repeat_min, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_search_xy", sep, wval, self->reinit_search_xy, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_search_z", sep, wval, self->reinit_search_z, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_gain_enable", sep, wval, BOOL2YNC(self->reinit_gain_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_file_enable", sep, wval, BOOL2YNC(self->reinit_file_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_xyoffset_enable", sep, wval, BOOL2YNC(self->reinit_xyoffset_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_xyoffset_max", sep, wval, self->reinit_xyoffset_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_enable", sep, wval, BOOL2YNC(self->reinit_zoffset_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_min", sep, wval, self->reinit_zoffset_min, del);
    mbb_printf(optr, "%s%*s%*s%s%*.2lf%s", pre, indent, (indent>0?" ":""), wkey, "reinit_zoffset_max", sep, wval, self->reinit_zoffset_max, del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "random_offset_enable", sep, wval, BOOL2YNC(self->random_offset_enable), del);
    mbb_printf(optr, "%s%*s%*s%s%*c%s", pre, indent, (indent>0?" ":""), wkey, "help", sep, wval, BOOL2YNC(self->help), del);
    size_t slen = mbb_length(optr);
    if(NULL == *pdest){
        // set dest buffer (malloc'd, caller must free)
        *pdest = (char *)mbb_read(optr, slen);
        retval = slen;
    } else {
        if(olen >= slen){
            // copy buffer contents to caller buffer
            memcpy(*pdest, mbb_head(optr), slen);
            retval = slen;
        } else {
            // buffer provided too small
            // best to fail so it gets noticed
            fprintf(stderr,"%s:%d - ERR destination buffer too small (%zu/%zu)\n", __func__, __LINE__, olen, slen);
        }
    }
    // release byte buffer
    mbb_destroy(&optr);
    return retval;
}
static int s_mbtrnpp_show_cfg(FILE *fpout, mbtrnpp_cfg_t *self, bool hashstart, int indent)
{
 int retval = 0;
    char *buf = NULL;
    const char *pre = (hashstart ? "##  " : " ");
    const char *kvsep = " ";
    const char *del = "\n";
    // passing NULL buf ptr returns as malloc'd string
    int slen = s_mbtrnpp_cfgstr(&buf, 0, self, pre, kvsep, del, indent, 25, 30);
    if ( (slen > 0)  && (NULL != buf)) {
        fprintf(fpout, "%s", buf);
        retval=slen;
        free(buf);
    } else {
        fprintf(stderr,"%s:%d - ERR s_mbtrnpp_cfgstr failed: len[%d] buf[%p]\n", __func__, __LINE__, slen, buf);
    }

    return retval;
}

static int s_mbtrnpp_show_opts(FILE *fpout, mbtrnpp_opts_t *self, bool hashstart, int indent)
{
    int retval = 0;
    char *buf = NULL;
    const char *pre = (hashstart ? "##  " : " ");
    const char *kvsep = " ";
    const char *del = "\n";
    // passing NULL buf ptr returns as malloc'd string
    int slen = s_mbtrnpp_optstr(&buf, 0, self, pre, kvsep, del, indent, 25, 30);
    if ( (slen > 0) && (NULL != buf)) {
        fprintf(fpout, "%s", buf);
        retval=slen;
        free(buf);
    } else {
        fprintf(stderr,"%s:%d - ERR s_mbtrnpp_optstr failed: len[%d] buf[%p]\n", __func__, __LINE__, slen, buf);
    }

    return retval;
}

static int s_parse_opt_output(mbtrnpp_cfg_t *cfg, char *opt_str)
{
    int retval=0;

    if(NULL!=cfg && NULL!=opt_str){

        // tokenize optarg
        char *ocopy = CHK_STRDUP(opt_str);
        char *tok[MBSYSOUT_OPT_N]={0};
        char *saveptr;
        for(int i = 0; i < MBSYSOUT_OPT_N; i++){
            tok[i] = (i==0  ? strtok_r(ocopy,",",&saveptr) : strtok_r(NULL,",",&saveptr));
            if(tok[i]==NULL)
                break;
        }
        // parse tokens
        for(int i = 0; i < MBSYSOUT_OPT_N; i++) {
            if(NULL==tok[i])
                break;
            if (NULL!=strstr(tok[i], "socket:")) {
                // enable mb1 socket (use specified IP)
                char *acpy = strdup(tok[i]);
                char *atok = strtok_r(acpy,":",&saveptr);
                if(NULL!=atok){
                    // uses defaults if NULL
                    char *shost = strtok_r(NULL,":",&saveptr);
                    char *sport = strtok_r(NULL,":",&saveptr);
                    //                    fprintf(stderr,"shost[%s] sport[%s]\n",shost,sport);

                    if(NULL!=shost){
                        MEM_CHKINVALIDATE(cfg->mb1svr_host);
                        cfg->mb1svr_host = strdup(shost);
                        retval++;
                    }
                    if(NULL!=sport){
                        sscanf(sport,"%d",&cfg->mb1svr_port);
                        retval++;
                    }
                }
                //                fprintf(stderr,"mb1svr[%s:%d]\n",cfg->mb1svr_host,cfg->mb1svr_port);
                free(acpy);
                cfg->output_flags |= OUTPUT_MB1_SVR_EN;
            }
            if (strcmp(tok[i], "socket") == 0) {
                // enable mb1 socket (use default IP)
                cfg->output_flags |= OUTPUT_MB1_SVR_EN;
            }

            if(NULL!=strstr(tok[i],"file:")){
                char *acpy = strdup((tok[i]+strlen("file:")));
                char *atok = strtok_r(acpy,":",&saveptr);
                //                fprintf(stderr,"output_mb1_file[%s]\n",atok);
                if(strlen(atok)>0){
                    strcpy(cfg->output_mb1_file,atok);
                    // enable mb1 data log (use specified name)
                    cfg->output_flags |= OUTPUT_MB1_FILE_EN;
                    retval++;
                }
                free(acpy);
            }
            if (strcmp(tok[i], "file") == 0) {
                // enable mb1 data log (use default MB-System name)
                cfg->output_flags |= OUTPUT_MB1_FILE_EN;
            }
        }
        free(ocopy);

        int flen = strlen(cfg->output_mb1_file);
        if (flen > 4 && MB_PATH_SIZE > (flen-4 + strlen("_trn.txt")+1)
            && strncmp(&(cfg->output_mb1_file[flen-4]), ".mb1", 4) == 0) {
          snprintf(cfg->output_trn_file, flen-4, "%s", cfg->output_mb1_file );
          strcat(cfg->output_trn_file, "_trn.txt");

        }
    }// err - invalid arg
    return retval;
}

static int s_parse_opt_mbout(mbtrnpp_cfg_t *cfg, char *opt_str)
{
    int retval=0;
    if(NULL!=cfg && NULL!=opt_str){

        // tokenize optarg
        char *ocopy = strdup(opt_str);
        char *tok[MBOUT_OPT_N] = {0};
        char *saveptr;
        for(int i = 0; i < MBOUT_OPT_N; i++){
            tok[i] = (i==0  ? strtok_r(ocopy,",",&saveptr) : strtok_r(NULL,",",&saveptr));
            if(tok[i]==NULL)
                break;
        }
        // parse tokens
        for(int i = 0; i < MBOUT_OPT_N; i++){
            if(NULL == tok[i])
                break;
            if(strstr(tok[i], "mb1svr") != NULL) {
                // enable mb1 socket output (optionally, specify host:port)
                char *acpy = strdup(tok[i]);
                char *atok = strtok_r(acpy, ":",&saveptr);
                if(NULL != atok){
                    // uses defaults if NULL
                    char *shost = strtok_r(NULL,":",&saveptr);
                    char *sport = strtok_r(NULL,":",&saveptr);
                    //                    fprintf(stderr,"shost[%s] sport[%s]\n",shost,sport);

                    if(NULL != shost){
                        MEM_CHKINVALIDATE(cfg->mb1svr_host);
                        cfg->mb1svr_host = strdup(shost);
                        retval++;
                    }
                    if(NULL != sport){
                        sscanf(sport,"%d",&cfg->mb1svr_port);
                        retval++;
                    }
                }
                //                fprintf(stderr,"mb1svr[%s:%d]\n",mbtrn_cfg->mb1svr_host,mbtrn_cfg->mb1svr_port);
                cfg->output_flags |= OUTPUT_MB1_SVR_EN;
                free(acpy);
            }
            if(strcmp(tok[i], "mb1")==0){
                // enable mb1 data log
                cfg->output_flags |= OUTPUT_MB1_BIN;
            }
            if(NULL != strstr(tok[i], "file:")) {
                char *acpy = strdup((tok[i] + strlen("file:")));
                char *atok = strtok_r(acpy,":",&saveptr);
                //                fprintf(stderr,"output_mb1_file[%s]\n",atok);
                if(strlen(atok)>0){
                    strcpy(cfg->output_mb1_file,atok);
                    // enable mb1 data log (use specified name)
                    cfg->output_flags |= OUTPUT_MB1_FILE_EN;
                }
                free(acpy);
            }
            if (strcmp(tok[i], "file") == 0) {
                // enable mb1 data log (use default MB-System name)
                cfg->output_flags |= OUTPUT_MB1_FILE_EN;
            }
            if(strcmp(tok[i],"reson")==0){
                // enable reson frame data log
                cfg->output_flags |= OUTPUT_RESON_BIN;
            }
#ifdef WITH_MB1_READER
            if(strcmp(tok[i],"mb1r")==0){
                // enable mb1 frame data log
                cfg->output_flags |= OUTPUT_MB1R_BIN;
            }
            if(strcmp(tok[i],"nomb1r")==0){
                // disable mb1 frame data log
                cfg->output_flags &= ~OUTPUT_MB1R_BIN;
            }
#endif
            if(strcmp(tok[i],"nomb1")==0){
                // disable mb1 data log
                cfg->output_flags &= ~OUTPUT_MB1_BIN;
            }
            if(strcmp(tok[i],"noreson")==0){
                // disable reson frame data log
                cfg->output_flags &= ~OUTPUT_RESON_BIN;
            }
            if(strcmp(tok[i],"nombsvr")==0){
                // disable mb1svr
                cfg->output_flags &= ~OUTPUT_MB1_SVR_EN;
                MEM_CHKINVALIDATE(cfg->mb1svr_host);
            }
            if(strcmp(tok[i],"nombtrnpp")==0){
                // disable mbtrnpp message log (not recommended)
                cfg->output_flags &= ~OUTPUT_MBTRNPP_MSG;
            }
        }
        free(ocopy);
    }// err - invalid arg

    return retval;
}

static int s_parse_opt_trnout(mbtrnpp_cfg_t *cfg, char *opt_str)
{
    int retval=0;
    if(NULL!=cfg && NULL!=opt_str){
        // tokenize optarg
        char *ocopy = strdup(opt_str);
        char *tok[TRNOUT_OPT_N] = {0};
        char *saveptr;
        for(int i = 0; i < TRNOUT_OPT_N; i++){
            tok[i] = (i==0 ? strtok_r(ocopy,",",&saveptr) : strtok_r(NULL,",",&saveptr));
            //                fprintf(stderr,"tok[%d][%s]\n",i,tok[i]);
            if(tok[i] == NULL)
                break;
        }
        // parse tokens
        for(int i = 0; i < TRNOUT_OPT_N; i++) {
            if(NULL == tok[i])
                break;
            if(strstr(tok[i], "trnsvr")!=NULL) {
                // enable trnsvr (trnsvr:host:port)
                char *acpy = strdup(tok[i]);
                char *atok = strtok_r(acpy,":",&saveptr);
                if(NULL!=atok){
                    char *shost = strtok_r(NULL,":",&saveptr);
                    char *sport = strtok_r(NULL,":",&saveptr);

                    if(NULL!=shost){
                        MEM_CHKINVALIDATE(cfg->trnsvr_host);
                        cfg->trnsvr_host = strdup(shost);
                    }
                    if(NULL!=sport){
                        sscanf(sport,"%d",&cfg->trnsvr_port);
                    }
                }
                cfg->output_flags |= OUTPUT_TRN_SVR_EN;
                free(acpy);
            }
            if(strstr(tok[i], "trnusvr")!=NULL){
                // enable trnsvr (trnusvr:host:port)
                char *acpy = strdup(tok[i]);
                char *atok = strtok_r(acpy,":",&saveptr);
                if(NULL != atok){
                    char *shost = strtok_r(NULL,":",&saveptr);
                    char *sport = strtok_r(NULL,":",&saveptr);

                    if(NULL != shost){
                        MEM_CHKINVALIDATE(cfg->trnusvr_host);
                        cfg->trnusvr_host = strdup(shost);
                        retval++;
                    }
                    if(NULL != sport){
                        sscanf(sport,"%d",&cfg->trnusvr_port);
                        retval++;
                    }
                }
                // fprintf(stderr,"trnusvr[%s:%d]\n",cfg->trnusvr_host,cfg->trnusvr_port);
                cfg->output_flags |= OUTPUT_TRNU_SVR_EN;
                free(acpy);
            }
            if(strstr(tok[i], "trnumsvr")!=NULL){
                // enable trnumsvr (trnumsvr:host:port:ttl)
                char *acpy = strdup(tok[i]);
                char *atok = strtok_r(acpy,":",&saveptr);

                if(NULL != atok){
                    char *shost = strtok_r(NULL,":",&saveptr);
                    char *sport = strtok_r(NULL,":",&saveptr);
                    char *sttl = strtok_r(NULL,":",&saveptr);

                    if(NULL != shost){
                        MEM_CHKINVALIDATE(cfg->trnumsvr_group);
                        cfg->trnumsvr_group = strdup(shost);
                        retval++;
                    }
                    if(NULL != sport){
                        sscanf(sport,"%d",&cfg->trnumsvr_port);
                        retval++;
                    }
                    if(NULL != sttl){
                        sscanf(sttl,"%d",&cfg->trnumsvr_ttl);
                        retval++;
                    }
                }
                // fprintf(stderr,"trnumsvr[%s:%d]\n",cfg->trnusmvr_host,cfg->trnumsvr_port);
                cfg->output_flags |= OUTPUT_TRNUM_SVR_EN;
                free(acpy);
            }
            if(strcmp(tok[i],"trnu")==0){
                // enable trn update data log
                cfg->output_flags |= OUTPUT_TRNU_ASC;
            }
            if(strcmp(tok[i],"trnub")==0){
                // enable trn update data log
                cfg->output_flags |= OUTPUT_TRNU_BIN;
            }
            if(strcmp(tok[i],"sout")==0){
                // enable trn update to stdout
                cfg->output_flags |= OUTPUT_TRNU_SOUT;
            }
            if(strcmp(tok[i],"serr")==0){
                // enable trn updatetp stderr
                cfg->output_flags |= OUTPUT_TRNU_SERR;
            }
            if(strcmp(tok[i],"debug")==0){
                // enable trn update per debug settings
                cfg->output_flags |= OUTPUT_TRNU_DEBUG;
            }
            if(strcmp(tok[i],"notrnsvr")==0){
                // disable trnsvr
                cfg->output_flags &= ~OUTPUT_TRN_SVR_EN;
                MEM_CHKINVALIDATE(cfg->trnsvr_host);
            }
            if(strcmp(tok[i],"notrnusvr")==0){
                // disable trnusvr
                cfg->output_flags &= ~OUTPUT_TRNU_SVR_EN;
                MEM_CHKINVALIDATE(cfg->trnusvr_host);
            }
            if(strcmp(tok[i],"notrnumsvr")==0){
                // disable trnumsvr
                cfg->output_flags &= ~OUTPUT_TRNUM_SVR_EN;
                MEM_CHKINVALIDATE(cfg->trnumsvr_group);
            }
        }
        free(ocopy);
    }// err - invalid arg

    return retval;
}

static int s_parse_opt_logdir(mbtrnpp_cfg_t *cfg, char *opt_str)
{
    int retval=-1;
    if(NULL!=cfg && NULL!=opt_str){
        strcpy(cfg->log_directory, opt_str);
        struct stat logd_stat;
        int logd_status=0;
        logd_status = stat(cfg->log_directory, &logd_stat);

        if (logd_status != 0) {
            cfg->make_logs = false;
            char *ps = CHK_STRDUP(cfg->log_directory);
            if(NULL!=ps){
                int status=0;
                if( (status=mkdir(ps,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))==0){
                    cfg->make_logs = true;
                    MEM_CHKINVALIDATE(cfg->trn_log_dir);
                   cfg->trn_log_dir=CHK_STRDUP(ps);
                } else {
                    fprintf(stderr, "\nCreate log directory %s failed [%d/%s]\n", ps,errno,strerror(errno));
                }
                free(ps);
            }
        } else if((logd_stat.st_mode & S_IFMT) != S_IFDIR) {
            fprintf(stderr, "\nSpecified log file directory %s is not a directory...\n", cfg->log_directory);
            cfg->make_logs = false;
        } else {
            cfg->make_logs = true;
            MEM_CHKINVALIDATE(cfg->trn_log_dir);
            cfg->trn_log_dir = CHK_STRDUP(cfg->log_directory);
        }
        int filestatus = 0;
        if ((filestatus = lstat("mbtrnpp-latest", &logd_stat)) == 0) {
            remove("mbtrnpp-latest");
            fprintf(stderr, "Delete old symlink mbtrnpp-latest\n");
        }
        int test = symlink(cfg->log_directory, "mbtrnpp-latest");
        if(test == 0)
        fprintf(stderr, "Create symlink mbtrnpp-latest->%s\n", cfg->log_directory);
        else
        fprintf(stderr, "Create symlink failed %s\n", cfg->log_directory);

        if(NULL==cfg->trn_log_dir){
            MEM_CHKINVALIDATE(cfg->trn_log_dir);
            cfg->trn_log_dir=strdup(CFG_TRN_LOG_DIR_DFL);
        }
        retval=0;
    }
    return retval;
}

static int s_parse_opt_input(mbtrnpp_cfg_t *cfg, char *opt_str)
{
    int retval=-1;
    if(NULL!=cfg && NULL!=opt_str){
        size_t opt_len=strlen(opt_str);

        if(opt_len>0 && opt_len<MB_PATH_SIZE){
            // set config input option
            sprintf(cfg->input,"%s",opt_str);
            // parse option and set mode
            char *psdef=NULL;
            if ((psdef=strstr(opt_str, "socket:"))!=NULL) {

                size_t sdef_len=strlen(psdef);
                if(sdef_len>0 && sdef_len<MB_PATH_SIZE){
                    psdef+=strlen("socket:");
                    // set socket mode and definition
                    cfg->input_mode = INPUT_MODE_SOCKET;
                    sprintf(cfg->socket_definition,"%s",psdef);
                } else {
                    fprintf(stderr,"socket definition length invalid [%s/%zu/%zu]\n",psdef,sdef_len,(size_t)MB_PATH_SIZE);
                }
//            fprintf(stderr, "socket_definition|%s\n", cfg->socket_definition);

            } else {
                // cfg->input is input file name
                cfg->input_mode = INPUT_MODE_FILE;
            }
        } else {
            fprintf(stderr,"input specifier length invalid [%s/%zu/%zu]\n",opt_str,opt_len,(size_t)MB_PATH_SIZE);
        }
    } else {
        fprintf(stderr,"%s: ERR - invalid argument\n",__func__);
    }
    return retval;
}

static char *s_mbtrnpp_peek_opt_cfg(int argc, char **argv, char **pbuf, size_t len)
{
    char *retval =NULL;
    for (int i = 0; i<argc; i++) {
        char *val=NULL;
        if( (val=strstr(argv[i], "config="))!=NULL) {
            char *dest=NULL;
            val+=strlen("config=");
            size_t vlen=strlen(val)+1;
            if(NULL==pbuf){
                dest=(char *)malloc(vlen);
            } else {
                char *buf=*pbuf;
                if(NULL==buf){
                    dest=(char *)malloc(vlen);
                    *pbuf=dest;
                } else {
                    if(len>=vlen){
                        dest=buf;
                    } else {
                        fprintf(stderr,"ERR - config path buffer too small\n");
                    }
                }
            }

            if(NULL!=dest){
                memcpy(dest,val,vlen);
                retval=dest;
            }
        }
    }
    return retval;
}

static int s_mbtrnpp_kvparse_fn(char *key, char *val, void *cfg)
{
    int retval =-1;

    if(NULL!=key &&  NULL!=cfg){
        mbtrnpp_opts_t *opts=(mbtrnpp_opts_t *)cfg;
//        fprintf(stderr, ">>>> PARSING key/val [%13s / %s]\n", key,val);
        if(NULL!=val){
            // process opts w/ required args
            if(strcmp(key,"verbose")==0 ){
                if(sscanf(val,"%d",&opts->verbose)==1){
                    retval=0;
                }
            } else if(strcmp(key,"input")==0 ){
                MEM_CHKFREE(opts->input);
                if( (opts->input=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"format")==0 ){
                if(sscanf(val,"%d",&opts->format)==1){
                    retval=0;
                }
            } else if(strcmp(key,"platform-file")==0 ){
                MEM_CHKFREE(opts->platform_file);
                if( (opts->platform_file=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"platform-target-sensor")==0 ){
                if(sscanf(val,"%d",&opts->platform_target_sensor)==1){
                    retval=0;
                }
            } else if(strcmp(key,"tide-model")==0 ){
                MEM_CHKFREE(opts->tide_model);
                if( (opts->tide_model=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"log-directory")==0 ){
                MEM_CHKFREE(opts->log_directory);
                if( (opts->log_directory=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"output")==0 ){
                MEM_CHKFREE(opts->output);
               if( (opts->output=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"projection")==0 ){
                if(sscanf(val,"%d",&opts->projection)==1){
                    retval=0;
                }
            } else if(strcmp(key,"swath-width")==0 || strcmp(key,"swath")==0 ){
                if(sscanf(val,"%lf",&opts->swath_width)==1){
                    retval=0;
                }
            } else if(strcmp(key,"soundings")==0 ){
                if(sscanf(val,"%d",&opts->soundings)==1){
                    retval=0;
                }
            } else if(strcmp(key,"median-filter")==0 ){
                MEM_CHKFREE(opts->median_filter);
                if( (opts->median_filter=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"mbhbn")==0 ){
                if(sscanf(val,"%d",&opts->mbhbn)==1){
                    retval=0;
                }
            } else if(strcmp(key,"mbhbt")==0 ){
                if(sscanf(val,"%lf",&opts->mbhbt)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trnhbt")==0 ){
                if(sscanf(val,"%lf",&opts->trnhbt)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trnuhbt")==0 ){
                if(sscanf(val,"%lf",&opts->trnuhbt)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trnumttl")==0 ){
                if(sscanf(val,"%d",&opts->trnumttl)==1){
                    retval=0;
                }
            } else if(strcmp(key,"delay")==0 ){
                if(sscanf(val,"%"PRId64"",&opts->delay)==1){
                    retval=0;
                }
            } else if(strcmp(key,"statsec")==0 ){
                if(sscanf(val,"%lf",&opts->statsec)==1){
                    retval=0;
                }
            } else if(strcmp(key,"statflags")==0 ){
                MEM_CHKFREE(opts->statflags_str);
                if( (opts->statflags_str=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }

                if(sscanf(val,"%u",&opts->statflags)==1){
                    retval=0;
                }
                if(NULL!=strstr(val,"MSF_STATUS") || NULL!=strstr(val,"msf_status")){
                    opts->statflags |= MSF_STATUS;
                    retval=0;
                }
                if(NULL!=strstr(val,"MSF_EVENT") || NULL!=strstr(val,"msf_event")){
                    opts->statflags |= MSF_EVENT;
                    retval=0;
                }
                if(NULL!=strstr(val,"MSF_ASTAT") || NULL!=strstr(val,"msf_astat")){
                    opts->statflags |= MSF_ASTAT;
                    retval=0;
                }
                if(NULL!=strstr(val,"MSF_PSTAT") || NULL!=strstr(val,"msf_pstat")){
                    opts->statflags |= MSF_PSTAT;
                    retval=0;
                }
                if(NULL!=strstr(val,"MSF_READER") || NULL!=strstr(val,"msf_reader")){
                    opts->statflags |= MSF_READER;
                    retval=0;
                }
            } else if(strcmp(key,"trn-utm")==0 ){
                if(sscanf(val,"%ld",&opts->trn_utm)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-map")==0 ){
                MEM_CHKFREE(opts->trn_map);
                if( (opts->trn_map=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-cfg")==0 ){
                MEM_CHKFREE(opts->trn_cfg);
                if( (opts->trn_cfg=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-par")==0 ){
                MEM_CHKFREE(opts->trn_par);
               if( (opts->trn_par=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-mid")==0 ){
                MEM_CHKFREE(opts->trn_mid);
               if( (opts->trn_mid=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-mtype")==0 ){
                if(sscanf(val,"%d",&opts->trn_mtype)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-ftype")==0 ){
                if(sscanf(val,"%d",&opts->trn_ftype)==1){
                    retval=0;
                }
            }  else if(strcmp(key,"trn-sensor-type")==0 ){
                if(sscanf(val,"%d",&opts->trn_sensor_type)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-fgrade")==0 ){
                if(sscanf(val,"%d",&opts->trn_fgrade)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-freinit")==0 ){
                if(sscanf(val,"%d",&opts->trn_freinit)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-mweight")==0 ){
                if(sscanf(val,"%d",&opts->trn_mweight)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-ncov")==0 ){
                if(sscanf(val,"%lf",&opts->trn_ncov)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-nerr")==0 ){
                if(sscanf(val,"%lf",&opts->trn_nerr)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-ecov")==0 ){
                if(sscanf(val,"%lf",&opts->trn_ecov)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-eerr")==0 ){
                if(sscanf(val,"%lf",&opts->trn_eerr)==1){
                    retval=0;
                }
            } else if(strcmp(key,"mb-out")==0 ){
                MEM_CHKFREE(opts->mb_out);
                if( (opts->mb_out=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-out")==0 ){
                MEM_CHKFREE(opts->trn_out);
                if( (opts->trn_out=CHK_STRDUP(val)) != NULL){
                    retval=0;
                }
            } else if(strcmp(key,"trn-decn")==0 ){
                if(sscanf(val,"%u",&opts->trn_decn)==1){
                    retval=0;
                }
            } else if(strcmp(key,"trn-decs")==0 ){
                if(sscanf(val,"%lf",&opts->trn_decs)==1){
                    retval=0;
                }
            } else if(strcmp(key,"covariance-magnitude-max")==0 ){
                if(sscanf(val,"%lf",&opts->covariance_magnitude_max)==1){
                    retval=0;
                }
            } else if(strcmp(key,"convergence-repeat-min")==0 ){
                if(sscanf(val,"%d",&opts->convergence_repeat_min)==1){
                    retval=0;
                }
            } else if(strcmp(key,"reinit-search")==0 ){
                if(sscanf(val,"%lf/%lf",&opts->reinit_search_xy,&opts->reinit_search_z)==1){
                    retval=0;
                }
            } else if(strcmp(key,"reinit-gain")==0 ){
                if( mkvc_parse_bool(val,&opts->reinit_gain_enable)==0){
                    retval=0;
                } else {
                    opts->reinit_gain_enable=true;
                    retval=0;
                }
            } else if(strcmp(key,"reinit-file")==0 ){
                if( mkvc_parse_bool(val,&opts->reinit_file_enable)==0){
                    retval=0;
                } else {
                    opts->reinit_file_enable=true;
                    retval=0;
                }
            } else if(strcmp(key,"reinit-xyoffset")==0 ){
                if(sscanf(val,"%lf",&opts->reinit_xyoffset_max)==1){
                    opts->reinit_xyoffset_enable = (opts->reinit_xyoffset_max > 0.0 ? true : false);
                    retval=0;
                } else {
                    opts->reinit_xyoffset_enable = false;
                    retval=0;
                }
            } else if(strcmp(key,"reinit-zoffset")==0 ){
                if(sscanf(val,"%lf/%lf",&opts->reinit_zoffset_min,&opts->reinit_zoffset_max)==2){
                    opts->reinit_zoffset_enable = true;
                    retval=0;
                }
            } else if(strcmp(key,"random-offset")==0 ){
                opts->random_offset_enable = true;
                retval=0;
            } else if(strcmp(key,"trn-en")==0 ){
                if( mkvc_parse_bool(val,&opts->trn_en)==0){
                    retval=0;
                } else {
                    opts->trn_en=true;
                    retval=0;
                }
            } else if(strcmp(key,"trn-dev")==0 ){
                r7k_device_t test = R7KC_DEV_INVALID;
                if( (test=r7k_parse_devid(val)) != R7KC_DEV_INVALID){
                    opts->trn_dev = test;
                }
                retval=0;
            } else if(strcmp(key,"config")==0 ){
                retval=0;
            } else {
                fprintf(stderr, "WARN - unsupported key/val [%s/%s]\n", key,val);
            }
        } else {
            // val is NULL
            // process args w/o required arguments
             if(strcmp(key,"trn-en")==0 ){
                opts->trn_en=true;
                retval=0;
            } else if(strcmp(key,"reinit-gain")==0 ){
                opts->reinit_gain_enable=true;
                retval=0;
            } else if(strcmp(key,"reinit-file")==0 ){
                opts->reinit_file_enable=true;
                retval=0;
            } else if(strcmp(key,"random-offset")==0 ){
                opts->random_offset_enable = true;
                retval=0;
            } else if(strcmp(key,"config")==0 ){
                retval=0;
            } else if(strcmp(key,"help")==0 ){
                opts->help=true;
                retval=0;
            } else {
                fprintf(stderr, "WARN - unsupported key/val [%s/NULL]\n", key);
            }
        }

        // perform mnemonic substitutions
        char *mval=NULL;
        s_sub_mnem(&opts->input,0,opts->input,CFG_MNEM_TRN_SOURCE_HOST,s_mnem_value(&mval,0,CFG_MNEM_TRN_SOURCE_HOST));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->output,0,opts->output,CFG_MNEM_SESSION,s_mnem_value(&mval,0,CFG_MNEM_SESSION));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->log_directory,0,opts->log_directory,CFG_MNEM_SESSION,s_mnem_value(&mval,0,CFG_MNEM_SESSION));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->mb_out,0,opts->mb_out,CFG_MNEM_TRN_HOST,s_mnem_value(&mval,0,CFG_MNEM_TRN_HOST));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_out,0,opts->trn_out,CFG_MNEM_TRN_HOST,s_mnem_value(&mval,0,CFG_MNEM_TRN_HOST));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_out,0,opts->trn_out,CFG_MNEM_TRN_GROUP,s_mnem_value(&mval,0,CFG_MNEM_TRN_GROUP));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_mid,0,opts->trn_mid,CFG_MNEM_TRN_SESSION,s_mnem_value(&mval,0,CFG_MNEM_TRN_SESSION));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->log_directory,0,opts->log_directory,CFG_MNEM_TRN_LOGFILES,s_mnem_value(&mval,0,CFG_MNEM_TRN_LOGFILES));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_map,0,opts->trn_map,CFG_MNEM_TRN_MAPFILES,s_mnem_value(&mval,0,CFG_MNEM_TRN_MAPFILES));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_par,0,opts->trn_par,CFG_MNEM_TRN_DATAFILES,s_mnem_value(&mval,0,CFG_MNEM_TRN_DATAFILES));
        MEM_CHKINVALIDATE(mval);
        s_sub_mnem(&opts->trn_cfg,0,opts->trn_cfg,CFG_MNEM_TRN_DATAFILES,s_mnem_value(&mval,0,CFG_MNEM_TRN_DATAFILES));
        MEM_CHKINVALIDATE(mval);
    } else {
        fprintf(stderr, "ERR - NULL key/val [%s / %s]\n", (key==NULL?"":key),val);
    }
    return retval;
}

static int s_mbtrnpp_load_config(char *config_path, mbtrnpp_opts_t *opts)
{
    int retval=-1;
    int err=0;
    int par=0;
    int inv=0;
    if(NULL!=config_path && NULL!=opts){
        int test=0;
        mkvc_reader_t *cfg_reader = mkvc_new(config_path,MBTRNPP_CONF_DEL,s_mbtrnpp_kvparse_fn);

        if( (test=mkvc_load_config(cfg_reader, opts, &par, &inv, &err))==0){
            retval=0;
        } else {
            fprintf(stderr,"ERR - mkvc_load_config ret[%d] par[%d] inv[%d] err[%d]\n",test,par,inv,err);
		}
        mkvc_destroy(&cfg_reader);
    }
    return retval;
}

static int s_mbtrnpp_process_cmdline(int argc, char **argv, mbtrnpp_opts_t *opts)
{
    int retval =-1;
    int err_count=0;
    for(int i = 1; i < argc; i++) {
        char *key=NULL;
        char *val=NULL;
        char *opt=strstr(argv[i],"--");
        if(NULL!=opt && mkvc_parse_kx(opt+2,MBTRNPP_CONF_DEL,&key,&val,false)==0 &&
           NULL!=key ){
            // parse key/value into configuration
            if(s_mbtrnpp_kvparse_fn(key,val,opts)!=0){
                fprintf(stderr, "ERR - invalid key/value [%s/%s]\n",key,val);
                err_count++;
            }
        } else {
            fprintf(stderr, "ERR - parse error in [%s]\n", argv[i]);
            err_count++;
        }
        // release key/value strings
        MEM_CHKINVALIDATE(key);
        MEM_CHKINVALIDATE(val);
    }

    retval = err_count;
    return retval;
}

static int s_mbtrnpp_configure(mbtrnpp_cfg_t *cfg, mbtrnpp_opts_t *opts)
{
    int retval =-1;

    if(NULL!=opts && NULL!=cfg){
        // verbose
        cfg->verbose = opts->verbose;

        // input
        s_parse_opt_input(cfg,opts->input);
        // output
        s_parse_opt_output(cfg,opts->output);
        // mb-out
        s_parse_opt_mbout(cfg,opts->mb_out);
        // trn-out
        s_parse_opt_trnout(cfg,opts->trn_out);
        // mbhbn
        cfg->mbsvr_hbtok = opts->mbhbn;
        // mbhbt
        cfg->mbsvr_hbto = opts->mbhbt;
        // trnhbt
        cfg->trnsvr_hbto = opts->trnhbt;
        // trnuhbt
        cfg->trnusvr_hbto = opts->trnuhbt;
        // delay
        cfg->mbtrnpp_loop_delay_msec = opts->delay;
        // statsec
        cfg->trn_status_interval_sec = opts->statsec;
        // statflags
        cfg->mbtrnpp_stat_flags = opts->statflags;
        // trn-en
        cfg->trn_enable = opts->trn_en;
        // trn-utm
        cfg->trn_utm_zone = opts->trn_utm;
        // trn-mtype
        cfg->trn_mtype = opts->trn_mtype;
        // trn-ftype
        cfg->trn_sensor_type = opts->trn_sensor_type;
        // trn-ftype
        cfg->trn_ftype = opts->trn_ftype;
        // trn-fgrade
        cfg->trn_fgrade = opts->trn_fgrade;
        // trn-freinit
        cfg->trn_freinit = opts->trn_freinit;
        // trn-mweight
        cfg->trn_mweight = opts->trn_mweight;
        // trn-ncov
        cfg->trn_max_ncov = opts->trn_ncov;
        // trn-nerr
        cfg->trn_max_nerr = opts->trn_nerr;
        // trn-ecov
        cfg->trn_max_ecov = opts->trn_ecov;
        // trn-eerr
        cfg->trn_max_eerr = opts->trn_eerr;
        // trn-map
        MEM_CHKFREE(cfg->trn_map_file);
        cfg->trn_map_file = CHK_STRDUP(opts->trn_map);
        // trn-cfg
        MEM_CHKFREE(cfg->trn_cfg_file);
        cfg->trn_cfg_file = CHK_STRDUP(opts->trn_cfg);
        // trn-par
        MEM_CHKFREE(cfg->trn_particles_file);
        cfg->trn_particles_file = CHK_STRDUP(opts->trn_par);
        // trn-mid
        MEM_CHKFREE(cfg->trn_mission_id);
        cfg->trn_mission_id = CHK_STRDUP(opts->trn_mid);
        // trn-decn
        cfg->trn_decn = opts->trn_decn;
        // trn-decs
        cfg->trn_decs = opts->trn_decs;
        // covariance-magnitude-max
        cfg->covariance_magnitude_max = opts->covariance_magnitude_max;
        // convergence-repeat-min
        cfg->convergence_repeat_min = opts->convergence_repeat_min;
        // reinit-search
        cfg->reinit_search_xy = opts->reinit_search_xy;
        cfg->reinit_search_z = opts->reinit_search_z;
        // reinit-gain
        cfg->reinit_gain_enable = opts->reinit_gain_enable;
        // reinit-file
        cfg->reinit_file_enable = opts->reinit_file_enable;
        // reinit-xyoffset
        cfg->reinit_xyoffset_enable = opts->reinit_xyoffset_enable;
        cfg->reinit_xyoffset_max = opts->reinit_xyoffset_max;
        // reinit-offset_z
        cfg->reinit_zoffset_enable = opts->reinit_zoffset_enable;
        cfg->reinit_zoffset_min = opts->reinit_zoffset_min;
        cfg->reinit_zoffset_max = opts->reinit_zoffset_max;
        cfg->random_offset_enable = opts->random_offset_enable;

        // format
        cfg->format = opts->format;
        // platform-file
        if(NULL!=opts->platform_file){
            strcpy(cfg->platform_file, opts->platform_file);
            cfg->use_platform_file=true;
        }
        // platform-target-sensor
        cfg->target_sensor = opts->platform_target_sensor;
        // tide-model
        if(NULL!=opts->tide_model){
            strcpy(cfg->tide_model, opts->tide_model);
            cfg->use_tide_model=true;
        }
        // log-directory
        s_parse_opt_logdir(cfg, opts->log_directory);
        // swath-width
        cfg->swath_width=opts->swath_width;
        // soundings
        cfg->n_output_soundings=opts->soundings;
        // median-filter
        if(NULL!=opts->median_filter){
            int n = sscanf(opts->median_filter, "%lf/%d/%d", &cfg->median_filter_threshold,
                       &cfg->median_filter_n_across, &cfg->median_filter_n_along);
            if (n == 3) {
                cfg->median_filter_en = true;
                cfg->n_buffer_max = cfg->median_filter_n_along;
            }
        } else {
            cfg->median_filter_en = false;
        }
        // device
        cfg->trn_dev=opts->trn_dev;
        retval=0;
    } else {
        fprintf(stderr, "ERR - invalid argument (NULL opts)\n");
    }

    return retval;
}

// validate configuration
static int s_mbtrnpp_validate_config(mbtrnpp_cfg_t *cfg)
{
    int retval=-1;
    if(NULL!=cfg){
        int err_count=0;

        if(cfg->median_filter_en){
            if(cfg->median_filter_n_across<0){
                err_count++;
                fprintf(stderr,"ERR - invalid median_filter_n_across [%d] valid range >0\n",cfg->median_filter_n_across);
            }
            if(cfg->median_filter_n_along<0){
                err_count++;
                fprintf(stderr,"ERR - invalid median_filter_n_along [%d] valid range >0\n",cfg->median_filter_n_along);
            }
            if(cfg->median_filter_threshold<0.0){
                err_count++;
                fprintf(stderr,"ERR - invalid median_filter_threshold [%lf] valid range >00\n",cfg->median_filter_threshold);
            }
            if(cfg->n_buffer_max<0){
                err_count++;
                fprintf(stderr,"ERR - invalid n_buffer_max [%d] valid range >0\n",cfg->n_buffer_max);
            }
        }

        if(cfg->swath_width<0.0){
            err_count++;
            fprintf(stderr,"ERR - invalid swath_width [%lf] valid range >0\n",cfg->swath_width);
        }

        switch (cfg->input_mode) {
            case INPUT_MODE_FILE:
                if(strlen(cfg->input)==0){
                    err_count++;
                    fprintf(stderr,"ERR - input path not set\n");
                }
                break;
            case INPUT_MODE_SOCKET:
                if(strlen(cfg->socket_definition)==0){
                    err_count++;
                    fprintf(stderr,"ERR - socket_definition not set\n");
                }
                break;
            default:
                err_count++;
                fprintf(stderr,"ERR - invalid input mode [%d]\n",cfg->input_mode);
                break;
        }

        if( (cfg->output_flags&OUTPUT_MB1_FILE_EN)!=0){
            if(strlen(cfg->output_mb1_file)==0){
                err_count++;
                fprintf(stderr,"ERR - output_mb1_file not set\n");
            }
        }

        if(cfg->trn_enable){
            // check TRN input source
            if(strlen(cfg->socket_definition)==0 &&
               strlen(cfg->input)==0){
                err_count++;
                fprintf(stderr,"ERR - input source not set\n");
            }

            // validate required TRN options
            if(NULL==cfg->trn_map_file){
                err_count++;
                fprintf(stderr,"ERR - trn_map_file not set\n");
            }
            if(NULL==cfg->trn_cfg_file){
                err_count++;
                fprintf(stderr,"ERR - trn_cfg_file not set\n");
            }
            if(cfg->trn_utm_zone<1 || cfg->trn_utm_zone>60){
                err_count++;
                fprintf(stderr,"ERR - invalid trn_utm_zone [%ld] valid range 1-60\n",cfg->trn_utm_zone);
            }
            if(cfg->trn_mtype<1 || cfg->trn_mtype>2){
                err_count++;
                fprintf(stderr,"ERR - invalid trn_mtype [%d] valid range 1-2\n",cfg->trn_mtype);
            }

            switch (cfg->trn_sensor_type) {
                case TRN_SENSOR_DVL:
                case TRN_SENSOR_MB:
                case TRN_SENSOR_PENCIL:
                case TRN_SENSOR_HOMER:
                case TRN_SENSOR_DELTAT:
                    break;

                default:
                    err_count++;
                    fprintf(stderr,"ERR - invalid trn sensor type [%d]\n",cfg->trn_sensor_type);
                    break;
            }

            if(cfg->trn_ftype<0 || cfg->trn_ftype>4){
                err_count++;
                fprintf(stderr,"ERR - invalid trn_mtype [%d] valid range 0-4\n",cfg->trn_ftype);
            }

            if((cfg->output_flags&OUTPUT_MB1_SVR_EN)){
                if(NULL==cfg->mb1svr_host ){
                    err_count++;
                    fprintf(stderr,"ERR - mb1svr_host not set\n");
                }
                if((cfg->mb1svr_port<1024 || cfg->mb1svr_port>65535)){
                    err_count++;
                    fprintf(stderr,"ERR - invalid mb1svr_port [%d] valid range 1024-65535\n",cfg->mb1svr_port);
                }
        	}
            if((cfg->output_flags&OUTPUT_TRN_SVR_EN)){
                if(NULL==cfg->trnsvr_host ){
                    err_count++;
                    fprintf(stderr,"ERR - trnsvr_host not set\n");
                }
                if((cfg->trnsvr_port<1024 || cfg->trnsvr_port>65535)){
                    err_count++;
                    fprintf(stderr,"ERR - invalid trnsvr_port [%d] valid range 1024-65535\n",cfg->trnsvr_port);
                }
            }
            if((cfg->output_flags&OUTPUT_TRNU_SVR_EN)){
                if(NULL==cfg->trnusvr_host ){
                    err_count++;
                    fprintf(stderr,"ERR - trnusvr_host not set\n");
                }
                if((cfg->trnusvr_port<1024 || cfg->trnusvr_port>65535)){
                    err_count++;
                    fprintf(stderr,"ERR - invalid trnusvr_port [%d] valid range 1024-65535\n",cfg->trnusvr_port);
                }
            }
            if((cfg->output_flags&OUTPUT_TRNUM_SVR_EN)){
                if(NULL==cfg->trnumsvr_group ){
                    err_count++;
                    fprintf(stderr,"ERR - trnumsvr_group not set\n");
                }
                if((cfg->trnumsvr_port<1024 || cfg->trnumsvr_port>65535)){
                    err_count++;
                    fprintf(stderr,"ERR - invalid trnumsvr_port [%d] valid range 1024-65535\n",cfg->trnumsvr_port);
                }
            }
       }

        retval=(err_count==0?0:-1);
    }
    return retval;
}

static void s_mbtrnpp_release_resources()
{

    fprintf(stderr,"release output servers...\n");
    // release output servers
    netif_destroy(&mb1svr);
    netif_destroy(&trnsvr);
    netif_destroy(&trnusvr);

    fprintf(stderr,"release TRN instance...\n");
    // release TRN instance
    wtnav_destroy(trn_instance);
    fprintf(stderr,"release TRN configuration...\n");
	// release TRN configuration
    trncfg_destroy(&trn_cfg);

    fprintf(stderr,"release stats instance...\n");
   // release stats instance
    mstats_profile_destroy(&app_stats);

    fprintf(stderr,"release log instances...\n");
	// release log instances
    mlog_delete_instance(mbtrnpp_mlog_id);
    mlog_delete_instance(mb1_blog_id);
    mlog_delete_instance(reson_blog_id);
    mlog_delete_instance(trnu_alog_id);
    mlog_delete_instance(trnu_blog_id);
#ifdef WITH_MB1_READER
    mlog_delete_instance(mb1r_blog_id);
    MEM_CHKFREE(mb1r_blog_path);
#endif
    fprintf(stderr,"release log paths...\n");
    // release log paths
    MEM_CHKFREE(mb1_blog_path);
    MEM_CHKFREE(mbtrnpp_mlog_path);
    MEM_CHKFREE(reson_blog_path);
    MEM_CHKFREE(trnu_alog_path);
    MEM_CHKFREE(trnu_blog_path);

    fprintf(stderr,"release app configuration...\n");
    // release app configuration
    s_mbtrnpp_free_opts(&mbtrn_opts);
    s_mbtrnpp_free_cfg(&mbtrn_cfg);

    fprintf(stderr,"release global variables...\n");
    // release global variables
    s_mbtrnpp_session_str(NULL, 0, RF_RELEASE);
    s_mbtrnpp_trnsession_str(NULL, 0, RF_RELEASE);
    s_mbtrnpp_cmdline_str(NULL, 0, 0, NULL, RF_RELEASE);
    fprintf(stderr,"done\n");

}

static void s_mbtrnpp_exit(int error)
{
    s_mbtrnpp_release_resources();
    exit(error);
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  char usage_message[] = "mbtrnpp \n"
                         "\t--verbose\n"
                         "\t--help\n"
                         "\t--config=path\n"
                         "\t--log-directory=path\n"
                         "\t--input=datalist|file|socket_definition\n"
                         "\t--output=file|'socket'\n"
                         "\t--swathwidth=value\n"
                         "\t--soundings=value\n"
                         "\t--median-filter=threshold/nx/ny\n"
                         "\t--format=format\n"
                         "\t--platform-file=file\n"
                         "\t--platform-target-sensor=sensor_id\n"
                         "\t--tide-model=file\n"
                         "\t--projection=projection_id\n"
                         "\t--statsec=d.d\n"
                         "\t--statflags=<MSF_STATUS:MSF_EVENT:MSF_ASTAT:MSF_PSTAT:MSF_READER>\n"
                         "\t--hbeat=n\n"
                         "\t--mbhbn=n\n"
                         "\t--mbhbt=d.d\n"
                         "\t--trnhbt=n\n"
                         "\t--trnuhbt=n\n"
                         "\t--delay=n\n"
                         "\t--trn-en\n"
                         "\t--trn-dev=s\n"
                         "\t--trn-utm\n"
                         "\t--trn-map\n"
                         "\t--trn-par\n"
                         "\t--trn-mid\n"
                         "\t--trn-cfg\n"
                         "\t--trn-mtype\n"
                         "\t--trn-sensor-type\n"
                         "\t--trn-ftype\n"
                         "\t--trn-fgrade\n"
                         "\t--trn-freinit\n"
                         "\t--trn-mweight\n"
                         "\t--trn-ncov\n"
                         "\t--trn-nerr\n"
                         "\t--trn-ecov\n"
                         "\t--trn-eerr\n"
                         "\t--mb-out=mb1svr[:host:port]/mb1/reson\n"
                         "\t--trn-out=trnsvr[:host:port]/trnusvr[:host:port]/trnumsvr[:group:port:ttl]/trnu/sout/serr/debug\n"
                         "\t--trn-decn\n"
                         "\t--trn-decs\n"
                         "\t--covariance-magnitude-max=covariance_magnitude_max\n"
                         "\t--convergence-repeat-min=convergence_repeat_min\n"
                         "\t--reinit-search=reinit_search_xy/reinit_search_z\n"
                         "\t--reinit-gain\n"
                         "\t--reinit-file\n"
                         "\t--reinit-xyoffset=xyoffset_max\n"
                         "\t--reinit-zoffset=offset_z_min/offset_z_max\n"
                         "\t--random-offset\n";
  extern char WIN_DECLSPEC *optarg;
//  int option_index;
  int errflg = 0;
//  int c;
//  int help;

  /* MBIO status variables */
  int status;
  int error = MB_ERROR_NO_ERROR;
  char *message;

  /* MBIO read control parameters */
  int read_datalist = false;
  int read_data = false;
//  int read_socket;
  void *datalist;
  int look_processed = MB_DATALIST_LOOK_UNSET;
  double file_weight;
  int system;
  int pings;
  int lonflip=0;
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
//  int obeams_bath;
//  int obeams_amp;
//  int opixels_ss;
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
  struct mb_platform_struct *platform = NULL;
  struct mb_sensor_struct *sensor_bathymetry = NULL;
  struct mb_sensor_struct *sensor_backscatter = NULL;
  struct mb_sensor_struct *sensor_position = NULL;
  struct mb_sensor_struct *sensor_depth = NULL;
  struct mb_sensor_struct *sensor_heading = NULL;
  struct mb_sensor_struct *sensor_rollpitch = NULL;
  struct mb_sensor_struct *sensor_heave = NULL;
  struct mb_sensor_struct *sensor_target = NULL;
//  int target_sensor = -1;

  /* tide model */
  int n_tide = 0;
  int itide_time = 0;
  double *tide_time_d = NULL;
  double *tide_tide = NULL;
  int tide_start_time_i[7], tide_end_time_i[7];

  /* buffer handling parameters */
  struct mbtrnpp_ping_struct ping[MBTRNPREPROCESS_BUFFER_DEFAULT];

  /* counting parameters */
  int n_pings_read = 0;
  int n_soundings_read = 0;
  int n_soundings_valid_read = 0;
  int n_soundings_flagged_read = 0;
  int n_soundings_null_read = 0;
  int n_pings_written = 0;
  int n_soundings_trimmed = 0;
  int n_soundings_decimated = 0;
  int n_soundings_flagged = 0;
  int n_soundings_written = 0;
  int n_tot_pings_read = 0;
  int n_tot_soundings_read = 0;
  int n_tot_soundings_valid_read = 0;
  int n_tot_soundings_flagged_read = 0;
  int n_tot_soundings_null_read = 0;
  int n_tot_pings_written = 0;
  int n_tot_soundings_trimmed = 0;
  int n_tot_soundings_decimated = 0;
  int n_tot_soundings_flagged = 0;
  int n_tot_soundings_written = 0;

  /* processing control variables */
  double tangent, threshold_tangent;
  int median_filter_n_total = 1;
  int median_filter_n_min = 1;
  double *median_filter_soundings = NULL;
  int n_median_filter_soundings = 0;
  double median;
  int n_output;

  /* mb1 output write control parameters */
  FILE *output_mb1_fp = NULL;
  char *output_buffer = NULL;
  int n_output_buffer_alloc = 0;
  size_t mb1_size, index;
  unsigned int checksum;

  /* log file parameters */
  FILE *logfp = NULL;
  double now_time_d;
  double log_file_open_time_d = 0.0;

  /* function pointers for reading realtime sonar data using a socket */
  int (*mbtrnpp_input_open)(int verbose, void *mbio_ptr, char *definition, int *error);
  int (*mbtrnpp_input_read)(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error);
  int (*mbtrnpp_input_close)(int verbose, void *mbio_ptr, int *error);

  int i_ping_process;
  int beam_start, beam_end, beam_decimation;
//  int i, ii, j, jj;
//  int jj0, jj1, dj;

  struct timeval timeofday;
  struct timezone timezone;

  /* set default values */
  mbtrn_cfg->format = 0;
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

#ifdef WITH_TEST_MNEM_SUB
    fprintf(stderr, "%s:%d - TODO - REMOVE MNEM-SUB TEST\n",__func__,__LINE__);
    s_test_mnem();
#endif

  // initialize session time string
    s_mbtrnpp_session_str(NULL,0,RF_NONE);
    s_mbtrnpp_trnsession_str(NULL,0,RF_NONE);
    // initialize command line string
    s_mbtrnpp_cmdline_str(NULL, 0, argc, argv, RF_NONE);

    fprintf(stderr,"command line:\n[%s]\n",s_mbtrnpp_cmdline_str(NULL, 0, 0, NULL, RF_NONE));

    // set run-time config defaults
    s_mbtrnpp_init_cfg(mbtrn_cfg);
    // set run-time option defaults
    s_mbtrnpp_init_opts(mbtrn_opts);

    fprintf(stderr,"\nconfiguration - default:\n");
    s_mbtrnpp_show_cfg(stderr, mbtrn_cfg,false,5);

    // load option overrrides from config file, if specified
    char *cfg_path=NULL;
    if(s_mbtrnpp_peek_opt_cfg(argc,argv,&cfg_path,0)!=NULL){
        fprintf(stderr,"loading config file [%s]\n",cfg_path);
        if(s_mbtrnpp_load_config(cfg_path,mbtrn_opts)!=0){
            MX_TRACE();
            fprintf(stderr,"ERR - error(s) in config file [%s]\n",cfg_path);
            errflg++;
        }
    }
    MEM_CHKINVALIDATE(cfg_path);
    fprintf(stderr,"options - post-config:\n");
    s_mbtrnpp_show_opts(stderr, mbtrn_opts,false,5);

    // load option overrrides from command line, if specified
    if(s_mbtrnpp_process_cmdline(argc,argv,mbtrn_opts)!=0){
        fprintf(stderr,"ERR - error(s) in cmdline\n");
        errflg++;
    };

    fprintf(stderr,"options - post-cmdline:\n");
    s_mbtrnpp_show_opts(stderr, mbtrn_opts,false,5);

    // configure using selected options
    if(s_mbtrnpp_configure(mbtrn_cfg, mbtrn_opts)!=0){
        fprintf(stderr,"ERR - error(s) in configure\n");
        errflg++;
    };

    // check configuration
    if(s_mbtrnpp_validate_config(mbtrn_cfg)!=0){
        errflg++;
    };

    fprintf(stderr,"\nconfiguration - final:\n");
    s_mbtrnpp_show_cfg(stderr, mbtrn_cfg,false,5);
    fprintf(stderr, "\n--------------------------------------------------------------------------------\n");
    fprintf(stderr, "MBtrnpp logging directory: %s\n", mbtrn_cfg->trn_log_dir);
    fprintf(stderr, "--------------------------------------------------------------------------------\n\n");

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      error = MB_ERROR_BAD_USAGE;
      s_mbtrnpp_exit(error);
  }

  /* print starting message */
  if (mbtrn_cfg->verbose == 1 || mbtrn_cfg->verbose <= -2 || mbtrn_opts->help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting debug statements */
  if (mbtrn_cfg->verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", mbtrn_cfg->verbose);
    fprintf(stderr, "dbg2       help:                     %d\n", mbtrn_opts->help);
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
    fprintf(stderr, "dbg2       input:                    %s\n", mbtrn_cfg->input);
    fprintf(stderr, "dbg2       format:                   %d\n", mbtrn_cfg->format);
    fprintf(stderr, "dbg2       output_mb1_file:          %s\n", mbtrn_cfg->output_mb1_file);
    fprintf(stderr, "dbg2       output_trn_file:          %s\n", mbtrn_cfg->output_trn_file);
    fprintf(stderr, "dbg2       swath_width:              %f\n", mbtrn_cfg->swath_width);
    fprintf(stderr, "dbg2       n_output_soundings:       %d\n", mbtrn_cfg->n_output_soundings);
    fprintf(stderr, "dbg2       median_filter_en:         %d\n", mbtrn_cfg->median_filter_en);
    fprintf(stderr, "dbg2       median_filter_n_across:   %d\n", mbtrn_cfg->median_filter_n_across);
    fprintf(stderr, "dbg2       median_filter_n_along:    %d\n", mbtrn_cfg->median_filter_n_along);
    fprintf(stderr, "dbg2       median_filter_threshold:  %f\n", mbtrn_cfg->median_filter_threshold);
    fprintf(stderr, "dbg2       n_buffer_max:             %d\n", mbtrn_cfg->n_buffer_max);
    fprintf(stderr, "dbg2       socket_definition:        %s\n", mbtrn_cfg->socket_definition);
    fprintf(stderr, "dbg2       mb1svr_host:              %s\n", mbtrn_cfg->mb1svr_host);
    fprintf(stderr, "dbg2       mb1svr_port:              %d\n", mbtrn_cfg->mb1svr_port);
  }

  /* if help desired then print it and exit */
  if (mbtrn_opts->help) {
      char help_message[] = "mbtrnpp reads raw multibeam data, applies automated cleaning\n\t"
      "and downsampling, and then passes the bathymetry on to a terrain relative navigation (TRN) process.\n";

    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    s_mbtrnpp_exit(error);
  }

#ifdef SOCKET_TIMING
  // print time message
  struct timeval stv = {0};
  gettimeofday(&stv, NULL);
  double start_sys_time = (double)stv.tv_sec + ((double)stv.tv_usec / 1000000.0) + (7 * 3600);
  fprintf(stderr, "%11.5lf systime %.4lf\n", mtime_dtime(), start_sys_time);
#endif

  mbtrnpp_init_debug(mbtrn_cfg->verbose);

#ifdef WITH_MBTNAV
    trn_cfg = trncfg_new(NULL, -1,
                         mbtrn_cfg->trn_utm_zone,
                         mbtrn_cfg->trn_mtype,
                         mbtrn_cfg->trn_sensor_type, mbtrn_cfg->trn_ftype, mbtrn_cfg->trn_fgrade,
                        mbtrn_cfg->trn_freinit,mbtrn_cfg->trn_mweight,
                        mbtrn_cfg->trn_map_file, mbtrn_cfg->trn_cfg_file,
                        mbtrn_cfg->trn_particles_file, mbtrn_cfg->trn_mission_id,
                        trn_oflags,mbtrn_cfg->trn_max_ncov,mbtrn_cfg->trn_max_nerr,
                        mbtrn_cfg->trn_max_ecov, mbtrn_cfg->trn_max_eerr);

    if (mbtrn_cfg->trn_enable &&  NULL!=trn_cfg ) {

        // If the environment variable TRN_LOGFILES is not already set then
        // set it so that the TRN logfiles are created within the mbtrnpp
        // log directory
        if (getenv("TRN_LOGFILES") == NULL) {
          setenv("TRN_LOGFILES", mbtrn_cfg->trn_log_dir, 0);
          fprintf(stderr, "Setting the Terrain-nav log directory to %s by creating the environment variable TRN_LOGFILES\n",
                  mbtrn_cfg->trn_log_dir);
        } else {
          fprintf(stderr, "Unable to set the Terrain-nav log directory to %s because the environment variable TRN_LOGFILES=%s exists\n",
                  mbtrn_cfg->trn_log_dir, getenv("TRN_LOGFILES"));
        }

        mbtrnpp_init_trn(&trn_instance,mbtrn_cfg->verbose, trn_cfg);

        // temporarily enable module debug
        mx_module_t *mod_save = NULL;
        if (mbtrn_cfg->verbose!=0) {
            mod_save = mxd_save(MBTRNPP_DEBUG);
            mxd_setModule(MBTRNPP_DEBUG, 5, false, NULL);
        }

        // initialize socket outputs
        int test=-1;
        if( (test=mbtrnpp_init_trnsvr(&trnsvr, trn_instance, mbtrn_cfg-> trnsvr_host,mbtrn_cfg->trnsvr_port,true))==0){
//            MX_DEBUG("TRN server netif OK [%s:%d]\n",mbtrn_cfg-> trnsvr_host,mbtrn_cfg->trnsvr_port);
            fprintf(stderr,"TRN server netif OK [%s:%d]\n",mbtrn_cfg-> trnsvr_host,mbtrn_cfg->trnsvr_port);

        } else {
            fprintf(stderr, "\nTRN server netif init failed [%d] [%d %s]\n",test,errno,strerror(errno));
        }

        if( (test=mbtrnpp_init_trnusvr(&trnusvr, mbtrn_cfg->trnusvr_host,mbtrn_cfg->trnusvr_port, trn_instance, true))==0){
//            MX_DEBUG("TRNU server netif OK [%s:%d]\n",mbtrn_cfg->trnusvr_host,mbtrn_cfg-> trnusvr_port);
            fprintf(stderr,"TRNU server netif OK [%s:%d]\n",mbtrn_cfg->trnusvr_host,mbtrn_cfg->trnusvr_port);
        } else {
            fprintf(stderr, "TRNU server netif init failed [%d] [%d %s]\n",test,errno,strerror(errno));
        }

        if( (test=mbtrnpp_init_trnumsvr(&trnumsvr, mbtrn_cfg->trnumsvr_group,mbtrn_cfg->trnumsvr_port, trn_instance, true))==0){
            //            MX_DEBUG("TRNUM server netif OK [%s:%d]\n",mbtrn_cfg->trnumsvr_group,mbtrn_cfg-> trnumsvr_port);
            fprintf(stderr,"TRNUM server netif OK [%s:%d]\n",mbtrn_cfg->trnumsvr_group,mbtrn_cfg->trnumsvr_port);
        } else {
            fprintf(stderr, "TRNUM server netif init failed [%d] [%d %s]\n",test,errno,strerror(errno));
        }

        if (mbtrn_cfg->verbose != 0) {
            mxd_restore(MBTRNPP_DEBUG, mod_save);
        }
    } else {
        fprintf(stderr,"WARN: skipping TRN init trn_enable[%c] trn_cfg[%p]\n",(mbtrn_cfg->trn_enable?'Y':'N'),trn_cfg);
    }

    trncfg_show(trn_cfg, true, 5);

    // log config options in mbtrnpp message log
    char *buf = NULL;
    int slen = s_mbtrnpp_optstr(&buf, 0, mbtrn_opts, NULL, "=", "\n", 0, 0, 0);
    if( slen>0 && NULL!=buf){
        mlog_tprintf(mbtrnpp_mlog_id, "opts:\n%s\n",buf);
    } else {
        fprintf(stderr,"s_mbtrnpp_optstr failed: len[%d] buf[%p]\n",slen,buf);
    }
    free(buf);
    // log config settings in mbtrnpp message log
    buf = NULL;
    slen = s_mbtrnpp_cfgstr(&buf, 0, mbtrn_cfg, NULL, "=", "\n", 0, 0, 0);
    if( slen>0 && NULL!=buf){
        mlog_tprintf(mbtrnpp_mlog_id, "cfg:\n%s\n",buf);
    } else {
        fprintf(stderr,"s_mbtrnpp_cfgstr failed: len[%d] buf[%p]\n",slen,buf);
    }
    free(buf);

#endif // WITH_MBTNAV

  /* load platform definition if specified */
  if (mbtrn_cfg->use_platform_file == true) {
    status = mb_platform_read(mbtrn_cfg->verbose, mbtrn_cfg->platform_file, (void **)&platform, &error);
    if (status == MB_FAILURE) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open and parse platform file: %s\n", mbtrn_cfg->platform_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      s_mbtrnpp_exit(error);
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
    if (mbtrn_cfg->target_sensor < 0)
      mbtrn_cfg->target_sensor = platform->source_bathymetry;
    if (mbtrn_cfg->target_sensor >= 0)
      sensor_target = &(platform->sensors[mbtrn_cfg->target_sensor]);
  }

  /* load tide model if specified */
  if (mbtrn_cfg->use_tide_model) {

    /* count the data points in the tide file */
    n_tide = 0;
    FILE *tfp = NULL;
    if ((tfp = fopen(mbtrn_cfg->tide_model, "r")) == NULL) {
      fprintf(stderr, "\nUnable to Open Tide Model File <%s> for reading\n", mbtrn_cfg->tide_model);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    char *result;
    mb_path tidebuffer;
    while ((result = fgets(tidebuffer, sizeof(mb_path), tfp)) == tidebuffer)
      n_tide++;
    fclose(tfp);

    /* allocate memory for tide model */
    if (n_tide > 0 && error == MB_ERROR_NO_ERROR) {
      status = mb_mallocd(mbtrn_cfg->verbose, __FILE__, __LINE__, n_tide * sizeof(double),
                          (void **)&tide_time_d, &error);
      status = mb_mallocd(mbtrn_cfg->verbose, __FILE__, __LINE__, n_tide * sizeof(double),
                          (void **)&tide_tide, &error);
      if (error != MB_ERROR_NO_ERROR) {
        mb_error(mbtrn_cfg->verbose, error, &message);
        fprintf(stderr, "\nMBIO Error allocating tide model arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        s_mbtrnpp_exit(error);
      }
    }

    /* read the data points in the tide file */
    n_tide = 0;
    if ((tfp = fopen(mbtrn_cfg->tide_model, "r")) == NULL) {
      fprintf(stderr, "\nUnable to Open Tide Model File <%s> for reading\n", mbtrn_cfg->tide_model);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    while ((result = fgets(tidebuffer, sizeof(mb_path), tfp)) == tidebuffer) {
      /* deal with tide in form: time_d tide - ignore comments */
      if (tidebuffer[0] != '#') {
        if (sscanf(tidebuffer, "%lf %lf", &tide_time_d[n_tide], &tide_tide[n_tide]) == 2) {
          if (tide_time_d[n_tide] > 0.0 && (n_tide == 0 || tide_time_d[n_tide] > tide_time_d[n_tide-1]))
          n_tide++;
        }
      }
    }
    fclose(tfp);

    /* get start and finish times of tide */
    if (n_tide > 0) {
      mb_get_date(mbtrn_cfg->verbose, tide_time_d[0], tide_start_time_i);
      mb_get_date(mbtrn_cfg->verbose, tide_time_d[n_tide - 1], tide_end_time_i);

      /* give the statistics */
      fprintf(stderr, "\n%d tide records read from file <%s>\n", n_tide, mbtrn_cfg->tide_model);
      fprintf(stderr, "Tide start time: %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
              tide_start_time_i[0], tide_start_time_i[1],
              tide_start_time_i[2], tide_start_time_i[3], tide_start_time_i[4],
              tide_start_time_i[5], tide_start_time_i[6]);
      fprintf(stderr, "Tide end time:   %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
              tide_end_time_i[0], tide_end_time_i[1],
              tide_end_time_i[2], tide_end_time_i[3], tide_end_time_i[4],
              tide_end_time_i[5], tide_end_time_i[6]);
    }

    /* else error reading the tide model */
    else {
          fprintf(stderr, "\nNo tide read from file <%s>\n", mbtrn_cfg->tide_model);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
    }
  }

  /* initialize output */
    if ( OUTPUT_FLAG_SET(OUTPUT_MBSYS_STDOUT)) {
    }
  /* if enabled initialize ipc with TRN */
  /* else open ipc to TRN */

 if ( OUTPUT_FLAG_SET(OUTPUT_MB1_SVR_EN) ) {

     mx_module_t *mod_save = NULL;
     if (mbtrn_cfg->verbose!=0) {
         mod_save = mxd_save(MBTRNPP_DEBUG);
         mxd_setModule(MBTRNPP_DEBUG, 5, false, NULL);
     }

    int test = -1;
     if( (test=mbtrnpp_init_mb1svr(&mb1svr, mbtrn_cfg->mb1svr_host,mbtrn_cfg->mb1svr_port,true))==0){
         MX_PRINT("MB1 server netif OK [%s:%d]\n",mbtrn_cfg->mb1svr_host,mbtrn_cfg->mb1svr_port);
      } else {
          fprintf(stderr, "MB1 server netif init failed [%d] [%d %s]\n",test,errno,strerror(errno));
      }

    if (mbtrn_cfg->verbose != 0) {
        mxd_restore(MBTRNPP_DEBUG, mod_save);
    }
  }

    /* if enabled open output file for mb1 data in MB-System supported format */
   if ( OUTPUT_FLAG_SET(OUTPUT_MB1_FILE_EN)) {
     if(NULL!=mbtrn_cfg->trn_log_dir){
         if(mbtrn_cfg->output_mb1_file[0]!='/' && mbtrn_cfg->output_mb1_file[0]!='.'){
             char *ocopy = strdup(mbtrn_cfg->output_mb1_file);
             if(NULL!=ocopy){
             sprintf(mbtrn_cfg->output_mb1_file,"%s/%s",mbtrn_cfg->trn_log_dir,ocopy);
             free(ocopy);
             }
         }
     }
    output_mb1_fp = fopen(mbtrn_cfg->output_mb1_file, "w");
  }

#ifdef WITH_MBTNAV
  /* if TRN is enabled then open file for ascii table of TRN results */
   if(NULL!=mbtrn_cfg->trn_log_dir){
       if(mbtrn_cfg->output_trn_file[0]!='/' && mbtrn_cfg->output_trn_file[0]!='.'){
           char *ocopy = strdup(mbtrn_cfg->output_trn_file);
           if(NULL!=ocopy){
           sprintf(mbtrn_cfg->output_trn_file,"%s/%s",mbtrn_cfg->trn_log_dir,ocopy);
           free(ocopy);
           }
       }
   }
  output_trn_fp = fopen(mbtrn_cfg->output_trn_file, "w");
#endif

  /* get number of ping records to hold */
  if (mbtrn_cfg->median_filter_en == true) {
    median_filter_n_total = mbtrn_cfg->median_filter_n_across * mbtrn_cfg->median_filter_n_along;
    median_filter_n_min = median_filter_n_total / 2;

    /* allocate memory for median filter */
    if (error == MB_ERROR_NO_ERROR) {
      status = mb_mallocd(mbtrn_cfg->verbose, __FILE__, __LINE__, median_filter_n_total * sizeof(double),
                          (void **)&median_filter_soundings, &error);
      if (error != MB_ERROR_NO_ERROR) {
        mb_error(mbtrn_cfg->verbose, error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        s_mbtrnpp_exit(error);
      }
    }
  }

  /* get format if required */
  if (mbtrn_cfg->format == 0)
    mb_get_format(mbtrn_cfg->verbose, mbtrn_cfg->input, NULL, &mbtrn_cfg->format, &error);

  /* determine whether to read one file or a list of files */
  if (mbtrn_cfg->format < 0)
    read_datalist = true;

  /* open file list */
  if (read_datalist == true) {
    if ((status = mb_datalist_open(mbtrn_cfg->verbose, &datalist, mbtrn_cfg->input, look_processed, &error)) != MB_SUCCESS) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open data list file: %s\n", mbtrn_cfg->input);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      s_mbtrnpp_exit(error);
    }
    if ((status = mb_datalist_read(mbtrn_cfg->verbose, datalist, ifile, dfile, &mbtrn_cfg->format, &file_weight, &error)) == MB_SUCCESS)
      read_data = true;
    else
      read_data = false;
  }
  /* else copy single filename to be read */
  else {
    strcpy(ifile, mbtrn_cfg->input);
    read_data = true;
  }

  /* set transmit_gain threshold according to format */
  double transmit_gain_threshold = 0.0;
  if (mbtrn_cfg->reinit_gain_enable) {
    if (mbtrn_cfg->format == MBF_RESON7KR || mbtrn_cfg->format == MBF_RESON7K3) {
        transmit_gain_threshold = TRN_XMIT_GAIN_RESON7K_DFL;
    }
    else if (mbtrn_cfg->format == MBF_KEMKMALL) {
        transmit_gain_threshold = TRN_XMIT_GAIN_KMALL_DFL;
    }
#ifdef WITH_MB1_READER
    else if (mbtrn_cfg->format == MBF_MBARIMB1) {
        transmit_gain_threshold = TRN_XMIT_GAIN_MB1_DFL;
    }
#endif // WITH_MB1_READER
    mlog_tprintf(mbtrnpp_mlog_id, "i,transmit gain threshold[%.2lf]\n", transmit_gain_threshold);
  }

  // calculate static position offset applied to all input navigation using
  // a random number generator - the range of the random offset is the circle
  // radius of the reinit_xyoffset_max parameter
  // the offset is calculated here in eastings northings but is converted to
  // decimal degrees lon and lat when the first data are read
  double nav_offset_east = 0.0;
  double nav_offset_north = 0.0;
  double nav_offset_lon = 0.0;
  double nav_offset_lat = 0.0;
  bool nav_offset_init = false;
  if (mbtrn_cfg->random_offset_enable) {
      srand(time(0) / getpid());
      // TODO: what is the intent of this loop? (klh)
      for (int i=0; i < 100; i++) {
          int j = rand();
          j+=1; // silence unused variable warning
      }
      double nav_offset_mag = mbtrn_cfg->reinit_xyoffset_max * ((double)rand()) / ((double)RAND_MAX);
      double nav_offset_bearing = 2.0 * M_PI * ((double)rand()) / ((double)RAND_MAX);
      nav_offset_east = nav_offset_mag * sin(nav_offset_bearing);
      nav_offset_north = nav_offset_mag * cos(nav_offset_bearing);
      fprintf(stderr, "Applying random static offset to input navigation: Magnitude: %f bearing: %f easting: %f m  northing: %f m\n",
                      nav_offset_mag, nav_offset_bearing * 180.0 / M_PI, nav_offset_east, nav_offset_north);
  }

  // kick off the first cycle here
  // future cycles start and end in the stats update
  MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT], mtime_dtime());
  MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_STATS_XT], mtime_dtime());

  /* plan on storing enough pings for median filter */
  mbtrn_cfg->n_buffer_max = mbtrn_cfg->median_filter_n_along;
  int n_ping_process = mbtrn_cfg->n_buffer_max / 2;
  int idataread = 0;

    /* loop over all files to be read */
  while (read_data == true) {
      char log_message[LOG_MSG_BUF_SZ];
      memset(log_message,0,LOG_MSG_BUF_SZ);

    /* open log file if specified */
    if (mbtrn_cfg->make_logs == true) {
      gettimeofday(&timeofday, &timezone);
      now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
      // fprintf(stderr,"CHECKING AT TOP OF LOOP: logfp:%p log_file_open_time_d:%.6ff now_time_d:%.6f\n", logfp,
      // log_file_open_time_d, now_time_d);
      if (logfp == NULL || (now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) {
        if (logfp != NULL) {
          status = mbtrnpp_logstatistics(mbtrn_cfg->verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                         n_soundings_flagged_read, n_soundings_null_read, n_pings_written, n_soundings_trimmed,
                                         n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
          n_tot_pings_read += n_pings_read;
          n_tot_soundings_read += n_soundings_read;
          n_tot_soundings_valid_read += n_soundings_valid_read;
          n_tot_soundings_flagged_read += n_soundings_flagged_read;
          n_tot_soundings_null_read += n_soundings_null_read;
          n_tot_pings_written += n_pings_written;
          n_tot_soundings_trimmed += n_soundings_trimmed;
          n_tot_soundings_decimated += n_soundings_decimated;
          n_tot_soundings_flagged += n_soundings_flagged;
          n_tot_soundings_written += n_soundings_written;
          n_pings_read = 0;
          n_soundings_read = 0;
          n_soundings_valid_read = 0;
          n_soundings_flagged_read = 0;
          n_soundings_null_read = 0;
          n_pings_written = 0;
          n_soundings_trimmed = 0;
          n_soundings_decimated = 0;
          n_soundings_flagged = 0;
          n_soundings_written = 0;

          status = mbtrnpp_closelog(mbtrn_cfg->verbose, &logfp, &error);
        }

        status = mbtrnpp_openlog(mbtrn_cfg->verbose, mbtrn_cfg->log_directory, &logfp, &error);
        if (status == MB_SUCCESS) {
          gettimeofday(&timeofday, &timezone);
          log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
          status = mbtrnpp_logparameters(mbtrn_cfg->verbose, logfp, mbtrn_cfg->input, mbtrn_cfg->format, mbtrn_cfg->output_mb1_file, mbtrn_cfg->swath_width, mbtrn_cfg->n_output_soundings,
                                         mbtrn_cfg->median_filter_en, mbtrn_cfg->median_filter_n_across, mbtrn_cfg->median_filter_n_along,
                                         mbtrn_cfg->median_filter_threshold, mbtrn_cfg->n_buffer_max, &error);
        }
        else {
          fprintf(stderr, "\nLog file could not be opened in directory %s...\n", mbtrn_cfg->log_directory);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          s_mbtrnpp_exit(error);
        }
      }
    }

    /* check for format with amplitude or sidescan data */
    status = mb_format_system(mbtrn_cfg->verbose, &mbtrn_cfg->format, &system, &error);
    status = mb_format_dimensions(mbtrn_cfg->verbose, &mbtrn_cfg->format, &beams_bath, &beams_amp, &pixels_ss, &error);

    /* initialize reading the input swath data over a socket interface
     * using functions defined in this code block and passed into the
     * init function as function pointers */
    if (strncmp(mbtrn_cfg->input, "socket", 6) == 0) {
      if (mbtrn_cfg->format == MBF_RESON7KR || mbtrn_cfg->format == MBF_RESON7K3) {
        mbtrnpp_input_open = &mbtrnpp_reson7kr_input_open;
        mbtrnpp_input_read = &mbtrnpp_reson7kr_input_read;
        mbtrnpp_input_close = &mbtrnpp_reson7kr_input_close;
      } else if (mbtrn_cfg->format == MBF_KEMKMALL) {
        mbtrnpp_input_open = &mbtrnpp_kemkmall_input_open;
        mbtrnpp_input_read = &mbtrnpp_kemkmall_input_read;
        mbtrnpp_input_close = &mbtrnpp_kemkmall_input_close;
      }
#ifdef WITH_MB1_READER
      else if (mbtrn_cfg->format == MBF_MBARIMB1) {
          mbtrnpp_input_open = &mbtrnpp_mb1r_input_open;
          mbtrnpp_input_read = &mbtrnpp_mb1r_input_read;
          mbtrnpp_input_close = &mbtrnpp_mb1r_input_close;
      }
#endif // WITH_MB1_READER
      else{
          fprintf(stderr,"ERR - Invalid output format [%d]\n",mbtrn_cfg->format);
      }
      if ((status = mb_input_init(mbtrn_cfg->verbose, mbtrn_cfg->socket_definition, mbtrn_cfg->format, pings, lonflip, bounds,
                                  btime_i, etime_i, speedmin, timegap,
                                  &imbio_ptr, &btime_d, &etime_d,
                                  &beams_bath, &beams_amp, &pixels_ss,
                                  mbtrnpp_input_open, mbtrnpp_input_read, mbtrnpp_input_close,
                                  &error)) != MB_SUCCESS) {
        sprintf(log_message, "MBIO Error returned from function <mb_input_init>");
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(mbtrn_cfg->verbose, error, &message);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Sonar data socket <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mlog_tprintf(mbtrnpp_mlog_id,"e,sonar data connection init failed\n");
        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBCON]);

        s_mbtrnpp_exit(error);
      }
      else {

        sprintf(log_message, "Sonar data socket <%s> initialized for reading", ifile);
          mlog_tprintf(mbtrnpp_mlog_id,"i,sonar data socket initialized\n");
          mlog_tprintf(mbtrnpp_mlog_id,"MBIO format id,%d\n", mbtrn_cfg->format);
        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);

        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        if (mbtrn_cfg->verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "MBIO format id: %d", mbtrn_cfg->format);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        if (mbtrn_cfg->verbose > 0)
          fprintf(stderr, "%s\n", log_message);
      }
    }

    /* otherwised open swath data files as is normal for MB-System programs */
    else {

      if ((status = mb_read_init(mbtrn_cfg->verbose, ifile, mbtrn_cfg->format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                                 &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) !=
          MB_SUCCESS) {

        sprintf(log_message, "MBIO Error returned from function <mb_read_init>");
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        mb_error(mbtrn_cfg->verbose, error, &message);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, message, &error);
        fprintf(stderr, "%s\n", message);

        sprintf(log_message, "Sonar File <%s> not initialized for reading", ifile);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

        sprintf(log_message, "Program <%s> Terminated", program_name);
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fprintf(stderr, "\n%s\n", log_message);

          mlog_tprintf(mbtrnpp_mlog_id,"e,sonar data file init failed\n");

        s_mbtrnpp_exit(error);
      }
      else {
        sprintf(log_message, "Sonar File <%s> of format <%d> initialized for reading", ifile, mbtrn_cfg->format);
          mlog_tprintf(mbtrnpp_mlog_id,"i,sonar data file initialized\n");
        if (logfp != NULL)
          mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        //if (mbtrn_cfg->verbose > 0)
          fprintf(stderr, "\n%s\n", log_message);
      }
    }

    /* allocate memory for data arrays */
    memset(ping, 0, MBTRNPREPROCESS_BUFFER_DEFAULT * sizeof(struct mbtrnpp_ping_struct));
    for (int i = 0; i < mbtrn_cfg->n_buffer_max; i++) {
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&ping[i].beamflag,
                                   &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                   (void **)&ping[i].beamflag_filter, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bath, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ping[i].amp, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&ping[i].bathacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&ping[i].bathalongtrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ping[i].ss, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                   (void **)&ping[i].ssacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(mbtrn_cfg->verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                   (void **)&ping[i].ssalongtrack, &error);
    }

    /* if option for AUV Sentry is set, then set flag in mb_io_ptr structure that
        will apply the Sentry sensordepth kluge to the multibem data - the sensordepth
        value has to be accessed in a nonstandard location in the data stream */
    bool auv_sentry = true;
    if (auv_sentry) {
      struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)imbio_ptr;
      mb_io_ptr->save10 = 1;
    }

    /* loop over reading data */
    int n_non_survey_data = 0;
    bool done = false;
    int num_kinds_read[MB_DATA_KINDS + 1] = { 0 };
    int num_kinds_read_tot[MB_DATA_KINDS + 1] = { 0 };
    while (!done) {
      /* open new log file if it is time */
      if (mbtrn_cfg->make_logs == true) {

        gettimeofday(&timeofday, &timezone);
        now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
        // fprintf(stderr,"CHECKING AT MIDDLE OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n", logfp,
        // log_file_open_time_d, now_time_d);
        if (logfp == NULL || (now_time_d - log_file_open_time_d) > MBTRNPREPROCESS_LOGFILE_TIMELENGTH) {
          if (logfp != NULL) {
            status = mbtrnpp_logstatistics(mbtrn_cfg->verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                           n_soundings_flagged_read, n_soundings_null_read, n_pings_written, n_soundings_trimmed,
                                           n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
            n_tot_pings_read += n_pings_read;
            n_tot_soundings_read += n_soundings_read;
            n_tot_soundings_valid_read += n_soundings_valid_read;
            n_tot_soundings_flagged_read += n_soundings_flagged_read;
            n_tot_soundings_null_read += n_soundings_null_read;
            n_tot_pings_written += n_pings_written;
            n_tot_soundings_trimmed += n_soundings_trimmed;
            n_tot_soundings_decimated += n_soundings_decimated;
            n_tot_soundings_flagged += n_soundings_flagged;
            n_tot_soundings_written += n_soundings_written;
            n_pings_read = 0;
            n_soundings_read = 0;
            n_soundings_valid_read = 0;
            n_soundings_flagged_read = 0;
            n_soundings_null_read = 0;
            n_pings_written = 0;
            n_soundings_trimmed = 0;
            n_soundings_decimated = 0;
            n_soundings_flagged = 0;
            n_soundings_written = 0;

            status = mbtrnpp_closelog(mbtrn_cfg->verbose, &logfp, &error);
          }

          status = mbtrnpp_openlog(mbtrn_cfg->verbose, mbtrn_cfg->log_directory, &logfp, &error);
          if (status == MB_SUCCESS) {
            gettimeofday(&timeofday, &timezone);
            log_file_open_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
            status = mbtrnpp_logparameters(mbtrn_cfg->verbose, logfp, mbtrn_cfg->input, mbtrn_cfg->format, mbtrn_cfg->output_mb1_file, mbtrn_cfg->swath_width, mbtrn_cfg->n_output_soundings,
                                           mbtrn_cfg->median_filter_en, mbtrn_cfg->median_filter_n_across, mbtrn_cfg->median_filter_n_along,
                                           mbtrn_cfg->median_filter_threshold, mbtrn_cfg->n_buffer_max, &error);
          }
          else {
            fprintf(stderr, "\nLog file could not be opened in directory %s...\n", mbtrn_cfg->log_directory);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            s_mbtrnpp_exit(error);
          }
        }
      }

      /* read the next data */
      error = MB_ERROR_NO_ERROR;

      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_GETALL_XT], mtime_dtime());
      status = mb_get_all(mbtrn_cfg->verbose, imbio_ptr, &store_ptr, &kind, ping[idataread].time_i, &ping[idataread].time_d,
                          &ping[idataread].navlon, &ping[idataread].navlat, &ping[idataread].speed,
                          &ping[idataread].heading, &ping[idataread].distance, &ping[idataread].altitude,
                          &ping[idataread].sensordepth, &ping[idataread].beams_bath, &ping[idataread].beams_amp,
                          &ping[idataread].pixels_ss, ping[idataread].beamflag, ping[idataread].bath, ping[idataread].amp,
                          ping[idataread].bathacrosstrack, ping[idataread].bathalongtrack, ping[idataread].ss,
                          ping[idataread].ssacrosstrack, ping[idataread].ssalongtrack, comment, &error);

      //            MX_LPRINT(MBTRNPP, 4, "mb_get_all - status[%d] kind[%d] err[%d]\n",status, kind,
      //            error);
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_GETALL_XT], mtime_dtime());
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_PING_XT], mtime_dtime());
      if (error <= 0) {
        num_kinds_read[kind]++;
        num_kinds_read_tot[kind]++;
      }
      if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
        ping[idataread].count = ndata;
        ndata++;
        n_pings_read++;
        n_soundings_read += ping[idataread].beams_bath;
        n_non_survey_data = 0;

        // apply transmit gain thresholding
        double transmit_gain;
        double pulse_length;
        double receive_gain;
        status = mb_gains(mbtrn_cfg->verbose, imbio_ptr, store_ptr, &kind, &transmit_gain, &pulse_length, &receive_gain, &error);
        if (transmit_gain < transmit_gain_threshold) {
            for (int i = 0; i < ping[idataread].beams_bath; i++) {
                if (mb_beam_ok(ping[idataread].beamflag[i])) {
                    ping[idataread].beamflag[i] = (char)(MB_FLAG_SONAR | MB_FLAG_FLAG);
                }
            }
        }

          // count soundings
        for (int i = 0; i < ping[idataread].beams_bath; i++) {
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

        status = mb_extract_nav(mbtrn_cfg->verbose, imbio_ptr, store_ptr, &kind, ping[idataread].time_i, &ping[idataread].time_d,
                                &ping[idataread].navlon, &ping[idataread].navlat, &ping[idataread].speed,
                                &ping[idataread].heading, &ping[idataread].sensordepth, &ping[idataread].roll,
                                &ping[idataread].pitch, &ping[idataread].heave, &error);
        status = mb_extract_altitude(mbtrn_cfg->verbose, imbio_ptr, store_ptr, &kind, &ping[idataread].sensordepth,
                                     &ping[idataread].altitude, &error);

        // apply static nav offset if specified
        if (mbtrn_cfg->random_offset_enable) {
          if (!nav_offset_init) {
            double mtodeglon, mtodeglat;
            mb_coor_scale(mbtrn_cfg->verbose, ping[idataread].navlat, &mtodeglon, &mtodeglat);
            nav_offset_lon = nav_offset_east * mtodeglon;
            nav_offset_lat = nav_offset_north * mtodeglat;
            nav_offset_init = true;
          }
          ping[idataread].navlon += nav_offset_lon;
          ping[idataread].navlat += nav_offset_lat;
        }

        // apply tide model if specified
        if (n_tide > 0 && ping[idataread].time_d >= tide_time_d[0]
          && ping[idataread].time_d <= tide_time_d[n_tide-1]) {
          double tidevalue = 0.0;
          mb_linear_interp(mbtrn_cfg->verbose, tide_time_d - 1, tide_tide - 1, n_tide,
                            ping[idataread].time_d, &tidevalue, &itide_time, &error);
          ping[idataread].sensordepth -= tidevalue;
          for (int i = 0; i < ping[idataread].beams_bath; i++) {
            if (ping[idataread].beamflag[i] != MB_FLAG_NULL) {
                ping[idataread].bath[i] -= tidevalue;
            }
          }
        }

        /* only process and output if enough data have been read */
        if (ndata == mbtrn_cfg->n_buffer_max) {
          for (int i = 0; i < mbtrn_cfg->n_buffer_max; i++) {
            if (ping[i].count == n_ping_process)
              i_ping_process = i;
          }

          // fprintf(stderr, "\nProcess some data: ndata:%d counts: ", ndata);
          // for (i = 0; i < mbtrn_cfg->n_buffer_max; i++) {
          //    fprintf(stderr,"%d ", ping[i].count);
          //}
          // fprintf(stderr," : process %d\n", i_ping_process);

          /* apply swath width */
          threshold_tangent = tan(DTR * 0.5 * mbtrn_cfg->swath_width);
          beam_start = ping[i_ping_process].beams_bath - 1;
          beam_end = 0;
          for (int j = 0; j < ping[i_ping_process].beams_bath; j++) {
            if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
              tangent = ping[i_ping_process].bathacrosstrack[j] /
                        (ping[i_ping_process].bath[j] - ping[i_ping_process].sensordepth);
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
          if(beam_start<0 || beam_end<0)
          mlog_tprintf(mbtrnpp_mlog_id,"e,ping array boundary violation beam_start/end[%d/%d] n_pings_read[%d]\n",beam_start,beam_end,n_pings_read);

          // test boundaries (zero min)
          beam_start = MAX(beam_start, 0);
          beam_end = MAX(beam_end, 0);

          /* apply decimation - only consider outputting decimated soundings */
            if(mbtrn_cfg->n_output_soundings == 0) {
                mlog_tprintf(mbtrnpp_mlog_id,"e,n_outputsoundings == 0 - invalid\n", beam_start, beam_end, n_pings_read);
            }
          beam_decimation = ((beam_end - beam_start + 1) / mbtrn_cfg->n_output_soundings);
            if(beam_decimation <= 0) {
                beam_decimation = 1;
                static bool warned = false;
                if(!warned)
                mlog_tprintf(mbtrnpp_mlog_id,"e,beam_decimation <= 0 - invalid end[%d] start[%d] using decimation[%d]\n", beam_end, beam_start, beam_decimation);
                warned = true;
            }
          int dj = mbtrn_cfg->median_filter_n_across / 2;
          n_output = 0;
          for (int j = beam_start; j <= beam_end; j++) {

            if (beam_decimation > 0 && (j - beam_start) % beam_decimation == 0) {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                /* apply median filtering to this sounding */
                if (median_filter_n_total > 1) {
                  /* accumulate soundings for median filter */
                  n_median_filter_soundings = 0;
                  int jj0 = MAX(beam_start, j - dj);
                  int jj1 = MIN(beam_end, j + dj);
                  for (int ii = 0; ii < mbtrn_cfg->n_buffer_max; ii++) {
                    for (int jj = jj0; jj <= jj1; jj++) {
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
                  // fprintf(stderr, "Beam %3d of %d:%d bath:%.3f n:%3d:%3d median:%.3f ", j, beam_start,
                  // beam_end, ping[i_ping_process].bath[j], n_median_filter_soundings, median_filter_n_min,
                  // median);

                  /* apply median filter - also flag soundings that don't have enough neighbors to filter */
                  if (n_median_filter_soundings < median_filter_n_min ||
                      fabs(ping[i_ping_process].bath[j] - median) > mbtrn_cfg->median_filter_threshold * median) {
                    ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    n_soundings_flagged++;

                    // fprintf(stderr, "**filtered**");
                  }
                  // fprintf(stderr, "\n");
                }
                if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                  if (n_output < mbtrn_cfg->n_output_soundings) {
                    n_output++;
                  } else {
                    ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    n_soundings_decimated++;
                  }
                }
                else {
                  n_soundings_decimated++;
                }
              }
            }
            else if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
              ping[i_ping_process].beamflag_filter[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
              n_soundings_decimated++;
            }
          }

          /* write out results to stdout as text */
          if ( OUTPUT_FLAG_SET(OUTPUT_MBSYS_STDOUT) ) {
            fprintf(stderr, "Ping: %.9f %.7f %.7f %.3f %.3f %4d\n", ping[i_ping_process].time_d,
                    ping[i_ping_process].navlat, ping[i_ping_process].navlon, ping[i_ping_process].sensordepth,
                    (double)(DTR * ping[i_ping_process].heading), n_output);
            for (int j = 0; j < ping[i_ping_process].beams_bath; j++) {
              if (mb_beam_ok(ping[i_ping_process].beamflag_filter[j])) {
                fprintf(stderr, "%3.3d starboard:%.3f forward:%.3f down:%.3f\n", j,
                        ping[i_ping_process].bathacrosstrack[j], ping[i_ping_process].bathalongtrack[j],
                        ping[i_ping_process].bath[j] - ping[i_ping_process].sensordepth);
                n_soundings_written++;
              }
            }
            n_pings_written++;
          }

          /* pack the data into a TRN MB1 packet and either send it to TRN or write it to a file */
        if (!OUTPUT_FLAGS_ZERO()) {
            n_pings_written++;

            /* make sure buffer is large enough to hold the packet */
            mb1_size = MBTRNPREPROCESS_MB1_HEADER_SIZE + n_output * MBTRNPREPROCESS_MB1_SOUNDING_SIZE +
                       MBTRNPREPROCESS_MB1_CHECKSUM_SIZE;
            if (n_output_buffer_alloc < mb1_size) {
              if ((status = mb_reallocd(mbtrn_cfg->verbose, __FILE__, __LINE__, mb1_size, (void **)&output_buffer, &error)) ==
                  MB_SUCCESS) {
                n_output_buffer_alloc = mb1_size;
              }
              else {
                mb_error(mbtrn_cfg->verbose, error, &message);
                fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
                fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				mlog_tprintf(mbtrnpp_mlog_id,"e,MBIO error allocating data arrays [%s]\n");
                s_mbtrnpp_exit(error);
              }
            }

            // get ping number
            mb_pingnumber(mbtrn_cfg->verbose, imbio_ptr, &ping_number, &error);

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
            mb_put_binary_double(true, ping[i_ping_process].sensordepth, &output_buffer[index]);
            index += 8;
            mb_put_binary_double(true, (double)(DTR * ping[i_ping_process].heading), &output_buffer[index]);
            index += 8;

            mb_put_binary_int(true, ping_number, &output_buffer[index]);
            index += 4;

            mb_put_binary_int(true, n_output, &output_buffer[index]);
            index += 4;

            MX_LPRINT(MBTRNPP, 1,
                     "\nts[%.3lf] beams[%03d] ping[%06u]\nlat[%.4lf] lon[%.4lf] hdg[%6.2lf] sd[%7.2lf]\nv[%+6.2lf] "
                     "p/r/y[%.3lf / %.3lf / %.3lf]\n",
                     ping[i_ping_process].time_d, n_output, ping_number, ping[i_ping_process].navlat,
                     ping[i_ping_process].navlon, (double)(DTR * ping[i_ping_process].heading),
                     ping[i_ping_process].sensordepth, ping[i_ping_process].speed, ping[i_ping_process].pitch,
                     ping[i_ping_process].roll, ping[i_ping_process].heave);

            for (int j = 0; j < ping[i_ping_process].beams_bath; j++) {
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
                mb_put_binary_double(true, (ping[i_ping_process].bath[j] - ping[i_ping_process].sensordepth),
                                     &output_buffer[index]);
                index += 8;

                  MX_LPRINT(MBTRNPP, 2, "n[%03d] atrk/X[%+10.3lf] ctrk/Y[%+10.3lf] dpth/Z[%+10.3lf]\n", j,
                         ping[i_ping_process].bathalongtrack[j], ping[i_ping_process].bathacrosstrack[j],
                         (ping[i_ping_process].bath[j] - ping[i_ping_process].sensordepth));
              }
            }

            /* add the checksum */
            checksum = 0;
            unsigned char *cp = (unsigned char *)output_buffer;
            for (int j = 0; j < index; j++) {
              // checksum += (unsigned int) output_buffer[j];
              checksum += (unsigned int)(*cp++);
            }

            mb_put_binary_int(true, checksum, &output_buffer[index]);
            index += 4;
            MX_LPRINT(MBTRNPP, 3, "mb1 record chk[%08X] idx[%zu] mb1sz[%zu]\n", checksum, index, mb1_size);

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_PING_XT], mtime_dtime());

            /* output MB1, TRN data */
            if ( !OUTPUT_FLAGS_ZERO() ) {

                // begin: move after TRN update for sim sync
//                MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_PROC_MB1_XT], mtime_dtime());
//
//                // do MB1 processing/output
//                mbtrnpp_process_mb1(output_buffer, mb1_size, trn_cfg);
//
//                MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_PROC_MB1_XT], mtime_dtime());
                // end: move after TRN update for sim sync

#ifdef WITH_MBTNAV

                bool update_trn = true;

                // if gain thresholding applied and gain too low, do not process and set reinit flag
                if (mbtrn_cfg->reinit_gain_enable && (transmit_gain < transmit_gain_threshold)) {
                  update_trn = false;
                  if (!reinit_flag) {
                    fprintf(stderr, "--Reinit set due to transmit gain %f < threshold %f\n",
                            transmit_gain, transmit_gain_threshold);
                    mlog_tprintf(mbtrnpp_mlog_id,"i,set reinit due to transmit gain [%.2lf] lower than threshold [%.2lf]\n",
                                  transmit_gain, transmit_gain_threshold);
                    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_GAIN_LO]);
                    reinit_flag = true;
                  }
                }
                // if ok pass filtered ping to TRN for processing
                if (update_trn) {

                  // if reinit_flag set then reinit the TRN filter
                  if (reinit_flag) {
                    reinitialized = true;
                    // TRN reinit function options are:
                    //
                    //   (1) reinit w/ zero offset and default standard deviations
                    //       which correspond to the particle filter distribution widths
                    //   wtnav_reinit_filter(trn_instance, true);
                    //
                    //   (2) Reinit w/ offset set to last good offset estimate and
                    //       default standard deviations
                    //   wtnav_reinit_filter_offset(trn_instance, true, use_offset_n, use_offset_e, use_offset_z);
                    //
                    //   (3) Reinit w/ offset set to last good offset estimate and
                    //       specified standard deviations (here set to default values)
                    //   d_triplet_t xyz_sdev={0., 0., 0.};
                    //   wtnav_get_init_stddev_xyz(trn_instance, &xyz_sdev);
                    //   wtnav_reinit_filter_box(trn_instance, true, use_offset_n, use_offset_e, use_offset_z,
                    //                                              xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);
                    //
                    d_triplet_t xyz_sdev={0., 0., 0.};
                    xyz_sdev.x = MIN((n_reinit_since_use + 1), 10) * mbtrn_cfg->reinit_search_xy;
                    xyz_sdev.y = xyz_sdev.x;
                    xyz_sdev.z = mbtrn_cfg->reinit_search_z;
                    //wtnav_get_init_stddev_xyz(trn_instance, &xyz_sdev);
                    fprintf(stderr, "--reinit time_d:%.6f centered on offset: %f %f %f  sd: %f %f %f\n",
                                  ping[i_ping_process].time_d, use_offset_e, use_offset_n, use_offset_z,
                                  xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);
                    wtnav_reinit_filter_box(trn_instance, true, use_offset_n, use_offset_e, use_offset_z,
                                              xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);

                    mlog_tprintf(mbtrnpp_mlog_id, "i,trn filter reinit time_d:%.6f centered on offset: %f %f %f\n",
                                  ping[i_ping_process].time_d, use_offset_e, use_offset_n, use_offset_z);
                    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_REINIT]);
                    reinit_flag = false;
                    n_reinit++;
                    n_reinit_since_use++;
                    reinit_time = ping[i_ping_process].time_d;
                  }

                  MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_PROC_TRN_XT], mtime_dtime());

                  // do TRN processing, output, and tests for reinitializing TRN
                  mbtrnpp_trn_process_mb1(trn_instance, (mb1_t *)output_buffer, trn_cfg);

                  MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_PROC_TRN_XT], mtime_dtime());

                }

                else {
                    int time_i[7];
                    mb_get_date(0, ping[i_ping_process].time_d, time_i);
                    fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                                    "| %11.6f %11.6f %8.3f | Ping not processed - low gain condition\n",
                    time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], ping[i_ping_process].time_d,
                    ping[i_ping_process].navlon, ping[i_ping_process].navlat, ping[i_ping_process].sensordepth);
                    mbtrnpp_trnu_pubempty_osocket(ping[i_ping_process].time_d, ping[i_ping_process].navlat,
                      ping[i_ping_process].navlon, ping[i_ping_process].sensordepth,trnusvr);
                }

#endif // WITH_MBTNAV

                // begin: move after TRN update for sim sync
                MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_PROC_MB1_XT], mtime_dtime());

                // do MB1 processing/output
                // after TRN processing/update to enable synchronization, e.g. with sim
                // i.e. when MB1 record is published, TRN processing has completed
                mbtrnpp_process_mb1(output_buffer, mb1_size, trn_cfg);

                MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_PROC_MB1_XT], mtime_dtime());
                // end: move after TRN update for sim sync


                MBTRNPP_UPDATE_STATS(app_stats, mbtrnpp_mlog_id, mbtrn_cfg->mbtrnpp_stat_flags);

            } // end MBTRNPREPROCESS_OUTPUT_TRN

            /* write the packet to a file */
            if ( OUTPUT_FLAG_SET(OUTPUT_MB1_FILE_EN) ) {

                if(NULL!=output_mb1_fp && NULL!=output_buffer){
                    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_FWRITE_XT], mtime_dtime());

                    size_t obytes=0;
                    if( (obytes=fwrite(output_buffer, mb1_size, 1, output_mb1_fp))>0){
                        MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_MB_FWRITE_BYTES],mb1_size);
                    } else {
                        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBLOGWR]);
                    }

                    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_FWRITE_XT], mtime_dtime());

                } else {
                    fprintf(stderr,"%s:%d - ERR fwrite failed obuf[%p] fp[%p]\n",__FUNCTION__,__LINE__,output_buffer,output_mb1_fp);
                }
              // fprintf(stderr, "WRITE SIZE: %zu %zu %zu\n", mb1_size, index, index - mb1_size);
            }
          } // else !stdout
        } // data read (ndata == mbtrn_cfg->n_buffer_max)

        /* move data in buffer */
        if (ndata >= mbtrn_cfg->n_buffer_max) {
          ndata--;
          for (int i = 0; i < mbtrn_cfg->n_buffer_max; i++) {
            ping[i].count--;
            if (ping[i].count < 0) {
              idataread = i;
            }
          }
        }
        else {
          idataread++;
          if (idataread >= mbtrn_cfg->n_buffer_max)
            idataread = 0;
        }
      }
      else {

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
          MX_LPRINT(MBTRNPP, 4, "mb_get_all failed: status[%d] kind[%d] err[%d]\n", status, kind, error);

        // deal with fatal error > 0 - this is usually MB_ERROR_EOF
        if ((status == MB_FAILURE) && (error > 0)) {
          if (mbtrn_cfg->input_mode == INPUT_MODE_SOCKET) {

            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBGETALL]);

            fprintf(stderr, "EOF (input socket) - clear status/error\n");
            status = MB_SUCCESS;
            error = MB_ERROR_NO_ERROR;

          }
          else {
            done = true;
            status = MB_SUCCESS;
            error = MB_ERROR_NO_ERROR;
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBFAILURE]);
          }
        } else {
          n_non_survey_data++;
          MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_NONSURVEY]);
          if (n_non_survey_data > 0 && n_non_survey_data % 25 == 0) {
            int time_i[7];
            mb_get_date(0, ping[idataread].time_d, time_i);
            fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                            "| Read 25 non-survey data records...\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], ping[idataread].time_d);

            for (int i = 0; i < MB_DATA_KINDS; i++) {
              if (num_kinds_read[i] > 0) {
                if (mb_notice_message(mbtrn_cfg->verbose, i, &message) == MB_SUCCESS) {
                  fprintf(stderr, "     %6d %s\n", num_kinds_read[i], message);
                  num_kinds_read[i] = 0;
                }
              }
            }
            double dzero = 0.0;
            mbtrnpp_trnu_pubempty_osocket(ping[idataread].time_d, dzero, dzero, dzero, trnusvr);
          }
        }
        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
      }
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_POST_XT], mtime_dtime());
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_POST_XT], mtime_dtime());
    } // while(!done) [main loop]

    /* close the files */
    if (mbtrn_cfg->input_mode == INPUT_MODE_SOCKET) {
      fprintf(stderr, "socket input mode - continue (probably shouldn't be here)\n");
        mlog_tprintf(mbtrnpp_mlog_id,"e,invalid code path - socket input mode\n");
      read_data = true;

      // empty the ring buffer
      ndata = 0;
    }
    else {
      status = mb_close(mbtrn_cfg->verbose, &imbio_ptr, &error);

      // empty the ring buffer
      ndata = 0;

      sprintf(log_message, "Multibeam File <%s> of format <%d> closed", ifile, mbtrn_cfg->format);
      mlog_tprintf(mbtrnpp_mlog_id,"i,closing file/format [%s/%d]\n", ifile, mbtrn_cfg->format);

      if (logfp != NULL) {
        mbtrnpp_postlog(mbtrn_cfg->verbose, logfp, log_message, &error);
        fflush(logfp);
      }
      fprintf(stderr, "%s\n", log_message);

      // force a reinit when data from the next file is opened
      if (mbtrn_cfg->reinit_file_enable && !reinit_flag) {
        fprintf(stderr, "--Reinit set due to closing input swath file\n");
          mlog_tprintf(mbtrnpp_mlog_id,"i,mbtrnpp: set reinit due to closing input swath file [%s]\n", ifile);
        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_EOF]);
        reinit_flag = true;
      }

      /* give the statistics */
      /* figure out whether and what to read next */
      if (read_datalist == true) {
        if ((status = mb_datalist_read(mbtrn_cfg->verbose, datalist, ifile, dfile, &mbtrn_cfg->format, &file_weight, &error)) == MB_SUCCESS) {
          MX_DEBUG("read_datalist status[%d] - continuing\n", status);
          read_data = true;
        }
        else {
          MX_DEBUG("read_datalist status[%d] - done\n", status);
          read_data = false;
        }
      }
      else {
       MX_MMSG(MXDEBUG, "read_datalist == NO\n");
        read_data = false;
      }
      mlog_tprintf(mbtrnpp_mlog_id,"i,read_datalist[%s] read_data[%s] status[%d] ifile[%s] dfile[%s] error[%d]\n",
                     (read_datalist?"Y":"N"),(read_data?"Y":"N"),status,ifile,dfile,error );

    }
    /* end loop over files in list */
  }

  fprintf(stderr, "\nDone reading data\n");
  mlog_tprintf(mbtrnpp_mlog_id,"i,closing data list - OK\n");
  if (read_datalist == true) {
    mb_datalist_close(mbtrn_cfg->verbose, &datalist, &error);
    fprintf(stderr, "Closed input datalist\n");
  }

  /* close log file */
  gettimeofday(&timeofday, &timezone);
//  now_time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  // fprintf(stderr,"CHECKING AT BOTTOM OF LOOP: logfp:%p log_file_open_time_d:%.6f now_time_d:%.6f\n", logfp,
  // log_file_open_time_d, now_time_d);
  if (logfp != NULL) {
    status = mbtrnpp_logstatistics(mbtrn_cfg->verbose, logfp, n_pings_read, n_soundings_read, n_soundings_valid_read,
                                   n_soundings_flagged_read, n_soundings_null_read, n_pings_written, n_soundings_trimmed,
                                   n_soundings_decimated, n_soundings_flagged, n_soundings_written, &error);
    n_tot_pings_read += n_pings_read;
    n_tot_soundings_read += n_soundings_read;
    n_tot_soundings_valid_read += n_soundings_valid_read;
    n_tot_soundings_flagged_read += n_soundings_flagged_read;
    n_tot_soundings_null_read += n_soundings_null_read;
    n_tot_pings_written += n_pings_written;
    n_tot_soundings_trimmed += n_soundings_trimmed;
    n_tot_soundings_decimated += n_soundings_decimated;
    n_tot_soundings_flagged += n_soundings_flagged;
    n_tot_soundings_written += n_soundings_written;
    n_pings_read = 0;
    n_soundings_read = 0;
    n_soundings_valid_read = 0;
    n_soundings_flagged_read = 0;
    n_soundings_null_read = 0;
    n_pings_written = 0;
    n_soundings_trimmed = 0;
    n_soundings_decimated = 0;
    n_soundings_flagged = 0;
    n_soundings_written = 0;

    status = mbtrnpp_closelog(mbtrn_cfg->verbose, &logfp, &error);
  }

  /* close output */
  if ( OUTPUT_FLAG_SET(OUTPUT_MB1_FILE_EN) ) {
    fclose(output_mb1_fp);
    mb_freed(mbtrn_cfg->verbose, __FILE__, __LINE__, (void **)&output_buffer, &error);
  }

  /* close output */
#ifdef WITH_MBTNAV
  if (output_trn_fp != NULL)
    fclose(output_trn_fp);
#endif

  /* deallocate arrays allocated with mb_mallocd() */
  if (median_filter_soundings != NULL) {
    mb_freed(mbtrn_cfg->verbose, __FILE__, __LINE__, (void **)&median_filter_soundings, &error);
  }
  if (tide_time_d != NULL) {
    mb_freed(mbtrn_cfg->verbose, __FILE__, __LINE__, (void **)&tide_time_d, &error);
  }
  if (tide_tide != NULL) {
    mb_freed(mbtrn_cfg->verbose, __FILE__, __LINE__, (void **)&tide_tide, &error);
  }

  // release the config strings
  MEM_CHKINVALIDATE(mbtrn_cfg->trn_map_file);
  MEM_CHKINVALIDATE(mbtrn_cfg->trn_cfg_file);
  MEM_CHKINVALIDATE(mbtrn_cfg->trn_particles_file);
  MEM_CHKINVALIDATE(mbtrn_cfg->trn_mission_id);

  /* check memory */
  //if (mbtrn_cfg->verbose >= 4)
    status = mb_memory_list(mbtrn_cfg->verbose, &error);

  /* give the statistics */
  if (mbtrn_cfg->verbose >= 1) {
  }

    mlog_tprintf(mbtrnpp_mlog_id, "uptime,%0.3lf\n", app_stats->uptime);
    mlog_tprintf(mbtrnpp_mlog_id,"i,end session\n");
    mlog_tprintf(netif_log(mb1svr),"i,end session\n");
    mlog_tprintf(netif_log(trnsvr),"i,end session\n");
    mlog_tprintf(netif_log(trnusvr),"i,end session\n");
    mlog_tprintf(netif_log(trnumsvr),"i,end session\n");

  fprintf(stderr, "\nExiting program - error mode:[%d]\n", error);

    /* end it all */
  s_mbtrnpp_exit(error);
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
  mb_path log_file;
  char log_message[LOG_MSG_BUF_SZ]={0};

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
  char user[256], host[256], date[32];
  status = mb_user_host_date(verbose, user, host, date, error);
  gettimeofday(&timeofday, &timezone);
  time_d = timeofday.tv_sec + 0.000001 * timeofday.tv_usec;
  status = mb_get_date(verbose, time_d, time_i);
  sprintf(date, "%4.4d%2.2d%2.2d_%2.2d%2.2d%2.2d%6.6d", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
          time_i[6]);

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
      char *log_message = "Closing mbtrnpp log file";
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
                          int n_output_soundings, bool median_filter_en, int median_filter_n_across, int median_filter_n_along,
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
    fprintf(stderr, "dbg2       median_filter_en:             %d\n", (median_filter_en?1:0));
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

    sprintf(log_message, "       median_filter_en:         %d", (median_filter_en?1:0));
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
                          int n_soundings_flagged_read, int n_soundings_null_read, int n_pings_written, int n_soundings_trimmed,
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
    fprintf(stderr, "dbg2       n_pings_written:              %d\n", n_pings_written);
    fprintf(stderr, "dbg2       n_soundings_trimmed:          %d\n", n_soundings_trimmed);
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

    sprintf(log_message, "       n_pings_written:              %d", n_pings_written);
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
      double stats_now = mtime_etime();
      double stats_nowd = mtime_dtime();

    if (log_clock_res) {
      // log the timing clock resolution (once)
      struct timespec res;
      clock_getres(CLOCK_MONOTONIC, &res);
      mlog_tprintf(mbtrnpp_mlog_id, "%.3lf,i,clkres_mono,s[%ld] ns[%ld]\n", stats_now, res.tv_sec, res.tv_nsec);
      log_clock_res = false;
    }

    // we can only measure the previous stats cycle...
    if (stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].n > 0) {
      // get the timing of the last cycle
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_STATS_XT], stats_prev_start);
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_STATS_XT], stats_prev_end);
    }
    else {
      // seed the first cycle
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_STATS_XT], (stats_nowd - 0.0001));
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_STATS_XT], stats_nowd);
    }

    // end the cycle timer here
    // [start at the end if this function]
    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT], stats_nowd);

    // measure dtime execution time (twice), while we're at it
    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_DTIME_XT], mtime_dtime());
    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_DTIME_XT], mtime_dtime());
    MST_METRIC_DIV(app_stats->stats->metrics[MBTPP_CH_MB_DTIME_XT], 2.0);

    // update uptime
    stats->uptime = stats_now - stats->session_start;

      MX_LPRINT(MBTRNPP, 4, "cycle_xt: stat_now[%.4lf] stat_nowd[%.4lf] start[%.4lf] stop[%.4lf] value[%.4lf]\n", stats_now,stats_nowd,
             app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT].start, app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT].stop,
             app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT].value);

    // update stats
    mstats_update_stats(stats->stats, MBTPP_CH_COUNT, flags);
    mstats_t *mb1svr_stats = netif_stats(mb1svr);
    mstats_update_stats(mb1svr_stats, NETIF_CH_COUNT, flags);
    mstats_t *trnsvr_stats = netif_stats(trnsvr);
    mstats_update_stats(trnsvr_stats, NETIF_CH_COUNT, flags);
    mstats_t *trnusvr_stats = netif_stats(trnusvr);
    mstats_update_stats(trnusvr_stats, NETIF_CH_COUNT, flags);
    mstats_t *trnumsvr_stats = netif_stats(trnumsvr);
    mstats_update_stats(trnumsvr_stats, NETIF_CH_COUNT, flags);

      MX_LPRINT(MBTRNPP, 4, "cycle_xt.p: N[%"PRId64"] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
             app_stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].n, app_stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].sum,
             app_stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].min, app_stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].max,
             app_stats->stats->per_stats[MBTPP_CH_MB_CYCLE_XT].avg);

      MX_LPRINT(MBTRNPP, 4, "cycle_xt.a: N[%"PRId64"] sum[%.3lf] min[%.3lf] max[%.3lf] avg[%.3lf]\n",
             app_stats->stats->agg_stats[MBTPP_CH_MB_CYCLE_XT].n, app_stats->stats->agg_stats[MBTPP_CH_MB_CYCLE_XT].sum,
             app_stats->stats->agg_stats[MBTPP_CH_MB_CYCLE_XT].min, app_stats->stats->agg_stats[MBTPP_CH_MB_CYCLE_XT].max,
             app_stats->stats->agg_stats[MBTPP_CH_MB_CYCLE_XT].avg);

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
      MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_LOG_XT], mtime_dtime());

      mlog_tprintf(mbtrnpp_mlog_id, "%.3lf,i,uptime,%0.3lf\n", stats_now, stats->uptime);
      mstats_log_stats(stats->stats, stats_now, log_id, flags);
      mstats_log_stats(mb1svr_stats, stats_now, netif_log(mb1svr), flags);
      mstats_log_stats(trnsvr_stats, stats_now, netif_log(trnsvr), flags);
      mstats_log_stats(trnusvr_stats, stats_now, netif_log(trnusvr), flags);
      mstats_log_stats(trnumsvr_stats, stats_now, netif_log(trnumsvr), flags);

      if (flags & MSF_READER) {
        mstats_log_stats(reader_stats, stats_now, log_id, flags);
      }

      // reset period stats
      mstats_reset_pstats(stats->stats, MBTPP_CH_COUNT);
      mstats_reset_pstats(reader_stats, R7KR_MET_COUNT);
      mstats_reset_pstats(mb1svr_stats, NETIF_CH_COUNT);
      mstats_reset_pstats(trnsvr_stats, NETIF_CH_COUNT);
      mstats_reset_pstats(trnusvr_stats, NETIF_CH_COUNT);
      mstats_reset_pstats(trnumsvr_stats, NETIF_CH_COUNT);

      // reset period timer
      stats->stats->stat_period_start = stats_now;

      // stop log execution timer
      MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_LOG_XT], mtime_dtime());
    }

    // start cycle timer
    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_CYCLE_XT], mtime_dtime());

    // update stats execution time variables
    stats_prev_start = stats_nowd;
    stats_prev_end = mtime_dtime();
  }
  else {
    fprintf(stderr, "mbtrnpp_update_stats: invalid argument\n");
  }
  return 0;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_init_debug(int verbose) {

    // enable MXERROR by default
    mxd_setModule(MXINFO, 0, true, "info");
    mxd_setModule(MXERROR, 1, false, "err");
    mxd_setModule(MXDEBUG, 0, true, "debug");
    mxd_setModule(MXWARN, 0, true, "warn");

    mxd_setModule(MBTRNPP, 0, true, "mbtrn");
    mxd_setModule(R7KR, 0, true, "r7kr");
    mxd_setModule(R7KR_DEBUG, 0, true, "r7kr.debug");
    mxd_setModule(R7KR_ERROR, 0, true, "r7kr.err");
    mxd_setModule(R7KC, 0, true, "r7kc");
    mxd_setModule(R7KC_DEBUG, 0, true, "r7kc.debug");
    mxd_setModule(R7KC_ERROR, 0, true, "r7kc.err");
    mxd_setModule(R7KC_PARSER, 0, true, "r7kc.parser");
    mxd_setModule(R7KC_DRFCON, 0, true, "r7kc.drfcon");
    mxd_setModule(MB1R, 0, true, "mb1r");
    mxd_setModule(MB1R_DEBUG, 0, true, "mb1r.debug");
    mxd_setModule(MB1R_ERROR, 0, true, "mb1r.err");

    switch (verbose) {
        case 0:
            break;
        case 1:
            mxd_setModule(MBTRNPP, 1, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            break;
        case 2:
            mxd_setModule(MBTRNPP, 2, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(R7KC_PARSER, 5, false, NULL);
            break;
        case -1:
            mxd_setModule(MBTRNPP, 1, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(NETIF, 2, false, NULL);
            break;
        case -2:
            mxd_setModule(MBTRNPP, 2, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(NETIF, 3, false, NULL);
            break;
        case -3:
            mxd_setModule(MBTRNPP, 3, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R_ERROR, 5, false, NULL);
            mxd_setModule(R7KC_PARSER, 5, false, NULL);
            mxd_setModule(NETIF, 4, false, NULL);
            break;
        case -4:
            mxd_setModule(MBTRNPP, 4, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MBTRNPP, 5, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R_ERROR, 5, false, NULL);
            mxd_setModule(R7KC_PARSER, 5, false, NULL);
            mxd_setModule(R7KC_DRFCON, 5, false, NULL);
            mxd_setModule(NETIF, 5, false, NULL);
            mxd_setModule(MXMSOCK, 5, false, NULL);
            break;
        case -5:
            mxd_setModule(MBTRNPP, 5, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(MBTRNPP, 5, false, NULL);
            mxd_setModule(MXWARN, 5, false, NULL);
            mxd_setModule(MXDEBUG, 5, false, NULL);
            mxd_setModule(R7KR, 5, false, NULL);
            mxd_setModule(R7KR_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R, 5, false, NULL);
            mxd_setModule(MB1R_DEBUG, 5, false, NULL);
            mxd_setModule(MB1R_ERROR, 5, false, NULL);
            mxd_setModule(R7KC_PARSER, 5, false, NULL);
            mxd_setModule(R7KC_DRFCON, 5, false, NULL);
            mxd_setModule(NETIF, 5, false, NULL);
            mxd_setModule(MXMSOCK, 5, false, NULL);
            break;
        default:
            mxd_setModule(MXWARN, 5, false, NULL);
            break;
    }

    if(verbose < 0){
        fprintf(stderr, "%s:%d verbose[%d]\n", __func__, __LINE__, verbose);
        mxd_show();
    }

    // open mb1 data log
    if ( OUTPUT_FLAG_SET(OUTPUT_MB1_BIN) ) {
        mb1_blog_path = (char *)malloc(512);
        sprintf(mb1_blog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, MB1_BLOG_NAME,
                s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);
        mb1_blog_id = mlog_get_instance(mb1_blog_path, &mb1_blog_conf, MB1_BLOG_NAME);
        fprintf(stderr,"MB1 binary log [%s]\n",mb1_blog_path);
        mlog_show(mb1_blog_id, true, 5);
        mlog_open(mb1_blog_id, flags, mode);
    }

    // open trn message log
    if (OUTPUT_FLAG_SET(OUTPUT_MBTRNPP_MSG) ) {
        mbtrnpp_mlog_path = (char *)malloc(512);
        sprintf(mbtrnpp_mlog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, MBTRNPP_MLOG_NAME, s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);
        mbtrnpp_mlog_id = mlog_get_instance(mbtrnpp_mlog_path, &mbtrnpp_mlog_conf, MBTRNPP_MLOG_NAME);
        fprintf(stderr,"mbtrnpp message log [%s]\n",mbtrnpp_mlog_path);
        mlog_show(mbtrnpp_mlog_id, true, 5);
        mlog_open(mbtrnpp_mlog_id, flags, mode);
        mlog_tprintf(mbtrnpp_mlog_id, "*** mbtrn session start ***\n");
        mlog_tprintf(mbtrnpp_mlog_id, "cmdline [%s]\n", s_mbtrnpp_cmdline_str(NULL, 0, 0, NULL, RF_NONE));
        mlog_tprintf(mbtrnpp_mlog_id, "r7kr v[%s] build[%s]\n", R7KR_VERSION_STR, LIBMFRAME_BUILD);
    } else {
        // put to stderr if log disabled
        fprintf(stderr, "*** mbtrn session start ***\n");
        fprintf(stderr, "cmdline [%s]\n", s_mbtrnpp_cmdline_str(NULL, 0, 0, NULL, RF_NONE));
    }

    // open trn message log
    if (OUTPUT_FLAG_SET(OUTPUT_TRNU_ASC) ) {
        trnu_alog_path = (char *)malloc(512);
        sprintf(trnu_alog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, TRNU_ALOG_NAME, s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);
        trnu_alog_id = mlog_get_instance(trnu_alog_path, &trnu_alog_conf, TRNU_ALOG_NAME);
        fprintf(stderr,"trn update log [%s]\n",trnu_alog_path);
        mlog_show(trnu_alog_id, true, 5);
        mlog_open(trnu_alog_id, flags, mode);
        mlog_tprintf(trnu_alog_id, "*** trn update session start ***\n");
        mlog_tprintf(trnu_alog_id, "cmdline [%s]\n", s_mbtrnpp_cmdline_str(NULL, 0, 0, NULL, RF_NONE));
        mlog_tprintf(trnu_alog_id, "r7kr v[%s] build[%s]\n", R7KR_VERSION_STR, LIBMFRAME_BUILD);
    }

    if ( OUTPUT_FLAG_SET(OUTPUT_TRNU_BIN) ) {
        trnu_blog_path = (char *)malloc(512);
        sprintf(trnu_blog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, TRNU_BLOG_NAME,
                s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);
        trnu_blog_id = mlog_get_instance(trnu_blog_path, &trnu_blog_conf, TRNU_BLOG_NAME);
        fprintf(stderr,"TRNU binary log [%s]\n",trnu_blog_path);
        mlog_show(trnu_blog_id, true, 5);
        mlog_open(trnu_blog_id, flags, mode);
    }

    app_stats = mstats_profile_new(MBTPP_EV_COUNT, MBTPP_STA_COUNT, MBTPP_CH_COUNT, mbtrnpp_stats_labels, mtime_dtime(),
                                   mbtrn_cfg->trn_status_interval_sec);

    return 0;
}
/*--------------------------------------------------------------------*/


#ifdef WITH_MBTNAV

char *mbtrnpp_trn_updatestr(char *dest, int len, trn_update_t *update, int indent)

{
    if(NULL!=dest && NULL!=update){
//        char *cp=dest;
        snprintf(dest,len-1,"%*sMLE: %.2lf,%.4lf,%.4lf,%.4lf\n%*sMSE: %.2lf,%.4lf,%.4lf,%.4lf\n%*sCOV: %.2lf,%.2lf,%.2lf\n%*s RI: %d filter_state: %d success: %d cycle: %d ping: %d mb1_time: %0.3lf update_time: %0.3lf isconv:%hd isvalid:%hd\n",
                 indent,"",
                 update->mle_dat->time,
                 (update->mle_dat->x-update->pt_dat->x),
                 (update->mle_dat->y-update->pt_dat->y),
                 (update->mle_dat->z-update->pt_dat->z),
                 indent,"",
                 update->mse_dat->time,
                 (update->mse_dat->x-update->pt_dat->x),
                 (update->mse_dat->y-update->pt_dat->y),
                 (update->mse_dat->z-update->pt_dat->z),
                 indent,"",
                 sqrt(update->mse_dat->covariance[0]),
                 sqrt(update->mse_dat->covariance[2]),
                 sqrt(update->mse_dat->covariance[5]),
                 indent,"",
                 update->reinit_count,
                 update->filter_state,
                 update->success,
                 update->mb1_cycle,
                 update->ping_number,
                 update->mb1_time,
                 update->update_time,
                 update->is_converged,
                 update->is_valid
                 );
    }
    return dest;
}
/*--------------------------------------------------------------------*/

int mbtrnpp_trn_pub_ostream(trn_update_t *update,
                             FILE *stream)
{
    int retval=-1;


    if(NULL!=update->mse_dat && NULL!=update->pt_dat && NULL!=update->mle_dat){
        char str[256]={0};
        fprintf(stream,"\nTRN Update:\n%s", mbtrnpp_trn_updatestr(str,256,update,0));
        retval=0;
    }


    return retval;
}

int mbtrnpp_trn_pub_odebug(trn_update_t *update)
{
    int retval=-1;


    if(NULL!=update->mse_dat && NULL!=update->pt_dat && NULL!=update->mle_dat){
        char str[256]={0};


        MX_LPRINT(MBTRNPP, 1, "\nTRN Update:\n%s", mbtrnpp_trn_updatestr(str,256,update,0));
        MX_DEBUG("\nTRN Update:\n%s", mbtrnpp_trn_updatestr(str,256,update,0));
        retval=0;
    }


    return retval;
}

int mbtrnpp_trn_pub_olog(trn_update_t *update,
                          mlog_id_t log_id)
{
    int retval=-1;
    if(NULL!=update){
        if(NULL!=update->pt_dat)
            retval=0;
        mlog_tprintf(log_id,"trn_pt_dat,%lf,%.4lf,%.4lf,%.4lf\n",
                     update->pt_dat->time,
                     update->pt_dat->x,
                     update->pt_dat->y,
                     update->pt_dat->z);

        if(NULL!=update->mle_dat)
            mlog_tprintf(log_id,"trn_mle_dat,%lf,%.4lf,%.4lf,%.4lf\n",
                         update->mle_dat->time,
                         update->mle_dat->x,
                         update->mle_dat->y,
                         update->mle_dat->z);

        if(NULL!=update->mse_dat)
            mlog_tprintf(log_id,"trn_mse_dat,%lf,%.4lf,%.4lf,%.4lf,%.4lf,%.4lf,%.4lf,%.4lf\n",
                         update->mse_dat->time,
                         update->mse_dat->x,
                         update->mse_dat->y,
                         update->mse_dat->z,
                         update->mse_dat->covariance[0],
                         update->mse_dat->covariance[2],
                         update->mse_dat->covariance[5],
                         update->mse_dat->covariance[1]);

        if(NULL!=update->mse_dat && NULL!=update->pt_dat && NULL!=update->mle_dat)
            mlog_tprintf(log_id,"trn_est,%lf,%.4lf,%.4lf,%.4lf,%.4lf,%.4lf,%.4lf,%.2lf,%.2lf,%.2lf\n",
                         update->mse_dat->time,
                         (update->mle_dat->x-update->pt_dat->x),
                         (update->mle_dat->y-update->pt_dat->y),
                         (update->mle_dat->z-update->pt_dat->z),
                         (update->mse_dat->x-update->pt_dat->x),
                         (update->mse_dat->y-update->pt_dat->y),
                         (update->mse_dat->z-update->pt_dat->z),
                         sqrt(update->mse_dat->covariance[0]),
                         sqrt(update->mse_dat->covariance[2]),
                         sqrt(update->mse_dat->covariance[5]));
        mlog_tprintf(log_id,"trn_state,reinit_flag,%d,fstate,%d,success,%d,cycle,%d,ping,%d,mb1_time,%0.3lf,update_time,%0.3lf,isconv,%hd,isval,%hd\n",update->reinit_count,update->filter_state,update->success,update->mb1_cycle,update->ping_number,update->mb1_time,update->update_time,update->is_converged,update->is_valid);
    }

    return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_pub_blog(trn_update_t *update,
                             mlog_id_t log_id)
{
    int retval=-1;

    if(NULL!=update && log_id!=MLOG_ID_INVALID){
        retval=0;

        int iobytes=0;
        double offset_n = update->mse_dat->x - update->pt_dat->x;
        double offset_e = update->mse_dat->y - update->pt_dat->y;
        double offset_z = update->mse_dat->z - update->pt_dat->z;
        // serialize data
        trnu_pub_t pub_data={
            TRNU_PUB_SYNC,
            {
                {update->pt_dat->time,update->pt_dat->x,update->pt_dat->y,update->pt_dat->z,
                    {update->pt_dat->covariance[0],update->pt_dat->covariance[2],update->pt_dat->covariance[5],update->pt_dat->covariance[1]}
                },
                {update->mle_dat->time,update->mle_dat->x,update->mle_dat->y,update->mle_dat->z,
                    {update->mle_dat->covariance[0],update->mle_dat->covariance[2],update->mle_dat->covariance[5],update->mle_dat->covariance[1]}
                },
                {update->mse_dat->time,update->mse_dat->x,update->mse_dat->y,update->mse_dat->z,
                    {update->mse_dat->covariance[0],update->mse_dat->covariance[2],update->mse_dat->covariance[5],update->mse_dat->covariance[1]}
                },
                {update->mse_dat->time,offset_n,offset_e,offset_z,
                    {update->mse_dat->covariance[0],update->mse_dat->covariance[2],update->mse_dat->covariance[5],update->mse_dat->covariance[1]}
                },
                {use_offset_time,use_offset_n,use_offset_e,use_offset_z,
                    {use_covariance[0],use_covariance[1],use_covariance[2],use_covariance[3]}
                },
            },
            update->reinit_count,
            update->reinit_tlast,
            update->filter_state,
            update->success,
            update->is_converged,
            update->is_valid,
            update->mb1_cycle,
            update->ping_number,
            n_converged_streak,
            n_converged_tot,
            n_unconverged_streak,
            n_unconverged_tot,
            update->mb1_time,
            reinit_time,
            update->update_time
        };

        if( (iobytes=mlog_write(log_id,(byte *)&pub_data, sizeof(pub_data)))>0){
            retval=iobytes;
        }

    }
    return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trnu_pub_osocket(trn_update_t *update,
                             netif_t *netif)
{
    int retval=-1;

    if(NULL!=update && NULL!=netif){
        retval=0;

        double offset_n = update->mse_dat->x - update->pt_dat->x;
        double offset_e = update->mse_dat->y - update->pt_dat->y;
        double offset_z = update->mse_dat->z - update->pt_dat->z;
//        double covariance_mag = sqrt(update->mse_dat->covariance[0] * update->mse_dat->covariance[0]
//                                     + update->mse_dat->covariance[1] * update->mse_dat->covariance[1]
//                                     + update->mse_dat->covariance[2] * update->mse_dat->covariance[2]);

        // serialize data
        trnu_pub_t pub_data={
            TRNU_PUB_SYNC,
            {
                {update->pt_dat->time,update->pt_dat->x,update->pt_dat->y,update->pt_dat->z,
                    {update->pt_dat->covariance[0],update->pt_dat->covariance[2],update->pt_dat->covariance[5],update->pt_dat->covariance[1]}
                },
                {update->mle_dat->time,update->mle_dat->x,update->mle_dat->y,update->mle_dat->z,
                    {update->mle_dat->covariance[0],update->mle_dat->covariance[2],update->mle_dat->covariance[5],update->mle_dat->covariance[1]}
                },
                {update->mse_dat->time,update->mse_dat->x,update->mse_dat->y,update->mse_dat->z,
                    {update->mse_dat->covariance[0],update->mse_dat->covariance[2],update->mse_dat->covariance[5],update->mse_dat->covariance[1]}
                },
                {update->mse_dat->time,offset_n,offset_e,offset_z,
                    {update->mse_dat->covariance[0],update->mse_dat->covariance[2],update->mse_dat->covariance[5],update->mse_dat->covariance[1]}
                },
                {use_offset_time,use_offset_n,use_offset_e,use_offset_z,
                    {use_covariance[0],use_covariance[1],use_covariance[2],use_covariance[3]}
                },
            },
            update->reinit_count,
            update->reinit_tlast,
            update->filter_state,
            update->success,
            update->is_converged,
            update->is_valid,
            update->mb1_cycle,
            update->ping_number,
            n_converged_streak,
            n_converged_tot,
            n_unconverged_streak,
            n_unconverged_tot,
            update->mb1_time,
            reinit_time,
            update->update_time
        };

        size_t iobytes = 0;
        if( netif_pub(netif,(char *)&pub_data, sizeof(pub_data), &iobytes) == 0){
            retval = iobytes;
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_TRNU_PUBN]);
        } else {
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ETRNUPUB]);
        }

    }
    return retval;
}

int mbtrnpp_trnu_pubempty_osocket(double time, double lat, double lon, double depth, netif_t *netif)
{
    int retval=-1;

    if(NULL!=netif){
        retval=0;

            int izero = 0;
            short int szero = 0;
            double dzero = 0.0;

            // serialize data
            trnu_pub_t pub_data={
                TRNU_PUB_SYNC,
                {
                    {time, lat, lon, depth,
                        {dzero, dzero, dzero, dzero}
                    },
                    {dzero, dzero, dzero, dzero,
                        {dzero, dzero, dzero, dzero}
                    },
                    {dzero, dzero, dzero, dzero,
                        {dzero, dzero, dzero, dzero}
                    },
                    {dzero, dzero, dzero, dzero,
                        {dzero, dzero, dzero, dzero}
                    },
                    {use_offset_time,use_offset_n,use_offset_e,use_offset_z,
                        {use_covariance[0],use_covariance[1],use_covariance[2],use_covariance[3]}
                    },
                },
                izero,
                dzero,
                izero,
                izero,
                szero,
                szero,
                izero,
                izero,
                izero,
                izero,
                izero,
                izero,
                dzero,
                dzero,
                dzero,
            };

            size_t iobytes = 0;
            if( netif_pub(netif,(char *)&pub_data, sizeof(pub_data), &iobytes) == 0){
                retval=iobytes;
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_TRNU_PUBEMPTYN]);
            } else {
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_ETRNUPUBEMPTY]);
            }
    }
    return retval;
}

int mbtrnpp_init_trn(wtnav_t **pdest, int verbose, trn_config_t *cfg)
{
    int retval = -1;

    if (NULL != cfg && NULL!=pdest) {
        wtnav_t *instance = wtnav_new(cfg);
        if (NULL!=instance) {
            if (wtnav_initialized(instance)) {
                *pdest = instance;
                retval = 0;
                fprintf(stderr, "%s : TRN initialize - OK\n",__FUNCTION__);
            }
            else {
                fprintf(stderr, "%s : ERR - TRN wtnav initialization failed\n",__FUNCTION__);
                wtnav_destroy(instance);
            }
        }
        else {
            fprintf(stderr, "%s : ERR - TRN new failed\n",__FUNCTION__);
        }
    }
    else {
        fprintf(stderr, "%s : ERR - TRN config NULL\n",__FUNCTION__);
    }

    return retval;
}

int mbtrnpp_init_trnsvr(netif_t **psvr, wtnav_t *trn, char *host, int port, bool verbose)
{
    int retval = -1;

    MX_DEBUG("configuring trn server socket using %s:%d\n", host, port);
    if(NULL!=psvr && NULL!=host){
        netif_t *svr  = netif_new("trnsvr",host,
                          port,
                          ST_TCP,
                          IFM_REQRES,
                          mbtrn_cfg->trnsvr_hbto,
                          trnif_msg_read_ct,
                          trnif_msg_handle_ct,
                          NULL);

        if(NULL!=svr){
            *psvr = svr;
            netif_set_reqres_res(svr,trn);
            fprintf(stderr,"trnsvr netif:\n");
            netif_show(svr,true,5);
            netif_init_log(svr, "trnsvr", (NULL!=mbtrn_cfg->trn_log_dir?mbtrn_cfg->trn_log_dir:"."), s_mbtrnpp_session_str(NULL,0,RF_NONE));
            mlog_tprintf(svr->mlog_id,"*** trnsvr session start (TEST) ***\n");
            mlog_tprintf(svr->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());
            retval = netif_connect(svr);
        } else {
            fprintf(stderr,"%s:%d - ERR allocation\n",__FUNCTION__,__LINE__);
        }
    } else {
        fprintf(stderr,"%s:%d - ERR invalid args\n",__FUNCTION__,__LINE__);
    }
    return retval;
}
/*--------------------------------------------------------------------*/
int mbtrnpp_init_mb1svr(netif_t **psvr, char *host, int port, bool verbose)
{
    int retval = -1;
   if(NULL!=psvr && NULL!=host){
       MX_DEBUG("configuring MB1 server socket using %s:%d\n", host, port);
       fprintf(stderr,"configuring MB1 server socket using %s:%d hbto[%lf]\n",host,port,mbtrn_cfg->mbsvr_hbto);
        netif_t *svr = netif_new("mb1svr",host,
                          port,
                          ST_UDP,
                          IFM_REQRES,
                          mbtrn_cfg->mbsvr_hbto,
                          trnif_msg_read_mb,
                          trnif_msg_handle_mb,
                          trnif_msg_pub_mb);

        if(NULL!=svr){
            *psvr = svr;
//            netif_set_reqres_res(svr,trn);
            fprintf(stderr,"mb1svr netif:\n");
            netif_show(svr,true,5);
            netif_init_log(svr, "mb1svr", (NULL!=mbtrn_cfg->trn_log_dir?mbtrn_cfg->trn_log_dir:"."), s_mbtrnpp_session_str(NULL,0,RF_NONE));
            mlog_tprintf(svr->mlog_id,"*** mb1svr session start (TEST) ***\n");
            mlog_tprintf(svr->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());
            retval = netif_connect(svr);
        } else {
            fprintf(stderr,"%s:%d - ERR allocation\n",__FUNCTION__,__LINE__);
        }
   } else {
       fprintf(stderr,"%s:%d - ERR invalid args\n",__FUNCTION__,__LINE__);
   }
    return retval;
}
/*--------------------------------------------------------------------*/

int s_mbtrnpp_trnu_reset_callback()
{
    int retval=0;
    int reinits_pre=wtnav_get_num_reinits(trn_instance);

    double reset_time = mtime_etime();

    // TRN reinit function options are:
    //
    //   (1) reinit w/ zero offset and default standard deviations
    //       which correspond to the particle filter distribution widths
    //   wtnav_reinit_filter(trn_instance, true);
    //
    //   (2) Reinit w/ offset set to last good offset estimate and
    //       default standard deviations
    //   wtnav_reinit_filter_offset(trn_instance, true, use_offset_n, use_offset_e, use_offset_z);
    //
    //   (3) Reinit w/ offset set to last good offset estimate and
    //       specified standard deviations (here set to default values)
    //   d_triplet_t xyz_sdev={0., 0., 0.};
    //   wtnav_get_init_stddev_xyz(trn_instance, &xyz_sdev);
    //   wtnav_reinit_filter_box(trn_instance, true, use_offset_n, use_offset_e, use_offset_z,
    //                                              xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);
    //
    d_triplet_t xyz_sdev={0., 0., 0.};
    xyz_sdev.x = MIN((n_reinit_since_use + 1), 10) * mbtrn_cfg->reinit_search_xy;
    xyz_sdev.y = xyz_sdev.x;
    xyz_sdev.z = mbtrn_cfg->reinit_search_z;
    // wtnav_get_init_stddev_xyz(trn_instance, &xyz_sdev);
    fprintf(stderr, "--reinit (cli_req) systime:%.6f centered on offset: %f %f %f  sd: %f %f %f\n",
                  reset_time, use_offset_e, use_offset_n, use_offset_z,
                  xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);
    wtnav_reinit_filter_box(trn_instance, true, use_offset_n, use_offset_e, use_offset_z,
                              xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);

    mlog_tprintf(mbtrnpp_mlog_id, "i,trn filter reinit.cli systime:%.6f centered on offset: %f %f %f  sd: %f %f %f\n",
                 reset_time, use_offset_e, use_offset_n, use_offset_z,
                 xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);

    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_REINIT]);
    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_TRNUCLI_RESET]);

//    reinit_flag = false;
    n_reinit++;
    n_reinit_since_use++;
    reinit_time = reset_time;

    int reinit_post=wtnav_get_num_reinits(trn_instance);

    if(reinit_post<=reinits_pre){
        retval=-1;
    }

//    fprintf(stderr,"%s:%d - TRNU CLIENT REINIT REQ reinit pre/post[%d/%d]\n",__func__,__LINE__,reinits_pre,reinit_post);

    return retval;
}

int s_mbtrnpp_trnu_reset_ofs_callback(double ofs_x, double ofs_y, double ofs_z)
{
    int retval=0;
    int reinits_pre=wtnav_get_num_reinits(trn_instance);

    double reset_time = mtime_etime();

    d_triplet_t xyz_sdev={0., 0., 0.};
    xyz_sdev.x = mbtrn_cfg->reinit_search_xy;
    xyz_sdev.y = xyz_sdev.x;
    xyz_sdev.z = mbtrn_cfg->reinit_search_z;
    fprintf(stderr, "--reinit_ofs (cli_req) systime:%.6f centered on offset: %f %f %f  sd: %f %f %f\n",
                  reset_time, use_offset_e, use_offset_n, use_offset_z,
                  xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);
    wtnav_reinit_filter_box(trn_instance, true, ofs_x, ofs_y, ofs_z,
                              xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);

    mlog_tprintf(mbtrnpp_mlog_id, "i,trn filter reinit_ofs.cli systime:%.6f centered on offset: %f %f %f  sd: %f %f %f\n",
                 reset_time, ofs_x, ofs_y, ofs_z,
                 xyz_sdev.x, xyz_sdev.y, xyz_sdev.z);

    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_REINIT]);
    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_TRNUCLI_RESET]);

    //    reinit_flag = false;
    n_reinit++;
    n_reinit_since_use++;
    reinit_time = reset_time;

    int reinit_post=wtnav_get_num_reinits(trn_instance);

    if(reinit_post<=reinits_pre){
        retval=-1;
    }

    return retval;
}

int s_mbtrnpp_trnu_reset_box_callback(double ofs_x, double ofs_y, double ofs_z, double sx, double sy, double sz)
{
    int retval=0;
    int reinits_pre=wtnav_get_num_reinits(trn_instance);

    double reset_time = mtime_etime();

    fprintf(stderr, "--reinit_box (cli_req) systime:%.6f centered on offset: %lf %lf %lf %lf %lf %lf\n",
            reset_time, ofs_x, ofs_y, ofs_z, sx, sy, sz);

    wtnav_reinit_filter_box(trn_instance, true, ofs_x, ofs_y, ofs_z, sx, sy, sz);

    mlog_tprintf(mbtrnpp_mlog_id, "i,trn filter reinit_box.cli systime:%.6f centered on offset: %lf %lf %lf %lf %lf %lf\n",
                 reset_time, ofs_x, ofs_y, ofs_z, sx, sy, sz);

    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_REINIT]);
    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_TRNUCLI_RESET]);

    //    reinit_flag = false;
    n_reinit++;
    n_reinit_since_use++;
    reinit_time = reset_time;
    int reinit_post=wtnav_get_num_reinits(trn_instance);

    if(reinit_post<=reinits_pre){
        retval=-1;
    }

    return retval;
}

int mbtrnpp_init_trnusvr(netif_t **psvr, char *host, int port, wtnav_t *trn, bool verbose)
{
    int retval = -1;
    MX_DEBUG("configuring trnu (update) server socket using %s:%d\n", host, port);
    if(NULL!=psvr && NULL!=host){
        netif_t *svr = netif_new("trnusvr",host,
                                 port,
                                 ST_UDP,
                                 IFM_REQRES,
                                 mbtrn_cfg->trnusvr_hbto,
                                 trnif_msg_read_trnu,
                                 trnif_msg_handle_trnu,
                                 trnif_msg_pub_trnu);


        if(NULL!=svr){
            *psvr = svr;
            g_trnu_res->trn=trn_instance;
            g_trnu_res->reset_callback = s_mbtrnpp_trnu_reset_callback;
            g_trnu_res->reset_ofs_callback = s_mbtrnpp_trnu_reset_ofs_callback;
            g_trnu_res->reset_box_callback = s_mbtrnpp_trnu_reset_box_callback;

            netif_set_reqres_res(svr,g_trnu_res);
            //            trnif_res_t rr_resources={trn};
            //netif_set_reqres_res(svr,trn);
            fprintf(stderr,"trnusvr netif:\n");
            netif_show(svr,true,5);
            netif_init_log(svr, "trnusvr", (NULL!=mbtrn_cfg->trn_log_dir?mbtrn_cfg->trn_log_dir:"."), s_mbtrnpp_session_str(NULL,0,RF_NONE));
            mlog_tprintf(svr->mlog_id,"*** trnusvr session start (TEST) ***\n");
            mlog_tprintf(svr->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());
            retval = netif_connect(svr);
        } else {
            fprintf(stderr,"%s:%d - ERR allocation\n",__FUNCTION__,__LINE__);
        }
    } else {
        fprintf(stderr,"%s:%d - ERR invalid args\n",__FUNCTION__,__LINE__);
    }
    return retval;
}

int mbtrnpp_init_trnumsvr(netif_t **psvr, char *host, int port, wtnav_t *trn, bool verbose)
{
    int retval = -1;
    MX_DEBUG("configuring trnum (update) server socket using %s:%d\n", host, port);
    if(NULL!=psvr && NULL!=host){
        netif_t *svr = netif_mcast_new("trnumsvr",host,
                                 port,
                                 ST_UDPM,
                                 IFM_REQRES,
                                 mbtrn_cfg->trnumsvr_ttl,
                                 trnif_msg_read_trnu,
                                 trnif_msg_handle_trnu,
                                 trnif_msg_pub_trnu);


        if(NULL!=svr){
            *psvr = svr;
            g_trnu_res->trn=trn_instance;
            g_trnu_res->reset_callback=s_mbtrnpp_trnu_reset_callback;

            netif_set_reqres_res(svr,g_trnu_res);
            //            trnif_res_t rr_resources={trn};
            //netif_set_reqres_res(svr,trn);
            fprintf(stderr,"trnumsvr netif:\n");
            netif_show(svr,true,5);
            netif_init_log(svr, "trnumsvr", (NULL!=mbtrn_cfg->trn_log_dir?mbtrn_cfg->trn_log_dir:"."), s_mbtrnpp_session_str(NULL,0,RF_NONE));
            mlog_tprintf(svr->mlog_id,"*** trnumsvr session start (TEST) ***\n");
            mlog_tprintf(svr->mlog_id,"libnetif v[%s] build[%s]\n",netif_get_version(),netif_get_build());
            retval = netif_connect(svr);
        } else {
            fprintf(stderr,"%s:%d - ERR allocation\n",__FUNCTION__,__LINE__);
        }
    } else {
        fprintf(stderr,"%s:%d - ERR invalid args\n",__FUNCTION__,__LINE__);
    }
    return retval;
}

/*--------------------------------------------------------------------*/
int mbtrnpp_trn_get_bias_estimates(wtnav_t *self, wposet_t *pt, trn_update_t *pstate) {
    int retval = -1;
    wposet_t *mle = wposet_dnew();
    wposet_t *mse = wposet_dnew();

    if ( (NULL != self) && (NULL != pt) && (NULL != pstate)) {

        wtnav_estimate_pose(self, mle, 1);
        wtnav_estimate_pose(self, mse, 2);

//fprintf(stderr,"\n%s:%d:%s MLE,MSE\n",__FILE__, __LINE__, __FUNCTION__);
//wposet_show(mle,true,5);
//fprintf(stderr,"\n");
//wposet_show(mse,true,5);

        if (wtnav_last_meas_successful(self)) {
            wposet_pose_to_cdata(&pstate->pt_dat, pt);
            wposet_pose_to_cdata(&pstate->mle_dat, mle);
            wposet_pose_to_cdata(&pstate->mse_dat, mse);
            pstate->success=1;
            retval = 0;
        }
        else {
            MX_DMSG(MXDEBUG, "Last Meas Invalid\n");
            mlog_tprintf(trnu_alog_id,"ERR: last meas invalid\n");
        }
        wposet_destroy(mle);
        wposet_destroy(mse);
    }

    return retval;
}

/*--------------------------------------------------------------------*/
int mbtrnpp_check_reinit(trn_update_t *pstate, trn_config_t *cfg)
{
    int retval = -1;

    if(NULL!=pstate && NULL!=cfg){

        if (use_offset_time == 0.0)
          use_offset_time = pstate->mse_dat->time;
        if (pstate->mse_dat->time > 0.0) {
          double offset_n = pstate->mse_dat->x - pstate->pt_dat->x;
          double offset_e = pstate->mse_dat->y - pstate->pt_dat->y;
          double offset_z = pstate->mse_dat->z - pstate->pt_dat->z;
          double covariance_mag = sqrt(pstate->mse_dat->covariance[0] * pstate->mse_dat->covariance[0]
                    + pstate->mse_dat->covariance[1] * pstate->mse_dat->covariance[1]
                    + pstate->mse_dat->covariance[2] * pstate->mse_dat->covariance[2]);
          if (covariance_mag <= mbtrn_cfg->covariance_magnitude_max) {
            converged = true;
            n_converged_streak++;
            n_unconverged_streak = 0;
            n_converged_tot++;
          } else {
            converged = false;
            n_converged_streak = 0;
            n_unconverged_streak++;
            n_unconverged_tot++;
          }
          if (n_converged_streak >= mbtrn_cfg->convergence_repeat_min) {
            use_trn_offset = true;
            use_offset_time = pstate->mse_dat->time;
            use_offset_n = offset_n;
            use_offset_e = offset_e;
            use_offset_z = offset_z;
            use_covariance[0] = pstate->mse_dat->covariance[0];
            use_covariance[1] = pstate->mse_dat->covariance[2];
            use_covariance[2] = pstate->mse_dat->covariance[5];
            use_covariance[3] = pstate->mse_dat->covariance[1];
            n_reinit_since_use = 0;
          } else {
            use_trn_offset = false;
          }

          // check if offsets are within acceptable limits, set reinit_flag if not
          double xyoffsetmag = sqrt(offset_n * offset_n + offset_e * offset_e);
          if (mbtrn_cfg->reinit_xyoffset_enable && (xyoffsetmag > mbtrn_cfg->reinit_xyoffset_max)
              && n_converged_streak > 10) {
            if (!reinit_flag) {
              fprintf(stderr, "--Reinit set due to xy offset magntitude %f > threshold %f\n",
                      xyoffsetmag, mbtrn_cfg->reinit_xyoffset_max);
              mlog_tprintf(mbtrnpp_mlog_id,"i,reinit due to xyoffset magnitude [%.3lf] > threshold [%.3lf]\n",
                          xyoffsetmag, mbtrn_cfg->reinit_xyoffset_max);
              MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_xyoffset]);
              reinit_flag = true;
            }
          }
          if (mbtrn_cfg->reinit_zoffset_enable
              && (offset_z < mbtrn_cfg->reinit_zoffset_min
                  || offset_z > mbtrn_cfg->reinit_zoffset_max)
              && n_converged_streak > 10) {
            if (!reinit_flag) {
              fprintf(stderr, "--Reinit set due to z offset %f outside allowed range %f %f\n",
                      offset_z, mbtrn_cfg->reinit_zoffset_min, mbtrn_cfg->reinit_zoffset_max);
              mlog_tprintf(mbtrnpp_mlog_id,"i,reinit due to offset_z [%.3lf] outside of allowed range: [%.3lf] to [%.3lf]\n",
                            offset_z, mbtrn_cfg->reinit_zoffset_min, mbtrn_cfg->reinit_zoffset_max);
              MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_offset_z]);
              reinit_flag = true;
            }
          }
        }

        retval=0;
    }

    return retval;
}

/*--------------------------------------------------------------------*/
int mbtrnpp_trn_publish(trn_update_t *pstate, trn_config_t *cfg)
{
    int retval = -1;

    if(NULL!=pstate && NULL!=cfg){
        // publish to selected outputs
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_SVR_EN) ){

            MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_PUB_XT], mtime_dtime());

            mbtrnpp_trnu_pub_osocket(pstate, trnusvr);

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_PUB_XT], mtime_dtime());
        }
        if( OUTPUT_FLAG_SET(OUTPUT_TRNUM_SVR_EN) ){

            MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUM_PUB_XT], mtime_dtime());

            mbtrnpp_trnu_pub_osocket(pstate, trnumsvr);

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUM_PUB_XT], mtime_dtime());
        }
// fprintf(stderr, "%s:%d:%s: pt_dat: %f  %f %f %f  %f %f %f %f  mle_dat: %f  %f %f %f  %f %f %f %f  mse_dat: %f  %f %f %f  %f %f %f %f"
// " %d %f %d %d %d %d %d %d %f %f\n",
// __FILE__, __LINE__, __func__,
// pstate->pt_dat->time,pstate->pt_dat->x,pstate->pt_dat->y,pstate->pt_dat->z,
// pstate->pt_dat->covariance[0],pstate->pt_dat->covariance[2],pstate->pt_dat->covariance[5],pstate->pt_dat->covariance[1],
// pstate->mle_dat->time,pstate->mle_dat->x,pstate->mle_dat->y,pstate->mle_dat->z,
// pstate->mle_dat->covariance[0],pstate->mle_dat->covariance[2],pstate->mle_dat->covariance[5],pstate->mle_dat->covariance[1],
// pstate->mse_dat->time,pstate->mse_dat->x,pstate->mse_dat->y,pstate->mse_dat->z,
// pstate->mse_dat->covariance[0],pstate->mse_dat->covariance[2],pstate->mse_dat->covariance[5],pstate->mse_dat->covariance[1],
// pstate->reinit_count, pstate->reinit_tlast, pstate->filter_state, pstate->success, pstate->is_converged,
// pstate->is_valid, pstate->mb1_cycle, pstate->ping_number, pstate->mb1_time, pstate->update_time);
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_ASC) ){
            MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_LOG_XT], mtime_dtime());

            mbtrnpp_trn_pub_olog(pstate, trnu_alog_id);

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_LOG_XT], mtime_dtime());
        }
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_BIN) ){
            MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_BLOG_XT], mtime_dtime());

            mbtrnpp_trn_pub_blog(pstate, trnu_blog_id);

            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNU_BLOG_XT], mtime_dtime());
        }
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_DEBUG) ){
            mbtrnpp_trn_pub_odebug(pstate);
        }
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_SOUT) ){
            mbtrnpp_trn_pub_ostream(pstate, stdout);
        }
        if( OUTPUT_FLAG_SET(OUTPUT_TRNU_SERR)){
            mbtrnpp_trn_pub_ostream(pstate, stderr);
        }

        if (pstate->mse_dat->time > 0.0) {
          char *useornot[2] = {"---", "USE"};
          char *convergedornot[3] = {"---", "CNV", "RNT"};
          int convergestate = reinitialized ? 2 : (converged ? 1 : 0);
          int time_i[7];
          mb_get_date(0, (double)pstate->mse_dat->time, time_i);
          double offset_n = pstate->mse_dat->x - pstate->pt_dat->x;
          double offset_e = pstate->mse_dat->y - pstate->pt_dat->y;
          double offset_z = pstate->mse_dat->z - pstate->pt_dat->z;
          double covariance_mag = sqrt(pstate->mse_dat->covariance[0] * pstate->mse_dat->covariance[0]
                    + pstate->mse_dat->covariance[1] * pstate->mse_dat->covariance[1]
                    + pstate->mse_dat->covariance[2] * pstate->mse_dat->covariance[2]);

          // NOTE: TRN convention is x:northing y:easting z:down
          //       Output here is in order easting northing z
          if ((n_converged_tot + n_unconverged_tot - 1) % 25 == 0) {
            fprintf(stderr, "---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
            fprintf(stderr, "YYYY/MM/DD-HH:MM:SS.SSSSSS TTTTTTTTTT.TTTTTT | Nav: Easting  Northing     Z     | TRN: Easting  Northing     Z     | Off: East   North     Z   | Cov: East     North       Z   :     Mag   | Best Off: T      E      N      Z    |   Ncs   Nct   Nus   Nut  Nr | Use \n");
            fprintf(stderr, "---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
          }
          fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                          "| %11.3f %11.3f %8.3f | %11.3f %11.3f %8.3f "
                          "| %8.3f %8.3f %7.3f | %9.3f %9.3f %9.3f : %9.3f "
                          "| %12.6f %7.3f %7.3f %6.3f | %5d %5d %5d %5d %3d | %s %s\n",
          time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], pstate->pt_dat->time,
          pstate->pt_dat->y, pstate->pt_dat->x, pstate->pt_dat->z,
          pstate->mse_dat->y, pstate->mse_dat->x, pstate->mse_dat->z,
          offset_e, offset_n, offset_z,
          pstate->mse_dat->covariance[1], pstate->mse_dat->covariance[0], pstate->mse_dat->covariance[2], covariance_mag,
          pstate->pt_dat->time - use_offset_time, use_offset_e, use_offset_n, use_offset_z,
          n_converged_streak, n_converged_tot, n_unconverged_streak, n_unconverged_tot, n_reinit,
          convergedornot[convergestate], useornot[use_trn_offset]);

          if (output_trn_fp != NULL)
            if ((n_converged_tot + n_unconverged_tot - 1) == 0) {
              char user[256], host[256], date[32];
              int error = MB_ERROR_NO_ERROR;
              mb_user_host_date(0, user, host, date, &error);
              fprintf(output_trn_fp, "##---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
              fprintf(output_trn_fp, "## Terrain Relative Navigation Log\n");
              fprintf(output_trn_fp, "## Generated by program %s\n", program_name);
              fprintf(output_trn_fp, "## Executed on cpu <%s> by user <%s> at <%s>\n", host, user, date);
              fprintf(output_trn_fp, "## MB-System version <%s>\n", MB_VERSION);
              fprintf(output_trn_fp, "## Reference topography model: %s\n", cfg->map_file);
              fprintf(output_trn_fp, "##---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
              fprintf(output_trn_fp, "## Parameters:\n");
              s_mbtrnpp_show_cfg(output_trn_fp, mbtrn_cfg,true,5);
              fprintf(output_trn_fp, "## \n");
              fprintf(output_trn_fp, "##---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
              fprintf(output_trn_fp, "## YYYY/MM/DD-HH:MM:SS.SSSSSS TTTTTTTTTT.TTTTTT | Nav: Easting  Northing Z   | TRN: Easting  Northing     Z     | Off: East   North  Z   | Cov: East  North       Z   :    Mag   | Best Off: T    E      N      Z    | Ncs   Nct   Nus   Nut  Nr | CNV USE \n");
              fprintf(output_trn_fp, "##---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
            }
            fprintf(output_trn_fp, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                          "%11.3f %11.3f %8.3f %11.3f %11.3f %8.3f "
                          "%8.3f %8.3f %7.3f %9.3f %9.3f %9.3f %9.3f "
                          "%12.6f %7.3f %7.3f %6.3f %5d %5d %5d %5d %3d %s %s\n",
          time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], pstate->pt_dat->time,
          pstate->pt_dat->y, pstate->pt_dat->x, pstate->pt_dat->z,
          pstate->mse_dat->y, pstate->mse_dat->x, pstate->mse_dat->z,
          offset_e, offset_n, offset_z,
          pstate->mse_dat->covariance[1], pstate->mse_dat->covariance[0], pstate->mse_dat->covariance[2], covariance_mag,
          pstate->pt_dat->time - use_offset_time, use_offset_e, use_offset_n, use_offset_z,
          n_converged_streak, n_converged_tot, n_unconverged_streak, n_unconverged_tot, n_reinit,
          convergedornot[convergestate], useornot[use_trn_offset]);

        // save the reinit state for the next iteration output
        reinitialized = reinit_flag;
        }

        retval=0;
    }

    return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_update(wtnav_t *self, mb1_t *src, wposet_t **pt_out, wmeast_t **mt_out, trn_config_t *cfg) {
  int retval = -1;

  if (NULL != self && NULL != src && NULL != pt_out && NULL != mt_out) {
      int test = -1;

    if ((test = wmeast_mb1_to_meas(mt_out, src, cfg->utm_zone)) == 0) {

      if ((test = wposet_mb1_to_pose(pt_out, src, cfg->utm_zone)) == 0) {
        // must do motion update first if pt time <= mt time
        wtnav_motion_update(self, *pt_out);
        wtnav_meas_update(self, *mt_out, cfg->sensor_type);
        //                fprintf(stderr,"%s:%d DONE [PT, MT]\n",__FUNCTION__,__LINE__);
        //                wposet_show(*pt_out,true,5);
        //                wmeast_show(*mt_out,true,5);
        retval = 0;
      }
      else {
        MX_DEBUG("wposet_mb1_to_pose failed [%d]\n", test);
          mlog_tprintf(trnu_alog_id,"ERR: mb1_to_pose failed [%d]\n", test);
      }
    }
    else {
      MX_DEBUG("wmeast_mb1_to_meas failed [%d]\n", test);
        mlog_tprintf(trnu_alog_id,"ERR: mb1_to_meas failed [%d]\n", test);
    }
  }

  return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_trn_process_mb1(wtnav_t *tnav, mb1_t *mb1, trn_config_t *cfg)
{
    int retval=-1;

    static int mb1_count=0;

    mlog_tprintf(trnu_alog_id,"trn_mb1_count,%lf,%d\n",mtime_etime(),++mb1_count);

    // ignore if trn disabled
    if(mbtrn_cfg->trn_enable){
        // check decimation
        bool do_process=false;

        // TODO: arbitrate between time/count decimation
        if(mbtrn_cfg->trn_decn>0){
            if( ((++trn_dec_cycles)%mbtrn_cfg->trn_decn)==0 ){
                do_process=true;
                trn_dec_cycles=0;
            }
        } else if(mbtrn_cfg->trn_decs>0.0){
            double now=mtime_dtime();
            if( ((mtime_dtime()-trn_dec_time)) > mbtrn_cfg->trn_decs){
                do_process=true;
                trn_dec_time=now;
            }
        } else {
            // always process if decimation disabled
            // (trn_decs<=0 && mbtrn_cfg->trn_decn<=0 )
            do_process=true;
        }

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNSVR_XT], mtime_dtime());

        // server: update (trn_server) client connections
        netif_update_connections(trnsvr);

        // server: service (trn_server) client requests
        netif_reqres(trnsvr);

        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNSVR_XT], mtime_dtime());

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUSVR_XT], mtime_dtime());

       // server: update (trnu server) client connections
        netif_update_connections(trnusvr);
        // server: service (trnu server) client requests
        netif_reqres(trnusvr);

        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUSVR_XT], mtime_dtime());

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUMSVR_XT], mtime_dtime());
       // server: update (trnum server) client connections
        netif_update_connections(trnumsvr);
        // server: service (trnum server) client requests
        netif_reqres(trnumsvr);
        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_TRNUMSVR_XT], mtime_dtime());

        if (do_process) {
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_TRN_PROCN]);

            if(NULL!=tnav && NULL!=mb1 && NULL!=cfg){
                static int process_count=0;

                mlog_tprintf(trnu_alog_id,"trn_update_start,%lf,%lf,%d\n",mtime_etime(),mb1->ts,++process_count);
                MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_PROC_XT], mtime_dtime());

                wmeast_t *mt = NULL;
                wposet_t *pt = NULL;
                trn_update_t trn_state={NULL,NULL,NULL,0,0,0,0,0.0,0.0},*pstate=&trn_state;

                // get TRN update
                MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_UPDATE_XT], mtime_dtime());

                int test=mbtrnpp_trn_update(tnav, mb1, &pt, &mt,cfg);

                MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_UPDATE_XT], mtime_dtime());

                if( test==0){
                    // get TRN bias estimates
                    MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_BIASEST_XT], mtime_dtime());

                    test=mbtrnpp_trn_get_bias_estimates(tnav, pt, pstate);

                    MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_BIASEST_XT], mtime_dtime());

                  if( test==0){
                        if(NULL!=pstate->pt_dat &&  NULL!= pstate->mle_dat && NULL!=pstate->mse_dat ){

                            // get number of reinits
                            MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_TRN_NREINITS_XT], mtime_dtime());

                            // check if reinit will be required on next processing
                            mbtrnpp_check_reinit(pstate, cfg);

                            pstate->reinit_count = wtnav_get_num_reinits(tnav);
                            pstate->filter_state = wtnav_get_filter_state(tnav);
                            pstate->is_converged = (converged ? 1 : 0);
                            pstate->is_valid = (use_trn_offset ? 1 : 0);
                            // pstate->is_valid = ( (mb1->ts > 0. &&
                            //                       pstate->mse_dat->covariance[0] <= cfg->max_northing_cov &&
                            //                       pstate->mse_dat->covariance[2] <= cfg->max_easting_cov &&
                            //                       fabs(pstate->mse_dat->x-pstate->pt_dat->x) <= cfg->max_northing_err &&
                            //                       fabs(pstate->mse_dat->y-pstate->pt_dat->y) <= cfg->max_easting_err
                            //                     )? 1 : 0);
                            pstate->mb1_cycle=mb1_count;
                            pstate->ping_number=mb1->ping_number;
                            pstate->mb1_time=mb1->ts;
                            pstate->update_time=mtime_etime();

                            MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_NREINITS_XT], mtime_dtime());

                            // publish to selected outputs
                            mbtrnpp_trn_publish(pstate, cfg);

                            retval=0;

                        } else {
                            MX_DEBUG("ERR: pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p]\n",pt,pstate->pt_dat,pstate->mle_dat,pstate->mse_dat);
                            mlog_tprintf(trnu_alog_id,"ERR: NULL data pt[%p] pt_dat[%p] mle_dat[%p] mse_dat[%p] ts[%.3lf] beams[%u] ping[%d] lat[%.5lf] lon[%.5lf] hdg[%.2lf] sd[%.1lf]\n",
                                         pt,pstate->pt_dat,pstate->mle_dat,pstate->mse_dat,
                                         mb1->ts, mb1->nbeams, mb1->ping_number,
                                         mb1->lat, mb1->lon, mb1->hdg, mb1->depth);
                        }
                    } else {
                        mlog_tprintf(trnu_alog_id,"ERR: trncli_get_bias_estimates failed [%d] [%d/%s]\n",test,errno,strerror(errno));

                        MX_BPRINT((mxd_level(MBTRNPP) >= 3 || mxd_level(MXDEBUG) != 0), "ERR: trn_get_bias_estimates failed [%d] [%d/%s]\n",test,errno,strerror(errno));

                        int time_i[7];
                        mb_get_date(0, mb1->ts, time_i);
                        fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                                        "| %11.6f %11.6f %8.3f | %d filtered beams - Ping not used - failed bias estimate\n",
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], mb1->ts,
                        mb1->lon, mb1->lat, mb1->depth, mb1->nbeams);
                        mbtrnpp_trnu_pubempty_osocket(mb1->ts, mb1->lat, mb1->lon, mb1->depth, trnusvr);
                        mbtrnpp_trnu_pubempty_osocket(mb1->ts, mb1->lat, mb1->lon, mb1->depth, trnumsvr);
                    }
                } else {
                    mlog_tprintf(trnu_alog_id,"ERR: trncli_send_update failed [%d] [%d/%s]\n",test,errno,strerror(errno));

                    MX_BPRINT((mxd_level(MBTRNPP) >= 3 || mxd_level(MXDEBUG) != 0), "ERR: trn_update failed [%d] [%d/%s]\n",test,errno,strerror(errno));

                    int time_i[7];
                    mb_get_date(0, mb1->ts, time_i);
                    fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
                                    "| %11.6f %11.6f %8.3f | %d filtered beams - Ping not used - failed trn processing\n",
                    time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], mb1->ts,
                    mb1->lon, mb1->lat, mb1->depth, mb1->nbeams);
                    mbtrnpp_trnu_pubempty_osocket(mb1->ts, mb1->lat, mb1->lon, mb1->depth, trnusvr);
                    mbtrnpp_trnu_pubempty_osocket(mb1->ts, mb1->lat, mb1->lon, mb1->depth, trnumsvr);
                }
                wmeast_destroy(mt);
                wposet_destroy(pt);
                if(NULL!=pstate->pt_dat)
                free(pstate->pt_dat);
                if(NULL!=pstate->mse_dat)
                free(pstate->mse_dat);
                if(NULL!=pstate->mle_dat)
                free(pstate->mle_dat);

                MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_TRN_PROC_XT], mtime_dtime());
            }// if tnav, mb1,cfg != NULL
            mlog_tprintf(trnu_alog_id,"trn_update_end,%lf,%d\n",mtime_etime(),retval);
        }// if do_process
        //else {
        //    int time_i[7];
        //    mb_get_date(0, mb1->ts, time_i);
        //    fprintf(stderr, "%4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %.6f "
        //                    "| %11.6f %11.6f %8.3f | Ping not processed - decimated\n",
        //    time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], mb1->ts,
        //    mb1->lon, mb1->lat, mb1->depth);
        //}
    }// if trn_en

    return retval;
}

#endif // WITH_MBTNAV

int mbtrnpp_process_mb1(char *src, size_t len, trn_config_t *cfg)
{
    int retval=-1;

    if(NULL!=src && NULL!=cfg){

        // log current TRN message
        if ( OUTPUT_FLAG_SET(OUTPUT_MB1_BIN) ) {
            mlog_write(mb1_blog_id, (byte *)src, len);
        }

        if ( OUTPUT_FLAG_SET(OUTPUT_MB1_SVR_EN) ) {
            // server: update (mb1 server) client connections
            netif_update_connections(mb1svr);
            // server: service (mb1 server) client requests
            netif_reqres(mb1svr);
           // publish mb1 sounding to all clients
            if(netif_pub(mb1svr,(char *)src, len, NULL) == 0){
	            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_PUBN]);
            } else {
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBPUB]);
            }
        }
        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CYCLES]);

        //                struct timeval stv={0};
        //                gettimeofday(&stv,NULL);
        //                double stime = (double)stv.tv_sec+((double)stv.tv_usec/1000000.0);
        //                double ptime=ping[i_ping_process].time_d;
        //                fprintf(stderr,"mbtx : ptime[%.3lf] stime[%.3lf]
        //                (s-p)[%+6.3lf]**\n",ptime,stime,(stime-ptime)); fprintf(stderr,"mbtx :
        //                (s-p)[%+6.3lf]**\n",(stime-ptime));

        if (mbtrn_cfg->mbtrnpp_loop_delay_msec > 0) {
            MX_LPRINT(MBTRNPP, 5, "delaying msec[%"PRId64"]\n", mbtrn_cfg->mbtrnpp_loop_delay_msec);
            mtime_delay_ms(mbtrn_cfg->mbtrnpp_loop_delay_msec);
        }

        retval=0;
    }
    return retval;
}

/*--------------------------------------------------------------------*/

int mbtrnpp_reson7kr_input_open(int verbose, void *mbio_ptr, char *definition, int *error)
{

  // local variables
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;

  uint32_t reson_nsubs = 11;
  uint32_t reson_subs[] = {1003, 1006, 1008, 1010, 1012, 1013, 1015, 1016, 7000, 7004, 7027};

  // print input debug statements
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p,%p\n", mbio_ptr, &mbio_ptr);
    fprintf(stderr, "dbg2       hostname:   %s\n", definition);
  }

  // get pointer to mbio descriptor
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  // set initial status
  status = MB_SUCCESS;

    // Open and initialize the socket based input for reading using function
    // mbtrnpp_mb1r_input_read(). mbtrnpp_mb1r_input_read allocates a buffer
    // to hold a complete and validated mb1 record, and returns bytes from
    // that record as requested by the MBIO read functions.
    // Store the relevant pointers and parameters within the
    // mb_io_struct structure *mb_io_ptr.

  mb_path hostname;
  int port = 0;
  size_t size = 0;

  // copy def (strtok is destructive)
  char *defcpy = strdup(definition);
  char *addr[2]={NULL,NULL};
  char *saveptr;
  // separate hostname, numeric tokens
  addr[0]=strtok_r(defcpy,":",&saveptr);
  addr[1]=strtok_r(NULL,"",&saveptr);

  // parse hostname, port, size
  if(NULL!=addr[0])
  strcpy(hostname, addr[0]);
  if(NULL!=addr[1])
  sscanf(addr[1], "%d:%zu", &port, &size);
  // release definition copy
  free(defcpy);

  if (strlen(hostname) == 0)
  strcpy(hostname, "localhost");
  if (port == 0)
  port = R7K_7KCENTER_PORT;
  if (size == 0)
  size = SONAR_READER_CAPACITY_DFL;

  MX_DEBUG("configuring r7kr_reader using %s:%d\n", hostname, port);
  r7kr_reader_t *reader = r7kr_reader_new(mbtrn_cfg->trn_dev,hostname, port, size, reson_subs, reson_nsubs);

  if (NULL != mb_io_ptr && NULL != reader) {

    // set r7k_reader
    mb_io_ptr->mbsp = (void *) reader;

    if (reader->state == R7KR_CONNECTED || reader->state == R7KR_SUBSCRIBED) {
      // update application performance profile
      MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);
    }

    // get global 7K reader performance profile
    reader_stats = r7kr_reader_get_stats(reader);
    mstats_set_period(reader_stats, app_stats->stats->stat_period_start, app_stats->stats->stat_period_sec);

    // configure reader data log
    if ( OUTPUT_FLAG_SET(OUTPUT_RESON_BIN) ) {
      // open mbr data log
      reson_blog_path = (char *)malloc(512);
      sprintf(reson_blog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, RESON_BLOG_NAME, s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);

      reson_blog_id = mlog_get_instance(reson_blog_path, &reson_blog_conf, RESON_BLOG_NAME);

      mlog_show(reson_blog_id, true, 5);
      mlog_open(reson_blog_id, flags, mode);

      r7kr_reader_set_log(reader, reson_blog_id);
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

  // print output debug statements
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:             %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbtrnpp_reson7kr_validate_nf(r7k_nf_t *pnf)
{
    int retval = -1;
    if (pnf->protocol_version == R7K_NF_PROTO_VER) {
        if (pnf->offset >= (R7K_NF_BYTES)) {
            if (pnf->packet_size == (pnf->total_size+R7K_NF_BYTES)) {
                if (pnf->total_records == 1) {
                    retval = 0;
                }
            }
        }
    }
    return -1;
}

int mbtrnpp_reson7kr_validate_drf(r7k_drf_t *pdrf)
{
    int retval = -1;

    if ( (uint16_t)pdrf->protocol_version == (uint16_t)R7K_DRF_PROTO_VER) {
        if ((uint32_t)pdrf->sync_pattern == (uint32_t)R7K_DRF_SYNC_PATTERN) {
            if ((uint32_t)pdrf->size <= (uint32_t)R7K_MAX_FRAME_BYTES) {
                // conditionally validate
                // (pending optional nf size)
                retval = 0;
            } else {
                fprintf(stderr, "%s : ERR size [%"PRIu32"/%"PRIu32"]\n",__func__, pdrf->size, (uint32_t)R7K_MAX_FRAME_BYTES);
            }
        } else {
            fprintf(stderr, "%s : ERR sync pattern [%"PRIu32"/%"PRIu32"]\n",__func__,pdrf->sync_pattern, (uint32_t)R7K_DRF_SYNC_PATTERN);
        }
    } else {
        fprintf(stderr, "%s : ERR proto ver [%"PRIu32"/%"PRIu32"]\n",__func__,pdrf->protocol_version, R7K_DRF_PROTO_VER);
    }

#if MBTRNPP_R7KR_VALIDATE_CHECKSUM
    // validate checksum
    byte *pd=(byte *)pdrf;
    uint32_t vchk = r7k_checksum( pd, (uint32_t)(pdrf->size-R7K_CHECKSUM_BYTES));
    pd = (byte *)pdrf;
    pd += ((size_t)pdrf->size-R7K_CHECKSUM_BYTES);
    uint32_t *pchk = (uint32_t *)pd;

    if (vchk != (uint32_t)(*pchk) ) {
        retval = -1;
    }
#endif

    return retval;
}

int mbtrnpp_reson7kr_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error)
{

    // local variables
    int status = MB_SUCCESS;
    struct mb_io_struct *mb_io_ptr;

    // print input debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
        fprintf(stderr, "dbg2  Input arguments:\n");
        fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
        fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
        fprintf(stderr, "dbg2       size:       %zu\n", *size);
        fprintf(stderr, "dbg2       buffer:     %p\n", buffer);
    }

    // get pointer to mbio descriptor
    mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

    // set initial status
    *error = MB_ERROR_NO_ERROR;

    // Read the requested number of bytes (= size) off the input and  place
    // those bytes into the buffer.
    // This requires reading full s7k frames from the socket, storing the data
    // in buffer (implemented here), and parceling those bytes out as requested.

    // use the socket reader
    // read and return single frame
    uint32_t sync_bytes=0;
    int64_t rbytes=-1;
    r7kr_reader_t *reader = (r7kr_reader_t *)mb_io_ptr->mbsp;

    // frame buffer for byte-wise reads
    static byte *frame_buf = NULL;
    static r7k_drf_t *fb_pdrf = NULL;
    static byte *fb_pread=NULL;
    static size_t bytes_read=0;
    static bool read_frame=true;
    bool read_err = false;

    if(NULL == frame_buf)
    {
        frame_buf = (byte *)malloc(R7K_MAX_FRAME_BYTES);
        memset(frame_buf, 0, R7K_MAX_FRAME_BYTES);
        fb_pread = frame_buf;
        fb_pdrf = (r7k_drf_t *)(frame_buf);
        bytes_read = 0;
    }

    // if valid reader...
    if(NULL != reader && NULL != frame_buf)
    {
        if(read_frame)
        {
            // read frame into buffer
            memset(frame_buf, 0, R7K_MAX_FRAME_BYTES);
            fb_pread = frame_buf;

            // read S7K frame from the socket
            // returns number of bytes read or -1 error
            // r7kr_read_stripped_frame using R7KR_NET_STREAM
            // returns only DRF, i.e. strips network frame (NF) header
            if ( (rbytes = r7kr_read_stripped_frame(reader, (byte *) frame_buf,
                                                    R7K_MAX_FRAME_BYTES, R7KR_NET_STREAM,
                                                    0.0, R7KR_READ_TMOUT_MSEC,
                                                    &sync_bytes)) >= 0)
            {

                // validate (should already be valid)
                if(rbytes<=R7K_MAX_FRAME_BYTES &&
                   mbtrnpp_reson7kr_validate_drf(fb_pdrf)==0)
                {
                    // update frame read pointers
                    fb_pread = frame_buf;
                    read_frame = false;
                    MX_LPRINT(MBTRNPP, 3, "read frame len[%zu]:\n",(size_t)rbytes);
                } else {
                    // frame invalid
                    read_err = true;
                    MX_LPRINT(MBTRNPP, 3, "invalid frame rbytes[%zu] size[%zu]\n",(size_t)rbytes, (size_t)fb_pdrf->size);
                }
            } else {
                // read error
                read_err = true;
                MX_LPRINT(MBTRNPP, 3, "r7kr_read_stripped_frame failed rbytes[%"PRId64"]\n",rbytes);
            }

        } else {
            // there's a frame in the buffer
            size_t bytes_rem = frame_buf + fb_pdrf->size - fb_pread;
            size_t readlen = (*size <= bytes_rem ? *size : bytes_rem);
            MX_LPRINT(MBTRNPP, 3, "reading framebuf size[%zu] rlen[%zu] rem[%zu] err[%c]\n", (size_t)*size, readlen, bytes_rem, (read_err?'Y':'N'));
        }

        if(!read_err){
            // return bytes requested:
            // smaller of bytes read and bytes remaining
            int64_t bytes_rem = (int64_t)(frame_buf + fb_pdrf->size - fb_pread);
            size_t readlen = (*size <= bytes_rem ? *size : bytes_rem);
            if(readlen > 0){
                memcpy(buffer, fb_pread, readlen);
                *size = (size_t)readlen;
                *error = MB_ERROR_NO_ERROR;
                // update frame cursor
                fb_pread += readlen;
                bytes_rem -= readlen;
                if(bytes_rem <= 0)
                {
                    MX_LPRINT(MBTRNPP, 4, "* buffer empty rem[%"PRId64"]\n", bytes_rem);
                    // if nothing left, read a frame next time
                    read_frame = true;
                }
            } else {
                // buffer empty
                status   = MB_FAILURE;
                *error   = MB_ERROR_EOF;
                *size    = (size_t)-1;
                read_frame = true;
                MX_LPRINT(MBTRNPP, 4, "buffer empty readlen[%zu] rem[%"PRId64"]\n", readlen, bytes_rem);
            }
        }
    } else {
        fprintf(stderr, "%s : ERR - frame buffer is NULL\n", __func__);
    }

    if(read_err)
    {
        // read error - invalid frame or socket error
        status   = MB_FAILURE;
        *error   = MB_ERROR_EOF;
        *size    = (size_t)rbytes;

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
        MX_LPRINT(MBTRNPP, 4, "r7kr_read_stripped_frame failed: sync_bytes[%d] status[%d] err[%d]\n",sync_bytes,status, *error);

        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBFRAMERD]);
        MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_MB_SYNC_BYTES],sync_bytes);

        // check connection status (socket errors)
        // only reconnect if disconnected
        if ((NULL!=reader && reader->state==R7KR_INITIALIZED) || (me_errno==ME_ESOCK) || (me_errno==ME_EOF)  ) {

            fprintf(stderr,"EOF (input socket) - clear status/error\n");
            status = MB_SUCCESS;
            *error = MB_ERROR_NO_ERROR;

            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBSOCKET]);

            // empty the reader's record frame container
            r7kr_reader_purge(reader);

            mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket status[%s]\n",r7kr_strstate(reader->state));
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_DISN]);

            // re-connect reader
            if (r7kr_reader_connect(reader,true)==0) {
                read_frame = true;
                read_err = false;
                fprintf(stderr,"mbtrnpp: input socket connected status[%s]\n",r7kr_strstate(reader->state));
                mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket connected status[%s]\n",r7kr_strstate(reader->state));
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);
            } else {
                fprintf(stderr,"mbtrnpp: input socket reconnect failed status[%s]\n",r7kr_strstate(reader->state));
                mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket reconnect failed status[%s]\n",r7kr_strstate(reader->state));
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBCON]);

                struct timespec twait={0},trem={0};
                twait.tv_sec=5;
                nanosleep(&twait,&trem);
            }
        }

        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
    }

    // print output debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:              %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:             %d\n", status);
    }

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

  /* set flag to enable Sentry sensordepth kluge */
  int *kluge_set = (int *)&mb_io_ptr->save10;
  *kluge_set = 1;

  // Open and initialize the socket based input for reading using function
  // mbtrnpp_kemkmall_input_read().
  // - use mb_io_ptr->mbsp to hold pointer to socket i/o structure
  // - the socket definition = "hostInterface:broadcastGroup:port"
  int port=-1;
  mb_path bcastGrp;
  mb_path hostInterface;
  struct sockaddr_in localSock;
  struct ip_mreq group;
  char *token;
  char *saveptr;
  if ((token = strtok_r(definition, ":", &saveptr)) != NULL) {
    strncpy(hostInterface, token, sizeof(mb_path));
  }
  if ((token = strtok_r(NULL, ":", &saveptr)) != NULL) {
    strncpy(bcastGrp, token, sizeof(mb_path));
  }
  if ((token = strtok_r(NULL, ":", &saveptr)) != NULL) {
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

      mlog_tprintf(mbtrnpp_mlog_id,"e,datagram socket [%d/%s]\n",errno,strerror(errno));
      status=MB_FAILURE;
      *error=MB_ERROR_OPEN_FAIL;
      return status;
  }

  /* Enable SO_REUSEADDR to allow multiple instances of this */
  /* application to receive copies of the multicast datagrams. */
  int reuse = 1;
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
      perror("Setting SO_REUSEADDR error");
      close(sd);
      mlog_tprintf(mbtrnpp_mlog_id,"e,setsockopt SO_REUSEADDR [%d/%s]\n",errno,strerror(errno));
      status=MB_FAILURE;
      *error=MB_ERROR_OPEN_FAIL;
      return status;
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
      mlog_tprintf(mbtrnpp_mlog_id,"e,bind [%d/%s]\n",errno,strerror(errno));
      status=MB_FAILURE;
      *error=MB_ERROR_OPEN_FAIL;
      return status;
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
    mlog_tprintf(mbtrnpp_mlog_id,"e,setsockopt IP_ADD_MEMBERSHIP [%d/%s]\n",errno,strerror(errno));
    status=MB_FAILURE;
	*error=MB_ERROR_OPEN_FAIL;
	return status;
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

    MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);

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
  unsigned int num_bytes_dgm_end=0;
  mbsys_kmbes_emdgm_type emdgm_type=UNKNOWN;
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
  if (emdgm_type == MRZ || emdgm_type == MWC) {
      unsigned short numOfDgms=0;
      unsigned short dgmNum=0;

    mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE], &numOfDgms);
    mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE+2], &dgmNum);
    if (numOfDgms > 1) {
        static int dgmsReceived=0;
        static unsigned int pingSecs, pingNanoSecs;
        static int totalDgms;

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
        if(dgmNum>0){
            memcpy(mRecordBuf[dgmNum-1], buffer, header.numBytesDgm);
        } else {
            fprintf(stderr,"%s: ERR - dgNum<0\n",__func__);
        }

      if (dgmsReceived == totalDgms) {

        int totalSize = sizeof(struct mbsys_kmbes_m_partition)
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

          for (int dgm=1; dgm < totalDgms; dgm++) {
            status = mbtrnpp_kemkmall_rd_hdr(verbose, mRecordBuf[dgm], (void *)&header, (void *)&emdgm_type, error);
            int copy_len = header.numBytesDgm - sizeof(struct mbsys_kmbes_m_partition)
                                  - sizeof(struct mbsys_kmbes_header) - 4;
            void *ptr = (void *)(mRecordBuf[dgm]+
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
#ifdef WITH_MB1_READER
/*--------------------------------------------------------------------*/
int mbtrnpp_mb1r_input_open(int verbose, void *mbio_ptr, char *definition, int *error)
{

    // local variables
    int status = MB_SUCCESS;
    struct mb_io_struct *mb_io_ptr;

    // print input debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
        fprintf(stderr, "dbg2  Input arguments:\n");
        fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
        fprintf(stderr, "dbg2       mbio_ptr:   %p,%p\n", mbio_ptr, &mbio_ptr);
        fprintf(stderr, "dbg2       hostname:   %s\n", definition);
    }

    // get pointer to mbio descriptor
    mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

    // set initial status
    status = MB_SUCCESS;

    // Open and initialize the socket based input for reading using function
    // mbtrnpp_mb1r_input_read(). mbtrnpp_mb1r_input_read allocates a buffer
    // to hold a complete and validated mb1 record, and returns bytes from
    // that record as requested by the MBIO read functions.
    // Store the relevant pointers and parameters within the
    // mb_io_struct structure *mb_io_ptr.

    mb_path hostname;
    int port = 0;
    size_t size = 0;

    // copy def (strtok is destructive)
    char *defcpy = strdup(definition);
    char *addr[2]={NULL,NULL};
    char *saveptr;
    // separate hostname, numeric tokens
    addr[0]=strtok_r(defcpy,":",&saveptr);
    addr[1]=strtok_r(NULL,"",&saveptr);

    // parse hostname, port, size
    if(NULL!=addr[0])
        strcpy(hostname, addr[0]);
    if(NULL!=addr[1])
        sscanf(addr[1], "%d:%zu", &port, &size);
    // release definition copy
    free(defcpy);

    if (strlen(hostname) == 0)
        strcpy(hostname, "localhost");
    if (port == 0)
        port = MB1_IP_PORT_DFL;
    if (size == 0)
        size = MB1_MAX_SOUNDING_BYTES;

    MX_DEBUG("configuring mb1r_reader using %s:%d\n", hostname, port);

    mb1r_reader_t *reader = mb1r_reader_new(hostname, port, size);

    if (NULL != mb_io_ptr && NULL != reader) {

        // set mb1_reader
        mb_io_ptr->mbsp = (void *) reader;

        if (reader->state == MB1R_CONNECTED || reader->state == MB1R_SUBSCRIBED) {
            // update application performance profile
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);
        }

        // get global reader performance profile
        reader_stats = mb1r_reader_get_stats(reader);
        mstats_set_period(reader_stats, app_stats->stats->stat_period_start, app_stats->stats->stat_period_sec);

        // configure reader data log
        if ( OUTPUT_FLAG_SET(OUTPUT_MB1R_BIN) ) {
            // open mbr data log
            mb1r_blog_path = (char *)malloc(512);
            sprintf(mb1r_blog_path, "%s//%s-%s%s", mbtrn_cfg->trn_log_dir, MB1R_BLOG_NAME, s_mbtrnpp_session_str(NULL,0,RF_NONE), MBTRNPP_LOG_EXT);

            mb1r_blog_id = mlog_get_instance(mb1r_blog_path, &mb1r_blog_conf, MB1R_BLOG_NAME);

            mlog_show(mb1r_blog_id, true, 5);
            mlog_open(mb1r_blog_id, flags, mode);

            mb1r_reader_set_log(reader, mb1r_blog_id);
        }

        if (verbose >= 1) {
            mb1r_reader_show(reader, true, 5);
        }
    }
    else {
        fprintf(stderr, "ERR - mb1r_reader_new failed (NULL) [%d:%s]\n", errno, strerror(errno));
        status = MB_FAILURE;
        *error = MB_ERROR_INIT_FAIL;
    }

    // print output debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:              %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:             %d\n", status);
    }

    return (status);
}
int mbtrnpp_mb1r_input_read(int verbose, void *mbio_ptr, size_t *size, char *buffer, int *error)
{

    // local variables
    int status = MB_SUCCESS;
    struct mb_io_struct *mb_io_ptr;

    // print input debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
        fprintf(stderr, "dbg2  Input arguments:\n");
        fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
        fprintf(stderr, "dbg2       mbio_ptr:   %p\n", mbio_ptr);
        fprintf(stderr, "dbg2       size:       %zu\n", *size);
        fprintf(stderr, "dbg2       buffer:     %p\n", buffer);
    }

    // get pointer to mbio descriptor
    mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

    // set initial status
    status = MB_SUCCESS;

    // Read the requested number of bytes (= size) off the input and  place
    // those bytes into the buffer.
    // This requires reading full MB1 records from the socket, storing the data
    // in buffer (implemented here), and parceling those bytes out as requested.

    // use the socket reader
    // read and return single frame
    uint32_t sync_bytes=0;
    int64_t rbytes=-1;
    mb1r_reader_t *reader = (mb1r_reader_t *)mb_io_ptr->mbsp;

    // frame buffer for byte-wise reads
    static byte *frame_buf = NULL;
    static mb1_t *fb_pmb1 = NULL;
    static byte *fb_pread=NULL;
    static size_t bytes_read=0;
    static bool read_frame=true;
    bool read_err = false;

    if(NULL == frame_buf)
    {
        frame_buf = (byte *)malloc(MB1_MAX_SOUNDING_BYTES);
        memset(frame_buf, 0, MB1_MAX_SOUNDING_BYTES);
        fb_pread = frame_buf;
        fb_pmb1 = (mb1_t *)frame_buf;
        bytes_read = 0;
    }

    // if valid reader...
    if(NULL != reader && NULL != frame_buf)
    {
        if(read_frame)
        {
            // read frame into buffer
            memset(frame_buf, 0, MB1_MAX_SOUNDING_BYTES);
            fb_pread = frame_buf;

            // read MB1 frame from the socket
            // returns number of bytes read or -1 error
            if ( (rbytes = mb1r_read_frame(reader, (byte *) frame_buf,
                                           MB1_MAX_SOUNDING_BYTES, MB1R_NET_STREAM,
                                           0.0, MB1R_READ_TMOUT_MSEC,
                                           &sync_bytes)) >= 0)
            {
                // validate
                if(rbytes<=MB1_MAX_SOUNDING_BYTES &&
                   fb_pmb1->size == rbytes &&
                   fb_pmb1->nbeams<=MB1_MAX_BEAMS &&
                   mb1_validate_checksum(fb_pmb1)==0)
                {
                    // update frame read pointers
                    fb_pread = frame_buf;
                    read_frame = false;
                    read_err = false;
                    MX_LPRINT(MBTRNPP, 3, "read frame len[%zu]:\n",(size_t)rbytes);
//                   if(verbose>=2 || verbose<=-2){
//                        mb1_show((mb1_t *)frame_buf,(verbose<-2 || verbose>=2 ? true : false),5);
//                    }
                } else {
                    // frame invalid
                    read_err = true;
                    MX_LPRINT(MBTRNPP, 3, "invalid frame rbytes[%zu] size[%zu]\n",(size_t)rbytes, (size_t)fb_pmb1->size);
                }
            } else {
                // read error
                read_err = true;
                MX_LPRINT(MBTRNPP, 3, "mb1r_read_frame failed rbytes[%zu]\n",(size_t)rbytes);
            }

        } else {
            // there's a frame in the buffer
            size_t bytes_rem = frame_buf + fb_pmb1->size - fb_pread;
            size_t readlen = (*size <= bytes_rem ? *size : bytes_rem);
            MX_LPRINT(MBTRNPP, 3, "reading framebuf size[%zu] rlen[%zu] rem[%zu] err[%c]\n", (size_t)*size, readlen, bytes_rem, (read_err?'Y':'N'));
        }

        if(!read_err){
            int64_t bytes_rem = frame_buf + fb_pmb1->size - fb_pread;
            size_t readlen = (*size <= bytes_rem ? *size : bytes_rem);
            if(readlen > 0){
                memcpy(buffer, fb_pread, readlen);
                *size = (size_t)readlen;
                *error = MB_ERROR_NO_ERROR;
                // update frame cursor
                fb_pread += readlen;
                bytes_rem -= readlen;
                if(bytes_rem <= 0)
                {
                    MX_LPRINT(MBTRNPP, 4, "* buffer empty rem[%"PRId64"]\n", bytes_rem);
                    // if nothing left, read a frame next time
                    read_frame = true;
                }
            } else {
                // buffer empty
                status   = MB_FAILURE;
                *error   = MB_ERROR_EOF;
                *size    = (size_t)0;
                read_frame = true;
                MX_LPRINT(MBTRNPP, 4, "buffer empty readlen[%zu] rem[%"PRId64"]\n", readlen, bytes_rem);
            }
        }
    } else {
        fprintf(stderr, "%s : ERR - frame buffer is NULL\n", __func__);
    }

    if(read_err)
    {
        status   = MB_FAILURE;
        *error   = MB_ERROR_EOF;
        *size    = (size_t)0;

        MST_METRIC_START(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
        MX_LPRINT(MBTRNPP, 4, "mb1r_read_frame failed: sync_bytes[%d] status[%d] err[%d]\n",sync_bytes,status, *error);

        MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBFRAMERD]);
        MST_COUNTER_ADD(app_stats->stats->status[MBTPP_STA_MB_SYNC_BYTES],sync_bytes);

        // check connection status
        // only reconnect if disconnected
        if ((NULL!=reader && reader->state==MB1R_INITIALIZED) || (me_errno==ME_ESOCK) || (me_errno==ME_EOF)  ) {

            fprintf(stderr,"EOF (input socket) - clear status/error\n");
            status = MB_SUCCESS;
            *error = MB_ERROR_NO_ERROR;

            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBSOCKET]);

            // empty the reader's record frame container
            mb1r_reader_purge(reader);

            mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket status[%s]\n",mb1r_strstate(reader->state));
            MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_DISN]);

            // re-connect reader
            if (mb1r_reader_connect(reader,true)==0) {
                read_frame = true;
                read_err = false;
                fprintf(stderr,"mbtrnpp: input socket re-connected status[%s]\n",mb1r_strstate(reader->state));
                mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket connected status[%s]\n",mb1r_strstate(reader->state));
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_MB_CONN]);
            } else {
                fprintf(stderr,"mbtrnpp: input socket reconnect failed status[%s]\n",mb1r_strstate(reader->state));
                mlog_tprintf(mbtrnpp_mlog_id,"mbtrnpp: input socket reconnect failed status[%s]\n",mb1r_strstate(reader->state));
                MST_COUNTER_INC(app_stats->stats->events[MBTPP_EV_EMBCON]);

                struct timespec twait={0},trem={0};
                twait.tv_sec=5;
                nanosleep(&twait,&trem);
            }
        }

        MST_METRIC_LAP(app_stats->stats->metrics[MBTPP_CH_MB_GETFAIL_XT], mtime_dtime());
    }

    // print output debug statements
    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       size:       %zu\n", *size);
        fprintf(stderr, "dbg2       buffer:     %p\n", buffer);
        fprintf(stderr, "dbg2       error:              %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:             %d\n", status);
    }

    return (status);
}

int mbtrnpp_mb1r_input_close(int verbose, void *mbio_ptr, int *error)
{
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
     * mbtrnpp_mb1r_input_read(). Deallocate the internal, hidden buffer and any
     * other resources that were allocated by mbtrnpp_reson7kr_input_init(). */
    mb1r_reader_t *reader = (mb1r_reader_t *)mb_io_ptr->mbsp;
    mb1r_reader_destroy(&reader);
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
#endif // WITH_MB1_READER
