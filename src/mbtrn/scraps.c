/// mbtrn-test
//static int s_test_mbtrn(const char *file)
//{
//    int retval=0;
//    // configure test server
//    // emulates 7k center, using data from a file
//    iow_file_t *mb_data    = iow_file_new(file);
//    iow_socket_t *svr_sock = iow_socket_new("localhost",R7K_7KCENTER_PORT, ST_TCP);
//    mbtrn_server_t *test_svr    = mbtrn_server_new(svr_sock, mb_data);
//
//    MDEBUG("\nstarting test server\n");
//    mbtrn_server_start(test_svr);
//
//    // set up mbtrn client socket
//    // [connect to 7k center/test server]
//    iow_socket_t *cli_sock = iow_socket_new("localhost",IP_PORT_7K, ST_TCP);
//    mbtrn_connection_t *src = mbtrn_scon_new(cli_sock);
//    mbtrn_connection_t *dest = mbtrn_fcon_new(NULL);
//
//    // create/configure the filter
//    mbtrn_config_t *mbtrn = mbtrn_new(src, dest, MAX_FRAME_BYTES_7K);
//    int status=0;
//    uint32_t error_status=0x0;
//    if( (NULL != mbtrn) && ( (status=mbtrn_config_connect(mbtrn))==0) ){
//        uint32_t frame_count=0;
//        while (error_status==0) {
//            if(mbtrn_read_frame(mbtrn)>0){
//
//                if(mbtrn_filter_frame(mbtrn)==0){
//
//                    if (mbtrn_send(mbtrn)==0) {
//                        MDEBUG("frames sent [%d] \n",++frame_count);
//                    }else{
//                        // send failed
//                        MERROR("send failed\n");
//                        error_status|=0x1;
//                    }
//                }else{
//                    // filter failed
//                    MERROR("filter failed\n");
//                    error_status|=0x2;
//                }
//            }else{
//                // read frame failed
//                MERROR("read frame failed\n");
//                error_status|=0x4;
//            }
//            if (frame_count>=5) {
//                iow_send(mbtrn->src->sock_if,"STOP\n",6);
//                error_status=0x10;
//            }
//        } // end while
//    }else{
//    	// create or connect failed
//        if (NULL==mbtrn) {
//            MERROR("mbtrn_new returned NULL\n");
//        }
//        if (status!=0) {
//            MERROR("connect failed\n");
//        }
//        error_status|=0x8;
//    }
//    if (error_status!=0x0) {
//        MERROR("mbtrn stopped - status[%0X]\n",error_status);
//        retval=error_status;
//    }
//    MDEBUG("releasing resources\n");
//    mbtrn_server_stop(test_svr);
//    mbtrn_server_destroy(&test_svr);
//    mbtrn_destroy(&mbtrn);
//
//    return retval;
//}


static int s_subscribe(uint32_t *records, uint32_t count)
{
    
}




/// mbtrn.c

int mbtrn_init(char *s7k_file)
{
    MTRACE();
    struct mb_io_struct;
    
    int verbose=2;
    char *file=s7k_file;
    int format=88, pings=1, lonflip=0;
    double bounds[4]={-360.0,360.0,-90.0,90.0};
    int btime_i[7]={1962,2,21,10,30,0,0}, etime_i[7]={2062,2,21,10,30,0,0};
    double speedmin=0.0,timegap=1.0;
    void *mbio_ptr=NULL;
    double btime_d=0.0, etime_d=1.0;
    int beams_bath, beams_amp, pixels_ss;
    int error;
    //    char buff[5][256];
    
    //    MDEBUG("calling mb_buffer_init\n");
    //    mb_buffer_init(verbose, (void **)buff, &error);
    
    MDEBUG("calling mb_read_init\n");
    int stat = mb_read_init(verbose, file, \
                            format,  pings,  lonflip,  bounds,\
                            btime_i,  etime_i,\
                            speedmin,  timegap,\
                            &mbio_ptr,  &btime_d,  &etime_d,\
                            &beams_bath,  &beams_amp,  &pixels_ss,\
                            &error);
    MDEBUG("mb_read_init returned %d\n",stat);
    return stat;
}

mbtrn_config_t *mbtrn_new(mbtrn_connection_t *src, mbtrn_connection_t *dest, uint32_t isize)
{
    mbtrn_config_t *self=(mbtrn_config_t *)malloc(sizeof(mbtrn_config_t));
    if (self) {
        memset(self,0,sizeof(mbtrn_config_t));
        self->src = src;
        self->dest = dest;
        if(isize>0){
            self->in_buf=(byte *)malloc(isize*sizeof(byte));
        }
        if (self->in_buf) {
            self->in_size = isize;
        }else{
            self->in_size = 0;
        }
        self->pin = self->in_buf;
        self->auto_free = true;
    }
    return self;
}

void mbtrn_destroy(mbtrn_config_t **pself)
{
    if (pself) {
        mbtrn_config_t *self = *pself;
        if (self) {
            if (self->auto_free) {
                mbtrn_connection_destroy(&self->src);
                mbtrn_connection_destroy(&self->dest);
            }
            free(self->in_buf);
            free(self);
            *pself=NULL;
        }
    }
}

