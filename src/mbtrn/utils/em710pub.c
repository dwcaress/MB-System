
#include <arpa/inet.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mbsys_simrad3.h"

#include "mframe.h"
#include "mlist.h"
#include "mfile.h"
#include "mlog.h"
#include "mxdebug.h"
#include "mxd_app.h"


typedef enum {
    EM710_INVALID = 0,
    EM710_UDP,
    EM710_LOG
}data_fmt_t;

typedef struct app_cfg_s
{
    mlist_t *file_paths;
    mlist_t *file_list;
    char *host;
    int port;
    int sock_fd;
    int verbose;
    data_fmt_t fmt;
    uint32_t start_offset;
    unsigned long delay_ms;
    struct sockaddr_in svr_addr;
    struct sockaddr_in cli_addr;
    socklen_t cli_len;
    bool is_udp;
}app_cfg_t;


static void s_show_help();
static app_cfg_t *s_cfg_new();
static void s_cfg_destroy(app_cfg_t **pself);
static void s_cfg_show(app_cfg_t *self);
static void s_parse_args(int argc, char **argv, app_cfg_t *cfg);


static void s_show_help()
{
    char help_message[] = "\n publish em710 raw UDP data\n";
    char usage_message[] = "\n em710pub [options] file [file...]\n"
    "\n Options:\n"
    "  --verbose=n    : verbose output level\n"
    "  --help         : show this help message\n"
    "  --host=s       : host IP address or name\n"
    "  --port=n       : TCP/IP port\n"
    "  --format=s     : log or udp\n"
    "  --delay=n      : delay (msec)\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}

static app_cfg_t *s_cfg_new()
{
    app_cfg_t *new_cfg = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    if ( new_cfg != NULL) {
        new_cfg->file_paths = mlist_new();
        new_cfg->file_list = mlist_new();
        mlist_autofree(new_cfg->file_paths, free);

        new_cfg-> host = strdup("127.0.0.1");
        new_cfg->port = 10001;
        new_cfg->verbose = 0;
        new_cfg->sock_fd = -1;
        new_cfg->fmt = EM710_LOG;
        new_cfg->start_offset = 0;
        new_cfg->delay_ms = 0;
        // must initialize addr len before recv,
        // else it will not fill in and can't send message to cli
        memset(&new_cfg->svr_addr, 0, sizeof(new_cfg->svr_addr));
        memset(&new_cfg->cli_addr, 0, sizeof(new_cfg->cli_addr));
        new_cfg->cli_len = sizeof(new_cfg->cli_addr);
    }
    return new_cfg;
}

static void s_cfg_destroy(app_cfg_t **pself)
{
    if(pself != NULL) {
        app_cfg_t *self = *pself;
        if (self != NULL) {

            mfile_file_t *file = (mfile_file_t *)mlist_first(self->file_list);
            while (NULL!=file) {
                mfile_file_destroy(&file);
                file = (mfile_file_t *)mlist_next(self->file_list);
            }
            mlist_destroy(&self->file_list);

            mlist_destroy(&self->file_paths);
            free(self->host);
            free(self);
        }
        *pself = NULL;
    }
}

static void s_cfg_show(app_cfg_t *self)
{
    fprintf(stderr,"verbose   %d\n", self->verbose);
    fprintf(stderr,"host      %s\n", self->host);
    fprintf(stderr,"port      %d\n", self->port);
    const char *fmt;
    if(self->fmt == EM710_LOG)
        fmt = "EM710_LOG";
    else if(self->fmt == EM710_UDP)
        fmt = "EM710_UDP";
    else
        fmt = "unknown";

    fprintf(stderr,"format    %s\n", fmt) ;
    fprintf(stderr,"delay_ms  %lu\n", self->delay_ms);
    fprintf(stderr,"paths     %p\n", self->file_paths);
    fprintf(stderr,"fd        %d\n", self->sock_fd);
    fprintf(stderr,"files:\n");
    char *path = (char *)mlist_first(self->file_paths);
    int i = 0;
    while (NULL != path) {
        fprintf(stderr, "[%3d]      %s\n", i++, path);
        path = (char *)mlist_next(self->file_paths);
    }

}

static void s_parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;

    static struct option options[] = {
        {"verbose", required_argument, NULL, 0},
        {"help", no_argument, NULL, 0},
        {"host", required_argument, NULL, 0},
        {"port", required_argument, NULL, 0},
        {"format", required_argument, NULL, 0},
        {"delay", required_argument, NULL, 0},
        {NULL, 0, NULL, 0}};

    // process argument list
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1){
        switch (c) {
                // long options all return c=0
            case 0:
                // verbose
                if (strcmp("verbose", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->verbose);
                }
                // help
                else if (strcmp("help", options[option_index].name) == 0) {
                    help = true;
                }
                // host
                else if (strcmp("host", options[option_index].name) == 0) {
                    if (NULL!=cfg->host) {
                        free(cfg->host);
                    }
                    cfg->host=strdup(optarg);
                }
                // port
                else if (strcmp("port", options[option_index].name) == 0) {
                    sscanf(optarg,"%d",&cfg->port);
                }
                // format
                else if (strcmp("format", options[option_index].name) == 0) {
                    if(strcasecmp(optarg,"log") == 0) {
                        cfg->fmt = EM710_LOG;
                    } else if(strcasecmp(optarg,"udp") == 0) {
                        cfg->fmt = EM710_UDP;
                    } else {
                        fprintf(stderr, "ERR - invalid format %s; use log or udp\n", optarg);
                    }
                }
                // delay
                else if (strcmp("delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%lu",&cfg->delay_ms);
                }
        }
    }

    for (int i=optind; i<argc; i++) {
        mlist_add(cfg->file_paths,strdup(argv[i]));
    }

    if (mlist_size(cfg->file_paths) > 0) {
        char *file_path = (char *)mlist_first(cfg->file_paths);
        while (file_path != NULL) {
            mfile_file_t *file = mfile_file_new(file_path);
            mlist_add(cfg->file_list, file);
            file_path = (char *)mlist_next(cfg->file_paths);
        }
    }else{
        fprintf(stderr,"ERR - no input files\n");
    }

}

