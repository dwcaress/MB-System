///
/// @file mkvconf.c
/// @authors k. headley
/// @date 03 apr 2020

/// Key/Value config file reader
/// user implements value parser function

/// @sa doxygen-examples.c for more examples of Doxygen markup

/////////////////////////
// Terms of use
/////////////////////////

// Copyright Information
//
// Copyright 2002-YYYY MBARI
// Monterey Bay Aquarium Research Institute, all rights reserved.
//
// Terms of Use
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version. You can access the GPLv3 license at
// http://www.gnu.org/licenses/gpl-3.0.html
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details
// (http://www.gnu.org/licenses/gpl-3.0.html)
//
// MBARI provides the documentation and software code "as is", with no warranty,
// express or implied, as to the software, title, non-infringement of third party
// rights, merchantability, or fitness for any particular purpose, the accuracy of
// the code, or the performance or results which you may obtain from its use. You
// assume the entire risk associated with use of the code, and you agree to be
// responsible for the entire cost of repair or servicing of the program with
// which you are using the code.
//
// In no event shall MBARI be liable for any damages, whether general, special,
// incidental or consequential damages, arising out of your use of the software,
// including, but not limited to, the loss or corruption of your data or damages
// of any kind resulting from use of the software, any prohibited use, or your
// inability to use the software. You agree to defend, indemnify and hold harmless
// MBARI and its officers, directors, and employees against any claim, loss,
// liability or expense, including attorneys' fees, resulting from loss of or
// damage to property or the injury to or death of any person arising out of the
// use of the software.
//
// The MBARI software is provided without obligation on the part of the
// Monterey Bay Aquarium Research Institute to assist in its use, correction,
// modification, or enhancement.
//
// MBARI assumes no responsibility or liability for any third party and/or
// commercial software required for the database or applications. Licensee agrees
// to obtain and maintain valid licenses for any additional third party software
// required.


/// Headers
#include <fcntl.h>
#include "mkvconf.h"
#include "mxdebug.h"

/// Macros

/// Declarations
typedef struct mkvc_s{
    char *fpath;
    FILE *fp;
    char *del;
    mkvc_parse_fn parser;
}mkvc_reader_t;

/// Imports

/// Module Global Variables

/// Function Definitions

static char *s_trim_trailing(char *tok)
{
    char *cur=NULL;
    if(tok!=NULL){
        cur=tok+strlen(tok);
        while(cur>tok && (*cur=='\0' || isspace(*cur))){
            *cur='\0';
            cur--;
            if(cur<=tok)break;
        }
    }
    return cur;
}

