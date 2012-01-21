/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*
/  See README file for copying and redistribution conditions.
*/
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "types_win32.h"
#include "xdr_win32.h"


#define BYTES_PER_XDR_UNIT 4







static long ntohl(long val)

{

 u_long ret,uval;

 uval=(u_long)val;
 ret = (((uval/0x1000000) & 0x0ff)            )

     + (((uval/0x10000  ) & 0x0ff) * 0x100    )

     + (((uval/0x100    ) & 0x0ff) * 0x10000  )

     + (((uval          ) & 0x0ff) * 0x1000000);

 return((long)ret);


}





static long htonl(long val)

{

 u_long ret,uval;

 uval=(u_long)val;
 ret = (((uval/0x1000000) & 0x0ff)            )

     + (((uval/0x10000  ) & 0x0ff) * 0x100    )

     + (((uval/0x100    ) & 0x0ff) * 0x10000  )

     + (((uval          ) & 0x0ff) * 0x1000000);

 return((long)ret);

    

}





static int xdrGetLong(XDR* xdrs,long* lp)

{

 if (fread((char*)lp, sizeof(long),(size_t)1, (FILE *)xdrs->x_private) != 1)

    return (FALSE);

 *lp = ntohl(*lp);

 return (TRUE);

}





static int xdrPutLong(XDR* xdrs,long *lp)

{

 long mylong;



 mylong = htonl(*lp);

 lp = &mylong; 

 if (fwrite((char*)lp, sizeof(long), 1, (FILE *)xdrs->x_private) != 1)

    return (FALSE);

 return (TRUE);

}





static int xdrGetBytes(XDR* xdrs,char* addr,u_int len)