static void header_show(struct mbsys_simrad3_header *header)
{
    fprintf(stderr, "numBytesDgm      %08u/x%08X\n", header->numBytesDgm, header->numBytesDgm);
    fprintf(stderr, "dgmSTX           %02X\n", header->dgmSTX);
    fprintf(stderr, "dgmType          %02X/%c\n", header->dgmType, header->dgmType);
    fprintf(stderr, "emModeNum        %04hu/x%04X\n", header->emModeNum, header->emModeNum);
    fprintf(stderr, "date             %08u/x%08X\n", header->date, header->date);
    fprintf(stderr, "timeMs           %08u/x%08X\n", header->timeMs, header->timeMs);
    fprintf(stderr, "counter          %04hu/x%04X\n", header->counter, header->counter);
    fprintf(stderr, "sysSerialNum     %04hu/x%04X\n", header->sysSerialNum, header->sysSerialNum);
    fprintf(stderr, "secHeadSerialNum %04hu/x%04X\n", header->secHeadSerialNum, header->secHeadSerialNum);
    byte *bp = (byte *)header;
    byte *petx = (bp + (header->numBytesDgm + 1));
    byte *pchk = (petx+1);
    fprintf(stderr, "dgmETX           %02X\n", *((unsigned char *)petx));
    fprintf(stderr, "chksum           %04X\n", *((unsigned short *)pchk));
    fprintf(stderr, "\nframe bytes:\n");
    for (int i=0;i<header->numBytesDgm + 4;i++)
    {
        if(i%16 == 0)
            fprintf(stderr, "\n%08X: ",i);
        fprintf(stderr, "%02x ",bp[i]);
    }
    fprintf(stderr, "\n\n");
}