// trim leading/trailing whitespace and comments from token
static char *s_trim_token(char **ptok)
{
    char *retval=NULL;
    if(NULL!=ptok && NULL!=*ptok){
        // make a working copy
        char *tcpy=strdup(*ptok);

        // trim, truncate comments
        char *cur = tcpy;
        char *start=cur;
        char *end = cur+strlen(cur);
        int err_count=0;

        // trim leading space
        while(cur<end && isspace(*cur) )cur++;
        start=cur;

        // truncate at comment
        cur=end;
        while(cur<start){
            if(*cur=='#' || (*cur=='/' && *(cur-1)=='/') ){
                *cur='\0';
                break;
            }
            cur--;
        }

        // trim trailing space
        end=s_trim_trailing(start);

        // check quotes:
        // find opening/closing quotes, check mismatch/ordering
        enum{SQO,SQC,DQO,DQC};
        char *qcur[4]={strstr(start,"\'"),strrchr(start,'\''),strstr(start,"\""),strrchr(start,'"')};
        bool sq_pair=false;
        bool dq_pair=false;
        // If there is a quote, both open/close will be non-NULL
        // If so then check whether they are the same
        if(qcur[SQO]!=NULL){
			// contains single quote
            if(qcur[SQC]>qcur[SQO]){
                // valid squote pair
                sq_pair=true;
            }else{
                // mismatched squote
                fprintf(stderr,"ERR - mismatched single quote in [%s]\n",tcpy);
                err_count++;
            }
        }
        if(qcur[DQO]!=NULL){
            // contains double quote
            if(qcur[DQC]>qcur[DQO]){
                // valid dquote pair
                dq_pair=true;
            }else{
                // mismatched dquote
                fprintf(stderr,"ERR - mismatched double quote in [%s]\n",tcpy);
                err_count++;
            }
        }

        // check quote order
        // if there are multiple quote pairs,
        // there must be one enclosing pair (nesting is valid)
        // nested quotes are NOT checked for mismatch
        if(sq_pair && dq_pair){
            // there are both single and double quotes
            // valid: ["''"] ['""']
            if( (qcur[SQO]>qcur[DQO] && qcur[SQC]<qcur[DQC]) ||
               (qcur[DQO]>qcur[SQO] && qcur[DQC]<qcur[SQC]) ){

                if( (qcur[SQO]==start && qcur[SQC]==end)  ||
                    (qcur[DQO]==start && qcur[DQC]==end)  ){
                    // quote OK
                }else{
                    // text outside of quotes
                    fprintf(stderr,"ERR - unenclosed content in [%s]\n",tcpy);
                    err_count++;
                }
            }else{
                // quote error
                fprintf(stderr,"ERR - invalid quote in [%s]\n",tcpy);
                err_count++;
            }

            // truncate after last quote
            if(qcur[SQC]>qcur[DQC]){
                *(qcur[SQC]+1)='\0';
            }else{
                *(qcur[DQC]+1)='\0';
            }
        }else if(sq_pair){
            if(start<qcur[SQO] || (end-1)>qcur[SQC]){
                // text outside of quotes
                fprintf(stderr,"ERR - unenclosed content in [%s]\n",tcpy);
                err_count++;
            }
            // truncate after single quote
            *(qcur[SQC]+1)='\0';
        }else if(dq_pair){
            if(start<qcur[DQO] || (end-1)>qcur[DQC]){
                // text outside of quotes
                fprintf(stderr,"ERR - unenclosed content in [%s]\n",tcpy);
                err_count++;
            }
            // truncate after double quote
            *(qcur[DQC]+1)='\0';
        }

        //        fprintf(stderr,"tcpy[%p] start[%p] end[%p] len[%d]\n",*tcpy,start,end,strlen(start));
        if(err_count==0 && strlen(start)>0){
            MF_MEM_CHKINVALIDATE(*ptok);
            *ptok=strdup(start);
            retval=*ptok;
        }else{
            *ptok=NULL;
        }
//        fprintf(stderr,"ERR - strlen[%lu] err_count[%d] ret[%s]\n",strlen(start),err_count,retval);

        MF_MEM_CHKFREE(tcpy);
    }
    return retval;
}

// get one key/value token string from line
// returns new, trimmed key/value token strings
// caller must release using free()
int mkvc_parse_kx(char *line, const char *del, char **pkey, char **pval, bool val_required)
{

    int retval=-1;

    if(NULL!=line && NULL!=del && NULL!=pkey && NULL!=pval){
        // make a working copy of the line
        char *lcopy=strdup(line);
        // get the key token
        char *tok=strtok(lcopy,del);

        if(tok!=NULL){
            // set key return value (caller must free)
            MF_MEM_CHKFREE(*pkey);
           *pkey=strdup(tok);
            // value token is everything to the end of the line
             tok=strtok(NULL,del);

            if(NULL!=tok && (strlen(tok)>0) ){
                MF_MEM_CHKFREE(*pval);
                *pval=strdup(tok);
            }
        }else{
            MX_TRACE();
        }

        MF_MEM_CHKFREE(lcopy);

        if(NULL!=*pkey && (NULL!=*pval || !val_required)){

            char *test[2]={s_trim_token(pkey),s_trim_token(pval)};

        	if( test[0]!=NULL && (test[1]!=NULL || !val_required)){
                retval=0;
            }else{
                // report error detail
                if(test[0]!=NULL){
                    // key valid, val invalid
                    s_trim_trailing(*pval);
                    fprintf(stderr, "ERR - invalid val token k[%s] v[%s]\n",*pkey,*pval?*pval:"");
                }

                if(test[1]!=NULL){
                    // key invalid, val valid
                    s_trim_trailing(*pkey);
                    fprintf(stderr, "ERR - invalid key token k[%s] v[%s]\n",*pkey?*pkey:"",*pval);
                }
                // release resources
                MF_MEM_CHKINVALIDATE(*pkey);
                MF_MEM_CHKINVALIDATE(*pval);
            }
        }else{
            MF_MEM_CHKINVALIDATE(*pkey);
            MF_MEM_CHKINVALIDATE(*pval);
            MX_TRACE();
        }
    }
    return retval;
}
int mkvc_parse_kv(char *line, const char *del, char **pkey, char **pval)
{
    return mkvc_parse_kx(line, del, pkey, pval, true);
}

