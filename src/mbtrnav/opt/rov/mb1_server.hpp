/// @file mb1_server.hpp
/// @authors k. headley
/// @date 21mar2022

/// Summary: MB1 server component

// ///////////////////////
// Copyright 2022  Monterey Bay Aquarium Research Institute
// Distributed under MIT license. See LICENSE file for more information.

#ifndef MB1_SERVER_HPP
#define MB1_SERVER_HPP

#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "mb1_msg.h"

#define MB1SVR_RTO_MS_DFL 500
#define MB1SVR_FD_INVALID -1
#define MB1SVR_PORT_INVALID -1
#define MB1SVR_HOST_DFL "localhost"
#define MB1SVR_PORT_DFL 8001
#define MB1SVR_DBG(...) if(_debug != 0)fprintf(__VA_ARGS__)
#define MB1SVR_NDBG(n,...) if(n <= _debug)fprintf(__VA_ARGS__)

namespace trn {

class mb1_server
{

public:

    mb1_server()
    :_mb1svr_host(NULL), _mb1svr_port(MB1SVR_PORT_INVALID), _mb1svr_fd(MB1SVR_FD_INVALID),
    _rto_ms(MB1SVR_RTO_MS_DFL), _connected(false), _fdmax(0),_addr_size(0), _debug(0)
    {
        memset(&_iobuf, 0, MB1_MAX_SOUNDING_BYTES);
        FD_ZERO(&_read_fds);
        FD_ZERO(&_write_fds);
        FD_ZERO(&_err_fds);
        FD_ZERO(&_active_set);
        memset(&_sel_tv,0,sizeof(struct timeval));
        memset((char*)&_mb1svr_addr, 0, sizeof(struct sockaddr_in));
        memset((char*)&_client_addr, 0, sizeof(struct sockaddr_in));
    }

    mb1_server(char *host, int port)
    :_mb1svr_host(NULL), _mb1svr_port(MB1SVR_PORT_INVALID), _mb1svr_fd(MB1SVR_FD_INVALID),
    _rto_ms(MB1SVR_RTO_MS_DFL), _connected(false), _fdmax(0), _addr_size(0), _debug(0)
    {
        if(NULL!=host)
        {
            if(NULL != _mb1svr_host)
            {
                free(_mb1svr_host);
            }
            _mb1svr_host = strdup(host);
        }
        _mb1svr_port = port;
        FD_ZERO(&_read_fds);
        FD_ZERO(&_write_fds);
        FD_ZERO(&_err_fds);
        FD_ZERO(&_active_set);
        memset(&_sel_tv,0,sizeof(struct timeval));
        memset((char*)&_mb1svr_addr, 0, sizeof(struct sockaddr_in));
        memset((char*)&_client_addr, 0, sizeof(struct sockaddr_in));
    }

    ~mb1_server()
    {
        close(_mb1svr_fd);
        free(_mb1svr_host);
    }