int mbtrn_config_connect(mbtrn_config_t *mbtrn)
{
    int retval=-1;
    if (NULL!=mbtrn && NULL != mbtrn->src && NULL != mbtrn->dest) {
        
        int status=0;
        switch (mbtrn->src->type) {
            case CT_SOCKET:
                if( (status=iow_connect(mbtrn->src->sock_if)) == 0){
                }else{
                    MERROR("source connection failed [%d]\n",status);
                }
                break;
                
            case CT_FILE:
                if( (status=iow_fopen(mbtrn->src->file_if))==0){
                    
                }else{
                    MERROR("source open failed [%d]\n",status);
                }
                break;
                
            case CT_STDIN:
                break;
                
            case CT_NULL:
            case CT_STDOUT:
            default:
                MERROR("Invalid source type [%d]\n",mbtrn->src->type);
                break;
        }
        if (status==0) {
            switch (mbtrn->dest->type) {
                case CT_SOCKET:
                    if( (status=iow_connect(mbtrn->dest->sock_if)) == 0){
                    }else{
                        MERROR("destination connection failed [%d]\n",status);
                    }
                    break;
                    
                case CT_FILE:
                    if( (status=iow_fopen(mbtrn->dest->file_if))==0){
                        
                    }else{
                        MERROR("destination open failed [%d]\n",status);
                        status=0;
                    }
                    break;
                    
                case CT_STDOUT:
                    break;
                    
                case CT_NULL:
                case CT_STDIN:
                default:
                    MERROR("Invalid destination type [%d]\n",mbtrn->dest->type);
                    break;
            }
        }
        retval=status;
    }else{
        MERROR("NULL argument conf[%p] src[%p] dest[%p]\n",mbtrn,(mbtrn?mbtrn->src:0),(mbtrn?mbtrn->dest:0));
    }
    return retval;
}


//uint32_t mbtrn_poll2(mbtrn_reader_t *self, uint32_t len, uint32_t timeout)
//{
//    uint32_t retval=-1;
//
//    if (NULL!=self && NULL!=self->cbuf) {
//
//        // working buffer, large enough to hold one record
//        byte record[R7K_MAX_RECORD_BYTES]={0};
//
//        uint32_t rbytes=0;
//        uint32_t wbytes=0;
//        uint32_t total_bytes=0;
//
//        // read records until we reach hwm
//        // [or space remaining is less than a record]
//        while (total_bytes<=len && cbuf_space(self->cbuf)>R7K_MAX_RECORD_BYTES) {
//            // clear working buffer
//            memset(record,0,R7K_MAX_RECORD_BYTES);
//            // extract next Data Record Frame from socket
//            if ( (rbytes=r7k_read_record(mbtrn_get_sockif(self),record,timeout)) > 0) {
//                int status=0;
//                // write it to the circular buffer
//                if( (wbytes=cbuf_write(self->cbuf,record,rbytes,CB_NONE,&status))==rbytes){
//                	total_bytes+=wbytes;
//                }else{
//                    MERROR("cbuf error ret/stat[%d/%d]\n",wbytes,status);
//                }
//            }else{
//                MERROR("read_record error [%d]\n",rbytes);
//            }
//        }
//
//    }else{
//        MERROR("invalid argument\n");
//    }
//
//    return retval;
//}

// read bytes from Data Record Frames buffered by this reader
uint32_t mbtrn_read(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, int *status)
{
    uint32_t retval=0;
    
    // TODO: use  worker thread to fill buffer asynchronously.
    // The synchronous version must read and refill (poll) as needed.
    // A high/low water mark (HWM,LWH) modulates polling.
    // This will cause a delay when polling is done.
    // Optionally, it could top up each read cycle, which would spread
    // delays across reads.
    
    // Complete records are stored in the buffer, and they can vary in length.
    // If the buffer is a static size, records that won't fit are discarded.
    // HOwever, poll() will stop adding records if the buffer gets within
    // one (max) record size of being full
    if (NULL != self && NULL!=dest && NULL!=status) {
        uint32_t nbytes=0;
        
        if (cbuf_available(self->cbuf)>len) {
            // read, then poll if needed
            cbuf_flag_t cflags=(flags==MBR_ALLOW_PARTIAL?CB_ALLOW_PARTIAL:CB_NONE);
            if( (nbytes=cbuf_read(self->cbuf,dest,len,cflags,status)) >0){
                *status=MBR_OK;
                retval=nbytes;
            }else{
                *status=MBR_EREAD;
            }
            
            // only poll if we're below LWM
            if (cbuf_space(self->cbuf) < self->lwm) {
                if( mbtrn_poll(self, self->hwm, MBTRN_POLL_TIMEOUT_MSEC) <=0 ){
                    *status|=MBR_EPOLL;
                }// else status set by read
            }
            
        }else{
            // poll, then read
            if( mbtrn_poll(self, self->hwm, MBTRN_POLL_TIMEOUT_MSEC) <=0 ){
                *status|=MBR_EPOLL;
            }else{
                *status|=MBR_OK;
            }
            
            if (flags==MBR_ALLOW_PARTIAL) {
                // read what we can
                cbuf_flag_t cflags=(flags==MBR_ALLOW_PARTIAL?CB_ALLOW_PARTIAL:CB_NONE);
                if( (nbytes=cbuf_read(self->cbuf,dest,len,cflags,status)) >0){
                    *status=MBR_OK;
                    retval=nbytes;
                }else{
                    *status=MBR_EREAD;
                }
            }else{
                // not enough to read
                MERROR("poll failed not enough bytes available\n");
            }
        }
    }else{
        MERROR("invalid argument\n");
    }
    
    return retval;
}

int mbtrn_read_frame(mbtrn_config_t *mbtrn)
{
    int retval=-1;
    if (NULL!=mbtrn) {
        retval=1;
        MERROR("not implemented returning %d\n",retval);
    }else{
        MERROR("NULL configuration\n");
    }
    return retval;
}

int mbtrn_filter_frame(mbtrn_config_t *mbtrn)
{
    int retval=-1;
    if (NULL!=mbtrn) {
        retval=0;
        MERROR("not implemented returning %d\n",retval);
        
    }else{
        MERROR("NULL configuration\n");
    }
    return retval;
}