mkvc_reader_t *mkvc_new(const char *file, const char *del, mkvc_parse_fn parser)
{
    mkvc_reader_t *instance = (mkvc_reader_t *)malloc(sizeof(mkvc_reader_t));
    if(NULL!=instance && NULL!=file && NULL!=parser){
        instance->fpath=strdup(file);
        instance->del=(NULL!=del ? strdup(del) : MKVC_DEL_DFL);
        instance->parser=parser;
    }
    return instance;
}

void mkvc_destroy(mkvc_reader_t **pself)
{
    if(NULL!=pself){
        mkvc_reader_t *self = *pself;
        if(NULL!=self){

            if(NULL!=self->fpath){
                free(self->fpath);
            }
            if(NULL!=self->del){
                free(self->del);
            }
            free(self);
            *pself=NULL;
        }
    }
    return;
}


static bool s_is_ignore(char *line)
{
    bool retval=true;

    if(NULL!=line){
        size_t len = strlen(line)+1;
        if((len-1)>0){
            char *cp=line;
            bool quit=false;
            while(!quit && *cp!='\0' && cp<(line+len)){
                switch (*cp) {
                    case ' ':
                    case '\t':
                    case '\r':
                    case '\n':
                        break;

                    case '#':
                        quit=true;
                        break;

                    case '/':
                        if(*(cp+1)!='/'){
                            retval=false;
                        }
                        quit=true;
                        break;
                    case '\0':
                        quit=true;
                        break;
                    default:
                        retval=false;
                        quit=true;
                        break;
                }
                cp++;
            }
        }
    }

    return retval;
}

// load configuration from file
// returns number of valid parsed options on success, -1 otherwise
int mkvc_load_config(mkvc_reader_t *self, void *cfg, int *r_par, int *r_inv, int *r_err)
{
    int retval=-1;
    int par_count=0;
    int inv_count=0;
    int err_count=0;
    if(NULL!=self && NULL!=self->fpath && NULL!=self->del ){
        self->fp=fopen(self->fpath,"r+");
        if(NULL!=self->fp){
            // read and parse until end of file
            char line[MKVC_LINE_BUF_LEN]={0};
            char *pline=line;
            while((pline=fgets(line,MKVC_LINE_BUF_LEN,self->fp))!=NULL){
                if(!s_is_ignore(pline)){
                    // extract one key/value pair per line
                    // [mbtrnpp_get_kv returns new strings, must release using free()]
                    char *key=NULL;
                    char *val=NULL;

                    if(mkvc_parse_kx(pline,self->del,&key,&val,false)==0 &&
                       NULL!=key){
                        // parse key/value into configuration
                        if(self->parser(key,val,cfg)==0){
                            par_count++;
                        }else{
                            fprintf(stderr, "ERR - invalid key/value [%s/%s]\n",key,val);
                            inv_count++;
                        }
                    }else{
                        s_trim_trailing(pline);
                        fprintf(stderr, "ERR - parse error in [%s]\n", pline);
                        err_count++;
                    }
                    // release key/value strings
                    if(NULL!=key)free(key);
                    if(NULL!=val)free(val);
                }
            }
            retval=(err_count>0?-1:0);


            fclose(self->fp);
            self->fp=NULL;
        }else{
            fprintf(stderr, "ERR - could not open config file [%s] [%d/%s]\n", self->fpath,errno,strerror(errno));
            err_count++;
        }
    }else{
        fprintf(stderr, "ERR - NULL config path\n");
        err_count++;
    }

    if(NULL!=r_par)*r_par=par_count;
    if(NULL!=r_inv)*r_inv=inv_count;
    if(NULL!=r_err)*r_err=err_count;

//    fprintf(stderr,"load_cfg ret[%d] par[%d] inv[%d] err[%d]\n",retval,par_count,inv_count,err_count);
    return retval;
}

