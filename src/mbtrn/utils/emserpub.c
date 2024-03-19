#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <ctype.h>

#include "mframe.h"
#include "mlist.h"
#include "mfile.h"
//#include "mlog.h"
#include "mxdebug.h"
#include "mxd_app.h"

#if defined(__CYGWIN__)
#define WIN_DECLSPEC __declspec(dllimport)
#else
#define WIN_DECLSPEC
#endif

// default IO buffer size
#define IBUF_BYTES_DFL 4096

// XON/XOFF doesn't make sense with binary data
#define EMS_WITH_XONXOFF 0
#define XON 0x11
#define XOFF 0x13


// app configuration
typedef struct app_cfg_s {
    int verbose;
    char *ser_device;
    unsigned int ser_baud;
    unsigned int ser_delay_us;
    int flow;
    uint64_t ibuf_sz;
    unsigned char *ibuf;
    mlist_t *file_paths;
}app_cfg_t;

// app state
typedef struct app_ctx_s {
    FILE *fp;
    int fd;
    int64_t total_rbytes;
    int64_t total_wbytes;
    int64_t burst_count;
    long fend;
    bool tx_flag;
    bool quit_flag;

} app_ctx_t;

static void s_parse_args(int argc, char **argv, app_cfg_t *cfg);
static void s_show_help();
static void s_termination_handler (int signum);
static app_cfg_t *app_cfg_new();
static void app_cfg_destroy(app_cfg_t **pself);
static void app_cfg_show(app_cfg_t *self);
static app_ctx_t *app_ctx_new();
static void app_ctx_destroy(app_ctx_t **pself);
static int init_ctx(app_ctx_t *ctx, app_cfg_t *cfg);
static void config_serial(int fd, app_cfg_t *cfg);
static bool cts_is_set(int fd);
static bool wait_flow_on(app_ctx_t *ctx, app_cfg_t *cfg);
static bool check_flow_on(app_ctx_t *ctx, app_cfg_t *cfg);
static int write_data(app_ctx_t *ctx, app_cfg_t *cfg);

// user interrupt flag (SIGINT)
static bool g_interrupt=false;

// parse command line options
static void s_parse_args(int argc, char **argv, app_cfg_t *cfg)
{
    extern char WIN_DECLSPEC *optarg;
    int option_index;
    int c;
    bool help=false;
    bool version=false;

    static struct option options[] = {
        {"verbose", required_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {"device", required_argument, NULL, 'd'},
        {"baud", required_argument, NULL, 'b'},
        {"delay", required_argument, NULL, 'D'},
        {"flow", required_argument, NULL, 'f'},
        {"ibuf", required_argument, NULL, 'i'},
        {NULL, 0, NULL, 0}};

    // process argument list
    while ((c = getopt_long(argc, argv, "hd:b:D:f:i:v:", options, &option_index)) != -1){
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
                // port
                else if (strcmp("device", options[option_index].name) == 0) {
                    free(cfg->ser_device);
                    cfg->ser_device = strdup(optarg);
                }
                // baud
                else if (strcmp("baud", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->ser_baud);
                }
                // delay
                else if (strcmp("delay", options[option_index].name) == 0) {
                    sscanf(optarg,"%u",&cfg->ser_delay_us);
                }
                // flow
                else if (strcmp("flow", options[option_index].name) == 0) {
                    int flow=0;
                    sscanf(optarg,"%c", (char *)&flow);
                    if(toupper(flow) == 'N')
                        cfg->flow = 'N';
                    else if(toupper(flow) == 'R')
                        cfg->flow = 'R';
#if EMS_WITH_XONXOFF
                    else if(toupper(flow) == 'X')
                        cfg->flow = 'X';
#endif
                    else
                        fprintf(stderr, "WARN: flow control (%c) not supported\n", flow);
                }
                // ibuf
                else if (strcmp("ibuf", options[option_index].name) == 0) {
                    uint64_t x = 0;
                    if(sscanf(optarg,"%llu", &x) == 1 && x > 0){
                        cfg->ibuf_sz = x;

                        unsigned char *bp = (unsigned char *)realloc(cfg->ibuf, x);
                        memset(bp, 0, x);

                        cfg->ibuf = bp;
                    }

                }
            case 'v':
                sscanf(optarg,"%d",&cfg->verbose);
                break;
            case 'h':
                help = true;
                break;
            case 'd':
                free(cfg->ser_device);
                cfg->ser_device = strdup(optarg);
                break;
            case 'b':
                sscanf(optarg,"%u",&cfg->ser_baud);
               break;
            case 'D':
                sscanf(optarg,"%u",&cfg->ser_delay_us);
                break;
            case 'f':
            {
                int flow=0;
                sscanf(optarg,"%c", (char *)&flow);
                if(toupper(flow) == 'N')
                    cfg->flow = 'N';
                else if(toupper(flow) == 'R')
                    cfg->flow = 'R';
#if EMS_WITH_XONXOFF
                else if(toupper(flow) == 'X')
                    cfg->flow = 'X';
#endif
                else
                    fprintf(stderr, "WARN: flow control (%c) not supported\n", flow);
            }
                break;
            case 'i':
            {
                uint64_t x = 0;
                if(sscanf(optarg,"%llu", &x) == 1 && x > 0){
                    cfg->ibuf_sz = x;

                    unsigned char *bp = (unsigned char *)realloc(cfg->ibuf, x);
                    memset(bp, 0, x);

                    cfg->ibuf = bp;
                }
            }
                break;
            default:
                break;
        }
    }

    for (int i=optind; i<argc; i++) {
        mlist_add(cfg->file_paths,strdup(argv[i]));
    }

    if(help){
        s_show_help();
        app_cfg_destroy(&cfg);
        exit(0);
    }
    return;
}