int mbtrn_send(mbtrn_config_t *mbtrn)
{
    int retval=-1;
    if (NULL!=mbtrn) {
        retval=0;
        MERROR("not implemented returning %d\n",retval);
        
    }else{
        MERROR("NULL configuration\n");
    }
    return retval;
}

uint32_t mbtrn_available(mbtrn_reader_t *self)
{
    uint32_t retval=0;
    if (NULL != self && NULL != self->cbuf) {
        retval =  cbuf_available(self->cbuf);
    }
    return retval;
}

int mbtrn_set_hwm(mbtrn_reader_t *self, uint32_t value)
{
    int retval=-1;
    if (NULL != self && value<=self->src->capacity && value>self->lwm) {
        self->hwm=value;
    }else{
        MDEBUG("invalid argument\n");
    }
    return retval;
}

int mbtrn_set_lwm(mbtrn_reader_t *self, uint32_t value)
{
    int retval=-1;
    if (NULL != self && value<=self->src->capacity && value<self->hwm) {
        self->lwm=value;
    }else{
        MDEBUG("invalid argument\n");
    }
    return retval;
}

//uint32_t mbtrn_reader_poll(mbtrn_reader_t *self, uint32_t len, uint32_t tmout_ms)
//{
//    uint32_t retval=-1;
//
//    if (NULL!=self && NULL!=self->cbuf) {
//
//        // working buffer, large enough to hold one ping
//        byte buf[MBTRN_TRN_PING_BYTES]={0};
//
//        int rbytes=0;
//        int wbytes=0;
//        int status=-1;
//        int parsed_frames=0;
//
//        // flush socket
//        // then wait for for some bytes to arrive
//        mbtrn_reader_flush(self,MBTRN_TRN_PING_BYTES, 500);
//        usleep(MBTRN_PING_INTERVAL_USEC);
//
//        // read for a little more than one ping time (timeout)
//        if ((rbytes=iow_read_tmout(mbtrn_reader_sockif(self), buf, MBTRN_TRN_PING_BYTES, tmout_ms, &status))>0) {
//
//            if ( (status==IO_OK || status==IO_ETMOUT) &&
//                (rbytes>=(int)R7K_EMPTY_FRAME_BYTES) ) {
//
//                byte records[MBTRN_TRN_PING_BYTES]={0};
//
//                MDEBUG("buf[%p] rec[%p] req[%d] rd[%d] to[%u]\n",buf,records,MBTRN_TRN_PING_BYTES,rbytes,tmout_ms);
//                if ( (parsed_frames = r7k_parse_raw(buf,records,MBTRN_TRN_REC_HINT,rbytes)) > 0) {
//
//                    // write it to the circular buffer
//                    if( (wbytes=cbuf_write(self->cbuf,records,rbytes,CB_NONE,&status))==rbytes){
//                        // success
//                        MDEBUG("wrote %d bytes/%d frames to cbuf\n",rbytes,parsed_frames);
//                        retval = parsed_frames;//rbytes
//                    }else{
//                        MERROR("cbuf error ret/stat[%d/%d]\n",wbytes,status);
//                    }
//
//                }else{
//                    MDEBUG("parse_raw err [%d]\n",status);
//                }
//            }else{
//            	// error
//                MDEBUG("read err stat[%d] rb[%d]\n",status,rbytes);
//            }
//        }else{
//            // error
//            MDEBUG("read err stat[%d] rb[%d]\n",status,rbytes);
//        }
//    }else{
//        MERROR("invalid argument\n");
//    }
//
//    return retval;
//}


//void mbtrn_reader_show(mbtrn_reader_t *self, bool verbose, uint16_t indent)
//{
//    if (NULL != self) {
//        fprintf(stderr,"%*s[self      %10p]\n",indent,(indent>0?" ":""), self);
//        fprintf(stderr,"%*s[src       %10p]\n",indent,(indent>0?" ":""), self->src);
//
//        fprintf(stderr,"%*s[cbuf      %10p]\n",indent,(indent>0?" ":""), self->cbuf);
//        if (verbose) {
//            cbuf_show(self->cbuf,verbose,indent+3);
//        }
//        fprintf(stderr,"%*s[nrecords  %10u]\n",indent,(indent>0?" ":""), self->nrecords);
//        fprintf(stderr,"%*s[hwm       %10u]\n",indent,(indent>0?" ":""), self->hwm);
//        fprintf(stderr,"%*s[lwm       %10u]\n",indent,(indent>0?" ":""), self->lwm);
//        fprintf(stderr,"%*s[status    %2d/%s]\n",indent,(indent>0?" ":""), self->status, mbtrn_statstr(self->status));
//        fprintf(stderr,"%*s[sub_count %10u]\n",indent,(indent>0?" ":""), self->sub_count);
//        fprintf(stderr,"%*s[sub_list  %10p]\n",indent,(indent>0?" ":""), self->sub_list);
//        if (verbose) {
//            for (uint32_t i=0; i<self->sub_count; i++) {
//                fprintf(stderr,"%*s[sub[%02d]  %10u]\n",indent+3,(indent+3>0?" ":""),i, self->sub_list[i]);
//            }
//        }
//    }
//}

