///
/// @file iowrap.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// MBRT platform-independent IO wrappers

/////////////////////////
// Terms of use 
/////////////////////////
/*
Copyright Information

Copyright 2000-2018 MBARI
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

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#include "iowrap.h"
#include "mdebug.h"

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

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

/////////////////////////
// Declarations 
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn char * iow_addr2str(iow_socket_t * s, char * dest)
/// @brief print socket IP address as string (address:port).
/// @param[in] s socket reference
/// @param[in] dest data buffer
/// @return pointer to destination buffer on success, NULL otherwise
char *iow_addr2str(iow_socket_t *s, char *dest, size_t len)
{
    char *retval=NULL;
    if (NULL != s && NULL != dest && len>0) {
        snprintf(dest,len,"%s:%d",s->addr->host,s->addr->port);
        retval=dest;
    }
    return retval;
}
// End function iow_addr2str


#ifdef WITH_TEST

int iow_test()
{
    int retval=0;
    
    const char *host="localhost";
    int port=9999;

    iow_socket_t *s=iow_socket_new(host,port,ST_TCP);
    iow_socket_t *c=iow_socket_new(host,port,ST_TCP);
    
    // configure server socket
    iow_configure(s,host,port, ST_TCP,5);
    MINFO("socket fd - s[%d] c[%d]\n",s->fd,c->fd);

    // start test server thread
    iow_thread_t svr_thread;
    iow_thread_start(&svr_thread, iow_test_svr, (void *)s);
    
    usleep(1500);
    byte sbuf[8]="REQ";
    byte rbuf[8]={0};
    int rbytes=0;

    // make some client connections
    // that send REQ and receive ACK
    for (int i=0; i<3; i++) {
        iow_configure(c,host,port, ST_TCP,0);
        if(iow_connect(c)==0){
            MINFO("\nclient connected, sending to fd[%d]\n",c->fd);
        	iow_send(c,sbuf,strlen((char *)sbuf));

            memset(rbuf,0,8);
            if( (rbytes=iow_recv(c,rbuf,8)) <=0 ){
                fprintf(stderr,"recv failed, returned [%d]\n",rbytes);
            }else{
                MINFO("client received [%d] bytes [%s]\n",rbytes,rbuf);
            }
            close(c->fd);
        }else{
            MINFO("client connect failed\n");
        }
        usleep(500);
    }
    
    MINFO("\nclient requesting server stop\n");
    // signal server to stop
    iow_configure(c,host,port, ST_TCP,0);
    if(iow_connect(c)==0){
        iow_send(c,(byte *)"STOP",strlen("STOP"));
        close(c->fd);
    }
    
    // join server thread
    int *pstat= &s->status;
    svr_thread.status=(void **)&pstat;
    if ( iow_thread_join ( &svr_thread) ) {
        fprintf(stderr,"error joining thread.");
        retval=-1;
    }
    MINFO("server returned status [%d]\n",s->status);
    iow_socket_destroy(&s);
    iow_socket_destroy(&c);
    
    return retval;
}
#else
/// @fn int iow_test()
/// @brief TBD.
/// @return TBD
int iow_test()
{
    fprintf(stderr,"ERR - server test not implemented\n");
return -1;
}
// End function iow_test


#endif