int mkvc_parse_bool(const char *src, bool *dest)
{
    int retval=-1;
    if(NULL!=src && NULL!=dest){
        // flip retval so we don't have to set it many times
        retval=0;
        if(strcasecmp(src,"Y")==0){
            *dest=true;
        }else if(strcasecmp(src,"N")==0){
            *dest=false;
        }else if(strcasecmp(src,"TRUE")==0){
            *dest=true;
        }else if(strcasecmp(src,"FALSE")==0){
            *dest=false;
        }else if(strcasecmp(src,"1")==0){
            *dest=true;
        }else if(strcasecmp(src,"0")==0){
            *dest=false;
        }else {
            retval=-1;
        }
    }// invalid arg
    return retval;
}

#ifdef WITH_MKVCONF_TEST
bool g_mkvconf_test_quit=false;

typedef struct cfg_s{
    int ipar;
    unsigned int xpar;
    float fpar;
    char cpar;
    char *spar;
    bool bpar;
    bool flagpar;
}cfg_t;

// parse key/value strings into configuration values
static int s_test_parser(char *key, char *val, void *config)
{
    int retval=0;
    if(NULL!=key && NULL!=config){
        cfg_t *cfg=(cfg_t *)config;
//        fprintf(stderr, ">>>> PARSING key/val [%s:%s]\n", key,val);
        retval=-1;

        if(NULL!=val){
            // args requiring values
            if(strcmp(key,"ipar")==0 ){
                if(sscanf(val,"%d",&cfg->ipar)==1){
                    retval=0;
                }
            }else if(strcmp(key,"xpar")==0 ){
                if(sscanf(val,"%x",&cfg->xpar)==1 || sscanf(val,"%X",&cfg->xpar)==1){
                    retval=0;
                }
            }else if(strcmp(key,"fpar")==0 ){
                if(sscanf(val,"%f",&cfg->fpar)==1){
                    retval=0;
                }
            }else if(strcmp(key,"cpar")==0 ){
                if(sscanf(val,"%c",&cfg->cpar)==1){
                    retval=0;
                }
            }else if(strcmp(key,"spar")==0 ){
                if((cfg->spar=strdup(val))!=NULL){
                    retval=0;
                }
            }else if(strcmp(key,"bpar")==0 ){
                // flip retval so we don't have to set it many times
                retval=0;
                if(strcasecmp(val,"Y")==0){
                    cfg->bpar=true;
                }else if(strcasecmp(val,"N")==0){
                    cfg->bpar=false;
                }else if(strcasecmp(val,"TRUE")==0){
                    cfg->bpar=true;
                }else if(strcasecmp(val,"FALSE")==0){
                    cfg->bpar=false;
                }else if(strcasecmp(val,"1")==0){
                    cfg->bpar=true;
                }else if(strcasecmp(val,"0")==0){
                    cfg->bpar=false;
                }else {
                    retval=-1;
                }
            }else{
                fprintf(stderr, "WARN - unsupported key/val [%s/%s]\n", key,val);
            }

        }else{
            // args with optional/no values
            if(NULL!=key && strcmp(key,"flagpar")==0 ){
                // flag par - no value required
                cfg->flagpar=true;
                retval=0;
            }else{
                fprintf(stderr, "WARN - unsupported key/val [%s/%s]\n", (NULL==key?"NULL":key), (NULL==val?"NULL":val));
            }
        }
    }else{
        fprintf(stderr, "ERR - NULL key/val [%s/%s]\n", (NULL==key?"NULL":key), (NULL==val?"NULL":val));
    }

    return retval;
}

