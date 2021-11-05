///
/// @file mserial.c
/// @authors k. Headley
/// @date 11 aug 2017

/// mframe serial port IO wrapper implementation

/// @sa doxygen-examples.c for more examples of Doxygen markup


/////////////////////////
// Terms of use
/////////////////////////
/*
 Copyright Information
 
 Copyright 2002-2017 MBARI
 Monterey Bay Aquarium Research Institute, all rights reserved.
 
 Terms of Use
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version. You can access the GPLv3 license at
 http://www.gnu.org/licenses/gpl-3.0.html
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details
 (http://www.gnu.org/licenses/gpl-3.0.html)
 
 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You
 assume the entire risk associated with use of the code, and you agree to be
 responsible for the entire cost of repair or servicing of the program with
 which you are using the code.
 
 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software,
 including, but not limited to, the loss or corruption of your data or damages
 of any kind resulting from use of the software, any prohibited use, or your
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss,
 liability or expense, including attorneys' fees, resulting from loss of or
 damage to property or the injury to or death of any person arising out of the
 use of the software.
 
 The MBARI software is provided without obligation on the part of the
 Monterey Bay Aquarium Research Institute to assist in its use, correction,
 modification, or enhancement.
 
 MBARI assumes no responsibility or liability for any third party and/or
 commercial software required for the database or applications. Licensee agrees
 to obtain and maintain valid licenses for any additional third party software
 required.
 */

/////////////////////////
// Headers 
/////////////////////////
#include "mserial.h"
#include "mutils.h"
#include "mlist.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "qframe"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
/// @def NOWARRANTY
/// @brief header software terms of use
#define NOWARRANTY  \
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
*/

///@var const char *MSER_PAR_STR[MSER_MAX_PAR]
///@brief parity enum string values
 const char *MSER_PAR_STR[MSER_MAX_PAR]={"N","E","O"};
///@var const char *MSER_FLOW_STR[MSER_MAX_FLOW]
///@brief flow control enum string values
 const char *MSER_FLOW_STR[MSER_MAX_FLOW]={"N","H","X"};
///@var const char *MSER_CSIZE_U32[MSER_MAX_CS]
///@brief character size (data bits) enum integer values
 uint32_t MSER_CSIZE_U32[MSER_MAX_CS]={5,6,7,8};

///@var static bool g_interrupt
///@brief global (SIGINT, SIGQUIT) signal flag
static bool g_interrupt=false;

/////////////////////////
// Declarations 
/////////////////////////

/// @typedef struct mser_device_s mser_device_t
/// @brief device configuration
struct mser_device_s
{
    /// @var mser_device_s::speed
    /// @brief communication speed
    int32_t speed;
    /// @var mser_device_s::parity
    /// @brief parity
    mser_parity_t parity;
    /// @var mser_device_s::stopb
    /// @brief stop bits
    mser_stopb_t stopb;
    /// @var mser_device_s::csize
    /// @brief character size
    mser_csize_t csize;
    /// @var mser_device_s::flow
    /// @brief flow control
    mser_flow_t flow;
    /// @var mser_device_s::term
    /// @brief terminal settings
    struct termios term;
    /// @var mser_device_s::oterm
    /// @brief original (saved) terminal settings
    struct termios oterm;
};

/// @typedef struct mser_serial_s mser_serial_t
/// @brief serial port structure
struct mser_serial_s
{
    /// @var mser_serial_s::device
    /// @brief device path
    char *path;
    /// @var mser_serial_s::device
    /// @brief device IO configuration
    mser_device_t *device;
    /// @var mser_serial_s::fd
    /// @brief device file descriptor
    int fd;
};

static mser_serial_t *mser_serial_new();
static void mser_serial_destroy(mser_serial_t **pself);
static void mser_serial_show_instance(mser_serial_t *self, bool verbose, uint16_t indent);
static void mser_serial_destroy(mser_serial_t **pself);
static mser_serial_t *mser_serial_new();
static void mser_device_destroy(mser_device_t **pself);
static mser_device_t *mser_device_new();
static int s_set_raw(mser_serial_t *self, unsigned min, unsigned time_dsec);
static int s_set_flow(mser_serial_t *self, mser_flow_t flow);
static int s_set_stopb(mser_serial_t *self, mser_stopb_t stopb);
static int s_set_csize(mser_serial_t *self, mser_csize_t csize);
static int s_set_parity(mser_serial_t *self, mser_parity_t parity);
static int s_set_speed(mser_serial_t *self, int32_t ispeed);
static mser_id_t s_add_instance(mser_serial_t* instance);
static mser_serial_t *s_lookup_instance(mser_id_t id);
static void s_init_device_list();
static void mser_device_show_instance(mser_device_t *self, bool verbose, uint16_t indent);
static void mser_serial_show_instance(mser_serial_t *self, bool verbose, uint16_t indent);

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////
static mlist_t *g_device_list=NULL;

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn static void s_init_device_list()
/// @brief initialize device list.
/// Creates entries for stdin, stdout, stderr.
/// @return none
static void s_init_device_list()
{
    if (NULL==g_device_list) {
        mser_serial_t *sin = NULL;
        mser_serial_t *sout = NULL;
        mser_serial_t *serr = NULL;
//        MDEBUG("creating device list\n");
        g_device_list=mlist_new();
        sin = mser_serial_new();
        sout = mser_serial_new();
        serr = mser_serial_new();
        sin->fd=STDIN_FILENO;
        sin->path=strdup("stdin");
        sout->fd=STDOUT_FILENO;
        sout->path=strdup("stdout");
        serr->fd=STDERR_FILENO;
        serr->path=strdup("stderr");
        tcgetattr(sin->fd,&sin->device->term);
        tcgetattr(sout->fd,&sout->device->term);
        tcgetattr(serr->fd,&serr->device->term);
        tcgetattr(sin->fd,&sin->device->oterm);
        tcgetattr(sout->fd,&sout->device->oterm);
        tcgetattr(serr->fd,&serr->device->oterm);
        mlist_add(g_device_list,sin);
        mlist_add(g_device_list,sout);
        mlist_add(g_device_list,serr);
    }
}
// End function s_init_device_list