//mbtrn_reader_t *mbtrn_reader_create(const char *host, int port, uint32_t capacity, uint32_t *slist,  uint32_t slist_len)
//{
//    mbtrn_reader_t *self = (mbtrn_reader_t *)malloc(sizeof(mbtrn_reader_t));
//    if (NULL != self) {
//
//        self->s = iow_socket_new(host,port,ST_TCP);
//        self->src = mbtrn_scon_new(self->s);
//        self->cbuf = cbuf_new(capacity);
//        self->sub_list = (uint32_t *)malloc(slist_len*sizeof(uint32_t));
//        memset(self->sub_list,0,slist_len*sizeof(uint32_t));
//        self->sub_count = slist_len;
//        memcpy(self->sub_list,slist,slist_len*sizeof(uint32_t));
//        self->nrecords=0;
//        self->status=MBR_OK;
//        self->hwm = capacity*MBTRN_READER_HWM;
//        self->lwm = capacity*MBTRN_READER_LWM;
//
//        if (NULL != self->src) {
//            MDEBUG("connecting to host [%s]\n",self->s->addr.host);
//            if(iow_connect(self->src->sock_if)==0){
//                if (r7k_subscribe(mbtrn_reader_sockif(self),self->sub_list,self->sub_count)==0) {
//                    self->status=MBR_OK;
//                }else{
//                    self->status=MBR_ESUB;
//                }
//            }else{
//                self->status=MBR_ECONNECT;
//            }
//        }else{
//            self->status=MBR_ECREATE;
//        }
//    }
//
//    return self;
//}

//void mbtrn_reader_destroy(mbtrn_reader_t **pself)
//{
//    if (NULL != pself) {
//        mbtrn_reader_t *self = *pself;
//        if (NULL != self) {
//            mbtrn_connection_destroy(&self->src);
//            cbuf_destroy(&self->cbuf);
//            // let connection auto-destroy socket
//            if (NULL != self->sub_list) {
//                free(self->sub_list);
//            }
//            free(self);
//            *pself = NULL;
//        }
//    }
//}

//typedef struct mbtrn_reader_s
//{
//    iow_socket_t *s;
//    mbtrn_connection_t *src;
//    cbuffer_t *cbuf;
//    uint32_t nrecords;
//    uint32_t hwm;
//    uint32_t lwm;
//    int status;
//    uint32_t sub_count;
//    uint32_t *sub_list;
//}mbtrn_reader_t;


//static int s_test_mbtrn(const char *host,const char *file)
//{
//    int retval=0;
//    uint32_t nsubs=11;
//    uint32_t subs[]={1003, 1006, 1008, 1010, 1012, 1013, 1015,
//        1016, 7000, 7004, 7027};
////    uint32_t nsubs=1;
////    uint32_t subs[]={7000};
//
//    // create socket connection and buffer
//    mbtrn_reader_t *reader = mbtrn_reader_create(host,R7K_7KCENTER_PORT,10*R7K_MAX_RECORD_BYTES, subs, nsubs);
//
//    mbtrn_reader_show(reader,true, 5);
//
//    byte app_data[R7K_MAX_RECORD_BYTES]={0};
//    uint32_t len=0;
//    uint32_t timeout_msec=2000;
//    int status=0;
//    uint32_t pstat=0,rstat=0;
//
//    for (int i=0; i<5; i++) {
////        if ( (len=mbtrn_available(reader)) > 0) {
////            MDEBUG("reading...\n");
////            rstat=mbtrn_read(reader, app_data, len, MBR_ALLOW_PARTIAL, &status);
////            MDEBUG("read returned [%d/%d]\n",rstat,status);
////            mbtrn_reader_show(reader,false,5);
////        }else{
//            MDEBUG("polling...\n");
//           // r7k_show_stream(reader->s,40000,350,0);
//            pstat=mbtrn_reader_poll(reader, 40000, 350);
//            MDEBUG("poll returned [%d]\n\n",pstat);
////        }
//    }
//
//    mbtrn_reader_destroy(&reader);
//
//    return retval;
//}



/// reson7k

int r7k_send_record(iow_socket_t *s, byte *rdata, uint32_t len)
{
    
    uint32_t checksum=0;
    r7k_drf_t drf;
    r7k_nf_t nf;
    
}