static int s_mk_test_conf(const char *file)
{
    int retval=-1;
    if(NULL!=file){
        fprintf(stderr, "creating test file [%s]\n", file);
        FILE *fp = NULL;
        if( (fp=fopen(file,"w"))!=NULL){
            fwrite("# test config (auto-generated)\n",strlen("# test config (auto-generated)\n"),1,fp);
            // 6 valid cases
            fwrite("// int param\n ipar=123\n",strlen("// int param\n ipar=123\n"),1,fp);
            fwrite("// hex param\n xpar=0xCAFE\n",strlen("// hex param\n xpar=0xCAFE\n"),1,fp);
            fwrite("// float param\n fpar=1.23\n",strlen("// float param\n fpar=1.23\n"),1,fp);
            fwrite("// char param\n cpar =X\n",strlen("// char param\n cpar =X\n"),1,fp);
            fwrite("// bool param\n bpar = Y\n",strlen("// bool param\n bpar = Y\n"),1,fp);
            fwrite("// str param\n spar=\"two strings walk into a bar...\\n\"\n",strlen("// str param\n spar=\"two strings walk into a bar...\\n\"\n"),1,fp);
            fwrite("// flag key w/o val\n flagpar\n",strlen("// flag key w/o val\n flagpar\n"),1,fp);

            // 2 valid, but not supported
            fwrite("// nested squotes\n nsq=\'nsq \'are\' \"OK\" \'\n",strlen("// nested squotes\n nsq=\'nsq \'are\' \"OK\" \'\n"),1,fp);
            fwrite("// nested dquotes\n ndq=\"ndq \"are\" \'OK\' \"\n",strlen("// nested squotes\n ndq=\"ndq \"are\" \'OK\' \"\n"),1,fp);

            // 11 pathologies
            fwrite("// mult quotes\n mq=\'msq\' are not \"OK\"\n",strlen("// mult quotes\n mq=\'msq\' are not \"OK\"\n"),1,fp);
            fwrite("// mult quotes\n mq=\"msq\" are not \'OK\'\n",strlen("// mult quotes\n mq=\"msq\" are not \'OK\'\n"),1,fp);
            fwrite("// mismatched squote\n mms=\'mms\n",strlen("// mismatched squote\n mms=\'mms\n"),1,fp);
            fwrite("// mismatched dquote\n mmd=\"mmd\n",strlen("// mismatched dquote\n mmd=\"mmd\n"),1,fp);
            fwrite("// misordered squotes\n mosq=\'mosq are \"bad\' \"\n",strlen("// misordered squotes\n mosq=\'mosq are \"bad\' \"\n"),1,fp);
            fwrite("// misordered dquotes\n modq=\'modq are \"bad\' \"\n",strlen("// misordered dquotes\n modq=\'modq are \"bad\' \"\n"),1,fp);
            fwrite("// unenclosed content\n uc=\'uc is\'bad\n",strlen("// unenclosed content\n uc=\'uc is\' bad\n"),1,fp);
            fwrite("// unenclosed content\n uc=\'uc is\' \"bad\"\n",strlen("// unenclosed content\n uc=\'uc is\' \"bad\"\n"),1,fp);
            fwrite("// mismatched nested\n mmn=\'mmn is \"not good\'\n",strlen("// mismatched nested\n mmn=\'mmn is \"not good\'\n"),1,fp);
            fwrite("// key w/o val\n noval=\n",strlen("// key w/o val\n noval=\n"),1,fp);
            fwrite("// val w/o key\n =-1\n",strlen("// val w/o key\n =-1\n"),1,fp);

            fflush(fp);
            fclose(fp);
            retval=0;
        }
    }else{
        fprintf(stderr, "ERR - NULL file path\n");
    }
    return retval;
}