/// @fn static mser_serial_t *s_lookup_instance(mser_id_t id)
/// @brief look up mser_serial_t instance by ID (handle)
/// @param[in] id device handle
/// @return mser_serial_t instance on success, -1 otherwise
static mser_serial_t *s_lookup_instance(mser_id_t id)
{
//    fprintf(stderr,"looking up instance id[%"PRId32"]\n",id);
    mser_serial_t *retval=(mser_serial_t *)mlist_element(g_device_list,id);
    return retval;
}
// End function s_lookup_instance

/// @fn static mser_id_t s_add_instance(mser_serial_t* instance)
/// @brief add serial port instance to list
/// @param[in] instance mser_serial_t reference
/// @return device handle (id) on success, -1 otherwise
static mser_id_t s_add_instance(mser_serial_t* instance)
{
    mser_id_t retval=-1;
    
    if (NULL!=instance) {
        if (NULL==g_device_list) {
        	s_init_device_list();
        }
        if ( (retval=mser_lookup_id(instance->path))<0) {
//            MDEBUG("adding instance:\n");
//            mser_serial_show_instance(instance,true,5);
            if(mlist_add(g_device_list,instance)==0){
                retval = (mser_id_t)(mlist_size(g_device_list)-1);
//                MDEBUG("instance got id[%"PRId32"]:\n",retval);

                //mser_lookup_id(instance->path);
            }
        }
    }
    
    return retval;
}
// End function s_add_instance

/// @fn static int  s_set_speed(mser_serial_t *self, int32_t ispeed)
/// @brief set speed (baud)
/// @param[in] self mser_serial_t reference
/// @param[in] ispeed speed (bps)
/// @return 0 on success, -1 otherwise
static int  s_set_speed(mser_serial_t *self, int32_t ispeed)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->device) {
        struct termios *terminal = &self->device->term;
        speed_t speed=B0;
        bool valid=true;
        
        switch (ispeed) {
            case 0:
                speed=B0;
                break;
            case 50:
                speed=B50;
                break;
            case 75:
                speed=B75;
                break;
            case 110:
                speed=B110;
                break;
            case 134:
                speed=B134;
                break;
            case 150:
                speed=B150;
                break;
            case 200:
                speed=B200;
                break;
            case 300:
                speed=B300;
                break;
            case 600:
                speed=B600;
                break;
            case 1200:
                speed=B1200;
                break;
            case 1800:
                speed=B1800;
                break;
            case 2400:
                speed=B2400;
                break;
            case 4800:
                speed=B4800;
                break;
            case 9600:
                speed=B9600;
                break;
            case 19200:
                speed=B19200;
                break;
            case 38400:
                speed=B38400;
                break;
            case 57600:
                speed=B57600;
                break;
            case 115200:
                speed=B115200;
                break;
            case 230400:
                speed=B230400;
                break;
            default:
                valid=false;
                break;
        }
        if (valid &&
            (cfsetispeed(terminal,speed)==0) &&
            (cfsetospeed(terminal,speed)==0)){
            self->device->speed=speed;
            retval=0;
        }
    }else{
        fprintf(stderr,"ERR - invalid argument self[%p] device[%p]\n",self,(NULL==self?NULL:self->device));
    }
    
    return retval;
}
// End function s_set_speed

/// @fn static int  s_set_parity(mser_serial_t *self, mser_parity_t parity)
/// @brief set parity setting
/// @param[in] self mser_serial_t reference
/// @param[in] parity parity setting
/// @return 0 on success, -1 otherwise
static int  s_set_parity(mser_serial_t *self, mser_parity_t parity)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->device) {
        bool valid=true;
        struct termios *terminal = &self->device->term;
        switch (parity) {
            case MSER_PAR_N:
                terminal->c_cflag &= ~PARENB;
                // Cygwin returns invalid argument exception 
                // when ISTRIP is cleared
#if defined(__CYGWIN__)
                terminal->c_cflag &= ~(INPCK);
#else
                terminal->c_cflag &= ~(INPCK | ISTRIP);
#endif
                break;
            case MSER_PAR_E:
                terminal->c_cflag |= PARENB;
                terminal->c_cflag &= ~PARODD;
                terminal->c_cflag |= (INPCK | ISTRIP);
                break;
            case MSER_PAR_O:
                terminal->c_cflag |= PARENB;
                terminal->c_cflag |= PARODD;
                terminal->c_cflag |= (INPCK | ISTRIP);
                break;
                
            default:
                valid=false;
                break;
        }
        if(valid && (retval=tcsetattr(self->fd, TCSANOW, terminal))==0){
           self->device->parity=parity;
        }else{
            fprintf(stderr,"fd[%d] term[%p] err[%d/%s]\n",self->fd,terminal,errno,strerror(errno));
        }
    }
    return retval;
}
// End function s_set_parity

/// @fn static int  s_set_csize(mser_serial_t *self, mser_csize_t csize)
/// @brief set character size (data bits)
/// @param[in] self mser_serial_t reference
/// @param[in] csize character size setting
/// @return 0 on success, -1 otherwise
static int  s_set_csize(mser_serial_t *self, mser_csize_t csize)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->device) {
        bool valid=true;
        struct termios *terminal = &self->device->term;
        switch (csize) {
            case MSER_CS_5:
                terminal->c_cflag &= ~CSIZE;
                terminal->c_cflag |= CS5;
                break;
            case MSER_CS_6:
                terminal->c_cflag &= ~CSIZE;
                terminal->c_cflag |= CS6;
                break;
            case MSER_CS_7:
                terminal->c_cflag &= ~CSIZE;
                terminal->c_cflag |= CS7;
                break;
            case MSER_CS_8:
                terminal->c_cflag &= ~CSIZE;
                terminal->c_cflag |= CS8;
                break;
                
            default:
                valid=false;
                break;
        }
        if(valid && (retval=tcsetattr(self->fd, TCSANOW, terminal))==0){
            self->device->csize=csize;
        }
    }
    return retval;
}
// End function s_set_csize