// show help message
static void s_show_help()
{
    char help_message[] = "\n publish em710 UDP capture data to serial port (emulate M3 serial output)\n";
    char usage_message[] = "\n emserpub [options] file [file...]\n"
    "\n Options:\n"
    "  -v, --verbose=n : verbose output level\n"
    "  -h, --help      : show this help message\n"
    "  -d, --device=s  : serial port device\n"
    "  -b, --baud=u    : serial comms rate\n"
    "  -f, --flow=c    : serial flow control (N: none R: RTS/CTS)\n"
    "  -i. --ibuf=u    : inbuf size (bytes)\n"
    "  -D, --delay=u   : interchacter delay (usec)\n"
    "\n";
    printf("%s",help_message);
    printf("%s",usage_message);
}

// signal handler
static void s_termination_handler (int signum)
{
    switch (signum) {
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            g_interrupt=true;
            break;
        default:
//            MX_ERROR_MSG("not handled[%d]\n",signum);
            break;
    }
}

// allocate app_cfg_t resources
static app_cfg_t *app_cfg_new()
{
    app_cfg_t *new_cfg = (app_cfg_t *)malloc(sizeof(app_cfg_t));
    if ( new_cfg != NULL) {

        new_cfg->verbose = 0;
        new_cfg->ser_device = strdup("/dev/ttyUSB0");
        new_cfg->ser_baud = 115200;
        new_cfg->ser_delay_us = 0;
        new_cfg->flow = 'R';
        new_cfg->ibuf_sz = IBUF_BYTES_DFL;
        new_cfg->ibuf = (unsigned char *)malloc(IBUF_BYTES_DFL);
        if(new_cfg->ibuf != NULL){
            memset(new_cfg->ibuf, 0, new_cfg->ibuf_sz);
        } else {
            fprintf(stderr, "ibuf alloc failed %p len %d %d/%s\n", new_cfg->ibuf, IBUF_BYTES_DFL, errno, strerror(errno));
            exit(-1);
        }
        new_cfg->file_paths = mlist_new();
        mlist_autofree(new_cfg->file_paths, free);
    }
    return new_cfg;
}

// release app_cfg_t resources
static void app_cfg_destroy(app_cfg_t **pself)
{
    if(pself != NULL) {
        app_cfg_t *self = *pself;
        if (self != NULL) {

            mlist_destroy(&self->file_paths);
            free(self->ser_device);
            free(self->ibuf);
            free(self);
        }
        *pself = NULL;
    }
}