    // Setup socket and connect to the MB-TRN server
    //
    int initialize(const char* host=NULL, int port=MB1SVR_PORT_DFL,
                               unsigned int recv_timeout_ms=MB1SVR_RTO_MS_DFL)
    {
        int retval = 0;

        // Disconnect before reinitializing
        if (_connected)
        {
            this->disconnect_svr();
            _mb1svr_fd = MB1SVR_FD_INVALID;
        }

        _rto_ms = recv_timeout_ms;

        _mb1svr_port = port;

        // Capture new server and port
        if (NULL != host)
        {
            if(_mb1svr_host){
                free(_mb1svr_host);
            }
            _mb1svr_host = strdup(host);

        } else {
            // NULL host
            if(NULL == _mb1svr_host){
                fprintf(stderr,"%s: NULL host\n", __func__);
                retval = -1;
            }else{
                MB1SVR_DBG(stderr,"%s: using host %s:%d\n", __func__, _mb1svr_host,_mb1svr_port);
            }
        }

        // get socket
        if ((_mb1svr_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("mb1_server::initialize - cannot create socket");
            _mb1svr_fd = MB1SVR_FD_INVALID;
            return errno;
        }

        struct timeval tv={0,0};
        tv.tv_sec  = _rto_ms/1000L;
        tv.tv_usec = 1000L*(_rto_ms%1000L);

        if (0 > setsockopt(_mb1svr_fd, SOL_SOCKET, SO_RCVTIMEO,
                           (const void **)&tv, sizeof(struct timeval)))
        {
            fprintf(stderr,"%s: setsockopt SO_RCVTIMEO failed: %d\n", __func__, errno);
            perror("mb1_server::initialize");
            return errno;
        }
        MB1SVR_DBG(stderr,"%s - SO_RCVTIMEO set to : %d ms\n", __func__, _rto_ms);

        // Reuse the socket
        int sockopt = 1;
        if (0 > setsockopt(_mb1svr_fd, SOL_SOCKET, SO_REUSEADDR,
                           (const void *)&sockopt, sizeof(int)))
        {
            fprintf(stderr,"%s: setsockopt SO_REUSEADDR failed: %d\n", __func__, errno);
            perror("mb1_server::initialize");
            return errno;
        }

        return retval;
    }

    // Connect server
    int connect_svr()
    {

        if(_mb1svr_fd > 0 && NULL != _mb1svr_host)
        {
            _connected = false;

            // Setup the MB-TRN server sockaddr_in using the ip and port arguments
            memset((char*)&_mb1svr_addr, 0, sizeof(_mb1svr_addr));
            _mb1svr_addr.sin_family = AF_INET;
            _mb1svr_addr.sin_port = htons(_mb1svr_port);
            _mb1svr_addr.sin_addr.s_addr = inet_addr(_mb1svr_host);

            // Bind to the client socket and set socket options
            if ( bind(_mb1svr_fd, (struct sockaddr *)&_mb1svr_addr, sizeof(_mb1svr_addr)) < 0)
            {
                fprintf(stderr,"%s: ERR - bind failed: fd %d host %s port %d [%d/%s]\n", __func__, _mb1svr_fd, _mb1svr_host, _mb1svr_port, errno, strerror(errno));
                perror("mb1_server::connect_svr");
                return errno;

            } else {
                // listen for connections
                if(listen(_mb1svr_fd, 2)<0)
                {
                    fprintf(stderr,"%s: ERR - listen failed: %d/%s\n", __func__, errno, strerror(errno));
                    perror("mb1_server::connect_svr");

                    return errno;
                }

                // initialize parameters for select
                _sel_tv.tv_sec = _rto_ms/1000;
                _sel_tv.tv_usec = ((_rto_ms%1000)*1000L);

                memset(&_addr_size,0,sizeof(socklen_t));
                memset(&_client_addr,0,sizeof(struct sockaddr_in));
                _fdmax = 0;

                FD_ZERO(&_read_fds);
                FD_ZERO(&_write_fds);
                FD_ZERO(&_err_fds);
                FD_ZERO(&_active_set);
                // add server socket to active_set set
                FD_SET(_mb1svr_fd,&_active_set);
                _fdmax = _mb1svr_fd;

                _read_fds = _active_set;
                _write_fds = _active_set;
                _err_fds = _active_set;

                _connected = true;
                MB1SVR_DBG(stderr,"%s - connected %s:%d\n", __func__, _mb1svr_host, _mb1svr_port);
            }
        } else {
            // invalid fd
            fprintf(stderr,"%s: ERR invalid file descriptor %d\n", __func__, _mb1svr_fd);
            perror("mb1_server::connect_svr");
        }
        return 0;
    }

    // close socket connection
    int disconnect_svr()
    {

        MB1SVR_DBG(stderr,"%s - closing socket connection\n", __func__);

        int i=0;
        for (i=_mb1svr_fd; i<=_fdmax; i++)
        {
            MB1SVR_DBG(stderr,"%s: ERR - closing fd[%d]\n", __func__, i);
            // remove from active_set
            FD_CLR(i, &_active_set);
            close(i);
        }
        _mb1svr_fd = MB1SVR_FD_INVALID;
        _connected = false;
        return 0;
    }


    int publish(byte *data, size_t len)
    {
        int retval = -1;
        int stat=0;

        if(!_connected)
        {
            if(connect_svr() != 0){
                return retval;
            }
        }

        _sel_tv.tv_sec = _rto_ms/1000;
        _sel_tv.tv_usec = ((_rto_ms%1000)*1000L);

        _read_fds = _active_set;
        _write_fds = _active_set;
        _err_fds = _active_set;

        MB1SVR_NDBG(4,stderr,"%s: server pending on select fd[%d] to[%u] fdmax[%d]\n", __func__,_mb1svr_fd, _rto_ms, _fdmax);
        if( (stat=select(_fdmax+1, &_read_fds, &_write_fds, &_err_fds, &_sel_tv)) != -1)
        {
            int newfd=-1;
            int i=0;
            bool do_close=false;

            for (i=_mb1svr_fd; i<=_fdmax; i++)
            {
                do_close=false;

                MB1SVR_NDBG(5,stderr,"%s: i[%d] _mb1svr_fd[%d] fdmax[%d]\n", __func__, i, _mb1svr_fd, _fdmax);
                if (FD_ISSET(i, &_read_fds))
                {
                    if (i==_mb1svr_fd)
                    {
                        MB1SVR_NDBG(5,stderr,"%s: server ready to read\n", __func__);

                        newfd = accept(_mb1svr_fd, (struct sockaddr *)&_client_addr, &_addr_size);
                        if (newfd != -1)
                        {
                            struct timeval rto={1,0};
                            int test;
                            MB1SVR_DBG(stderr,"%s: client connected on socket fd[%d]\n", __func__,newfd);

                            // add client to active list
                            FD_SET(newfd,&_active_set);

                            test = setsockopt(newfd, SOL_SOCKET, SO_RCVTIMEO,
                                              &rto, sizeof(rto));
                            if(test!=0)
                            {
                                fprintf(stderr,"%s: setsockopt [%d] failed[%d/%s]\n", __func__,newfd,errno,strerror(errno));
                            }

                            if (newfd>_fdmax)
                            {
                                _fdmaxprev = _fdmax;
                                _fdmax=newfd;
                            }
                        }else{
                            // accept failed
                            fprintf(stderr,"%s: accept failed [%d/%s]\n", __func__,errno,strerror(errno));
                        }
                    }else{

                        int nbytes=0;
                        // client ready to read
                        MB1SVR_NDBG(4,stderr,"%s: server client ready to read fd[%d]\n", __func__,i);
                        if (( nbytes = recv(i, _iobuf, MB1_MAX_SOUNDING_BYTES, 0)) > 0)
                        {
                            MB1SVR_NDBG(3,stderr,"server received msg on socket [%d] len[%d]\n",i,nbytes);
                            // test proto doesn't expect anything from clients
                        }else{
                            // recv failed
                            fprintf(stderr,"%s: ERR - recv failed fd[%d] nbytes[%d] [%d/%s]\n", __func__,i,nbytes,errno, strerror(errno));

                            // got error or connection closed by client
                            if (nbytes == 0)
                            {
                                // connection closed
                                fprintf(stderr,"%s: ERR - socket %d hung up\n", __func__, i);
                                do_close=true;
                            } else if(nbytes<0) {

                                fprintf(stderr,"%s: ERR - recv failed socket[%d] [%d/%s]\n", __func__ , i, errno, strerror(errno));
                                if(errno!=EAGAIN){
                                    MB1SVR_DBG(stderr,"%s: ERR - setting close flag for socket[%d]\n", __func__,i);
                                    do_close=true;
                                }
                            }
                        }
                    }
                }else{
                    // fd[i] not ready to read
                    // MMINFO(APP1,"readfs fd[%d/%d] ISSET:%s\n",i,fdmax,(FD_ISSET(i,&read_fds)?"TRUE":"FALSE"));
                }

                if (FD_ISSET(i, &_err_fds)){
                    if (i==_mb1svr_fd) {
                        fprintf(stderr,"%s: server socket err fd[%d]--stopping\n", __func__,i);
                    }else{
                        fprintf(stderr,"%s: client socket err fd[%d] err[%d/%s]\n", __func__, i, errno, strerror(errno));
                        do_close=true;
                    }
                }

                if (FD_ISSET(i, &_write_fds)){

                    if (i==_mb1svr_fd)
                    {
                        MB1SVR_DBG(stderr,"%s: server socket ready to write fd[%d]\n", __func__,i);
                    }else if(!do_close){

                        if(NULL != data && len!=0)
                        {
                            send(i, (const char*)data, len, 0);
                            if(_debug>=4){
                                // data is expected to be an MB1 record(mb1_t)
                                mb1_t *snd = (mb1_t *)data;
                                MB1SVR_DBG(stderr,"%s: sending frame fd[%d] p[%p] len[%lu]\n", __func__, i,  data, len);
                                mb1_show(snd, (_debug>=5?true:false), true);
                                if(_debug>=5)
                                    mb1_hex_show((byte *)snd,snd->size,16,true,5);
                            }
                        }
                    } else {
                        MB1SVR_DBG(stderr,"%s: socket %d marked for close; skipping write\n", __func__, i);
                    }
                }

                if (do_close)
                {
                    MB1SVR_DBG(stderr,"%s: ERR - closing fd[%d]\n", __func__, i);
                    // remove from active_set
                    FD_CLR(i, &_active_set);
                    close(i);
                    if(i==_fdmax)
                    {
                        _fdmax = _fdmaxprev;
                    }
                }
            }// for client
        }else{
            // select failed
            _sel_tv.tv_sec = _rto_ms/1000;
            _sel_tv.tv_usec = ((_rto_ms%1000)*1000L);
        }

        return retval;
    }

    void set_debug(int debug)
    {
        _debug = debug;
    }

protected:
    // server host IP address
    char *_mb1svr_host;
    // server IP port
    int  _mb1svr_port;
    // socket address struct
    struct sockaddr_in _mb1svr_addr;
    // socket file descripter
    int _mb1svr_fd;
    // receive timeout (msec)
    uint32_t _rto_ms;
    // connected flag
    bool _connected;
    // select file descriptor sets
    fd_set _active_set;
    fd_set _read_fds;
    fd_set _write_fds;
    fd_set _err_fds;
    // select file descriptors
    int _fdmax;
    int _fdmaxprev;
    // select timeout struct
    struct timeval _sel_tv;
    // client address
    struct sockaddr_in _client_addr;
    // client address size
    socklen_t _addr_size;
    // socket IO message buffer
    unsigned char  _iobuf[MB1_MAX_SOUNDING_BYTES];
    int _debug;
};

}

#endif











