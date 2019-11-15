/*******************************************************************************
NAME                           GCTP 

VERSION	PROGRAMMER      DATE
-------	----------      ----
	T. Mittan	2-26-93		Conversion from FORTRAN to C
	S. Nelson	12-14-93	Added assignments to inunit and 
					outunit for State Plane purposes.
c.1.0	S. Nelson	9-15-94		Added outdatum parameter call.
c.1.1	S. Nelson	11-94		Modified code so that UTM can accept
					any spheroid code.  Changed State
					Plane legislated distance units,
					for NAD83, to be consistant with
					FORTRAN version of GCTP.  Unit codes
					are specified by state laws as of
					2/1/92.
c.1.5	S. Nelson	 1-98		Changed the name datum to spheroid.

					In initialize test, before the init
					functions were called for State Plane
					every time.  Now, initialization will
					only be done if zone, spheroid, or
					the parameter array changes which is
					consistant with the other projections.

					For the UTM inverse projections the 1st
					2 elements of a temporary projection
					array were being assigned to a lat and
					long that lies in that zone area.  This
					has been eliminated.  UTM will be
					initialized the same as the other
					projections for inverse transformations.
					For forward transformations the
					temporary array still exists.
  
ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

#define TRUE 1
#define FALSE 0

static long iter = 0;			/* First time flag		*/
static long inpj[MAXPROJ + 1];		/* input projection array	*/
static long indat[MAXPROJ + 1];		/* input dataum array		*/
static long inzn[MAXPROJ + 1];		/* input zone array		*/
static double pdin[MAXPROJ + 1][COEFCT];/* input projection parm array	*/
static long outpj[MAXPROJ + 1];		/* output projection array	*/
static long outdat[MAXPROJ + 1];	/* output dataum array		*/
static long outzn[MAXPROJ + 1];		/* output zone array		*/
static double pdout[MAXPROJ+1][COEFCT];	/* output projection parm array	*/
static long (*for_trans[MAXPROJ + 1])();/* forward function pointer array*/
static long (*inv_trans[MAXPROJ + 1])();/* inverse function pointer array*/

			/* Table of unit codes as specified by state
			   laws as of 2/1/92 for NAD 1983 State Plane
			   projection, 1 = U.S. Survey Feet, 2 = Meters,
			   5 = International Feet	*/

static long NADUT[134] = {1, 5, 1, 1, 5, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 2, 2,
			  1, 1, 5, 2, 1, 2, 5, 1, 2, 2, 2, 1, 1, 1, 5, 2, 1, 5,
			  2, 2, 5, 2, 1, 1, 5, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 2};

static long NAD83[134] = {101,102,5010,5300,201,202,203,301,302,401,402,403,
                404,405,406,0000,501,502,503,600,700,901,902,903,1001,1002,
                5101,5102,5103,5104,5105,1101,1102,1103,1201,1202,1301,1302,
                1401,1402,1501,1502,1601,1602,1701,1702,1703,1801,1802,1900,
                2001,2002,2101,2102,2103,2111,2112,2113,2201,2202,2203,2301,
                2302,2401,2402,2403,2500,0000,0000,2600,0000,2701,2702,2703,
                2800,2900,3001,3002,3003,3101,3102,3103,3104,3200,3301,3302,
                3401,3402,3501,3502,3601,3602,3701,3702,3800,3900,0000,4001,
                4002,4100,4201,4202,4203,4204,4205,4301,4302,4303,4400,4501,
                4502,4601,4602,4701,4702,4801,4802,4803,4901,4902,4903,4904,
                5001,5002,5003,5004,5005,5006,5007,5008,5009,5200,0000,5400};

void gctp(incoor,insys,inzone,inparm,inunit,inspheroid,ipr,efile,jpr,pfile,
	outcoor, outsys,outzone,outparm,outunit,outspheroid,fn27,fn83,iflg)