/// @fn static int  s_set_stopb(mser_serial_t *self, mser_stopb_t stopb)
/// @brief set stop bits
/// @param[in] self mser_serial_t reference
/// @param[in] stopb stop bits setting
/// @return 0 on success, -1 otherwise
static int  s_set_stopb(mser_serial_t *self, mser_stopb_t stopb)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->device) {
        bool valid=true;
        struct termios *terminal = &self->device->term;
        switch (stopb) {
            case MSER_STOPB_1:
                terminal->c_cflag &= ~CSTOPB;
                break;
            case MSER_STOPB_2:
                terminal->c_cflag |= CSTOPB;
                break;
                
            default:
                valid=false;
                break;
        }
        if(valid && (retval=tcsetattr(self->fd, TCSANOW, terminal))==0){
            self->device->stopb=stopb;
        }
    }
    return retval;
}
// End function s_set_stopb

/// @fn static int  s_set_flow(mser_serial_t *self, mser_flow_t flow)
/// @brief set flow control
/// @param[in] self mser_serial_t reference
/// @param[in] flow flow control setting
/// @return 0 on success, -1 otherwise
static int  s_set_flow(mser_serial_t *self, mser_flow_t flow)
{
    int retval=-1;
    if (NULL!=self && NULL!=self->device) {
        bool valid=true;
        struct termios *terminal = &self->device->term;
        switch (flow) {
            case MSER_FLOW_N:
                terminal->c_cflag &= ~CRTSCTS;
                terminal->c_cflag &= ~(IXON | IXOFF | IXANY);
                break;
            case MSER_FLOW_H:
                terminal->c_cflag |= CRTSCTS;
                terminal->c_cflag &= ~(IXON | IXOFF | IXANY);
                break;
            case MSER_FLOW_X:
                terminal->c_cflag &= ~CRTSCTS;
                terminal->c_cflag |= (IXON | IXOFF | IXANY);
                break;
            default:
                valid=false;
                break;
        }
        if(valid && (retval=tcsetattr(self->fd, TCSANOW, terminal))==0){
            self->device->flow=flow;
        }
    }
    return retval;
}
// End function s_set_flow

/// @fn static int  s_set_raw(mser_serial_t *self, unsigned min, unsigned time_dsec)
/// @brief set raw terminal mode
/// @param[in] self mser_serial_t reference
/// @param[in] min minimum number of characters to block for in non-canonical mode
/// @param[in] time_dsec timeout in non-canonical mode
/// @return 0 on success, -1 otherwise
static int  s_set_raw(mser_serial_t *self, unsigned min, unsigned time_dsec)
{
    int retval=-1;
    if (NULL!=self && self->device) {

        struct termios *terminal = &self->device->term;
        
//        terminal->c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK);
// 
//        terminal->c_oflag = 0;
// 
//        terminal->c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN );
// 
//        // Ignore modem control lines.
//        terminal->c_cflag |= CLOCAL;
 
        cfmakeraw(terminal);
        
        // Minimum number of characters for noncanonical read (MIN).
        terminal->c_cc[VMIN]  = min;
 
        // Timeout in deciseconds for noncanonical read (TIME).
        terminal->c_cc[VTIME] = time_dsec;
        
        retval=tcsetattr(self->fd, TCSANOW, terminal);
    }
    return retval;
}
// End function s_set_raw

/// @fn static mser_device_t *mser_device_new()
/// @brief allocate mser_device_t instance
/// @return mser_device_t on success, NULL otherwise
static mser_device_t *mser_device_new()
{
    mser_device_t *retval=NULL;
    mser_device_t *instance = (mser_device_t *)malloc(sizeof(mser_device_t));
    if (NULL!=instance) {
        memset(instance,0,sizeof(mser_device_t));
        retval=instance;
   }
    return retval;
}
// End function mser_device_new

/// @fn static void mser_device_destroy(mser_device_t **pself)
/// @brief release mser_device_t instance resources
/// @param[in] pself pointer to mser_device_t reference
/// @return none
static void mser_device_destroy(mser_device_t **pself)
{
    if (NULL!=pself) {
        mser_device_t *self= *(pself);
        if (NULL!=self) {
            free(self);
            *pself=NULL;
        }
    }
}
// End function mser_device_destroy

/// @fn static mser_serial_t *mser_serial_new()
/// @brief allocate mser_serial_t instance
/// @return mser_serial_t on success, NULL otherwise
static mser_serial_t *mser_serial_new()
{
    mser_serial_t *retval=NULL;
    mser_serial_t *instance=(mser_serial_t *)malloc(sizeof(mser_serial_t));
    if (NULL!=instance) {
        memset(instance,0,sizeof(mser_serial_t));
        instance->device=mser_device_new();
        instance->path=NULL;
        instance->fd=-1;
        retval=instance;
    }
    return retval;
}
// End function mser_serial_new

/// @fn static void mser_serial_destroy(mser_serial_t **pself)
/// @brief release mser_serial_t instance resources
/// @param[in] pself pointer to mser_serial_t reference
/// @return none
static void mser_serial_destroy(mser_serial_t **pself)
{
    if (NULL!=pself) {
        mser_serial_t *self= *(pself);
        if (NULL!=self) {
            if (NULL!=self->path) {
                free(self->path);
            }
            if(NULL!=self->device){
                mser_device_destroy(&self->device);
            }
            free(self);
            *pself=NULL;
        }
    }
}
// End function mser_serial_destroy

