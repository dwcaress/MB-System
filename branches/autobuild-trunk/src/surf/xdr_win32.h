/*
/  See README file for copying and redistribution conditions.
*/

#ifndef XDRH_included
#define XDRH_included

#include "types_win32.h"

#ifndef TRUE
#define TRUE  (1==1)
#endif
#ifndef FALSE
#define FALSE (0==1)
#endif



typedef enum
{
	XDR_ENCODE=0,
	XDR_DECODE=1,
	XDR_FREE=2
}XdrOp;


typedef struct 
{
  XdrOp     x_op;
  char*   x_public;
  char*   x_private;
  char*   x_base;
  int       x_handy;
} XDR;


void xdrstdio_create(XDR* xdrs,FILE* file,XdrOp op);
int xdr_long(XDR* xdrs,long* lp);
int xdr_u_long(XDR* xdrs,u_long* ulp);
int xdr_short(XDR* xdrs,short* sp);
int xdr_u_short(XDR* xdrs,u_short* ulp);
int xdr_int(XDR* xdrs,int* ip);
int xdr_u_int(XDR* xdrs,u_int* ip);
int xdr_char(XDR* xdrs,char* cp);
int xdr_u_char(XDR* xdrs,u_char* cp);
int xdr_opaque(XDR* xdrs,char* cp,unsigned int cnt);
int xdr_bytes(XDR* xdrs,char** cpp,u_int* sizep,u_int maxsize);
int xdr_double(XDR* xdrs,double *dp);
int xdr_float(XDR* xdrs,float *fp);

#endif
