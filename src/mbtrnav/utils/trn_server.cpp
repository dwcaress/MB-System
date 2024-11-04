/****************************************************************************/
/* Copyright (c) 2013 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : This process acts as a server to a TerrainNavClient object.   */
/*            The client/server arrangement allows the auv control system   */
/*            to use a remote TerrainNav object.                            */
/* Filename : trn_server.cpp                                                */
/* Author   : Rich Henthorn                                                 */
/* Project  :                                                               */
/* Version  : 1.0                                                           */
/* Created  : 02/17/2013                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/****************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#include <stdlib.h>           // For atoi()
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "structDefs.h"       // Contains definitions of commsT class
#include "trn_common.h"
#include "TerrainNav.h"
#include "TNavConfig.h"
#include "trn_log.h"

// Use a macro to standardize extracting the message from an exception
//
#ifdef _QNX
#include "Exception.h"
#define EXP_MSG e.msg
#else
#include "myexcept.h"
#define EXP_MSG e.what()
#endif

#define NANOSEC_PER_SEC (1000000000L)
#define TRN_DEBUG 0
#define MAX_RECV_ATTEMPTS 3

#define MAPNAME_BUF_BYTES 512
#define CFGNAME_BUF_BYTES 512
#define PARTICLENAME_BUF_BYTES 512
#define DEBUG_BUF_BYTES 512
#define LOGBUF_BYTES 2400

static TerrainNav* _tercom;
static int _servfd;   // socket to bind
static int _connfd;   // socket to handle client
static bool _connected;
static struct commsT _ct;
static struct commsT _ack(TRN_ACK);
static struct commsT _nack(TRN_NACK);
static struct commsT _offset(TRN_GET_ESTNAVOFS,0.,0.,0.);
static struct commsT _sdev(TRN_GET_INITSTDDEVXYZ,0.,0.,0.);

static struct sockaddr_in _server_addr;     // Server Socket object

static char _sock_buf[TRN_MSG_SIZE];
static char logbuf[LOGBUF_BYTES];


// Return true/false if server has a connection to the client.
// Uses select() to determine if the client has hung-up
//
bool is_connected() {

	// If we haven't been connected or the client closed the connection,
	// don't bother checking.
	//
	if(!_connected) {
		return _connected;
	}

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000L;

	// Use select to see if the client closed the connection
	//
	fd_set clientfd;
	FD_ZERO(&clientfd);
	FD_SET(_connfd, &clientfd);
	char temp[5];

	// If the socket is readable but there are no bytes, the client sent FIN
	//
	int nready = select(_connfd + 1, &clientfd, 0, 0, &tv);
	if(nready > 0) {
		if(0 == recv(_connfd, temp, 1, MSG_PEEK)) {
			_connected = false;
			logs(TL_OMASK(TL_TRN_SERVER, TL_BOTH),"%s","Client closed connection");
			::close(_connfd);
		} else {
			_connected = true;   // Connected and there is data to read
		}
	} else {
		_connected = true;     // Connected but no data to read
	}
	return _connected;
}


// Get a message from the socket connection.
// Returns the length of the message packet read from the socket.
// A length of zero indicates socket read timed-out.
//
int get_msg() {
	bool debug = false;
	int len = 0;

	// Get a message as long as client is still connected
	//
	if(is_connected()) {
		int ntries = MAX_RECV_ATTEMPTS;
		int sl = 0;
		for(len = 0; len < TRN_MSG_SIZE;) {
			if(ntries != 3) {
				logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"%s","Get more after interrupted recv\n");
			}
			sl = recv(_connfd, _sock_buf + sl, TRN_MSG_SIZE - sl, 0);
			if(sl <= 0) {
				snprintf(logbuf, LOGBUF_BYTES, "get_msg timeout, errno[%d] sl[%d] - %s", errno, sl,strerror(errno)); // or error
				//perror(logbuf);
				logs(TL_OMASK(TL_TRN_SERVER, TL_LOG|TL_SERR),"%s\n",logbuf);

				if(errno == EINTR && ntries-- > 0) {  // try again
					snprintf(logbuf, LOGBUF_BYTES,
							"%d: recv call interrupt after %d bytes.\n",
							MAX_RECV_ATTEMPTS - ntries, len);
					logs(TL_OMASK(TL_TRN_SERVER, TL_LOG), "%s", logbuf);
					continue;
				} else {
					return 0;
				}
			}
			len += sl;
		}

		// Verbose debugging output
		//
		if(len > 0 && (debug || TRN_DEBUG)) {
            char obuf[DEBUG_BUF_BYTES]={0};
            char *bp=obuf;
            int wbytes=0;
            size_t rem = DEBUG_BUF_BYTES;
			for(int i = 0; i < 100; i++) {
                if( (wbytes = snprintf(bp, rem, "%x ", _sock_buf[i])) > 0){
                    bp += wbytes;
                    rem -= wbytes;
                }else{
                    fprintf(stderr,"WARN: get_msg - debug snprintf failed\n");
                }

                if (bp > (obuf+DEBUG_BUF_BYTES)) {
                    fprintf(stderr,"WARN: get_msg - debug buffer full, truncating\n");
                    break;
                }
			}
            logs(TL_SERR,"%s\n",obuf);
		}
	}
	return len;
}


// Sends a commsT object to client over socket connection.
//
size_t send_msg(commsT& msg) {
	size_t sl = 0;
	snprintf(logbuf, LOGBUF_BYTES, "Sending:%s", msg.to_s(_sock_buf, sizeof(_sock_buf)));
	if (msg.msg_type == TRN_NACK) logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"%s",logbuf);
	//logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"%s",logbuf);

	// Check to see if client is still connected first
	//
	if(is_connected()) {
		memset(_sock_buf, 0, sizeof(_sock_buf));
		msg.serialize(_sock_buf);

		// Send the whole message
        // TODO: check send return value (may return -1, and this doesn't handle it)
		for(sl = 0; sl < sizeof(_sock_buf);) {
            ssize_t test=0;
            if( (test=send(_connfd, _sock_buf, sizeof(_sock_buf), 0))>=0){
                sl+=test;
            }// else error
		}
	}
	return sl;
}


// Initialize local TerrainNav object for operation.
//
int init() {

	// Destruct any existing current TerrainNav
	//
	if(_tercom) {
		delete _tercom;
		_tercom = 0;
	}

	// Construct a TerrainNav object using the info from the client
	// Use environment variables to find location of maps and datafiles.
	//
    char mapname[MAPNAME_BUF_BYTES] = {0};
    char cfgname[CFGNAME_BUF_BYTES] = {0};
    char particlename[PARTICLENAME_BUF_BYTES] = {0};
	char* mapPath = getenv("TRN_MAPFILES");
    char* cfgPath = getenv("TRN_DATAFILES");
    char* logPath = getenv("TRN_LOGFILES");

    fprintf(stderr, "ENV: maps:%s cfgs:%s logs:%s\n", mapPath, cfgPath, logPath);
    fprintf(stderr, "CT: map:%s cfg:%s par:%s\n", _ct.mapname, _ct.cfgname, _ct.particlename);

	char dotSlash[] = "./";
	if(mapPath == NULL) {
		mapPath = dotSlash;
	}
	if(cfgPath == NULL) {
		cfgPath = dotSlash;
	}

        snprintf(mapname, MAPNAME_BUF_BYTES, "%s/%s", mapPath, _ct.mapname);
        snprintf(cfgname, CFGNAME_BUF_BYTES, "%s/%s", cfgPath, _ct.cfgname);
        snprintf(particlename, PARTICLENAME_BUF_BYTES, "%s/%s", cfgPath, _ct.particlename);

        // Let's see if these files exist right now as
        // this will save headaaches later
        //
        if (0 != access(mapname, F_OK))
        {
           logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Map %s not found",
                mapname);
           throw Exception("trn_server: map file not found");
        }

        if (0 != access(cfgname, F_OK))
        {
           logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Cfg %s not found",
                cfgname);
           throw Exception("trn_server: vehicle cfg file not found");
        }

        if ( (NULL!=_ct.particlename) && (0 != access(particlename, F_OK)) )
        {
           logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Particles %s not found - p/s/l[%p/%s/%zu]",
                particlename,_ct.particlename,_ct.particlename,strlen(_ct.particlename));
           throw Exception("trn_server: particles file not found");
        }

	// get TRN_LOGFILES from env
    // then create if needed and use it
#if 0
    int err;
    char *trnLogDir=getenv("TRN_LOGFILES");
    if (trnLogDir != NULL) {
        int err=0;
        if(mkdir(trnLogDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)!=0){
            // no problem to fail if directory already exists
            err = errno;
            if (err != EEXIST)
            {
               // If the error is about something other than EEXIST
               //
               fprintf(stderr,"\n\n\ttrn_server::init()-ERR! mkdir(%s) failed"
                  " [%d,%s]\n\n", trnLogDir, err,strerror(err));
               // if unable to create due to error,
               // just use current directory
               trnLogDir=".";
            }
        }
    }else{
        // if AUV_LOG_DIR not defined, use current directory
        trnLogDir=".";
    }

    if(_ct.logname == NULL)
    {
        snprintf(logname, LOGNAME_BUF_BYTES, ".");
    }
    else
    {
      // Let's ensure that a unique log folder is created here
      //
      snprintf(logname, LOGNAME_BUF_BYTES,"%s/%s",trnLogDir,_ct.logname);
      int n = 0;

      // mkdir() fails (!= 0) if the directory already exists
      //
      while (mkdir(logname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
      {
         err = errno;
         if ( err != EEXIST )
         {
            // If the error is about something other than EEXIST
            //
            fprintf(stderr,"\n\n\t\ttrn_server::init() - ERR: mkdir(%s) failed"
            " [%d,%s]\n\n", logname, err,strerror(err));
            // if unable to create due to error,
            // just use current directory
            snprintf(logname, LOGNAME_BUF_BYTES, ".");
         }
         else {
             snprintf(logname, LOGNAME_BUF_BYTES, "%s/%s-%02d", trnLogDir, _ct.logname, ++n);
         }
      }
    }
#endif

	// filter type and map type encoded in single integer
	// param = filter*100 + map
	//
	int mapType    = _ct.parameter / 10;
	int filterType = _ct.parameter % 10;

    fprintf(stderr, "Constructing tercom with map:%s, cfg:%s, map type: %d, and filter:%d\n",
         mapname, cfgname, mapType, filterType);

    logs( TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Constructing tercom with map:%s, cfg:%s, map type: %d, and filter:%d\n",
         mapname, cfgname, mapType, filterType);

    logs(TL_OMASK(TL_TERRAIN_NAV, TL_LOG),"TRNBeam,Time,Index,Num,Along,Cross,Alt\n");

    try
    {
       _tercom = new TerrainNav(mapname, cfgname, particlename, filterType, mapType,
        _ct.logname);

      // Acknowledge initialization if successful
      //
      if(_tercom->initialized()) {

        // TL_LOG file now created in TerrainNav object, can be used for
        // trn_server and trn_replay.
        // tl_new_logfile(logname);

	 logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"TerrainNav initialize - done");

	 send_msg(_ack);

      } else {
         logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Failed to initialized TerrainNav object, map:%s cfg:%s",
              mapname, cfgname);

         delete _tercom;   // Uninitialized tercom is no good anyway
         _tercom = NULL;
         send_msg(_nack);
      }
    }

	catch (Exception e)
	{
		// init exceptions are usually errors opening or loading config files
		//
		fprintf(stderr, "trn_server.cpp - init(): %s\n", e.what());
		delete _tercom;
		_tercom = NULL;
		send_msg(_nack);
	}

	return 0;
}


// Forwarded Interpolated Measurement Attitude message
//
int set_ima() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting IMA to %d", _ct.parameter);

	if(_tercom) {
		bool ima = _ct.parameter == 0 ? false : true;
		_tercom->setInterpMeasAttitude(ima);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Vehicle Drift Rate message
//
int set_vdr() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting VDR to %f", _ct.vdr);

	if(_tercom) {
		_tercom->setVehicleDriftRate(_ct.vdr);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Modified Weighting  message
//
int set_mw() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting weighting to %d", _ct.parameter);

	if(_tercom) {
		_tercom->setModifiedWeighting(_ct.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Filter Reinit message
//
int set_fr() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting filter reinits to %d", _ct.parameter);

	if(_tercom) {
		bool fr = _ct.parameter == 0 ? false : true;
		_tercom->setFilterReinit(fr);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Map Interpolation message
//
int set_mim() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting map interp method to %d", _ct.parameter);

	if(_tercom) {
		int mim = _ct.parameter;
		_tercom->setMapInterpMethod(mim);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Filter Gradient message
//
int filter_grd() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Setting filter gradiant to %d", _ct.parameter);

	if(_tercom) {
		if(_ct.parameter == 0) {
			_tercom->useLowGradeFilter();
		} else {
			_tercom->useHighGradeFilter();
		}

		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Get Filter Type request
//
int filter_type() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning filter type...");
	if(_tercom) {
		_ack.parameter = _tercom->getFilterType();

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d", _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded Filter State request
//
int filter_state() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning filter state...");
	if(_tercom) {
		_ack.parameter = _tercom->getFilterState();

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d\n", _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for number of filter reinitializations
//
int num_reinits() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG|TL_SERR),"Returning number of reinits...");
	if(_tercom) {
		_ack.parameter = _tercom->getNumReinits();

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG|TL_SERR),"%s - parameter = %d\n",__func__, _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for number of outstanding measurements
//
int out_meas() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning outstanding measurements...");
	if(_tercom) {
		if(_tercom->outstandingMeas()) {
			_ack.parameter = 1;
		} else {
			_ack.parameter = 0;
		}

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d", _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for last included measuerment
//
int last_meas() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning last measurement...");
	if(_tercom) {
		if(_tercom->lastMeasSuccessful()) {
			_ack.parameter = 1;
		} else {
			_ack.parameter = 0;
		}

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d\n", _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for convergence status
//
int is_conv() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning converged");
	if(_tercom) {
		if(_tercom->isConverged()) {
			_ack.parameter = 1;
		} else {
			_ack.parameter = 0;
		}

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d", _ack.parameter);
		send_msg(_ack);
	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}

static poseT _currEst;
// Forwarded measure update message
//
int measure_update() {
    static char obuf[256],*bp=NULL;

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Received measure update with time %f and %d measurements.",
	     _ct.mt.time, _ct.mt.numMeas );

	if(_tercom) {
		_tercom->measUpdate(&_ct.mt, _ct.parameter);
		// Some debugging output from Stanford ARL
		//
		if (_tercom->lastMeasSuccessful())
		{
			poseT mleEst, mmseEst;

         // Don't perform estimates here. Client should trigger these
         //
			// _tercom->estimatePose(&mleEst,  1);
			// _tercom->estimatePose(&mmseEst, 2);

            memset(obuf,0,256);
            bp=obuf;
            size_t rem = 256;
			int wb = snprintf(bp, rem, "\nARL : Estimation Bias(Max. Likelihood): (t = %.2f)\n",
				mleEst.time);
            bp=obuf+strlen(obuf);
            rem -= (wb > 0 ? wb : 0);
			wb = snprintf(bp, rem, "ARL : North: %.4f, East: %.4f, Depth: %.4f\n",
				mleEst.x-_currEst.x, mleEst.y-_currEst.y, mleEst.z-_currEst.z);
            bp=obuf+strlen(obuf);
            rem -= (wb > 0 ? wb : 0);
			wb = snprintf(bp, rem, "ARL : Estimation Bias  (Mean)         : (t = %.2f)\n",
				mmseEst.time);
            bp=obuf+strlen(obuf);
            rem -= (wb > 0 ? wb : 0);
			snprintf(bp, rem, "ARL : North: %.4f, East: %.4f, Depth: %.4f\n",
				mmseEst.x-_currEst.x, mmseEst.y-_currEst.y, mmseEst.z-_currEst.z);
            logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"%s",obuf);
		}

		// send_msg(_ack);
		// Send the measT object back to the client. The measT object
		// will contain the updated alphas
		//
		// printf("Alphas[0] = %f\n", _ct.mt.alphas[0]);
		send_msg(_ct);
      //_tercom->log();     // Log data

	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded motion update message
//
int motion_update() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Received motion update with time %f", _ct.pt.time);

	if(_tercom) {
		_tercom->motionUpdate(&_ct.pt);

		_currEst = _ct.pt;    // For debugging maintain the current position
		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"INS : North: %.2f, East: %.2f, Depth: %.2f\n",
			_currEst.x, _currEst.y, _currEst.z);

		send_msg(_ack);

		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"motion update completed");
      //_tercom->log();     // Log data

	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for MLE estimated position
//
int send_mle() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Client requests MLE...");
	if(_tercom) {
		_tercom->estimatePose(&_ct.pt, 1);
		send_msg(_ct);
      //_tercom->log();     // Log data

	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}


// Forwarded request for MMSE estimated position
//
int send_mmse() {

	logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Client requests MMSE...");
	if(_tercom) {
		_tercom->estimatePose(&_ct.pt, 2);
		send_msg(_ct);
      //_tercom->log();     // Log data

	} else {
		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
		send_msg(_nack);
	}

	return 1;

}

// set initialization xyz
//
int set_init_stddev_xyz() {

    int retval=-1;

    logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"%s %lf,%lf,%lf", __func__, _ct.xyz_sdev.x, _ct.xyz_sdev.y, _ct.xyz_sdev.z);

    if(_tercom) {
        _tercom->setInitStdDevXYZ(_ct.xyz_sdev.x, _ct.xyz_sdev.y, _ct.xyz_sdev.z);
        send_msg(_ack);
        retval=0;
    } else {
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
        send_msg(_nack);
    }

    return retval;

}

int get_init_stddev_xyz() {

    int retval=-1;

    if(_tercom) {
        _tercom->getInitStdDevXYZ(&_sdev.xyz_sdev);
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"%s %lf,%lf,%lf", __func__,  _sdev.xyz_sdev.x, _sdev.xyz_sdev.y, _sdev.xyz_sdev.z);
        _sdev.parameter = 0;
        send_msg(_sdev);
        retval=0;
    } else {
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
        send_msg(_nack);
    }

    return retval;

}

int set_est_nav_ofs() {
    int retval=-1;
    logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"%s %lf,%lf,%lf", __func__, _ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z);

    if(_tercom) {
        _tercom->setEstNavOffset(_ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z);
        send_msg(_ack);
        retval=0;
    } else {
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
        send_msg(_nack);
    }

    return retval;

}

int get_est_nav_ofs() {
    int retval=-1;

    if(_tercom) {
        _tercom->getEstNavOffset(&_offset.est_nav_ofs);
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"%s %lf,%lf,%lf\n",__func__,
             _offset.est_nav_ofs.x, _offset.est_nav_ofs.y, _offset.est_nav_ofs.z);
        _offset.parameter = 0;
        send_msg(_offset);
        retval=0;
    } else {
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
        send_msg(_nack);
    }

    return retval;

}

int is_init() {

    logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Returning is_init");
    if(_tercom) {
        if(_tercom->initialized()) {
            _ack.parameter = 1;
        } else {
            _ack.parameter = 0;
        }

        logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"parameter = %d", _ack.parameter);
        send_msg(_ack);
    } else {
        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"No TRN object! Have you initialized yet?");
        send_msg(_nack);
    }

    return 1;

}


// Main function for server process.
//
// Setup socket and listen for TRN client connection. When connected,
// enter Message loop to read and handle messages from client. Loop is
// exited when good-bye received or the connection is dropped by client.
// Server returns to listening for connection.
//
int main(int argc, char** argv) {
	char c;
	int port = 27027;
    int exit_after_n_cycles=-1;

	while((c = getopt(argc, argv, "ihp:x:")) != -1)
		switch(c) {
			case 'p':
				port = atoi(optarg);
				break;
            case 'x':
                exit_after_n_cycles=atoi(optarg);
                break;
            case 'i':
                TNavConfig::instance()->setIgnoreGps(1);
                fprintf(stderr,"TerrainNav will ignore the gpsValid flag\n");
                break;
            case 'h':
                fprintf(stderr,"\n");
                fprintf(stderr,"Usage: trn_server [-p <port>] [-i -x -h]\n");
                fprintf(stderr,"\n");
                fprintf(stderr,"-i    : ignore the gpsValid flag (just pretend we're at depth)\n");
                fprintf(stderr,"-x <n>: exit after n connections (for debugging)\n");
                fprintf(stderr,"-h    : print this help message\n");
                fprintf(stderr,"\n");
                exit(0);
                break;
			default:
				break;
		}

    // Configure module level logging overrides
    // By default, modules send output to the log file only
    // Initial trn_server output will go to stderr,
    // unless otherwise specified.
    // See trn_Log.h
//            tl_mconfig(TL_TRN_SERVER, TL_SERR, TL_ALL);
    tl_mconfig(TL_TRN_SERVER, TL_SERR, TL_NC);
    //    tl_mconfig(TL_STRUCT_DEFS, TL_SERR, TL_NC);
          tl_mconfig(TL_TERRAIN_NAV, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_NAV_AID_LOG, TL_SERR, TL_NC);
          tl_mconfig(TL_TNAV_CONFIG, TL_SERR, TL_NC);
          tl_mconfig(TL_TNAV_PARTICLE_FILTER, TL_SERR, TL_NC);
          tl_mconfig(TL_TNAV_FILTER, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TNAV_POINT_MASS_FILTER, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TNAV_SIGMA_POINT_FILTER, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TNAV_EXT_KALMAN_FILTER, TL_SERR, TL_NC);
    //    tl_mconfig(TL_MATRIX_ARRAY_CALCS, TL_SERR, TL_NC);
          tl_mconfig(TL_TERRAIN_MAP, TL_SERR, TL_NC);
    //    tl_mconfig(TL_TERRAIN_MAP_DEM, TL_SERR, TL_NC);

	_tercom = 0;
	int len = 0;
    int err=0;

	// Socket setup section
	//
	_servfd = socket(AF_INET, SOCK_STREAM, 0);
    err=errno;
	if(_servfd < 0) {
        fprintf(stderr,"trn_server: socket failed [%d - %s]\n",err,strerror(err));
		exit(1);
	}

	memset(&_server_addr, '0', sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_server_addr.sin_port = htons(port);

    const int optionval = 1;
#if !defined(__CYGWIN__)
    setsockopt(_servfd, SOL_SOCKET, SO_REUSEPORT, &optionval, sizeof(optionval));
#endif
    setsockopt(_servfd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(optionval));

	len = ::bind(_servfd, (struct sockaddr*)&_server_addr, sizeof(_server_addr));
    err=errno;
	if(len < 0) {
        fprintf(stderr,"trn_server: bind failed [%d - %s]\n",err,strerror(err));
		exit(1);
	}

	/////////////////////////////////////////////////////////////////////
	// Server loop: Accept connection, service client until client is
	// done, repeat.
	//
	while(true) {

      char *maps = getenv("TRN_MAPFILES");

      // Release the Map that was allocated
      // the last connection cycle (if any)
      //
      if (_tercom) _tercom->releaseMap();

//		logg("Listen for TerrainNavClient connection - message size[%d]...\n", TRN_MSG_SIZE);
//		logs(TL_OMASK(TL_TRN_SERVER, TL_LOG),"Listen for TerrainNavClient connection - message size[%d]...\n", TRN_MSG_SIZE);
		logs(TL_SERR,"Listen for TerrainNavClient connection - message size[%d], maps %s...\n", TRN_MSG_SIZE, maps);
		listen(_servfd, 10);

		_connfd = accept(_servfd, (struct sockaddr*)NULL, NULL);
		_connected = true;

		struct timeval tv;
		tv.tv_sec = 180;
		tv.tv_usec = 0;

		logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG)),"Listen and accept\n");
		setsockopt(_connfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
      int sockopt = 1;
      setsockopt(_connfd, SOL_SOCKET, SO_REUSEADDR,
                                (const void *)&sockopt, sizeof(int));
      logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Client connected!\n");
//      logs(TL_SERR, "Client connected!\n");


		///////////////////////////////////////////////////////////////////////
		// Message loop: Receive and respond to messages from the client until
		// the client breaks the connection (closes the link or says goodbye).
		//
		while(_connected) {
			memset(_sock_buf, '0', sizeof(TRN_MSG_SIZE));

			// Get a msg from the client
			//
			int len;
			if((len = get_msg()) < TRN_MSG_SIZE) {
				continue;
			}

			// Determine message type and respond
			//
			_ct.clean();
			len = _ct.unserialize(_sock_buf, TRN_MSG_SIZE);

#if TRN_DEBUG
 		  if (_ct.msg_type == TRN_MEAS)
      {
        int i;
        printf("server\n");
        if (_ct.mt.altitudes) for (i = 0; i < _ct.mt.numMeas; i++) printf("%.1f  ", _ct.mt.altitudes[i]);
        printf("\n"); for (i = 493; i < 543; i++) printf("%d:%2x ", i, _sock_buf[i]);
        printf("\n"); for (i = 543; i < 593; i++) printf("%d:%2x ", i, _sock_buf[i]);
        printf("\n"); for (i = 593; i < 643; i++) printf("%d:%2x ", i, _sock_buf[i]);
        printf("\n"); for (i = 643; i < 693; i++) printf("%d:%2x ", i, _sock_buf[i]);
        printf("\n");
      }
#endif

			// OK, we got a message, let's see if we have a tercom to
			// handle it
			//
 			if (!_tercom && _ct.msg_type != TRN_INIT) {
				send_msg(_nack);
				logs(TL_OMASK(TL_TRN_SERVER, TL_BOTH),"Unable to accept requests: server not initialized\n");
				continue;
			}

			try
            {
                logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"msg [%3d/%c]  %s\n",_ct.msg_type,_ct.msg_type,commsT::typestr(_ct.msg_type));

                switch(_ct.msg_type) {
                    case TRN_BYE:
                        logs(TL_SERR|TL_LOG,"Client closing connection\n");
                        //close(_connfd);
                        //_connected = false;
                        break;

                    case TRN_INIT:
                        init();
                        break;

                    case TRN_SET_IMA:
                        set_ima();
                        break;

                    case TRN_SET_VDR:
                        set_vdr();
                        break;

                    case TRN_MEAS:
                        measure_update();
                        break;

                    case TRN_MOTN:
                        motion_update();
                        break;

                    case TRN_MLE:
                        send_mle();
                        break;

                    case TRN_MMSE:
                        send_mmse();
                        break;

                    case TRN_SET_MW:
                        set_mw();
                        break;

                    case TRN_SET_FR:
                        set_fr();
                        break;

                    case TRN_SET_MIM:
                        set_mim();
                        break;

                    case TRN_FILT_GRD:
                        filter_grd();
                        break;

                    case TRN_OUT_MEAS:
                        out_meas();
                        break;

                    case TRN_LAST_MEAS:
                        last_meas();
                        break;

                    case TRN_IS_CONV:
                        is_conv();
                        break;

                    case TRN_FILT_TYPE:
                        filter_type();
                        break;

                    case TRN_FILT_STATE:
                        filter_state();
                        break;

                    case TRN_N_REINITS:
                        num_reinits();
                        break;

                    case TRN_FILT_REINIT:
                        _tercom->reinitFilter((_ct.parameter!=0));
                        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Filter reinitialized: id[%0x]\n", _ct.msg_type);
                        send_msg(_ack);
                        break;

                    case TRN_FILT_REINIT_OFFSET:
                        _tercom->reinitFilterOffset((_ct.parameter!=0), _ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z);

                        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Filter reinitialized w/ offset: id[%0x] ofs[%lf, %lf, %lf]\n",
                             _ct.msg_type, _ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z);

                        send_msg(_ack);
                        break;
                    case TRN_FILT_REINIT_BOX:
                        _tercom->reinitFilterBox((_ct.parameter!=0), _ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z, _ct.xyz_sdev.x, _ct.xyz_sdev.y, _ct.xyz_sdev.z );

                        logs(TL_OMASK(TL_TRN_SERVER, (TL_LOG|TL_SERR)),"Filter reinitialized w/ box: id[%0x] ofs[%lf, %lf, %lf] sdev[%lf, %lf, %lf]\n",
                             _ct.msg_type, _ct.est_nav_ofs.x, _ct.est_nav_ofs.y, _ct.est_nav_ofs.z,
                             _ct.xyz_sdev.x, _ct.xyz_sdev.y, _ct.xyz_sdev.z);

                        send_msg(_ack);
                        break;

                    case TRN_SET_INITSTDDEVXYZ:
                        set_init_stddev_xyz();
                        break;

                    case TRN_GET_INITSTDDEVXYZ:
                        get_init_stddev_xyz();
                        break;

                    case TRN_SET_ESTNAVOFS:
                        set_est_nav_ofs();
                        break;

                    case TRN_GET_ESTNAVOFS:
                        get_est_nav_ofs();
                        break;
                    case TRN_IS_INIT:
                        is_init();
                        break;

                    case TRN_ACK:
                    case TRN_NACK:
                    default:
                        logs(TL_OMASK(TL_TRN_SERVER, TL_BOTH),"No handler for message: id[%0x]\n", _ct.msg_type);
                        send_msg(_nack);
                }
            }
			catch (Exception e)
			{
				snprintf(logbuf, LOGBUF_BYTES, "trn_server: Exception during %c msg: %s",
					_ct.msg_type, EXP_MSG);
				fprintf(stderr, "%s\n", logbuf);
				logs(TL_OMASK(TL_TRN_SERVER, TL_BOTH), "%s\n", logbuf);
				send_msg(_nack);
			}

		}

      // release commsT resources allocated
      // once per connection cycle
      _ct.release();

      // for debug, quit after first connection
      // enabling diagnostics (e.g. valgrind) to complete
      if(--exit_after_n_cycles == 0)
      {
          break;
      }
	}

    // release allocated resources
    // (otherwise, valgrind will flag them as issues)
    close(_connfd);
    close(_servfd);
    delete _tercom;
    tl_release();
    TNavConfig::release();
    // return normally
    return 0;
}