static bool validate(byte *src, unsigned int len, app_cfg_t *cfg)
{
    bool retval = true;
    struct mbsys_simrad3_header *header = (struct mbsys_simrad3_header *)src;

    if(cfg->verbose > 1)
        header_show(header);

    byte *petx = src + header->numBytesDgm + 1;
    byte *psum = (byte *)&header->dgmSTX + 1;

    switch(header->dgmType){

        case ALL_INSTALLATION_U:
        case ALL_INSTALLATION_L:
        case ALL_REMOTE:
        case ALL_RUNTIME:
        case ALL_RAW_RANGE_BEAM_ANGLE:
        case ALL_XYZ88:
        case ALL_CLOCK:
        case ALL_ATTITUDE:
        case ALL_POSITION:
        case ALL_SURFACE_SOUND_SPEED:
            break;
        default:
            fprintf(stderr, "%s - ERR: invalid type %02x\n", __func__, header->dgmType);
            return false;
            break;
    };

    if(header->dgmSTX !=  EM3_START_BYTE) {
        fprintf(stderr, "%s - ERR: invalid STX %02X/%02X\n", __func__, header->dgmSTX, EM3_START_BYTE);
        return false;
    }

    if(*petx !=  EM3_END_BYTE) {
        fprintf(stderr, "%s - ERR: invalid ETX %02X/%02X len(%u)\n", __func__, *petx, EM3_END_BYTE, len);


        return false;
    }
    short unsigned int sum = 0;
    short unsigned int chk =  *((short unsigned int *)(petx + 1));

    while(psum < petx){
        sum += *psum;
        psum++;
    }
    if(sum != chk){
        fprintf(stderr, "%s - ERR: invalid checksum %04hu/%04hu\n", __func__, sum, chk);
        return false;
    }

    return retval;
}