// show app_cfg_t
static void app_cfg_show(app_cfg_t *self)
{
    fprintf(stderr,"\n");
    fprintf(stderr,"device    %s\n", self->ser_device);
    fprintf(stderr,"baud      %u\n", self->ser_baud);
    fprintf(stderr,"flow      %c\n", self->flow);
    fprintf(stderr,"delay_us  %u\n", self->ser_delay_us);
    fprintf(stderr,"ibuf_sz   %llu\n", self->ibuf_sz);
    fprintf(stderr,"verbose   %d\n", self->verbose);
    fprintf(stderr,"files:\n");
    char *path = (char *)mlist_first(self->file_paths);
    int i = 0;
    while (NULL != path) {
        fprintf(stderr, "[%3d]      %s\n", i++, path);
        path = (char *)mlist_next(self->file_paths);
    }
    fprintf(stderr,"\n");
}

// allocate app_ctx_t resources
static app_ctx_t *app_ctx_new()
{
    app_ctx_t *instance = (app_ctx_t *)malloc(sizeof(app_ctx_t));
    if(instance != NULL){
        instance->fp = NULL;
        instance->fd = -1;
        instance->total_rbytes = 0;
        instance->total_wbytes = 0;
        instance->burst_count = 0;
        instance->fend = 0;
        instance->tx_flag = true;
        instance->quit_flag = false;
    }
    return instance;
}

// release app_ctx_t resources
static void app_ctx_destroy(app_ctx_t **pself)
{
    if(pself != NULL){
        app_ctx_t *self = *pself;
        if(self != NULL){
            if(self->fp != NULL)
                fclose(self->fp);
            close(self->fd);
        }
        free(self);
        *pself = NULL;
    }
}

// initialze state
static int init_ctx(app_ctx_t *ctx, app_cfg_t *cfg)
{
    int retval = 0;
    // open output port
    ctx->fd = open(cfg->ser_device, O_RDWR|O_NOCTTY);

    if(ctx->fd < 0){
        fprintf(stderr, "could not open %s %d/%s\n", cfg->ser_device, errno, strerror(errno));
        return -1;
    }

    config_serial(ctx->fd, cfg);

    return retval;
}

// configure serial terminal
static void config_serial(int fd, app_cfg_t *cfg)
{
    struct termios tty;
    if(tcgetattr(fd, &tty) != 0) {
        fprintf(stderr, "Error %i from tcgetattr: %s\n", errno, strerror(errno));
    }

    cfmakeraw(&tty);

    if(cfg->flow == 'R') {
        tty.c_cflag |= CRTSCTS; // Disable RTS/CTS hardware flow control
    } else if(cfg->flow == 'X') {
        tty.c_iflag |= (IXON); // Enable s/w flow ctrl input
        tty.c_iflag |= (IXOFF); // Enable s/w flow ctrl output
        tty.c_iflag &= ~(IXANY); // Enable s/w flow ctrl
        tty.c_cc[VSTART] = XON;
        tty.c_cc[VSTOP] = XOFF;
        tty.c_cc[VTIME] = 1;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;
    }

#if 0
    tty.c_cflag &= ~(CSIZE|PARENB); // Clear parity bit
    tty.c_cflag &= ~CSTOPB; // Clear stop field (one stop bit)
    tty.c_cflag |= CS8;     // 8 bits per byte
    tty.c_cflag |= CREAD; // Turn on READ & ignore ctrl lines
    tty.c_cflag |= CLOCAL; // Turn on READ & ignore ctrl lines
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_lflag &= ~IEXTEN; // Disable implementation-defined input processing
    //    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Enable s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
    // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT IN LINUX)
    // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT IN LINUX)
    tty.c_cc[VTIME] = 0;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    if(cfg->flow == 'X'){
        tty.c_iflag &= (IXON); // Enable s/w flow ctrl
        tty.c_iflag &= ~(IXOFF | IXANY); // Enable s/w flow ctrl
        tty.c_cc[VSTOP] = XOFF;
        tty.c_cc[VSTART] = XON;
    }
#endif


    // Set in/out baud rate
    switch(cfg->ser_baud){
        case 1200:
            cfsetispeed(&tty, B1200);
            cfsetospeed(&tty, B1200);
            break;
        case 1800:
            cfsetispeed(&tty, B1800);
            cfsetospeed(&tty, B1800);
            break;
        case 2400:
            cfsetispeed(&tty, B2400);
            cfsetospeed(&tty, B2400);
            break;
        case 4800:
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
            break;
        case 9600:
            cfsetispeed(&tty, B9600);
            cfsetospeed(&tty, B9600);
            break;
        case 19200:
            cfsetispeed(&tty, B19200);
            cfsetospeed(&tty, B19200);
            break;
        case 38400:
            cfsetispeed(&tty, B38400);
            cfsetospeed(&tty, B38400);
            break;
        case 57600:
            cfsetispeed(&tty, B57600);
            cfsetospeed(&tty, B57600);
            break;
        case 115200:
            cfsetispeed(&tty, B115200);
            cfsetospeed(&tty, B115200);
            break;
        default:
            fprintf(stderr, "ERR - invalid ser_baud %u\n", cfg->ser_baud);
            break;
    };

    if(tcsetattr(fd, TCSANOW, &tty) != 0)
        fprintf(stderr, "ERR - tcsetattr failed %d/%s\n", errno, strerror(errno));

    return;
}