int r7k_resync(iow_socket_t *s, int retries, uint32_t timeout_msec)
{
    int retval=-1;
    // read and discard bytes to find valid network frame
    size_t header_len = sizeof(r7k_nf_headers_t);
    uint32_t data_len=0;
    byte hbuf[header_len];
    byte dbuf[R7K_MAX_FRAME_BYTES];
    byte *phbuf=hbuf;
    byte *pdbuf=NULL;
    r7k_drf_t *drf;
    enum state_e{S00=0,S01,S02,S03,S10,S11,SCOMPLETE,SFATAL}state;
    
    drf=(r7k_drf_t *)hbuf;
    
    for (int i=0; i<retries; i++) {
        // init buffer
        phbuf=hbuf;
        memset(phbuf,0,header_len);
        pdbuf = dbuf;
        memset(pdbuf,0,header_len);
        
        uint32_t nbytes=0;
        uint32_t sync_len=2*(sizeof(uint32_t)+sizeof(uint16_t));
        uint32_t read_len=0;
        bool found_sync=false;
        r7k_checksum_t checksum = 0;
        byte *pchk = NULL;
        
        state=S00;
        int i=0;
        while ( (state!=SCOMPLETE) && (state!=SFATAL) ) {
            switch (state) {
                case S00:
                    // read to sync pattern
                    read_len=sync_len;
                    phbuf=hbuf;
                    if( (nbytes=iow_read_tmout(s,phbuf,read_len,timeout_msec,NULL))==read_len){
                        state=S01;
                    }else{
                        if (nbytes>0) {
                            state=S02;
                        }else{
                            // error, restart;
                            MERROR("S00 error nbytes[%d]\n",nbytes);
                            nbytes=0;
                            state=S00;
                        }
                    }
                    break;
                case S01:
                    // check for sync pattern
                    drf=(r7k_drf_t *)hbuf;
                    if (drf->sync_pattern == R7K_DRF_SYNC_PATTERN) {
                        // S0x success
                        // reset nbytes for next state
                        nbytes=0;
                        state=S10;
                    }else{
                        nbytes=sync_len;
                        state=S03;
                    }
                    break;
                case S02:
                    // fill partial buffer
                    // input: nbytes >= 0
                    phbuf=hbuf+(nbytes>0?nbytes:0);
                    read_len=sync_len-(nbytes>0?nbytes:0);
                    if( (nbytes=iow_read_tmout(s,phbuf,read_len,timeout_msec,NULL))==read_len){
                        state=S01;
                    }else{
                        if (nbytes>0) {
                            // retry
                            state=S02;
                        }else{
                            // error, restart
                            state=S00;
                        }
                    }
                    break;
                case S03:
                    // shift and replace last byte
                    // input: nbytes >= 0
                    // nbytes==0 : don't shift
                    for (i=1;i<nbytes; i++) {
                        hbuf[i-1]=hbuf[i];
                    }
                    phbuf=hbuf+sync_len;
                    read_len=1;
                    if( (nbytes=iow_read_tmout(s,phbuf,read_len,timeout_msec,NULL))==read_len){
                        state=S01;
                    }else{
                        if (nbytes>0) {
                            // check for sync pattern
                            state=S01;
                        }else{
                            // error, retry
                            if (nbytes<0) {
                                MERROR("S03 error nbytes[%d]\n",nbytes);
                            }
                            // but don't shift
                            nbytes=0;
                            state=S03;
                        }
                    }
                    break;
                    
                case S10:
                    // read to checksum
                    read_len = drf->size-sizeof(r7k_drf_t);
                    if (drf->size <= R7K_MAX_FRAME_BYTES) {
                        pdbuf = dbuf;
                        phbuf=pdbuf+(nbytes>0?nbytes:0);
                        read_len=drf->size-(nbytes>0?nbytes:0);
                        if( (nbytes=iow_read_tmout(s,pdbuf,read_len,timeout_msec,NULL))==read_len){
                            state=S11;
                        }else{
                            if (nbytes>0) {
                                state=S10;
                            }else{
                                MERROR("S10 read failed\n");
                            }
                        }
                    }else{
                        // record size invalid, start over
                        state=S00;
                    }
                    break;
                case S11:
                    // validate checksum
                    checksum=0;
                    data_len = drf->size-sizeof(r7k_drf_t);
                    pchk = (pdbuf + data_len - sizeof(r7k_checksum_t));
                    checksum+=r7k_checksum(hbuf,sizeof(r7k_drf_t));
                    checksum+=r7k_checksum(pdbuf,data_len);
                    if (checksum == *((r7k_checksum_t *)pchk) ) {
                        // Success - resync complete
                        state = SCOMPLETE;
                    }else{
                        // failed retry
                        MERROR("S11 - checksum invalid [%0x/%0x]\n",checksum,*pchk);
                        state = S00;
                    }
                    
                    break;
                default:
                    break;
            }
        }// while
    }// retries
    
    return retval;
}

bool r7k_validate_record(byte *record)
{
    bool retval=true;
    
    return retval;
}

bool r7k_validate_headers(r7k_nf_headers_t *headers)
{
    bool retval=true;
    
    return retval;
}

int r7k_read_headers(iow_socket_t *s, byte* dest, int retries, uint32_t timeout_msec)
{
    int retval=-1;
    int nbytes=0;
    if (NULL != s && s->status==SS_CONNECTED) {
        
        // read nf, drf headers
        size_t header_len = sizeof(r7k_nf_headers_t);
        size_t total_len=0;
        byte work[header_len];
        byte *pdest = dest;
        size_t read_len=header_len;
        int status=0;
        memset(work,0,header_len);
        MTRACE();
        for (int i=0; i<retries; i++) {
            if ( (nbytes=iow_read_tmout(s,pdest,read_len,timeout_msec,NULL)) == read_len) {
                MTRACE();
                // validate...
                if (r7k_validate_headers((r7k_nf_headers_t *)work)) {
                    // success, copy headers and quit
                    memcpy(dest,work,header_len);
                    status=0;
                    retval=nbytes;
                    break;
                }else{
                    // invalid
                    status=-1;
                }
            }else{
                MDEBUG("nbytes %d\n",nbytes);
                if (nbytes>0) {
                    // incomplete, keep trying
                    read_len -= nbytes;
                    pdest += nbytes;
                    status = 0;
                }else{
                    // else error
                    status=-1;
                }
            }
            if (status < 0) {
                MTRACE();
                r7k_resync(s,10,1000);
            }
        }// retries...
    } // else invalid argument
    
    return retval;
}