/// @fn static void mser_device_show_instance(mser_device_t *self, bool verbose, uint16_t indent)
/// @brief output mser_device_t info to stderr.
/// @param[in] self mser_device_t instance reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
static void mser_device_show_instance(mser_device_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[speed     %10d]\n",indent,(indent>0?" ":""), self->speed);
        fprintf(stderr,"%*s[parity    %10s]\n",indent,(indent>0?" ":""), MSER_PAR2STR(self->parity));
        fprintf(stderr,"%*s[csize     %10u]\n",indent,(indent>0?" ":""), MSER_CS2U32(self->csize));
        fprintf(stderr,"%*s[stopb     %10d]\n",indent,(indent>0?" ":""), self->stopb);
        fprintf(stderr,"%*s[flow      %10s]\n",indent,(indent>0?" ":""), MSER_FLOW2STR(self->flow));
    }
}
// End function mser_serial_show

/// @fn static void mser_serial_show(mser_serial_t * self, _Bool verbose, uint16_t indent)
/// @brief output mser_serial info to stderr.
/// @param[in] self mser_serial_t instance reference
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
static void mser_serial_show_instance(mser_serial_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[path      %10s]\n",indent,(indent>0?" ":""), self->path);
        fprintf(stderr,"%*s[fd        %10d]\n",indent,(indent>0?" ":""), self->fd);
        fprintf(stderr,"%*s[device    %10p]\n",indent,(indent>0?" ":""), self->device);
        if (verbose) {
            mser_device_show_instance(self->device,verbose,indent+1);
        }
    }
}
// End function mser_serial_show_instance

/// @fn void mser_serial_show(mser_id_t id, bool verbose, uint16_t indent)
/// @brief output mser_serial info to stderr.
/// @param[in] id mser_serial_t instance handle
/// @param[in] verbose use verbose output
/// @param[in] indent output indentation spaces
/// @return none
void mser_serial_show(mser_id_t id, bool verbose, uint16_t indent)
{
    mser_serial_show_instance(s_lookup_instance(id),verbose,indent);
}
// End function mser_serial_show

/// @fn void mser_release()
/// @brief release device resources
/// @return none
void mser_release()
{
    if (NULL!=g_device_list) {
        mser_serial_t *next = (mser_serial_t *)mlist_first(g_device_list);
        while(NULL != next){
            mser_serial_destroy(&next);
            next = (mser_serial_t *)mlist_next(g_device_list);
        }
        mlist_destroy(&g_device_list);
        g_device_list=NULL;
    }
}
// End function mser_release

void mser_init()
{
    s_init_device_list();
}

/// @fn mser_id_t mser_lookup_id(char *path)
/// @brief lookup device ID by name
/// @param[in] path device path
/// @return device handle on success or -1 otherwise
mser_id_t mser_lookup_id(char *path)
{
    mser_id_t retval=-1;
    mser_serial_t *next=NULL;
    
    if (NULL==g_device_list) {
        s_init_device_list();
    }

    if (NULL!=path && NULL!=g_device_list) {
        int32_t i=-1;
        next = (mser_serial_t *)mlist_first(g_device_list);
        while(NULL != next){
            if(NULL!=next->path && strcmp(next->path,path)==0){
                // found match, next!=NULL
                retval = (++i);
                break;
            }
            next = (mser_serial_t *)mlist_next(g_device_list);
            i++;
        }
    }
//    fprintf(stderr,"lookup_id - returning path[%s] id[%"PRId32"] ret[%"PRId32"]\n",(NULL==next?"NULL":next->path),i,retval);
    return retval;
}
// End function mser_lookup_id

/// @fn mser_id_t mser_open(const char *path, int speed, mser_parity_t parity, mser_csize_t csize, mser_stopb_t stopb, mser_flow_t flow, unsigned min, unsigned time_dsec)
/// @brief open device using specified settings
/// @param[in] path device path
/// @param[in] speed IO speed (bit/s)
/// @param[in] parity parity settings*
/// @param[in] csize character size*
/// @param[in] stopb stop bits*
/// @param[in] flow flow control settings*
/// @param[in] min non-canonical blocking: min characters
/// @param[in] time_dsec non-canonical blocking: min characters
/// @sa *memonics defined in mserial.h
/// @return device handle on success or -1 otherwise
mser_id_t mser_open(const char *path, int speed, mser_parity_t parity, mser_csize_t csize, mser_stopb_t stopb, mser_flow_t flow, unsigned min, unsigned time_dsec)
{
    mser_id_t retval=-1;
    
    mser_serial_t *instance=NULL;
    
    if ( NULL!=path ) {
 
        char *key = strdup(path);
        mser_id_t id = mser_lookup_id(key);
        
        if ( id<0) {
             instance = mser_serial_new();
            if (NULL!=instance) {
                instance->path=key;
               id=s_add_instance(instance);
            }else{
                 free(key);
            }
        }else{
            instance=s_lookup_instance(id);
            free(key);
        }

        if (NULL!=instance) {

           mser_close(id);

            if( (instance->fd=open(path,O_RDWR,O_NOCTTY|O_NONBLOCK ))>0){
                uint32_t test_flags=0x0;
                int i=0;

                tcgetattr(instance->fd,&instance->device->term);
                // set bits for various phases of configuration
                // [captures multiple success/failures in return value]
               MFU_TEST_SET(test_flags,(s_set_raw(instance,min,time_dsec)==0),              i++);
               MFU_TEST_SET(test_flags,(s_set_speed(instance,speed)==0),      i++);
               MFU_TEST_SET(test_flags,(s_set_parity(instance,parity)==0),    i++);
               MFU_TEST_SET(test_flags,(s_set_csize(instance,csize)==0),      i++);
               MFU_TEST_SET(test_flags,(s_set_stopb(instance,stopb)==0),      i++);           MFU_TEST_SET(test_flags,(s_set_flow(instance,flow)==0),        i++);

                if (test_flags==0) {
                    retval=id;
                }else{
                    fprintf(stderr,"ERR - config %x\n",test_flags);
                }
            }else{
                fprintf(stderr,"ERR - open\n");
            }
        }else{
            fprintf(stderr,"ERR - NULL instance\n");
        }
    }else{
        fprintf(stderr,"ERR - invalid arg\n");
    }
//fprintf(stderr,"returning instance[%p] id[%"PRId32"] fd[%d]\n",instance,id,instance->fd);
     return retval;
}
// End function mser_open

