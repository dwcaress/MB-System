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

class DataLogReader;
struct poseT;
struct measT;
class TerrainNav;

class TrnClient : public TerrainNavClient {
    
public:
    
    TrnClient();
    explicit TrnClient(const char *host, int port=0);
    explicit TrnClient(const char *svr_log_dir, const char *host=NULL, int port=0);
    TrnClient(const TrnClient& other);
    virtual ~TrnClient() override;
    static void chkSetString(char **dest, const char *src);
    char *updateSessionDir();
    int loadCfgAttributes(const char *cfg_file, const char *usr_log_path = NULL);
    int setVerbose(int val);
    int initSocket();
    int connectSocket();
    TerrainNav* connectTRN();
    void show(int indent=0, int wkey=15, int wval=18);
    void show_addr(int indent=0, int wkey=15, int wval=18);
    void setQuitRef(bool *pvar);
    bool isQuitSet();
    TrnAttr &getTrnAttr();
    char *attGetServer();

protected:
    int verbose;
    bool *_quit_ref;
    char *_cfg_file;
    TrnAttr _trn_attr;
};

#endif // include guard