int r7k_read_netframe(iow_socket_t *s, byte* dest, uint32_t timeout_msec)
{
    int retval=-1;
    int nbytes=0;
    if (NULL != s && s->status==SS_CONNECTED && NULL != dest) {
        
        // read nf, drf headers
        size_t header_len = sizeof(r7k_nf_headers_t);
        byte *headers=dest;
        size_t total_len=0;
        
        memset(headers,0,header_len);
        
        int nbytes[2]={0};
        byte *pread[2]={0};
        int retries[2]={5,5};
        uint32_t read_len[2]={0};
        
        pread[0]=headers;
        read_len[0]=header_len;
        
        while (retries[0]--) {
            MTRACE();
            if ( (nbytes[0] = r7k_read_headers(s,pread[0],3,timeout_msec)) == read_len[0]) {
                MTRACE();
                
                // got headers
                r7k_nf_t *nf   = (r7k_nf_t *)(headers);
                r7k_drf_t *drf = (r7k_drf_t *)(headers+sizeof(r7k_nf_t));
                
                // point to record data
                byte *prdata = ((byte *)&drf->sync_pattern)+drf->offset;
                
                // read record data
                retries[1] = 5;
                pread[1]   = prdata;
                read_len[1]=drf->size - sizeof(r7k_drf_t);
                
                while (retries[1]--) {
                    MTRACE();
                    
                    if ( (nbytes[1] = iow_read_tmout(s,pread[1],read_len[1],timeout_msec,NULL)) == read_len[1]) {
                        // success
                        retval = header_len + drf->size - sizeof(r7k_drf_t);
                    }else{
                        // read failed
                        if (nbytes[1]>0) {
                            // partial, keep trying
                            pread[1]+=nbytes[1];
                            read_len[1] -= nbytes[1];
                        }else{
                            // error
                            MERROR("data read failed nbytes[%d]\n",nbytes[1]);
                        }
                    }
                }
            }else{
                // read failed
                if (nbytes[0]>0) {
                    // partial, keep trying
                    pread[0]+=nbytes[0];
                    read_len[0] -= nbytes[0];
                }else{
                    // error
                }
            }
        }
        
    }
    
    return retval;
}
// extract valid DRFs from raw network frames
// stops when src parsed or dest full
uint32_t r7k_parse(byte *src, uint32_t len, r7k_drf_container_t *dest, uint32_t *status)
{
    int retval=-1;
    
    uint32_t record_count = 0;
    uint32_t sync_bytes = 0;
    byte *pw =NULL;
    r7k_nf_t *pnf=NULL;
    r7k_drf_t *pdrf=NULL;
    r7k_checksum_t *pchk=NULL;
    byte *psrc = src;
    //    int ALLOC_INC = 64;
    //    int ALLOC_SZ=ALLOC_INC;
    //    r7k_nf_t *nf          = (r7k_nf_t *)malloc(ALLOC_SZ*sizeof(r7k_nf_t *));
    //    r7k_drf_t *drf        = (r7k_drf_t *)malloc(ALLOC_SZ*sizeof(r7k_drf_t *));
    //    r7k_checksum_t *chksm = (r7k_checksum_t *)malloc(ALLOC_SZ*sizeof(r7k_checksum_t *));
    
    if (NULL != src && NULL!=dest) {
        
        //        memset(nf,0,ALLOC_INC*sizeof(r7k_nf_t *));
        //        memset(drf,0,ALLOC_INC*sizeof(r7k_drf_t *));
        //        memset(chksm,0,ALLOC_INC*sizeof(r7k_checksum_t *));
        //        byte *pdest = dest;
        bool resync=false;
        
        // move src pointer along, and mark
        // valid record pointers in the arrays
        // stop when we've found max_recs or
        // the end of the source buffer
        
        
        //        MDEBUG("src[%p] dest[%p] psrc[%p] s+l[%p] mr[%d] rc[%u] len[%u]\n",src,dest,psrc,(src+len),max_recs,record_count,len);
        //        // r7k_hex_show(src,len,16,true,5);
        //        MDEBUG(">> %s %s\n",(record_count<max_recs?"Y":"N"),(psrc<(src+len)?"Y":"N"));
        
        while (psrc<(src+len)) {
            
            pnf = (r7k_nf_t *)psrc;
            
            // pnf is legit?...
            if (pnf->protocol_version == R7K_NF_PROTO_VER &&
                pnf->total_packets > 0 &&
                pnf->total_size >= R7K_DRF_BYTES) {
                
                pdrf=(r7k_drf_t *)(psrc + R7K_NF_BYTES);
                
                // check DRF
                if(pdrf->protocol_version == R7K_NF_PROTO_VER &&
                   pdrf->sync_pattern == R7K_DRF_SYNC_PATTERN &&
                   pdrf->size > R7K_DRF_BYTES){
                    
                    pw = (byte *)pdrf;
                    pchk = (r7k_checksum_t *)(pw+pdrf->size-R7K_CHECKSUM_BYTES);
                    uint32_t cs =r7k_checksum((byte *)pdrf,(pdrf->size-R7K_CHECKSUM_BYTES));
                    
                    if ( cs == (*pchk) ) {
                        
                        // add it to the frame container
                        // also adds frame offset info
                        r7k_drfcon_add(dest,(byte *)pdrf,pdrf->size);
                        
                        // got one
                        //                        nf[record_count]  = pnf;
                        //                        drf[record_count] = pdrf;
                        //                        chksm[record_count] = pchk;
                        
                        // update src pointer
                        psrc= ((byte *)pchk + R7K_CHECKSUM_BYTES);
                        
                        
                        //                        // copy the DRF to the destination
                        //                        memcpy(pdest, drf[record_count],drf[record_count]->size);
                        //                        pdest+=drf[i]->size;
                        
                        // increment record count
                        record_count++;
                        
                        // add more frame pointers if needed
                        //                        if ((record_count%ALLOC_INC)==0) {
                        //                            ALLOC_SZ+=ALLOC_INC;
                        //                            nf    = (r7k_nf_t *)realloc(ALLOC_SZ*sizeof(r7k_nf_t *));
                        //                            drf   = (r7k_drf_t *)realloc(ALLOC_SZ*sizeof(r7k_drf_t *));
                        //                            chksm = (r7k_checksum_t *)realloc(ALLOC_SZ*sizeof(r7k_checksum_t *));
                        //                        }
                        
                        // set retval to parsed bytes
                        //retval=(pdest-dest);
                        retval = r7k_drfcon_length(dest);
                        
                        resync=false;
                        *status=0;
                        //MDEBUG("Woot!\n");
                    }else{
                        MDEBUG("ERR - checksum [%u/%u]\n",*pchk,cs);
                        sync_bytes++;
                        resync=true;
                    }
                }else{
                    MDEBUG("ERR - drf\n");
                    r7k_drf_show(pdrf,true,3);
                    r7k_hex_show(psrc,0x50,16,true,5);
                    sync_bytes++;
                    resync=true;
                }
            }else{
                //                MDEBUG("ERR - nf\n");
                //                 r7k_nf_show(pnf,true,3);
                //                r7k_hex_show(psrc,0x50,16,true,5);
                sync_bytes++;
                resync=true;
            }
            if (resync) {
                // move the the next possible network frame
                psrc++;
                pnf = (r7k_nf_t *)psrc;
                while ( pnf->protocol_version == R7K_NF_PROTO_VER &&
                       pnf->total_packets > 0 &&
                       pnf->total_size >= R7K_DRF_BYTES){
                    psrc++;
                    pnf = (r7k_nf_t *)psrc;
                    sync_bytes++;
                }
                resync=false;
            }
        }
    }
    
    // set some status bits
    // indicating what if any was discarded
    // and parsed frame count
    if (psrc<(src+len)) {
        *status |=0x1;
    }
    //    if (dest<(dest+dlen)) {
    //        *status |=0x2;
    //    }
    *status |= (record_count<<2);
    
    // free frame pointers
    // might be cool to return these
    //    free(nf);
    //    free(drf);
    //    free(chksm);
    MDEBUG("valid[%d] sync[%d] rv[%d]\n",record_count,sync_bytes,retval);
    return retval;
}
//int r7k_parse_raw(byte *src, byte *dest, uint32_t max_recs, uint32_t len)
//{
//    int retval=-1;
//
//    uint32_t record_count = 0;
//    uint32_t invalid_count = 0;
//    byte *pw =NULL;
//    r7k_nf_t *pnf=NULL;
//    r7k_drf_t *pdrf=NULL;
//    r7k_checksum_t *pchk=NULL;
//    r7k_nf_t *nf[max_recs];
//    r7k_drf_t *drf[max_recs];
//    r7k_checksum_t *chksm[max_recs];
//
//    if (NULL != src && NULL!=dest) {
//
//        memset(nf,0,max_recs*sizeof(r7k_nf_t *));
//        memset(drf,0,max_recs*sizeof(r7k_drf_t *));
//        memset(chksm,0,max_recs*sizeof(r7k_checksum_t *));
//        byte *psrc = src;
//        byte *pdest = dest;
//        bool resync=false;
//
//        // move src pointer along, and mark
//        // valid record pointers in the arrays
//        // stop when we've found max_recs or
//        // the end of the source buffer
//
//
//        //        MDEBUG("src[%p] dest[%p] psrc[%p] s+l[%p] mr[%d] rc[%u] len[%u]\n",src,dest,psrc,(src+len),max_recs,record_count,len);
//        //        // r7k_hex_show(src,len,16,true,5);
//        //        MDEBUG(">> %s %s\n",(record_count<max_recs?"Y":"N"),(psrc<(src+len)?"Y":"N"));
//
//        while (record_count<max_recs && psrc<(src+len) ) {
//
//            pnf = (r7k_nf_t *)psrc;
//
//            // pnf is legit?...
//            if (pnf->protocol_version == R7K_NF_PROTO_VER &&
//                pnf->total_packets > 0 &&
//                pnf->total_size >= R7K_DRF_BYTES) {
//
//                pdrf=(r7k_drf_t *)(psrc + R7K_NF_BYTES);
//
//                // check DRF
//                if(pdrf->protocol_version == R7K_NF_PROTO_VER &&
//                   pdrf->sync_pattern == R7K_DRF_SYNC_PATTERN &&
//                   pdrf->size > R7K_DRF_BYTES){
//                    pw = (byte *)pdrf;
//                    pchk = (r7k_checksum_t *)(pw+pdrf->size-R7K_CHECKSUM_BYTES);
//                    uint32_t cs =r7k_checksum((byte *)pdrf,(pdrf->size-R7K_CHECKSUM_BYTES));
//                    if ( cs == (*pchk) ) {
//                        // got one
//                        nf[record_count]  = pnf;
//                        drf[record_count] = pdrf;
//                        chksm[record_count] = pchk;
//                        record_count++;
//                        psrc= ((byte *)pchk + R7K_CHECKSUM_BYTES);
//                        //MDEBUG("Woot!\n");
//                        retval=0;
//                        resync=false;
//                    }else{
//                        MDEBUG("ERR - checksum [%u/%u]\n",*pchk,cs);
//                        invalid_count++;
//                        resync=true;
//                    }
//                }else{
//                    MDEBUG("ERR - drf\n");
//                    r7k_drf_show(pdrf,true,3);
//                    r7k_hex_show(psrc,0x50,16,true,5);
//                    invalid_count++;
//                    resync=true;
//                }
//            }else{
//                //                MDEBUG("ERR - nf\n");
//                //                 r7k_nf_show(pnf,true,3);
//                //                r7k_hex_show(psrc,0x50,16,true,5);
//                invalid_count++;
//                resync=true;
//            }
//            if (resync) {
//                // move the the next possible network frame
//                psrc++;
//                pnf = (r7k_nf_t *)psrc;
//                while ( pnf->protocol_version == R7K_NF_PROTO_VER &&
//                       pnf->total_packets > 0 &&
//                       pnf->total_size >= R7K_DRF_BYTES){
//                    psrc++;
//                    pnf = (r7k_nf_t *)psrc;
//                    invalid_count++;
//                }
//                resync=false;
//            }
//        }
//    }
//    // copy Data Record Frames to destination
//    if (record_count>0) {
//        pw = dest;
//        for (int i=0; i<record_count; i++) {
//            memcpy(pw, drf[i],drf[i]->size);
//            pw+=drf[i]->size;
//            retval=i+1;
//            if (pw > (dest+len)) {
//                // stop if buffer full
//                MDEBUG("Buffer full - return [%d/%d] frames\n",i,record_count);
//                break;
//            }
//        }
//    }
//    MDEBUG("valid[%d] err[%d] rv[%d]\n",record_count,invalid_count,retval);
//    return retval;
//}