/// @fn int mser_close(mser_id_t id)
/// @brief close serial device. Caller should restore settings.
/// @param[in] id mser_serial_t device handle
/// @return 0 success or -1 otherwise
int mser_close(mser_id_t id)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self && self->fd>STDERR_FILENO) {
        retval=close(self->fd);
        self->fd=-1;
    }
    return retval;
}
// End function mser_close

/// @fn int mser_drain(mser_id_t id)
/// @brief drain serial port.
/// @param[in] id mser_serial_t device handle
/// @return 0 success or -1 otherwise
int mser_drain(mser_id_t id)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
    if (NULL!=self && self->fd>STDERR_FILENO) {
        retval=tcdrain(self->fd);
    }
    return retval;
}
// End function mser_drain

/// @fn int mser_flush(mser_id_t id)
/// @brief flush serial port (discard pending transmitted bytes).
/// @param[in] id mser_serial_t device handle
/// @return 0 success or -1 otherwise
int mser_flush(mser_id_t id, mser_direction_t dir)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
    if (NULL!=self && self->fd>STDERR_FILENO) {
        int queue=TCIOFLUSH;
        switch (dir) {
            case MSER_TX:
                queue=TCOFLUSH;
                break;
            case MSER_RX:
                queue=TCIFLUSH;
                break;
            default:
                break;
        }
        retval=tcflush(self->fd,queue);
    }
    return retval;
}
// End function mser_flush

/// @fn int mser_send_break(mser_id_t id, int msec)
/// @brief Assert break condition for msec milliseconds
/// @param[in] id mser_serial_t device handle
/// @param[in] msec break duration (milliseconds)
/// @return 0 success or -1 otherwise
int mser_send_break(mser_id_t id, int msec)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self) {
        retval=tcsendbreak(self->fd, msec);
    }
    return retval;
}
// End function mser_send_break

/// @fn int mser_set_blocking(mser_id_t id, bool enable)
/// @brief Set blocking behavior (overrides settings in mser_open())
/// @param[in] id mser_serial_t device handle
/// @param[in] enable enable if true, disable otherwise
/// @return 0 success or -1 otherwise
int mser_set_blocking(mser_id_t id, bool enable)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self) {
        retval=fcntl(self->fd, F_SETFL, (enable?0:FNDELAY));
    }
    return retval;
}
// End function mser_set_blocking

/// @fn int mser_set_canonical(mser_id_t id, bool enable)
/// @brief Set canonical behavior (overrides settings in mser_open())
/// @param[in] id mser_serial_t device handle
/// @param[in] enable enable if true, disable otherwise
/// @return 0 success or -1 otherwise
int mser_set_canonical(mser_id_t id, bool enable)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
    if (NULL!=self) {
         struct termios *terminal = &self->device->term;
        if(enable){
            terminal->c_lflag |= ICANON;
        }else{
            terminal->c_lflag &= ~(ICANON);
        }
       retval=tcsetattr(self->fd, TCSANOW, terminal);
    }
    return retval;
}
// End function mser_set_canonical

/// @fn int mser_set_echo(mser_id_t id, bool enable)
/// @brief Set canonical behavior (overrides settings in mser_open())
/// @param[in] id mser_serial_t device handle
/// @param[in] enable enable if true, disable otherwise
/// @return 0 success or -1 otherwise
int mser_set_echo(mser_id_t id, bool enable)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
    if (NULL!=self) {
        struct termios *terminal = &self->device->term;
        if(enable){
            terminal->c_lflag |= ECHO;
        }else{
            terminal->c_lflag &= ~(ECHO);
        }
        retval=tcsetattr(self->fd, TCSANOW, terminal);
    }
    return retval;
}
// End function mser_set_canonical

/// @fn int mser_save_term(mser_id_t id)
/// @brief Save current terminal settings.
/// @param[in] id mser_serial_t device handle
/// @return 0 success or -1 otherwise
int mser_save_term(mser_id_t id)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self && NULL!=self->device) {
        struct termios *oterm = &self->device->oterm;
        retval=tcgetattr(self->fd,oterm);
    }
    return retval;
}
// End function mser_save_term

/// @fn int mser_restore_term(mser_id_t id)
/// @brief Restore saved terminal settings.
/// @param[in] id mser_serial_t device handle
/// @return 0 success or -1 otherwise
int mser_restore_term(mser_id_t id)
{
    int retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self && NULL!=self->device) {
        struct termios *oterm = &self->device->oterm;
        retval=tcsetattr(self->fd,TCSANOW,oterm);
    }
    return retval;
}
// End function mser_restore_term

/// @fn int64_t mser_read(mser_id_t id, byte *buf, uint32_t len)
/// @brief read bytes into buffer.
/// @param[in] id mser_serial_t device handle
/// @param[in] buf destination buffer
/// @param[in] len maximum number of bytes to read
/// @return number of bytes returned in buf on success, -1 otherwise
int64_t mser_read(mser_id_t id, byte *buf, uint32_t len)
{
    int64_t retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
   if (NULL!=self && NULL!=buf) {
       ssize_t test=0;
        if ((test=read(self->fd,buf,len))<=0){
            fprintf(stderr,"ERR - read[%"PRId64"] [%d/%s]\n",retval,errno,strerror(errno));
            if (errno==EINTR) {
                g_interrupt=true;
            }
        }else{
            retval=test;
        }
    }
    return retval;
}
// End function mser_read