static void pub_file(mfile_file_t *src, app_cfg_t *cfg)
{

    static byte *frame_buf = NULL;

    if (frame_buf == NULL) {
        frame_buf = (byte *)malloc(MB_UDP_SIZE_MAX);
    }

    if(frame_buf != NULL) {
        memset(frame_buf, 0, MB_UDP_SIZE_MAX);

        uint32_t start_offset = cfg->start_offset;

        off_t file_end = mfile_seek(src, 0, MFILE_END);

        if ((off_t)start_offset >= file_end) {
            mfile_seek(src, file_end, MFILE_SET);
            start_offset -= file_end;
        }else{
            mfile_seek(src, start_offset, MFILE_SET);
            start_offset=0;
        }
        off_t file_cur = mfile_seek(src, 0, MFILE_CUR);
        off_t fb_cur = 0;

        if(cfg->fmt == EM710_LOG) {

            struct mbsys_simrad3_header *header = (struct mbsys_simrad3_header *)frame_buf;

            while(file_cur < file_end) {
                memset(frame_buf, 0, MB_UDP_SIZE_MAX);
                fb_cur = 0;
                uint32_t read_len = 4;
                int64_t rbytes = 0;
                off_t file_cur_save = file_cur;
                // read datagram size (4 bytes)
                if( (rbytes = mfile_read(src, frame_buf, read_len)) == read_len) {
                    fb_cur += read_len;
                    // read the rest of the datagram
                    read_len = header->numBytesDgm;
                    if( (rbytes = mfile_read(src, frame_buf + fb_cur, read_len)) == read_len) {
                        fb_cur += read_len;

                        if(cfg->verbose > 1){
                            fprintf(stderr, "\n");
                            header_show(header);
                        }
                        if(validate(frame_buf, (header->numBytesDgm + 4), cfg)){
                            size_t send_len = header->numBytesDgm;
                            ssize_t status = send(cfg->sock_fd, frame_buf+4, send_len, 0);
                            if( status != send_len){
                                fprintf(stderr, "ERR - send failed ret[%zd] %d/%s\n", status, errno, strerror(errno));
                            }

                        }
                    }
                    if(cfg->delay_ms > 0){
                        struct timespec delay_ms = {
                            (time_t)(cfg->delay_ms / 1000),
                            (long)(1000L * (cfg->delay_ms % 1000))
                        };
                        nanosleep(&delay_ms, NULL);
                    }
                }
                file_cur = mfile_seek(src, 0, MFILE_CUR);
            }
        } else if(cfg->fmt == EM710_UDP) {

            struct mbsys_simrad3_header *header = (struct mbsys_simrad3_header *)frame_buf;

            typedef enum {
                ST_START = 0,
                ST_STX_VALID,
                ST_TYPE_VALID,
                ST_MODEL_VALID,
                ST_ETX_VALID,
                ST_CHKSUM_VALID,
                ST_ERROR,
                ST_COUNT
            }state_t;
            char *st_names[ST_COUNT] = {
                "ST_START",
                "ST_STX_VALID",
                "ST_TYPE_VALID",
                "ST_MODEL_VALID",
                "ST_ETX_VALID",
                "ST_CHKSUM_VALID",
                "ST_ERROR"
            };
            state_t state = ST_START;

            int64_t rbytes = 0;
            size_t dgram_bytes = 0;
            byte *bp = frame_buf + 4;
            byte *pstx = frame_buf + 5;
            byte *petx = NULL;
            bool found_stx = false;
            bool found_type = false;
            bool found_model = false;
            bool found_etx = false;
            bool found_chk = false;
            uint32_t read_len = 1;
            uint64_t fpos_start = 0;
            unsigned short int chksum = 0;

            while(file_cur < file_end) {

                // read until STX

                if(state == ST_START) {
                    memset(frame_buf, 0, MB_UDP_SIZE_MAX);
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
                    // initialize state
                    rbytes = 0;
                    dgram_bytes = 0;
                    chksum = 0;
                    bp = frame_buf + 4;
                    fb_cur = 0;
                    found_etx = false;
                    found_chk = false;
                    found_stx = false;
                    found_type = false;
                    found_model = false;
                    //fpos_start = mfile_seek(src, 0, MFILE_CUR);
                    mfile_seek(src, fpos_start, MFILE_SET);
                    MX_BPRINT( (cfg->verbose > 0), "file_pos %llu/x%0llX\n", fpos_start, fpos_start);

                    // find STX (datagram start)

                    uint64_t skipped = 0;
                    while(!found_stx) {
                        read_len = 1;
                        rbytes = mfile_read(src, bp, read_len);

                        if(rbytes > 0)
                            file_cur += rbytes;

                        if(rbytes == read_len){
                            if( *bp == EM3_START_BYTE){
                                int64_t fpos = mfile_seek(src, 0, MFILE_CUR) - read_len;
//                                fprintf(stderr, "found STX %02X file_pos x%0llX bp %p ofs %zd\n", *bp, fpos, bp, (bp - frame_buf));
                                dgram_bytes = read_len;
                                pstx = bp;
                                found_stx = true;
                                bp += read_len;
                                state = ST_STX_VALID;
                                fpos_start = mfile_seek(src, 0, MFILE_CUR);
                                MX_BPRINT( (cfg->verbose > 0),"(skipped %llu bytes)\n", skipped);
                            } else {
                                skipped++;
                            }
                        } else {
                            fprintf(stderr, "ERR - file read failed on STX\n");
                            state = ST_ERROR;
                            break;
                        }
                    }
                } // state START

                if(state == ST_STX_VALID) {
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
                   // find type
                    read_len = 1;
                    rbytes = mfile_read(src, bp, read_len);

                    if(rbytes == read_len) {
                        switch(*bp){
                            case ALL_INSTALLATION_U:
                            case ALL_INSTALLATION_L:
                            case ALL_REMOTE:
                            case ALL_RUNTIME:
                            case ALL_RAW_RANGE_BEAM_ANGLE:
                            case ALL_XYZ88:
                            case ALL_CLOCK:
                            case ALL_ATTITUDE:
                            case ALL_POSITION:
                            case ALL_SURFACE_SOUND_SPEED:
                                found_type = true;
                                break;
                            default:
                                fprintf(stderr, "ERR - invalid type %02X bp=%p fp=%p\n", *bp, bp, frame_buf);
                                break;
                        };
                    } else {
                        fprintf(stderr, "ERR - file read failed on STX\n");
                        state = ST_ERROR;
                     }

                    if(state != ST_ERROR){
                       if(found_type) {
                           int64_t fpos = mfile_seek(src, 0, MFILE_CUR) - read_len;
                           MX_BPRINT( (cfg->verbose > 1), "found TYPE %02X file_pos x%0llX bp %p ofs %zd\n", *bp, fpos, bp, (bp - frame_buf));

                            bp += read_len;
                            dgram_bytes += read_len;
                            state = ST_TYPE_VALID;
                        } else {
                            // invalid type, reset just after original start position
//                            fpos_start++;
                            state = ST_START;
                        }
                    }
                }

                if(state == ST_TYPE_VALID) {
                    // find model
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
//                    fprintf(stderr, "type %02X/%c\n", header->dgmType, header->dgmType);

                    read_len = 2;
                    rbytes = mfile_read(src, bp, read_len);

                    if(rbytes == read_len) {
                        short unsigned *model = (short unsigned *)bp;
                        switch(*model){
                            case 0x1E:
                            case 0x1ED:
                                found_model = true;
                                break;
                            default:
                                fprintf(stderr, "ERR - invalid model %04X bp=%p fp=%p\n", *model, bp, frame_buf);
                                break;
                        };
                    } else {
                        fprintf(stderr, "ERR - file read failed on MODEL\n");
                        state = ST_ERROR;
                    }

                    if(state != ST_ERROR){
                        if(found_model) {
                            int64_t fpos = mfile_seek(src, 0, MFILE_CUR) - read_len;
                            MX_BPRINT( (cfg->verbose > 1), "found MODEL %04X file_pos x%0llX bp %p ofs %zd\n", *((unsigned short *)bp), fpos, bp, (bp - frame_buf));

                            bp += read_len;
                            dgram_bytes += read_len;
                            state = ST_MODEL_VALID;
                        } else {
                            // invalid model, reset just after original start position
                            state = ST_START;
                        }
                    }
                }

                if(state == ST_MODEL_VALID) {

                    // find ETX
                    // or exceed max datagram size
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
//                    fprintf(stderr, "frame_buf %p bp %p bp-frame_buf(%zd) fpos[%llu]\n", frame_buf, bp, (bp - frame_buf), mfile_seek(src, 0, MFILE_CUR));

                    petx = NULL;
                    read_len = 1;
                    while(!found_etx) {
                        rbytes = mfile_read(src, bp, read_len);

                        if(rbytes > 0){
                            // read/store next byte
                            if(rbytes == read_len) {
                                if(*bp == EM3_END_BYTE){
                                    int64_t fpos = mfile_seek(src, 0, MFILE_CUR) - read_len;
                                    MX_BPRINT( (cfg->verbose > 1), "found ETX %02X file_pos x%0llX bp %p ofs %zd\n", *bp, fpos, bp, (bp - frame_buf));
                                    petx = bp;

                                    found_etx = true;
                                } else {
                                    chksum += *bp;
                                }
                                bp += read_len;
                                dgram_bytes += read_len;
                            }

                        } else {
                            fprintf(stderr, "ERR - file read failed on ETX\n");
                            state = ST_ERROR;
                            break;
                        }

                        if(state != ST_ERROR){
                            if(found_etx) {
                                state = ST_ETX_VALID;
                            } else {
                                // invalid ETX, reset just after original start position
                                state = ST_START;
                            }
                        }

                        if( dgram_bytes >= MB_UDP_SIZE_MAX ||
                           (bp - frame_buf) >= MB_UDP_SIZE_MAX){
                            fprintf(stderr, "ERR - buffer length exceeded type (%02X) dgram_bytes(%zd) bp-frame_buf(%zd)\n", header->dgmType, dgram_bytes, (bp - frame_buf));
                            // invalid ETX, reset just after original start position
                            state = ST_START;
                            break;
                        }
                    }
                }

                if(state == ST_ETX_VALID) {
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
//                    fprintf(stderr, "frame_buf %p bp %p bp-frame_buf(%zd)\n", frame_buf, bp, (bp - frame_buf));

                    // read checksum
                    read_len = 2;
                    rbytes = mfile_read(src, bp, read_len);
                    unsigned int *pchk = NULL;

                    if(rbytes > 0){
                        // read/store next byte
                        if(rbytes == read_len) {
                             pchk = (unsigned int *)bp;
                            found_chk = true;
                        } else {
                            fprintf(stderr, "ERR - file read failed on CHKSUM\n");
                            state = ST_ERROR;
                        }

                        byte *pcs = pstx + 1;
                        chksum = 0;
                        while(pcs < petx){
                            chksum += *pcs;
//                            fprintf(stderr, "pcs %p petx %p *pcs %02X chksum %04X\n", pcs, petx, *pcs, chksum);
                            pcs++;
                        }

                        if(pchk != NULL && *pchk == chksum){
                            state = ST_CHKSUM_VALID;
                        } else {
                            found_chk = false;
                            found_etx = false;
                            // keep looking for ETX
                            state = ST_MODEL_VALID;
                        }
                        dgram_bytes += read_len;
                        bp += read_len;
                     } else {
                        fprintf(stderr, "ERR - file read failed on CHKSUM\n");
                        state = ST_ERROR;
                    }
                }

                if(state == ST_CHKSUM_VALID){
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
                   // done, success
                    header->numBytesDgm = dgram_bytes;
                    MX_BPRINT( (cfg->verbose > 0), "validating frame len[%zd]\n", dgram_bytes);

                    if(validate(frame_buf, dgram_bytes, cfg)){
                        size_t send_len = header->numBytesDgm;

                        ssize_t status = send(cfg->sock_fd, frame_buf + 4, send_len, 0);

                        if( status != send_len){
                            int serr = errno;
                            fprintf(stderr, "ERR - send failed ret[%zd] %d/%s\n", status, errno, strerror(errno));
                        }
                    }

                    if(cfg->delay_ms > 0){
                        struct timespec delay_ms = {
                            (time_t)(cfg->delay_ms / 1000),
                            (long)(1000000L * (cfg->delay_ms % 1000UL))
                        };
                        nanosleep(&delay_ms, NULL);
                    }
                    state = ST_START;
                }

                if(state == ST_ERROR){
                    MX_BPRINT( (cfg->verbose > 0), "state %s\n", st_names[state]);
                    // error (EOF)
                    fprintf(stderr, "ERR - EOF or buffer length exceeded; quitting\n");
                    break;
                }
            }
        } // if EM710_UDP
    }
}