// read a complete R7K message (Data Record Frame) into dest
// records may span multiple packets (Network Frames)
// [discards network frame (NF) headers]
// uses optional timeout, use zero to try indefinitely
int r7k_read_record(iow_socket_t *s, byte* dest, uint32_t timeout_msec)
{
    int retval=-1;
    if (NULL != s && s->status==SS_CONNECTED) {
        // TODO : allocate single frame buffer on stack
        // if most messages are single network frames, it would
        // prevent a lot of memory alloc/dealloc
        byte *frames[R7K_MAX_RECORD_FRAMES];
        int frame_count=0;
        
        r7k_nf_t *nf[R7K_MAX_RECORD_FRAMES]   = {0};
        r7k_drf_t *drf[R7K_MAX_RECORD_FRAMES] = {0};
        
        bool got_nf=false;
        int retries[2]={0};
        int nbytes=0;
        retries[0]=3;
        
        frames[0]=(byte *)malloc(R7K_MAX_FRAME_BYTES*sizeof(byte));
        frame_count++;
        MTRACE();
        while (retries[0]-- && !got_nf) {
            
            memset(frames[0],0,R7K_MAX_FRAME_BYTES);
            
            MTRACE();
            if ((nbytes = r7k_read_netframe(s,frames[0],timeout_msec)) > 0) {
                
                MTRACE();
                if (nf[0]->seq_number==0) {
                    got_nf=true;
                    break;
                }else{
                    // in middle of packet (assume packets arrive in order)
                    // discard netframes until get first in sequence
                    retries[1]=10;
                    while (retries[1]--) {
                        memset(frames[0],0,R7K_MAX_FRAME_BYTES);
                        MTRACE();
                        if ((nbytes = r7k_read_netframe(s,frames[0],timeout_msec))>0) {
                            nf[0] = (r7k_nf_t *)(frames[0]);
                            if (nf[0]->seq_number == 0) {
                                got_nf=true;
                                break;
                            }else{
                                MERROR("discarding netframe w/ seq[%u]\n",nf[0]->seq_number);
                            }
                        }else{
                            MERROR("read_netframe failed [%d]\n",nbytes);
                        }
                    }// while discarding
                }
            }else{
                MERROR("read_netframe failed [%d]\n",nbytes);
            }
        }// while
        
        if (got_nf) {
            // found NF sequence number 0
            if (nf[0]->total_packets==1) {
                // single packet - done
                drf[0] = (r7k_drf_t *)(&frames[0]+sizeof(r7k_nf_t));
                memcpy(dest,drf[0],drf[0]->size);
                retval = drf[0]->size;
            }else{
                // multiple packets
                int i=0;
                if (nf[0]->total_packets > R7K_MAX_RECORD_FRAMES) {
                    MERROR("total packets > max frames [%d > %d]\n",nf[0]->total_packets,R7K_MAX_RECORD_FRAMES);
                }else{
                    for (i=1; i<=nf[0]->total_packets; i++) {
                        
                        if (frames[i]==NULL) {
                            frames[i] = (byte *)malloc(R7K_MAX_FRAME_BYTES*sizeof(byte));
                            frame_count++;
                            nf[i] = (r7k_nf_t *)frames[i];
                            drf[i] = (r7k_drf_t *)(&frames[i]+sizeof(r7k_nf_t));
                            
                            if ((nbytes = r7k_read_netframe(s,frames[i],timeout_msec))<=0) {
                                // failed
                                MERROR("read_net_frame failed on packet [%d/%d]\n",i,nf[i]->total_packets);
                                break;
                            }
                            if (nf[i]->seq_number > nf[i]->total_packets) {
                                // not part of this sequence
                                MERROR("packet sequence out of range [%d/%d]\n",nf[i]->seq_number,nf[i]->total_packets);
                                break;
                            }
                            if ( nf[i]->seq_number != i) {
                                
                                MINFO("packet out of sequence [%d/%d]\n",i,nf[i]->seq_number);
                                
                                if(nf[i]->seq_number > i){
                                    // it arrived earlier than expected, put it in
                                    // the right slot
                                    int j=nf[i]->seq_number;
                                    // move pointers
                                    frames[j]=frames[i];
                                    frames[i]=NULL;
                                    nf[j]=nf[i];
                                    nf[i]=NULL;
                                    drf[j]=drf[i];
                                    // set the counter back to try ith again
                                    i--;
                                }else{
                                    // it claims to be an earlier packet, can't recover
                                    MERROR("would overwrite earlier frame\n");
                                }
                                
                                break;
                            }
                        }
                    }
                    
                    MDEBUG("frame_count[%d] total_packets[%d]\n",frame_count,nf[0]->total_packets);
                    if (frame_count==nf[0]->total_packets) {
                        // success
                        // copy the DRFs into the destination buffer
                        byte *pdest = dest;
                        for (int i=0; i<frame_count; i++) {
                            memcpy(pdest,&drf[i],drf[i]->size);
                            pdest+=drf[i]->size;
                        }
                        retval=nf[0]->total_size;
                    }
                }
            }
        }else{
            MERROR("could not find NF seq_no 0\n");
        }
        
        // release memory
        for (int i=0; i<R7K_MAX_RECORD_FRAMES; i++) {
            if (NULL!=frames[i]) {
                free(frames[i]);
            }
        }
        
    }else{
        MINFO("invalid socket/status\n");
    }
    
    return retval;
}