/// @fn int64_t mser_read_str(mser_id_t id, char *buf, uint32_t len)
/// @brief read NULL terminated string up to len bytes
/// @param[in] id mser_serial_t device handle
/// @param[in] buf destination buffer
/// @param[in] len maximum number of bytes to read
/// @return number of bytes returned in buf on success, -1 otherwise
int64_t mser_read_str(mser_id_t id, char *buf, uint32_t len)
{
    int64_t retval=-1;
    char del[1]={'\0'};
   retval=mser_read_del(id,(byte *)buf,len,del,1);
    return retval;
}
// End function mser_read_str

/// @fn int64_t mser_read_del(mser_id_t id, byte *buf, uint32_t len, const char *del, uint32_t dlen)
/// @brief read bytes until a delimiter is matched or buffer is full.
/// If buf==NULL bytes are discarded. If len==0, bytes will discarded indefinitely.
/// len==0 is not valid if buf!=NULL.
/// If del==NULL, len must be >0 and sets the number of characters to discard.
/// @param[in] id mser_serial_t device handle
/// @param[in] buf destination buffer
/// @param[in] len maximum number of bytes to read
/// @param[in] del delimiter
/// @param[in] dlen delimiter match len
/// @return number of bytes read/discarded on success, -1 otherwise
int64_t mser_read_del(mser_id_t id, byte *buf, uint32_t len, const char *del, uint32_t dlen)
{
    int64_t retval=-1;
    mser_serial_t *self = s_lookup_instance(id);

    if (NULL!=self &&
        ((NULL!=del && dlen>0) ||
         (NULL==del))
        &&
        ((buf!=NULL && len>0) ||
         (buf==NULL))
        ) {
        byte *ip=buf;
        const char *dp=del;
        byte rbuf[1]={0};
        bool quit=false;
        bool match=false;
        int64_t read_bytes=0;
        
        while( !match && !quit){
            rbuf[0]=0xFF;
            // read a byte
            if (read(self->fd,rbuf,1)==1) {
                // if successful, increment read count
                read_bytes++;
                if (NULL!=buf) {
                    // if buffer provided,
                    // add character
                    *ip=rbuf[0];
                    ip++;
                }
                
                if ( (NULL!=dp) && (rbuf[0] == (*dp)) ) {
                    // if character matches next delimiter
                    // increment delimiter pointer
                    dp++;
                }else{
                    // otherwise, reset delimiter pointer
                    dp=del;
                }
                
                if ( (NULL!=dp) && (dp>=(del+dlen)) ) {
                    // if delimiter match complete,
                    // set exit conditions
                    match=true;
                    retval=read_bytes;
                }
                if (len>0 && read_bytes>=len) {
                    // if buffer out of space
                    // exit
                    quit=true;
                    if (NULL==dp) {
                        // NULL delimiter,
                        // return number skipped
                        retval=read_bytes;
                    }
                }
            }else{
                // handle read errors
                int err=errno;
                switch (err) {
                    case EAGAIN:
#if !defined(__CYGWIN__)
                        fprintf(stderr,"EAGAIN\n");
#endif
                        break;
                    case EINTR:
                        fprintf(stderr,"EINTR\n");
                        quit=true;
                        g_interrupt=true;
                        break;
                    case EBADF:
                    case EFAULT:
                    case EIO:
                    case EINVAL:
                    case EISDIR:
                        fprintf(stderr,"IO err [%d/%s]\n",errno,strerror(errno));
                        quit=true;
                        break;
                    default:
                        fprintf(stderr,"%s: ERR - [%d/%s]\n",__FUNCTION__,errno,strerror(errno));
                        quit=true;
                        break;
                }
            }
        }
    }else{
        fprintf(stderr,"invalid arg\n");
    }
    return retval;
}
// End function mser_read_del

/// @fn int64_t mser_write(mser_id_t id, byte *buf, uint32_t len)
/// @brief write bytes to device
/// @param[in] id mser_serial_t device handle
/// @param[in] buf source buffer
/// @param[in] len maximum number of bytes to write
/// @return number of bytes written on success, -1 otherwise
int64_t mser_write(mser_id_t id, byte *buf, uint32_t len)
{
    int64_t retval=-1;
    mser_serial_t *self = s_lookup_instance(id);
  if (NULL!=self && NULL!=buf) {
      ssize_t test=0;
        if ((test=write(self->fd,buf,len))<0){
            fprintf(stderr,"ERR - write[%"PRId64"] [%d/%s]\n",retval,errno,strerror(errno));
        }else{
            retval=test;
        }
    }
    return retval;
}
// End function mser_write

/// @fn int64_t mser_write_str(mser_id_t id, byte *buf)
/// @brief write NULL terminated string to device (including NULL)
/// @param[in] id mser_serial_t device handle
/// @param[in] buf source buffer
/// @return number of bytes written on success, -1 otherwise
int64_t mser_write_str(mser_id_t id, char *buf)
{
    int64_t retval=-1;
   if (NULL!=buf) {
       retval=mser_write(id,(byte *)buf,strlen(buf)+1);
//       for(int i=0;(unsigned long)i<strlen(buf)+1;i++){
//        retval+= mser_write(id, (byte *)&buf[i], 1);
//           sleep(1);
//       }
    }
    return retval;
}
// End function mser_write_str

/// @fn int64_t mser_sync_str(mser_id_t id, const char *sync, uint32_t max_len)
/// @brief discard characters until string is matched (not including NULL) or
/// or max characters discarded.
/// @param[in] id mser_serial_t device handle
/// @param[in] sync search string
/// @param[in] max_len max number of characters to discard (0: unlimited)
/// @return number of bytes discarded on success, -1 otherwise
int64_t mser_sync_str(mser_id_t id, const char *sync, uint32_t max_len)
{
    return mser_read_del(id, NULL, max_len, sync, strlen(sync));
}

