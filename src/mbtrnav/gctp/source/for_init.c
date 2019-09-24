/*******************************************************************************
NAME                           FOR_INIT 

PURPOSE:	Initializes forward projection transformation parameters

PROGRAMMER              DATE		REASON
----------              ----		------
T. Mittan		3-09-93		Initial Development
S. Nelson		11-94		Added Clarke spheroid default to UTM
S. Nelson		 1-98		Changed datum to spheroid

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

void for_init(outsys,outzone,outparm,outspheroid,fn27,fn83,iflg,for_trans)

long outsys;		/* output system code				*/
long outzone;		/* output zone number				*/
double *outparm;	/* output array of projection parameters	*/
long outspheroid;	/* output spheroid				*/
char *fn27;		/* NAD 1927 parameter file			*/
char *fn83;		/* NAD 1983 parameter file			*/
long *iflg;		/* status flag					*/
long (*for_trans[])();	/* forward function pointer			*/
{
long zone;		/* zone number					*/
double azimuth;		/* azimuth					*/
double alf;		/* SOM angle					*/
double angle;		/* rotation anlge				*/
double lon1;		/* longitude point in utm scene			*/
double lon2;		/* 2nd longitude 				*/
double lat1;		/* 1st standard parallel			*/
double lat2;		/* 2nd standard parallel			*/
double center_long;	/* center longitude				*/
double center_lat;	/* center latitude				*/
double h;		/* height above sphere				*/
double lon_origin;	/* longitude at origin				*/
double lat_origin;	/* latitude at origin				*/
double r_major;		/* major axis in meters				*/
double r_minor;		/* minor axis in meters				*/
double scale_factor;	/* scale factor					*/
double false_easting;	/* false easting in meters			*/
double false_northing;	/* false northing in meters			*/
double shape_m;		/* constant used for Oblated Equal Area		*/
double shape_n;		/* constant used for Oblated Equal Area		*/
long   start;		/* where SOM starts beginning or end		*/
double time;		/* SOM time					*/
double radius;		/* radius of sphere				*/
long tmpspheroid;	/* temporary spheroid for UTM			*/
long path;		/* SOM path number				*/
long satnum;		/* SOM satellite number				*/
long mode;		/* which initialization method  to use A or B	*/

	/* Function declarations for function pointer use
	-----------------------------------------------*/
long utmfor();
long stplnfor();
long alberfor();
long lamccfor();
long merfor();
long psfor();
long polyfor();
long eqconfor();
long tmfor();
long sterfor();
long lamazfor();
long azimfor();
long gnomfor();
long orthfor();
long gvnspfor();
long sinfor();
long equifor();
long millfor();
long vandgfor();
long omerfor();
long somfor();
long hamfor();
long robfor();
long goodfor();
long molwfor();
long imolwfor();
long alconfor();
long wivfor();
long wviifor();
long obleqfor();

/* Initialize forward transformations
-----------------------------------*/
  /* find the correct major and minor axis
  --------------------------------------*/
  sphdz(outspheroid,outparm,&r_major,&r_minor,&radius);
  false_easting  = outparm[6];
  false_northing = outparm[7];

  if (outsys == UTM)
    {
    /* this is the call to initialize U T M
    -------------------------------------*/
     /* set Clarke 1866 spheroid if negative spheroid code  
        -----------------------------------------------*/
     if (outspheroid < 0)
        {
        tmpspheroid = 0;
	sphdz(tmpspheroid,outparm,&r_major,&r_minor,&radius);
        }
    zone = outzone;
    if (zone == 0)
      {
      lon1 = paksz(outparm[0],iflg)* 3600 * S2R;
      if (*iflg != 0)
        return;
      lat1 = paksz(outparm[1],iflg)* 3600 * S2R;
      if (*iflg != 0)
        return;
      zone = calc_utm_zone(lon1 * R2D);
      if (lat1 < 0)
         zone = -zone;
      }
    scale_factor = .9996;
    *iflg = utmforint(r_major,r_minor,scale_factor,zone);
    for_trans[outsys] = utmfor;
    }
  else
  if (outsys == SPCS)
    {
    /* this is the call to initialize STATE PLANE 
    -------------------------------------------*/
    *iflg = stplnforint(outzone,outspheroid,fn27,fn83);
    if (*iflg != 0)
       return;
    for_trans[outsys] = stplnfor;
    }
  else
  if (outsys == ALBERS)
    {
    /* this is the call to initialize ALBERS CONICAL EQUAL AREA 
    ----------------------------------------------------------*/
    lat1 = paksz(outparm[2],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat2 = paksz(outparm[3],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = alberforint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
		       false_easting, false_northing);
    for_trans[outsys] = alberfor;
    }
  else
  if (outsys == LAMCC)
    {
    /* this is the call to initialize LAMBERT CONFORMAL CONIC 
    --------------------------------------------------------*/
    lat1 = paksz(outparm[2],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat2 = paksz(outparm[3],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin  = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = lamccforint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
	   	       false_easting, false_northing);
    for_trans[outsys] = lamccfor;
    }
  else
  if (outsys == MERCAT)
    {
    /* this is the call to initialize MERCATOR
    ----------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat1   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = merforint(r_major,r_minor,center_long,lat1,false_easting,
		     false_northing);
    for_trans[outsys] = merfor;
    }
  else
  if (outsys == PS)
    {
    /* this is the call to initialize POLAR STEREOGRAPHIC 
    ----------------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat1  = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = psforint(r_major,r_minor,center_long,lat1,false_easting,
		    false_northing);
    for_trans[outsys] = psfor;
    }
  else
  if (outsys == POLYC)
    {
    /* this is the call to initialize POLYCONIC
    -----------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = polyforint(r_major,r_minor,center_long,lat_origin,false_easting,
		      false_northing); 
    for_trans[outsys] = polyfor;
    }
  else
  if (outsys == EQUIDC)
    {
    /* this is the call to initialize EQUIDISTANT CONIC 
    -------------------------------------------------*/
    lat1 = paksz(outparm[2],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat2 = paksz(outparm[3],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    if (outparm[8] == 0)
       mode = 0;
    else 
       mode = 1;
    *iflg = eqconforint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
		false_easting,false_northing,mode);
    for_trans[outsys] = eqconfor;
    }
  else
  if (outsys == TM)
    {
    /* this is the call to initialize TRANSVERSE MECTAR
    -------------------------------------------------*/
    scale_factor = outparm[2];
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = tmforint(r_major,r_minor,scale_factor,center_long,lat_origin,
		    false_easting,false_northing);
    for_trans[outsys] = tmfor;
    }
  else
  if (outsys == STEREO)
    {
    /* this is the call to initialize STEREOGRAPHIC
    ---------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = sterforint(radius,center_long,center_lat,false_easting,
		      false_northing); 
    for_trans[outsys] = sterfor;
    }
  else
  if (outsys == LAMAZ)
    {
    /* this is the call to initialize LAMBERT AZIMUTHAL
    -------------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat  = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = lamazforint(radius,center_long, center_lat,false_easting,
		       false_northing);
    for_trans[outsys] = lamazfor;
    }
  else
  if (outsys == AZMEQD)
    {
    /* this is the call to initialize AZIMUTHAL EQUIDISTANT
    -----------------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = azimforint(radius,center_long,center_lat,false_easting,
		      false_northing); 
    for_trans[outsys] = azimfor;
    }
  else
  if (outsys == GNOMON)
    {
    /* this is the call to initialize GNOMONIC 
    ----------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = gnomforint(radius,center_long,center_lat,false_easting,
		      false_northing);
    for_trans[outsys] = gnomfor;
    }
  else
  if (outsys == ORTHO)
    {
    /* this is the call to initalize ORTHOGRAPHIC
    -------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = orthforint(radius,center_long,center_lat,false_easting,
		      false_northing); 
    for_trans[outsys] = orthfor;
    }
  else
  if (outsys == GVNSP)
    {
    /* this is the call to initalize GENERAL VERTICAL NEAR-SIDE PERSPECTIVE
    ----------------------------------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    h = outparm[2];
    *iflg = gvnspforint(radius,h,center_long,center_lat,false_easting,
		       false_northing);
    for_trans[outsys] = gvnspfor;
    }
  else
  if (outsys == SNSOID)
    {
    /* this is the call to initialize SINUSOIDAL 
    -------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = sinforint(radius, center_long,false_easting,false_northing);
    for_trans[outsys] = sinfor;
    }
  else
  if (outsys == EQRECT)
    {
    /* this is the call to initialize EQUIRECTANGULAR
    -----------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    lat1   = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = equiforint(radius,center_long,lat1,false_easting,false_northing); 
    for_trans[outsys] = equifor;
    }
  else
  if (outsys == MILLER)
    {
    /* this is the call to initialize MILLER CYLINDRICAL 
    --------------------------------------------------*/
    center_long  = paksz(outparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = millforint(radius, center_long,false_easting,false_northing); 
    for_trans[outsys] = millfor;
    }
  else
  if (outsys == VGRINT)
    {
    /* this is the call to initialize VAN DER GRINTEN 
    -----------------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = vandgforint(radius, center_long,false_easting,false_northing); 
    for_trans[outsys] = vandgfor;
    }
  else
  if (outsys == HOM)
     {
     /* this is the call to initialize HOTLINE OBLIQUE MERCATOR
     ---------------------------------------------------------*/
     scale_factor = outparm[2];
     lat_origin = paksz(outparm[5],iflg)* 3600 * S2R;
     if (*iflg != 0)
        return;
     if (outparm[12] != 0)
        {
        mode = 1;
        azimuth = paksz(outparm[3],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        lon_origin = paksz(outparm[4],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        }
     else
        {
        mode = 0;
        lon1 = paksz(outparm[8],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        lat1 = paksz(outparm[9],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        lon2 = paksz(outparm[10],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        lat2 = paksz(outparm[11],iflg)* 3600 * S2R;
        if (*iflg != 0)
           return;
        }
     *iflg = omerforint(r_major,r_minor,scale_factor,azimuth,lon_origin,
                        lat_origin,false_easting, false_northing,lon1,lat1,
                        lon2,lat2,mode);
     for_trans[outsys] = omerfor;
     }
  else
  if (outsys == SOM)
    {
    /* this is the call to initialize SOM 
    -----------------------------------*/
    path = outparm[3];
    satnum = outparm[2];
    if (outparm[12] == 0)
       {
       mode = 1;
       alf = paksz(outparm[3],iflg)* 3600 * S2R;
       if (*iflg != 0)
          return;
       lon1 = paksz(outparm[4],iflg)* 3600 * S2R;
       if (*iflg != 0)
          return;
       time = outparm[8];
       start = outparm[10];
       }
    else
       mode = 0;
/*
    *iflg = somforint(r_major,r_minor,satnum,path,false_easting,false_northing);
*/
    *iflg = somforint(r_major,r_minor,satnum,path,alf,lon1,false_easting,
		      false_northing,time,start,mode);
    for_trans[outsys] = somfor;
    }
  else
  if (outsys == HAMMER)
    {
    /* this is the call to initialize HAMMER 
    --------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = hamforint(radius,center_long,false_easting,false_northing); 
    for_trans[outsys] = hamfor;
    }
  else
  if (outsys == ROBIN)
    {
    /* this is the call to initialize ROBINSON 
    ----------------------------------------*/
    center_long  = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = robforint(radius,center_long,false_easting,false_northing); 
    for_trans[outsys] = robfor;
    }
  else
  if (outsys == GOOD)
    {
    /* this is the call to initialize GOODE'S HOMOLOSINE
    ---------------------------------------------------*/
    *iflg = goodforint(radius);
    for_trans[outsys] = goodfor;
    }
  else
  if (outsys == MOLL)
    {
    /* this is the call to initialize MOLLWEIDE
    ------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = molwforint(radius, center_long,false_easting,false_northing);
    for_trans[outsys] = molwfor;
    }
  else
  if (outsys == IMOLL)
    {
    /* this is the call to initialize INTERRUPTED MOLLWEIDE
    -----------------------------------------------------*/
    *iflg = imolwforint(radius);
    for_trans[outsys] = imolwfor;
    }
  else
  if (outsys == ALASKA)
    {
    /* this is the call to initialize ALASKA CONFORMAL 
    ------------------------------------------------*/
    *iflg = alconforint(r_major,r_minor,false_easting,false_northing);
    for_trans[outsys] = alconfor;
    }
  else
  if (outsys == WAGIV)
    {
    /* this is the call to initialize WAGNER IV 
    -----------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = wivforint(radius, center_long,false_easting,false_northing);
    for_trans[outsys] = wivfor;
    }
  else
  if (outsys == WAGVII)
    {
    /* this is the call to initialize WAGNER VII 
    ------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = wviiforint(radius, center_long,false_easting,false_northing);
    for_trans[outsys] = wviifor;
    }
  else
  if (outsys == OBEQA)
    {
    /* this is the call to initialize OBLATED EQUAL AREA 
    ---------------------------------------------------*/
    center_long = paksz(outparm[4],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat  = paksz(outparm[5],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    shape_m = outparm[2];
    shape_n = outparm[3];
    angle = paksz(outparm[8],iflg)* 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = obleqforint(radius,center_long,center_lat,shape_m, shape_n, 
		angle,false_easting,false_northing);
    for_trans[outsys] = obleqfor;
    }
   
return;
}