{

 if (len != 0)

 {

  if(fread(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1)

    return(FALSE);

 }   

 return(TRUE);

}





static int xdrPutBytes(XDR* xdrs,char* addr,u_int len)

{

 if(len != 0)

 {

  if(fwrite(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1)

    return(FALSE);

 }   

 return(TRUE);

}







void xdrstdio_create(XDR* xdrs,FILE* file,XdrOp op)

{

 xdrs->x_op = op;

 xdrs->x_private = (char*)file;

 xdrs->x_handy = 0;

 xdrs->x_base = 0;

}







int xdr_long(XDR* xdrs,long* lp)

{

 switch(xdrs->x_op)

 {

  case XDR_ENCODE:

      return (xdrPutLong(xdrs, lp));

  case XDR_DECODE:

      return (xdrGetLong(xdrs, lp));

  default:

      break;

 }

 return (FALSE);

}







int xdr_u_long(XDR *xdrs,u_long* ulp)

{

 return(xdr_long(xdrs,(long*) ulp));

}







int xdr_short(XDR *xdrs,short *sp)

{

 long val;

 short sval;



 switch(xdrs->x_op)

 {

  case XDR_ENCODE:

      val = (long) *sp;

      return(xdrPutLong(xdrs, &val));

  case XDR_DECODE:

      if(xdrGetLong(xdrs, &val) == TRUE) 

      {

       sval = (short)(val & 0x7fff);

       if(val<0) sval = sval | 0x8000;

       *sp = sval;

       return(TRUE);

      }

  default:

      break;

 }

 return (FALSE);

}







int xdr_u_short(XDR *xdrs,u_short* ulp)

{

 long val = (long)((short)(*ulp));

 switch(xdrs->x_op)

 {

  case XDR_ENCODE:

           val=val & 0xffff;

           return(xdr_long(xdrs,&val));

  case XDR_DECODE:

           if(xdr_long(xdrs,&val) == TRUE)

           {

            *ulp = (u_short)val;

            return(TRUE);

           }

  default:

           break;

 }

 return(FALSE);                   

}







int xdr_u_int(XDR* xdrs,u_int* ip)

{

 if (sizeof (u_int) == sizeof (u_long)) 

   return (xdr_u_long(xdrs,(u_long*)ip)); 

 else 

   return (xdr_u_short(xdrs,(u_short*)ip));

}







int xdr_int(XDR* xdrs,int *ip)

{

 if (sizeof (int) == sizeof (long)) 

   return (xdr_long(xdrs,(long *)ip)); 

 else 

   return (xdr_short(xdrs,(short *)ip));

}







int xdr_char(XDR *xdrs,char *cp)

{

 int ii;



 ii = (int)(*cp);

 if (xdr_int(xdrs, &ii)) 

 {

  *cp = (char)ii;

  return(TRUE);

 }

 return(FALSE);

}







int xdr_u_char(XDR *xdrs,u_char *cp)

{

 u_int ii;



 ii = (u_int)(*cp);

 if(xdr_u_int(xdrs, &ii))

 {

  *cp = (u_char)ii;

  return(TRUE);

 }

 return(FALSE); 

}







int xdr_opaque(XDR *xdrs,char* cp,unsigned int cnt)

{

 static char crud[BYTES_PER_XDR_UNIT];

 u_int rndup;



 if(cnt == 0) return (TRUE);

 rndup = cnt % BYTES_PER_XDR_UNIT;

 if (rndup > 0) rndup = BYTES_PER_XDR_UNIT - rndup;



 switch(xdrs->x_op)

 {

  case XDR_DECODE: 

     if (!xdrGetBytes(xdrs, cp, cnt)) return(FALSE);

     if (rndup == 0) return(TRUE);

     return (xdrGetBytes(xdrs, crud, rndup));

  case XDR_ENCODE:

     if (!xdrPutBytes(xdrs, cp, cnt)) return (FALSE);

     if (rndup == 0) return(TRUE);

/*     return (xdrPutBytes(xdrs, xdr_zero, rndup)); */

  case XDR_FREE:

     return(TRUE);

  default:

     break;

 } 

 return (FALSE);

}









int xdr_bytes(XDR *xdrs,char **cpp,u_int *sizep,u_int maxsize)

/*   *cpp is a pointer to the bytes, *sizep is the count.

  If *cpp is NULL maxsize bytes are allocated              */

{

 char *sp = *cpp;

 u_int nodesize;



 if (!xdr_u_int(xdrs, sizep)) 

   return (FALSE);



 nodesize = *sizep;

 if((nodesize > maxsize) && (xdrs->x_op != XDR_FREE)) 

   return (FALSE);



 switch (xdrs->x_op) 

 {

  case XDR_DECODE:

       if (nodesize == 0) return (TRUE);

       if (sp == NULL) 

       {

        *cpp = sp = (char *)calloc(nodesize,sizeof(char));

        if(sp == NULL) return (FALSE);

       }

  case XDR_ENCODE:

	return (xdr_opaque(xdrs, sp, nodesize));

  case XDR_FREE:

	if (sp != NULL) 

        {

	 free(sp);

         *cpp = NULL;

        }

	return (TRUE);

  default:

        break;

 }

 return (FALSE);

}









int xdr_float(XDR* xdrs,float *fp)

/*

//  What IEEE single precision floating point 

// struct	ieee_single {

// 	 unsigned int	mantissa: 23;

//	 unsigned int	exp     : 8;

//	 unsigned int	sign    : 1;

//  };

*/

{

  switch (xdrs->x_op) 

  {

   case XDR_ENCODE:

        return (xdrPutLong(xdrs,(long *)fp));

   case XDR_DECODE:

        return (xdrGetLong(xdrs,(long *)fp));

   case XDR_FREE:

        return (TRUE);

   default:

        break;

  }

  return (FALSE);

}







int xdr_double(XDR* xdrs,double *dp)
{

 long *lp;

 switch(xdrs->x_op)
 {

  case XDR_ENCODE:
       lp = (long *)dp;
	   lp++;
       xdrPutLong(xdrs, lp);
       lp--;
       return(xdrPutLong(xdrs, lp));

  case XDR_DECODE:
       lp = (long *)dp;
	   lp++;
       xdrGetLong(xdrs, lp);
       lp--;
       return(xdrGetLong(xdrs, lp));
  case XDR_FREE:

       return (TRUE);

  default:

       break;

 }

 return (FALSE);

}