/// @fn int64_t mser_sync_n(mser_id_t id, uint32_t n)
/// @brief discard specified number of characters
/// @param[in] id mser_serial_t device handle
/// @param[in] n number of characters to discard
/// @return number of bytes discarded on success, -1 otherwise
int64_t mser_sync_n(mser_id_t id, uint32_t n)
{
    return mser_read_del(id, NULL, n, NULL, 0 );
}
// End function mser_sync_n

/// @fn int mser_test()
/// @brief mserial module test
/// @return 0 on success, bit vector indicating failed tests otherwise
int mser_test()
{
    int retval=-1;
    char loopback_path[256]={0};
    char *cp=NULL;
    size_t len=0;
    
    uint32_t test_flags=0x0;
    int test_index=0;
    int64_t test=0;
    mser_id_t loop_id =-1;

    // prompt for device path
    do{
        memset(loopback_path,0,256);
        fprintf(stdout,"enter loopback device path:");
        if( (cp=fgets(loopback_path,256,stdin))==NULL){
            g_interrupt=true;
            break;
        }
        len=strlen(loopback_path);
        
    }while ( ((NULL==cp) || (len<=1)));
    if(g_interrupt) goto g_int_exit;
    mfu_trim(loopback_path,len);

    // do loopback write/read test
    if( (loop_id=mser_open(loopback_path,9600,MSER_PAR_N,MSER_CS_8,MSER_STOPB_1,MSER_FLOW_N,0,1)) >= 0){
        int i=0;
        byte b=0;
        int err_count=0;
        int success_count=0;
        int LOOPBACK_TEST_CHARS=128;

        mser_save_term(loop_id);
        mser_serial_show(loop_id,true,5);

        while (i<LOOPBACK_TEST_CHARS && !g_interrupt) {
            if( (test=mser_write(loop_id,(byte *)&i,1))==1){
                fprintf(stderr,"wr[%02x] [%"PRId64"] ",i,test);
                if( (test=mser_read(loop_id,&b,1))==1){
                    fprintf(stderr,"rd[%02x] [%"PRId64"]\n",b,test);
                    if(b==i){
                        success_count++;
                    }else{
                        err_count++;
                    }
                }else{
                    err_count++;
                }
            }else{
                err_count++;
            }
            i++;
        }
  
        MFU_TEST_SET(test_flags,(success_count==LOOPBACK_TEST_CHARS),test_index++);
        
        fprintf(stderr,"done - r/w[%d] err[%d]\n",success_count,err_count);
    
    }else{
        fprintf(stderr,"ERR - mser_open path[%s] id[%d] [%d/%s]\n",loopback_path,loop_id,errno,strerror(errno));
        g_interrupt=true;
    }
    if(g_interrupt) goto g_int_exit;
{
    // use stdin to test some read functions
    // [use raw mode w/ signal processing enabled]
	mser_serial_t *sin =NULL;
	
    // save current terminal settings (restore when done)
    mser_save_term(SIN_ID);
    sin = s_lookup_instance(SIN_ID);
    s_set_raw(sin,(unsigned)1,(unsigned)1);
    // enable CTRL-C processing
    sin->device->term.c_lflag |= ISIG;
    tcsetattr(sin->fd,TCSANOW,&sin->device->term);
}
    if(g_interrupt) goto restore_sin;
{
    // test read_del (discard until delimiter)
    char del[]="quit";
    fprintf(stderr,"type characters then 'quit' to exit\r\n");
    test=mser_read_del(SIN_ID, NULL, 0, del, strlen(del));
    fprintf(stderr,"mser_read_del ret[%"PRId64"]\r\n",test);
    MFU_TEST_SET(test_flags,(test>=4),test_index++);
}
    if(g_interrupt) goto restore_sin;

    // test read_del (discard until delimeter or length)
    fprintf(stderr,"type up to 16 characters then 'quit' to exit\r\n");
    test=mser_sync_str(SIN_ID, "quit", 20);
    fprintf(stderr,"mser_sync_str ret[%"PRId64"]\r\n",test);
    MFU_TEST_SET(test_flags,(test>=4),test_index++);
    if(g_interrupt) goto restore_sin;

    // test read_del (discard until length)
    fprintf(stderr,"type 5 characters\r\n");
    test=mser_sync_n(SIN_ID,5);
    fprintf(stderr,"mser_sync_n ret[%"PRId64"]\r\n",test);
    MFU_TEST_SET(test_flags,(test==5),test_index++);

    // restore stdin terminal settings
restore_sin:
    mser_restore_term(SIN_ID);
 
    // restore loopback device terminal settings
    // and close [retain resources]
    mser_restore_term(loop_id);
    mser_close(loop_id);

    if(g_interrupt) goto g_int_exit;
{
    // open loopback [change speed, parity]
   mser_id_t id=-1;
    if( (id=mser_open(loopback_path,4800,MSER_PAR_E,MSER_CS_8,MSER_STOPB_1,MSER_FLOW_N,0,1)) >= 0){
        fprintf(stderr,"mser_open(4800) ret id[%d]\r\n",(int)id);
        mser_serial_show(id,true,5);
    }else{
        fprintf(stderr,"ERR - mser_open(4800) %s id[%d][%d/%s]\n",loopback_path,(int)id,errno,strerror(errno));
     }
    MFU_TEST_SET(test_flags,(id>=0),test_index++);

    // open loopback [change speed, parity]
    if( (id=mser_open(loopback_path,19200,MSER_PAR_O,MSER_CS_8,MSER_STOPB_1,MSER_FLOW_N,0,1)) >= 0){
        fprintf(stderr,"mser_open(19200) ret id[%d]\r\n",(int)id);
        mser_serial_show(id,true,5);
    }else{
        fprintf(stderr,"ERR - mser_open(19200) %s id[%d][%d/%s]\n",loopback_path,(int)id,errno,strerror(errno));
    }
    MFU_TEST_SET(test_flags,(id>=0),test_index++);

    // open invalid device
    if( (id=mser_lookup_id("foo"))>=0){
        fprintf(stderr,"ERR - mser_lookup_id failed for invalid path ret[%d]\n",(int)id);
    }
    MFU_TEST_SET(test_flags,(id<0),test_index++);
}
    // release device resources and exit
g_int_exit:
    retval=test_flags;
    fprintf(stderr,"mser_test returning [0x%08X]\n",test_flags);
    mser_release();

    return retval;
}
// End function mser_test