// return true of CTS is set
static bool cts_is_set(int fd)
{
    int modstat=0;
    if(ioctl(fd, TIOCMGET, &modstat) != 0)
        fprintf(stderr, "ERR TIOCMGET- %d/%s\n", errno, strerror(errno));

    return ((modstat&TIOCM_CTS) != 0);
}

// wait for flow control to enable output
static bool wait_flow_on(app_ctx_t *ctx, app_cfg_t *cfg)
{

    // monitor flow control, enable output on start
    if(cfg->flow == 'R'){

        // wait for CTS
        while( !g_interrupt){

            if(cts_is_set(ctx->fd)) {

                if(cfg->verbose >= 1)
                    fprintf(stderr, "\nENABLE TX (CTS)\n");
                ctx->tx_flag = true;
                ctx->burst_count = 0;
                return true;
            }
            usleep(10000);
        }
    }
#if EMS_WITH_XONXOFF
    else if(cfg->flow == 'X'){
        while(!g_interrupt){
            unsigned char flow_stat = 0;
            if(read(fd, &flow_stat, 1) == 1 && flow_stat == XON){
                if(cfg->verbose >= 1)
                    fprintf(stderr, "\nENABLE TX (XON)\n");
                tcflush(fd, TCIFLUSH);

                ctx->tx_flag = true;
                ctx->burst_count = 0;
                return true;
            }
            usleep(10000);
        }
    }
#endif

    return false;
}

// check flow control, return false on stop
static bool check_flow_on(app_ctx_t *ctx, app_cfg_t *cfg)
{
    // monitor flow control, disable output on stop
    if(cfg->flow == 'R'){
        // check CTS, stop sending if asserted
        if(!cts_is_set(ctx->fd)){
            if(cfg->verbose >= 1)
                fprintf(stderr, "\nDISABLE TX (CTS)\n");
            ctx->tx_flag = false;
            return false;
        }
    }
#if EMS_WITH_XONXOFF
    else if(cfg->flow == 'X'){
        unsigned char flow_stat = 0;
        if(read(ctx->fd, &flow_stat, 1) == 1 && flow_stat == XOFF){
            if(cfg->verbose >= 1)
                fprintf(stderr, "\nDISABLE TX (XOFF)\n");
            tcflush(ctx->fd, TCIFLUSH);
            ctx->tx_flag = false;
            return false;
        }
    }
#endif

    ctx->tx_flag = true;
    return true;
}