double *incoor;		/* input coordinates				*/
long *insys;		/* input projection code			*/
long *inzone;		/* input zone number				*/
double *inparm;		/* input projection parameter array		*/
long *inunit;		/* input units					*/
long *inspheroid;	/* input spheroid 				*/
long *ipr;		/* printout flag for error messages. 0=screen, 1=file,
			   2=both*/
char *efile;		/* error file name				*/
long *jpr;		/* printout flag for projection parameters 0=screen, 
			   1=file, 2 = both*/
char *pfile;		/* error file name				*/
double *outcoor;	/* output coordinates				*/
long *outsys;		/* output projection code			*/
long *outzone;		/* output zone					*/
double *outparm;	/* output projection array			*/
long *outunit;		/* output units					*/
long *outspheroid;	/* output spheroid				*/
char fn27[];		/* file name of NAD 1927 parameter file		*/
char fn83[]; 	 	/* file name of NAD 1983 parameter file		*/
long *iflg;		/* error flag					*/
{
double x;		/* x coordinate 				*/
double y;		/* y coordinate					*/
double factor;		/* conversion factor				*/
double lon;		/* longitude					*/
double lat;		/* latitude					*/
double temp;		/* temporary variable				*/
long i,j;		/* loop counters				*/
long ininit_flag;	/* input initilization flag			*/
long outinit_flag;	/* output initilization flag			*/
long ind;		/* temporary var used to find state plane zone 	*/
long unit;		/* temporary unit variable			*/
double temparr[COEFCT];	/* temporary projection array 			*/

/* setup initilization flags and output message flags
---------------------------------------------------*/
ininit_flag = FALSE;
outinit_flag = FALSE;
*iflg = 0;

*iflg = init(*ipr,*jpr,efile,pfile);
if (*iflg != 0)
   return;


/* check to see if initilization is required
only the first 13 projection parameters are currently used.
If more are added the loop should be increased.
---------------------------------------------------------*/
if (iter == 0)
   {
   for (i = 0; i < MAXPROJ + 1; i++)
      {
      inpj[i] = 0;
      indat[i] = 0;
      inzn[i] = 0;
      outpj[i] = 0;
      outdat[i] = 0;
      outzn[i] = 0;
      for (j = 0; j < COEFCT; j++)
         {
         pdin[i][j] = 0.0;
         pdout[i][j] = 0.0;
         }
      }
   ininit_flag = TRUE;
   outinit_flag = TRUE;
   iter = 1;
   }
else
   {
   if (*insys != GEO)
     {
     if ((inzn[*insys] != *inzone) || (indat[*insys] != *inspheroid) || 
         (inpj[*insys] != *insys))
        {
        ininit_flag = TRUE;
        }
     else
     for (i = 0; i < 13; i++)
        if (pdin[*insys][i] != inparm[i])
          {
          ininit_flag = TRUE;
          break;
          }
     }
   if (*outsys != GEO)
     {
     if ((outzn[*outsys] != *outzone) || (outdat[*outsys] != *outspheroid) || 
         (outpj[*outsys] != *outsys))
        {
        outinit_flag = TRUE;
        }
     else
     for (i = 0; i < 13; i++)
        if (pdout[*outsys][i] != outparm[i])
          {
          outinit_flag = TRUE;
          break;
          }
     }
   }

/* Check input and output projection numbers
------------------------------------------*/
if ((*insys < GEO) || (*insys > MAXPROJ))
   {
   p_error("Insys is illegal","GCTP-INPUT");
   *iflg = 1;
   return;
   }
if ((*outsys < GEO) || (*outsys > MAXPROJ))
   {
   p_error("Outsys is illegal","GCTP-OUTPUT");
   *iflg = 2;
   return;
   }

/* find the correct conversion factor for units
---------------------------------------------*/
unit = *inunit;

/* use legislated unit table for State Plane
-------------------------------------------*/
if ((*inspheroid == 0) && (*insys == SPCS) && (*inunit == STPLN_TABLE)) 
		unit = FEET;
if ((*inspheroid == 8) && (*insys == SPCS) && (*inunit == STPLN_TABLE))
		unit = NADUT[(*inzone)/100];

/* find the factor unit conversions--all transformations are in radians
   or meters
  --------------------------------*/
if (*insys == GEO)
   *iflg = untfz(unit,RADIAN,&factor); 
else
   *iflg = untfz(unit,METER,&factor); 
if (*insys == SPCS)
   *inunit = unit;
if (*iflg != 0)
   {
   close_file();
   return;
   }
 
x = incoor[0] * factor;
y = incoor[1] * factor;

/* Initialize inverse transformation
----------------------------------*/
if (ininit_flag)
   {
   inpj[*insys] = *insys;
   indat[*insys] = *inspheroid;
   inzn[*insys] = *inzone;
   for (i = 0;i < COEFCT; i++)
      pdin[*insys][i] = inparm[i];

   /* Call the initialization function
   ----------------------------------*/
   inv_init(*insys,*inzone,inparm,*inspheroid,fn27,fn83,iflg,inv_trans);
   if (*iflg != 0)
      {
      close_file();
      return;
      }
   }

/* Do actual transformations
--------------------------*/

/* Inverse transformations
------------------------*/
if (*insys == GEO)
   {
   lon = x;
   lat = y;
   }
else
if ((*iflg = inv_trans[*insys](x, y, &lon, &lat)) != 0)
   {
   close_file();
   return;
   }

/* DATUM conversion should go here
--------------------------------*/

/* 
   The datum conversion facilities should go here 
*/

/* Initialize forward transformation
----------------------------------*/
if (outinit_flag)
   {
   outpj[*outsys] = *outsys;
   outdat[*outsys] = *outspheroid;
   outzn[*outsys] = *outzone;
   for (i = 0;i < COEFCT; i++)
      pdout[*outsys][i] = outparm[i];

   /* If the projection is UTM, copy to a temporary array.  This way, the
      user does not have to enter the zone nor a lat/long within the zone.
      The program will calculate the zone from the input lat/long.
    ---------------------------------------------------------------------*/ 
   if (*outsys == UTM)
      {
      for (i = 2; i < COEFCT; i++)
          temparr[i] = outparm[i];
      temparr[0] = 0;
      temparr[1] = 0;
      if (outparm[0] == 0.0)
         {
         temparr[0] = pakr2dm(lon);
         temparr[1] = pakr2dm(lat);
         }
      else
         {
	 temparr[0] = outparm[0];
	 temparr[1] = outparm[1];
	 }
      for_init(*outsys,*outzone,temparr,*outspheroid,fn27,fn83,iflg,for_trans);
      }
   else
      for_init(*outsys,*outzone,outparm,*outspheroid,fn27,fn83,iflg,for_trans);
   if (*iflg != 0)
      {
      close_file();
      return;
      }
   }

/* Forward transformations
------------------------*/
if (*outsys == GEO)
   {
   outcoor[0] = lon;
   outcoor[1] = lat;
   }
else
if ((*iflg = for_trans[*outsys](lon, lat, &outcoor[0], &outcoor[1])) != 0)
   {
   close_file();
   return;
   }

/* find the correct conversion factor for units
---------------------------------------------*/
unit = *outunit;
/* use legislated unit table
----------------------------*/
     if ((*outspheroid == 0) && (*outsys == SPCS) && (*outunit == STPLN_TABLE)) 
		unit = 1;
     if ((*outspheroid == 8) && (*outsys == SPCS) && (*outunit == STPLN_TABLE))
		unit = NADUT[(*outzone)/100];

if (*outsys == GEO)
   *iflg = untfz(RADIAN,unit,&factor); 
else
   *iflg = untfz(METER,unit,&factor); 

if (*outsys == SPCS)
   *outunit = unit;

outcoor[0] *= factor;
outcoor[1] *= factor;
close_file();
return;
}