int connect_udp(app_cfg_t *cfg, int *error)
{
    int status = MB_SUCCESS;

    cfg->sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (cfg->sock_fd < 0)
    {
        perror("Opening datagram socket error");

//        mlog_tprintf(mbtrnpp_mlog_id,"e,datagram socket [%d/%s]\n",errno,strerror(errno));
        status=MB_FAILURE;
        *error=MB_ERROR_OPEN_FAIL;
        return status;
    }

    // Enable SO_REUSEADDR to allow multiple instances of this
    // application to receive copies of the multicast datagrams.
    int reuse = 1;
    if (setsockopt(cfg->sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
    {
        perror("Setting SO_REUSEADDR error");
        close(cfg->sock_fd);
//        mlog_tprintf(mbtrnpp_mlog_id,"e,setsockopt SO_REUSEADDR [%d/%s]\n",errno,strerror(errno));
        status=MB_FAILURE;
        *error=MB_ERROR_OPEN_FAIL;
        return status;
    }

    // Bind to the proper port number with the IP address
    // specified as INADDR_ANY.
    memset((char *) &cfg->svr_addr, 0, sizeof(cfg->svr_addr));
    cfg->svr_addr.sin_family = AF_INET;
    cfg->svr_addr.sin_port = htons(cfg->port);
    cfg->svr_addr.sin_addr.s_addr = inet_addr(cfg->host);


    MX_BPRINT( (cfg->verbose > 0), "socket connecting fd %d addr:port %s:%d\n", cfg->sock_fd, cfg->host, cfg->port);

    char msg[16] = {0};


    if (connect(cfg->sock_fd, (struct sockaddr *)&cfg->svr_addr, sizeof(cfg->svr_addr)) < 0) {
                perror("Connecting datagram socket error");
                close(cfg->sock_fd);
        ////        mlog_tprintf(mbtrnpp_mlog_id,"e,bind [%d/%s]\n",errno,strerror(errno));
                status=MB_FAILURE;
                *error=MB_ERROR_OPEN_FAIL;
                return status;
            }

    MX_BMSG((cfg->verbose > 0), "socket connected\n");
    return status;
}

int main(int argc, char **argv)
{
    app_cfg_t *cfg = s_cfg_new();

    s_parse_args(argc, argv, cfg);

    if (cfg->verbose>0) {
        s_cfg_show(cfg);
    }

    int error = MB_ERROR_NO_ERROR;
    int status = connect_udp(cfg, &error);

    mfile_file_t *file = (mfile_file_t *)mlist_first(cfg->file_list);

    while(file != NULL){
        int status = mfile_open(file, MFILE_RONLY);
        if(status >= 0){
            pub_file(file, cfg);
            mfile_close(file);
        } else {
            fprintf(stderr, "ERR - could not open file %s stat %d %d/%s\n", file->path, status, errno, strerror(errno));
        }
        file = (mfile_file_t *)mlist_next(cfg->file_list);
    }

    close(cfg->sock_fd);

    s_cfg_destroy(&cfg);

}