// read from input file and write to serial port
// and delay (if > 0)
// transfer size set by IO buffer size (ibuf)
static int write_data(app_ctx_t *ctx, app_cfg_t *cfg)
{
    int retval = 0;
    static uint64_t obytes = 0;

    if(!ctx->tx_flag)
        return 0;

    // read byte(s) from input file
    size_t rbytes = fread(cfg->ibuf, 1, (size_t)cfg->ibuf_sz, ctx->fp);

    if( rbytes > 0) {
        ctx->total_rbytes += rbytes;
        ctx->burst_count += rbytes;

        unsigned char *op = cfg->ibuf;
        ssize_t rem_bytes = rbytes;
        while(rem_bytes > 0){
            // write byte(s) to output (should block until sent)
            ssize_t wb = write(ctx->fd, op, rem_bytes);
            tcdrain(ctx->fd);

            if(wb > 0) {
                ctx->total_wbytes += wb;
                rem_bytes -= wb;
                op += wb;

                if(wb < rbytes){
                    fprintf(stderr, "\nWARN - write returned %zd/%zd\n", wb, rbytes);
                }
            } else{
                fprintf(stderr, "\nERR - write returned %zd ibuf %p len %llu %d/%s\n", wb, cfg->ibuf, cfg->ibuf_sz, errno, strerror(errno));
            }
        }

        // display bytes
        if(cfg->verbose >= 4 ){
            for(int i = 0; i < rbytes; i++){
                if((obytes % 16) == 0)
                    fprintf(stderr, "\n%08llx: ", obytes);
                fprintf(stderr, "%02X ", cfg->ibuf[i]);
                obytes++;
            }
        }

        // delay per configuration
        if(cfg->ser_delay_us > 0)
            usleep(cfg->ser_delay_us);

    } else {
        fprintf(stderr, "ERR - fread returned %zd feof %d ferr %d %d/%s\n",  rbytes, feof(ctx->fp), ferror(ctx->fp), errno, strerror(errno));
        if(feof(ctx->fp)){
            ctx->quit_flag = true;
            retval = -1;
        }

        clearerr(ctx->fp);
    }
    return retval;
}

int main(int argc, char **argv)
{
    // configure signal handling
    struct sigaction saStruct;
    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = s_termination_handler;
    sigaction(SIGINT, &saStruct, NULL);

    // get default configuration
    app_cfg_t *cfg = app_cfg_new();

    // parse options
    s_parse_args(argc, argv, cfg);
    app_cfg_show(cfg);

    // get context, initilize
    app_ctx_t *ctx = app_ctx_new();

    if(init_ctx(ctx, cfg) == 0){

        // if context valid, process input file

        if(cfg->verbose > 0 ){
            fprintf(stderr, "%s output device %s connected fd %d %u bps\n", __func__, cfg->ser_device, ctx->fd, cfg->ser_baud);
        }

        // iterate over input file path list
        char *path = (char *)mlist_first(cfg->file_paths);
        
        while (NULL != path) {

            // open next input file
            ctx->fp = fopen(path, "r");

            if(ctx->fp != NULL){
                // get file size, end pointer
                fseek(ctx->fp, 0, SEEK_END);
                ctx->fend = ftell(ctx->fp);
                fseek(ctx->fp, 0, SEEK_SET);

                if(cfg->verbose > 0 ){
                    fprintf(stderr, "%s input file %s open fp %p\n", __func__, path, ctx->fp);
                    fprintf(stderr, "%s ftell %ld fend %ld\n", __func__, ftell(ctx->fp), ctx->fend);
                }
            } else {
                fprintf(stderr, "ERR - fopen failed file %s %d/%s\n", path, errno, strerror(errno));
            }

            // process file
            while(ctx->fp != NULL && !ctx->quit_flag && !g_interrupt) {

                memset(cfg->ibuf, 0, cfg->ibuf_sz);

                // quit if end of input
                if(ftell(ctx->fp) >= ctx->fend)
                    break;

                // wait for flow control enable
                wait_flow_on(ctx, cfg);

                while(ctx->tx_flag && !g_interrupt && !ctx->quit_flag){

                    // quit if end of input
                    if(ftell(ctx->fp) >= ctx->fend)
                        break;

                    // check flow control, disable output on stop
                    check_flow_on(ctx, cfg);

                    // do output when enabled
                    write_data(ctx, cfg);
                }
            }

            // close file, get next
            if(ctx->fp != NULL)
                fclose(ctx->fp);

            path = (char *)mlist_next(cfg->file_paths);
        }

    } else {
        fprintf(stderr, "ERR - init_ctx failed; %d/%s\n", errno, strerror(errno));
    }

    if(cfg->verbose > 0 ){
        fprintf(stderr, "\n read %llu/%08llX wrote %llu/%08llX bytes\n", ctx->total_rbytes, ctx->total_rbytes, ctx->total_wbytes, ctx->total_wbytes);
    }

    app_cfg_destroy(&cfg);
    app_ctx_destroy(&ctx);

    return 0;
}