mser_term_t *mser_term_new(char *path, int speed, mser_parity_t parity,
                           mser_csize_t csize, mser_stopb_t stopb,
                           mser_flow_t flow, unsigned vm, unsigned vt)
{
    mser_term_t *retval=NULL;
    mser_term_t *instance = (mser_term_t *)malloc(sizeof(mser_term_t));
    if (NULL!=instance) {
        memset(instance,0,sizeof(mser_term_t));
        instance->speed=speed;
        instance->par=parity;
        instance->cs=csize;
        instance->stopb=stopb;
        instance->flow=flow;
        instance->vt=vt;
        instance->vm=vm;
        if(NULL!=path){
            instance->path=strdup(path);
        }else{
            instance->path=NULL;
        }
        retval=instance;
    }
    return retval;
    
}

mser_term_t *mser_parse_term(mser_term_t **pdest, char *path, char *term_str)
{
    mser_term_t *dest = NULL;

//     fprintf(stderr,"pdest[%p] path[%p/%s] term_str[%p/%s] dest[%p]\r\n",pdest,
//             path,(NULL==path?"NULL":path),
//             term_str,(NULL==term_str?"NULL":term_str),dest);
    
    if(NULL!=path && NULL!=pdest){

        if(NULL==*pdest){

            *pdest=(mser_term_t *)malloc(sizeof(mser_term_t));
            if(*pdest!=NULL){
	            memset(*pdest,0,sizeof(mser_term_t));
                dest=*pdest;
                dest->path=NULL;
            }else{
                fprintf(stderr,"%s - ERR malloc return NULL\r\n",__FUNCTION__);
            }
        }else{
            dest=*pdest;
        }
        if(NULL!=dest){
            
            if(NULL!=path){
                dest->path=strdup(path);
            }else{
                dest->path=NULL;
            }
            
            dest->hnd=-1;
{
            int speed=9600;
            char cpar='N';
            int csize=8;
            int cstopb=1;
            char cflow='N';
            unsigned vm=0;
            unsigned vt=1;
            
            if(NULL!=term_str){
                int i=0;
                if( (i=sscanf(term_str,"%d%c%1d%1d%cm%ut%u",&speed,&cpar,&csize,&cstopb,&cflow,&vm,&vt))<4){
                    fprintf(stderr,"WARN - term parse incomplete[%d/4]\r\n",i);
                }
            }
            dest->speed=speed;
            dest->vm=vm;
            dest->vt=vt;
            

            switch (toupper(cpar)) {
                case 'N':
                    dest->par=MSER_PAR_N;
                    break;
                case 'E':
                    dest->par=MSER_PAR_E;
                    break;
                case 'O':
                    dest->par=MSER_PAR_O;
                    break;
                default:
                    break;
            }
            switch (csize) {
                case 5:
                    dest->cs=MSER_CS_5;
                    break;
                case 6:
                    dest->cs=MSER_CS_6;
                    break;
                case 7:
                    dest->cs=MSER_CS_7;
                    break;
                case 8:
                    dest->cs=MSER_CS_8;
                    break;
                default:
                    break;
            }
            switch (cstopb) {
                case 0:
                    dest->stopb=MSER_STOPB_0;
                    break;
                case 1:
                    dest->stopb=MSER_STOPB_1;
                    break;
                case 2:
                    dest->stopb=MSER_STOPB_2;
                    break;
                default:
                    break;
            }
            switch (toupper(cflow)) {
                case 'N':
                    dest->flow=MSER_FLOW_N;
                    break;
                case 'H':
                    dest->flow=MSER_FLOW_H;
                    break;
                case 'X':
                    dest->flow=MSER_FLOW_X;
                    break;
                default:
                    break;
            }
}// scope
        }
    }
    return dest;
}


void mser_term_destroy(mser_term_t **pself)
{
    if (NULL!=pself) {
        mser_term_t *self= *(pself);
        if (NULL!=self) {
            if(NULL!=self->path)
                free(self->path);
            free(self);
            *pself=NULL;
        }
    }
}
mser_id_t mser_term_open(mser_term_t *term)
{
    mser_term_show(term,true,5);
    return mser_open(term->path,term->speed, term->par,term->cs,term->stopb,term->flow,term->vm,term->vt);
}

void mser_term_show(mser_term_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self    %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[hnd     %10d]\n",indent,(indent>0?" ":""), self->hnd);
        fprintf(stderr,"%*s[path    %10s]\n",indent,(indent>0?" ":""), self->path);
        fprintf(stderr,"%*s[speed   %10d]\n",indent,(indent>0?" ":""), self->speed);
        fprintf(stderr,"%*s[par     %10s]\n",indent,(indent>0?" ":""), MSER_PAR2STR(self->par));
        fprintf(stderr,"%*s[cs      %10u]\n",indent,(indent>0?" ":""), (unsigned int)MSER_CS2U32(self->cs));
        fprintf(stderr,"%*s[stopb   %10d]\n",indent,(indent>0?" ":""), self->stopb);
        fprintf(stderr,"%*s[flow    %10s]\n",indent,(indent>0?" ":""), MSER_FLOW2STR(self->flow));
    }
}