static void s_cfg_show(cfg_t *self, bool verbose, uint16_t indent)
{
    if (NULL != self) {
        fprintf(stderr,"%*s[self     %10p]\n",indent,(indent>0?" ":""), self);
        fprintf(stderr,"%*s[ipar     %10d]\n",indent,(indent>0?" ":""), self->ipar);
        fprintf(stderr,"%*s[xpar     %10x]\n",indent,(indent>0?" ":""), self->xpar);
        fprintf(stderr,"%*s[fpar     %10f]\n",indent,(indent>0?" ":""), self->fpar);
        fprintf(stderr,"%*s[cpar     %10c]\n",indent,(indent>0?" ":""), self->cpar);
        fprintf(stderr,"%*s[bpar     %10s]\n",indent,(indent>0?" ":""), (self->bpar?"true":"false"));
        fprintf(stderr,"%*s[spar     %10s]\n",indent,(indent>0?" ":""), self->spar);
        fprintf(stderr,"%*s[flagpar  %10s]\n",indent,(indent>0?" ":""), (self->flagpar?"true":"false"));
    }
}

int mkvconf_test()
{
    int retval=-1;

    const char *fpath="mkvc-test.conf";

    cfg_t cfg_instance, *cfg=&cfg_instance;
    memset(cfg,0,sizeof(cfg_t));

    if(s_mk_test_conf(fpath)==0){
        const char *del="=";
        mkvc_reader_t *reader = mkvc_new(fpath, del, s_test_parser);
        int r_par=0;
        int r_inv=0;
        int r_err=0;
        if(NULL!=reader){
            mkvc_load_config(reader, cfg,  &r_par, &r_inv, &r_err);
            s_cfg_show(cfg,true,5);
//            fprintf(stderr,"%d %x %f %c %s %c\n",
//                    cfg->ipar,
//                    cfg->xpar,
//                    cfg->fpar,
//                    cfg->cpar,
//                    cfg->spar,
//                    cfg->bpar?'Y':'N',
//                    cfg->flagpar?'Y':'N');

            // should be 7 parsed, 13 errors [12 invalid k/v, 1 parse error]
            if(r_par==7 && r_inv==12 && r_err==1){
                retval=0;
            }
            mkvc_destroy(&reader);
        }
    }

    bool bval=false;
    uint32_t bool_test=0,i=0;
    if( !(mkvc_parse_bool("true",&bval)==0 && bval==true)){
        bool_test|=(1<<i);
    }
    i++;
    if( !(mkvc_parse_bool("false",&bval)==0 && bval==false)){
        bool_test|=(1<<i);
    }
    i++;
    if( !(mkvc_parse_bool("Y",&bval)==0 && bval==true)){
        bool_test|=(1<<i);
    }
    i++;
    if( !(mkvc_parse_bool("N",&bval)==0 && bval==false)){
        bool_test|=(1<<i);
    }
    i++;
    if( !(mkvc_parse_bool("1",&bval)==0 && bval==true)){
        bool_test|=(1<<i);
    }
    i++;
    if( !(mkvc_parse_bool("0",&bval)==0 && bval==false)){
        bool_test|=(1<<i);
    }
    i++;
    if(bool_test==0){
        fprintf(stderr,"mkvc_parse_bool test     OK [0x%X]\n",bool_test);
    }else{
        fprintf(stderr,"mkvc_parse_bool test FAILED [0x%X]\n",bool_test);
    }
    if(NULL!=cfg->spar){
        free(cfg->spar);
    }
    fprintf(stderr,"mkvconf_test returning %d\n",retval);
    return retval;
}
#endif //WITH_MKVCONF_TEST
