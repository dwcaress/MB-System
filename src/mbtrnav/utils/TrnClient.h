/****************************************************************************/
/* Copyright (c) 2017 MBARI                                                 */
/* MBARI Proprietary Information. All rights reserved.                      */
/****************************************************************************/
/* Summary  : TRN test client using logged data from previous mission  */
/* Filename : TrnClient.h                                                   */
/* Author   : headley                                                       */
/* Project  :                                                              */
/* Version  : 1.0                                                           */
/* Created  : 10/24/2019                                                    */
/* Modified :                                                               */
/* Archived :                                                               */
/****************************************************************************/
/* Modification History:                                                    */
/* Began with a copy of test_client as there is a lot of stuff to reuse     */
/****************************************************************************/
#ifndef _TRN_CLIENT_H_
#define _TRN_CLIENT_H_

#include <sys/stat.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/time.h>
#include "TerrainNavClient.h"
#include "structDefs.h"
#include "TrnAttr.h"


#define TRNCLI_PORT_DFL 27027
#define VNORM_DIM 3
#undef WITH_VNORM_FN
#undef WITH_DEGTORAD_FN
#define SESSION_PREFIX_BUF_BYTES 64

class DataLogReader;
struct poseT;
struct measT;
class TerrainNav;

typedef enum {
    SID_YYYYJJJ = 0,
    SID_YYYYJJJHHMM,
    SID_ISO8601,
    SID_LCMTRN
}SIDFormat;

typedef enum {
    SID_GMT = 0,
    SID_LOC,
}SIDTime;

class TrnClient : public TerrainNavClient {
    
public:
    
    TrnClient();

    explicit TrnClient(const char *host, int port=0);

    TrnClient(const TrnClient& other);

    virtual ~TrnClient() override;

    static void chkSetString(char **dest, const char *src);

    int loadCfgAttributes(const char *cfg_file);

    int setVerbose(int val);

    int initSocket();

    void initServer();

    int connectSocket();

    TerrainNav* connectTRN();

    void show(int indent=0, int wkey=15, int wval=18);

    void show_addr(int indent=0, int wkey=15, int wval=18);

    void setQuitRef(bool *pvar);

    bool isQuitSet();

    TrnAttr &getTrnAttr();

    char *attGetServer();

    void setSessionID(const std::string &session_str);

    char *sessionPrefix(char **r_dest, size_t len, SIDTime sid_time=SID_GMT, SIDFormat sid_fmt=SID_YYYYJJJ);

protected:
    int verbose;
    bool *_quit_ref;
    char *_cfg_file;
    char *_sessionPrefix;
    std::string _sessionID;
    TrnAttr _trn_attr;
};

#endif // include guard
