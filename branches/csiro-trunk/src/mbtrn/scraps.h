/// mbtrn.h

typedef struct mbtrn_config_s
{
    mbtrn_connection_t *dest;
    mbtrn_connection_t *src;
    uint32_t in_size;
    byte *in_buf;
    byte *pin;
    bool auto_free;
}mbtrn_config_t;

uint32_t mbtrn_read(mbtrn_reader_t *self, byte *dest, uint32_t len, mbtrn_flags_t flags, int *status);
uint32_t mbtrn_available(mbtrn_reader_t *self);
int mbtrn_set_hwm(mbtrn_reader_t *self, uint32_t value);
int mbtrn_set_lwm(mbtrn_reader_t *self, uint32_t value);
int mbtrn_read_frame(mbtrn_config_t *mbtrn);
int mbtrn_filter_frame(mbtrn_config_t *mbtrn);
int mbtrn_send(mbtrn_config_t *mbtrn);

// mbtrn_config API

// create
mbtrn_config_t *mbtrn_new(mbtrn_connection_t *src, mbtrn_connection_t *dest, uint32_t isize);
void mbtrn_destroy(mbtrn_config_t **pself);

// configure
int mbtrn_set_src(mbtrn_config_t *mbtrn, mbtrn_connection_t *c);
int mbtrn_set_dest(mbtrn_config_t *mbtrn, mbtrn_connection_t *c);

// operations
int mbtrn_init(char *mbdata_file);
int mbtrn_config_connect(mbtrn_config_t *mbtrn);

// info
uint32_t mbtrn_isize(mbtrn_config_t *mbtrn);
uint32_t mbtrn_ilen(mbtrn_config_t *mbtrn);
void mbtrn_show_frame(mbtrn_config_t *mbtrn);
//uint32_t mbtrn_avail(mbtrn_config_t *mbtrn);

// utility/convenience
// run in thread?
int mbtrn_start(mbtrn_config_t *mbtrn);
int mbtrn_stop(mbtrn_config_t *mbtrn);

// separate src/dest connection
int mbtrn_connect_input(mbtrn_config_t *mbtrn);
int mbtrn_connect_output(mbtrn_config_t *mbtrn);




/// reson7k.h
typedef struct r7k_msg_s
{
    uint32_t msg_len;
    r7k_nf_t *nf;
    r7k_drf_t *drf;
    uint32_t data_size;
    byte *data;
    //    uint32_t rth_size;
    //    uint32_t rd_size;
    //    uint32_t od_size;
    //    byte *rth;
    //    byte *rd;
    //    byte *od;
    r7k_checksum_t checksum;
}r7k_msg_t;

int r7k_read_record(iow_socket_t *s, byte* dest, uint32_t timeout_msec);
int r7k_read_headers(iow_socket_t *s, byte* dest, int retries, uint32_t timeout_msec);
int r7k_resync(iow_socket_t *s, int retries, uint32_t timeout_msec);
bool r7k_validate_headers(r7k_nf_headers_t *headers);
bool r7k_validate_record(byte *record);

r7k_msg_t *r7k_msg_new(uint32_t data_len);
void r7k_msg_destroy(r7k_msg_t **pself);
void r7k_msg_show(r7k_msg_t *self, bool verbose, uint16_t indent);
uint32_t r7k_msg_size(r7k_msg_t *self);
uint32_t r7k_msg_update_checksum(r7k_msg_t *self);
byte *r7k_msg_serialize(r7k_msg_t *self);
int r7k_msg_send(iow_socket_t *s, r7k_msg_t *self);
int r7k_msg_receive(iow_socket_t *s, r7k_msg_t **dest, uint32_t timeout_msec);

//void r7k_msg_set_rth(r7k_msg_t *self, byte *data, uint32_t len);
//void r7k_msg_set_rd(r7k_msg_t *self, byte *data, uint32_t len);
//void r7k_msg_set_od(r7k_msg_t *self, byte *data, uint32_t len);
