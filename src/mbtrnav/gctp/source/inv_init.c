/*******************************************************************************
NAME                           INV_INIT 

PURPOSE:	Initializes inverse projection transformation parameters

PROGRAMMER              DATE		REASON
----------              ----		------
T. Mittan		3-09-93		Initial Development
S. Nelson		11-94		Added Clarke spheroid default to UTM
S. Nelson		 1-98		Changed datum to spheroid.

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

void inv_init(insys,inzone,inparm,inspheroid,fn27,fn83,iflg,inv_trans)

long insys;		/* input system code				*/
long inzone;		/* input zone number				*/
double *inparm;		/* input array of projection parameters		*/
long inspheroid;	/* input spheroid code				*/
char *fn27;		/* NAD 1927 parameter file			*/
char *fn83;		/* NAD 1983 parameter file			*/
long *iflg;		/* status flag					*/
long (*inv_trans[])();	/* inverse function pointer			*/
{
long zone;		/* zone number					*/
double azimuth;		/* azimuth					*/
double angle;		/* rotation anlge				*/
double alf;		/* SOM angle					*/
double lon;		/* longitude					*/
double lon1;		/* longitude point in utm scene			*/
double lon2;		/* 2nd longitude point 				*/
double lat;		/* latitude					*/
double lat1;		/* 1st standard parallel			*/
double lat2;		/* 2nd standard parallel			*/
double center_long;	/* center longitude				*/
double center_lat;	/* center latitude				*/
double h;		/* height above sphere				*/
double lat_origin;	/* latitude at origin				*/
double lon_origin;	/* longitude at origin				*/
double r_major;		/* major axis in meters				*/
double r_minor;		/* minor axis in meters				*/
double scale_factor;	/* scale factor					*/
double false_easting;	/* false easting in meters			*/
double false_northing;	/* false northing in meters			*/
double radius;		/* radius of sphere				*/
double shape_m;		/* constant used for Oblated Equal Area		*/
double shape_n;		/* constant used for Oblated Equal Area		*/
long   start;		/* start of SOM Beginning or end		*/
double time;		/* SOM time					*/
long path;		/* SOM path number				*/
long satnum;		/* SOM satellite number				*/
long mode;		/* which format is used	A or B			*/
long tmpspheroid;	/* temporary spheroid for UTM			*/

	/* Function declarations for inverse function pointer
	---------------------------------------------------*/
long utminv();
long stplninv();
long alberinv();
long lamccinv();
long merinv();
long psinv();
long polyinv();
long eqconinv();
long tminv();
long sterinv();
long lamazinv();
long aziminv();
long gnominv();
long orthinv();
long gvnspinv();
long sininv();
long equiinv();
long millinv();
long vandginv();
long omerinv();
long sominv();
long haminv();
long robinv();
long goodinv();
long molwinv();
long imolwinv();
long alconinv();
long wivinv();
long wviiinv();
long obleqinv();

/* Initialize inverse transformations
-----------------------------------*/
  /* find the correct major and minor axis
  --------------------------------------*/
  sphdz(inspheroid,inparm,&r_major,&r_minor,&radius);
  false_easting  = inparm[6];
  false_northing = inparm[7];

  if (insys == UTM)
     {
     /* this is the call to initialize U T M
     -------------------------------------*/
     /* set Clarke 1866 spheroid if negative spheroid code
        ----------------------------------------------*/
     if (inspheroid < 0)
        {
        tmpspheroid = 0;
        sphdz(tmpspheroid,inparm,&r_major,&r_minor,&radius);
        }
     zone = inzone;
     if (zone == 0)
        {
        lon1 = paksz(inparm[0],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        lat1 = paksz(inparm[1],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        zone = calc_utm_zone(lon1 * R2D);
        if (lat1 < 0)
           zone = -zone;
        }
     scale_factor = .9996;
     *iflg = utminvint(r_major,r_minor,scale_factor,zone);
     inv_trans[insys] = utminv;
     }
  else
  if (insys == SPCS)
     {
     /* this is the call to initialize STATE PLANE 
     --------------------------------------------*/
     *iflg = stplninvint( inzone,inspheroid,fn27,fn83);
        if (*iflg != 0)
           return;
     inv_trans[insys] = stplninv;
     }
  else
  if (insys == ALBERS)
     {
     /* this is the call to initialize ALBERS 
     ---------------------------------------*/
     lat1 = paksz(inparm[2],iflg) * 3600 * S2R;
     if (*iflg != 0)
         return;
     lat2 = paksz(inparm[3],iflg) * 3600 * S2R;
     if (*iflg != 0)
         return;
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat_origin  = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = alberinvint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
			false_easting, false_northing);
     inv_trans[insys] = alberinv;
     }
  else
  if (insys == LAMCC)
     {
     /* this is the call to initialize LAMBERT CONFORMAL CONIC 
     --------------------------------------------------------*/
     lat1 = paksz(inparm[2],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat2 = paksz(inparm[3],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat_origin  = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = lamccinvint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
			false_easting, false_northing);
     inv_trans[insys] = lamccinv;
     }
  else
  if (insys == MERCAT)
     {
     /* this is the call to initialize MERCATOR
     ----------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat1   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = merinvint(r_major,r_minor,center_long,lat1,false_easting,
                     false_northing);
     inv_trans[insys] = merinv;
     }
  else
  if (insys == PS)
     {
     /* this is the call to initialize POLAR STEREOGRAPHIC 
     ----------------------------------------------------*/
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat1  =  paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = psinvint(r_major,r_minor,center_long,lat1,false_easting,
	      false_northing);
     inv_trans[insys] = psinv;
     }
  else
  if (insys == POLYC)
     {
     /* this is the call to initialize POLYCONIC
     -----------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat_origin   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = polyinvint(r_major,r_minor,center_long,lat_origin,false_easting,
		       false_northing); 
     inv_trans[insys] = polyinv;
     }
  else
    if (insys == EQUIDC)
    {
     /* this is the call to initialize EQUIDISTANT CONIC
     ---------------------------------------------------*/
    lat1 = paksz(inparm[2],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    lat2 = paksz(inparm[3],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    lat_origin   = paksz(inparm[5],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    if (inparm[8] == 0)
       mode = 0;
    else
       mode = 1;
    *iflg = eqconinvint(r_major,r_minor,lat1,lat2,center_long,lat_origin,
		false_easting,false_northing,mode);
    inv_trans[insys] = eqconinv;
    }
  else
  if (insys == TM)
     {
     /* this is the call to initialize TRANSVERSE MERCATOR
     -------------------------------------------------*/
     scale_factor = inparm[2];
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat_origin   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = tminvint(r_major,r_minor,scale_factor,center_long,lat_origin,
		     false_easting, false_northing);
     inv_trans[insys] = tminv;
     }
  else
  if (insys == STEREO)
     {
     /* this is the call to initialize STEREOGRAPHIC
     ---------------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = sterinvint(radius,center_long,center_lat,false_easting, 
		       false_northing); 
     inv_trans[insys] = sterinv;
     }
  else
  if (insys == LAMAZ)
     {
     /* this is the call to initialize LAMBERT AZIMUTHAL EQUAL-AREA 
     -------------------------------------------------------------*/
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat  = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = lamazinvint(radius, center_long, center_lat,false_easting,
			false_northing);
     inv_trans[insys] = lamazinv;
     }
  else
  if (insys == AZMEQD)
     {
     /* this is the call to initialize AZIMUTHAL EQUIDISTANT
     ------------------------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = aziminvint(radius,center_long,center_lat,false_easting,
		       false_northing); 
     inv_trans[insys] = aziminv;
     }
  else
  if (insys == GNOMON)
     {
     /* this is the call to initialize GNOMONIC 
     ----------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = gnominvint(radius,center_long,center_lat,false_easting,
                      false_northing);
     inv_trans[insys] = gnominv;
     }
  else
  if (insys == ORTHO)
     {
     /* this is the call to initialize ORTHOGRAPHIC
     --------------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = orthinvint(radius,center_long,center_lat,false_easting,
		       false_northing); 
     inv_trans[insys] = orthinv;
     }
  else
  if (insys == GVNSP)
     {
     /* this is the call to initialize GENERAL VERTICAL NEAR SIDED PERSPECTIVE 
     -----------------------------------------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     center_lat   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     h = inparm[2];
     *iflg = gvnspinvint(radius,h,center_long,center_lat,false_easting,
			false_northing);
     inv_trans[insys] = gvnspinv;
     }
  else
  if (insys == SNSOID)
     {
     /* this is the call to initialize SINUSOIDAL 
     --------------------------------------------*/
     center_long    = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = sininvint(radius, center_long,false_easting,false_northing);
     inv_trans[insys] = sininv;
     }
  else
  if (insys == EQRECT)
     {
     /* this is the call to initialize EQUIRECTANGULAR
     -----------------------------------------------*/
     center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     lat1   = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = equiinvint(radius,center_long,lat1,false_easting,
		       false_northing); 
     inv_trans[insys] = equiinv;
     }
  else
  if (insys == MILLER)
    {
    /* this is the call to initialize MILLER CYLINDRICAL
    --------------------------------------------------*/
    center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = millinvint(radius, center_long,false_easting,false_northing);
    inv_trans[insys] = millinv;
    }
  else
  if (insys == VGRINT)
    {
    /* this is the call to initialize VAN DER GRINTEN 
    -----------------------------------------------*/
    center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = vandginvint(radius, center_long,false_easting,false_northing);
    inv_trans[insys] = vandginv;
    }
  else
  if (insys == HOM)
     {
     /* this is the call to initialize HOTLINE OBLIQUE MERCATOR 
     ---------------------------------------------------------*/
     scale_factor = inparm[2];
     lat_origin = paksz(inparm[5],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     if (inparm[12] != 0)
        {
        mode = 1;
        azimuth = paksz(inparm[3],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        lon_origin = paksz(inparm[4],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        }
     else
        {
        mode = 0;
        lon1 = paksz(inparm[8],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        lat1 = paksz(inparm[9],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        lon2 = paksz(inparm[10],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        lat2 = paksz(inparm[11],iflg) * 3600 * S2R;
        if (*iflg != 0)
           return;
        
        }
     *iflg = omerinvint(r_major,r_minor,scale_factor,azimuth,lon_origin,
			lat_origin,false_easting, false_northing,lon1,lat1,
			lon2,lat2,mode);
     inv_trans[insys] = omerinv;
     }
  else
  if (insys == SOM)
     {
     /* this is the call to initialize SOM 
     -----------------------------------*/
     path = inparm[3];
     satnum = inparm[2];
    if (inparm[12] == 0)
       {
       mode = 1;
       alf = paksz(inparm[3],iflg) * 3600 * S2R;
       if (*iflg != 0)
          return;
       lon1 = paksz(inparm[4],iflg) * 3600 * S2R;
       if (*iflg != 0)
          return;
       time = inparm[8];
       start = inparm[10];
       }
    else
       mode = 0;
/*
     *iflg = sominvint(r_major,r_minor,satnum,path,false_easting,
			false_northing);
*/
     *iflg = sominvint(r_major,r_minor,satnum,path,alf,lon1,false_easting,
                      false_northing,time,start,mode);
     inv_trans[insys] = sominv;
     }
  else
  if (insys == HAMMER)
    {
    /* this is the call to initialize HAMMER 
    --------------------------------------*/
    center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = haminvint(radius, center_long,false_easting,false_northing);
    inv_trans[insys] = haminv;
    }
  else
  if (insys == ROBIN)
    {
    /* this is the call to initialize ROBINSON 
    ----------------------------------------*/
    center_long  = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = robinvint(radius, center_long,false_easting,false_northing);
    inv_trans[insys] = robinv;
    }
  else
  if (insys == GOOD)
     {
     /* this is the call to initialize GOODE'S HOMOLOSINE
     ---------------------------------------------------*/
     *iflg = goodinvint(radius);
     inv_trans[insys] = goodinv;
     }
  else
  if (insys == MOLL)
     {
     /* this is the call to initialize MOLLWEIDE
     -------------------------------------------*/
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = molwinvint(radius, center_long,false_easting,false_northing);
     inv_trans[insys] = molwinv;
     }
  else
  if (insys == IMOLL)
     {
     /* this is the call to initialize INTERRUPTED MOLLWEIDE 
     -----------------------------------------------------*/
     *iflg = imolwinvint(radius);
     inv_trans[insys] = imolwinv;
     }
  else
  if (insys == ALASKA)
     {
     /* this is the call to initialize ALASKA CONFORMAL 
     ------------------------------------------------*/
     *iflg = alconinvint(r_major,r_minor,false_easting,false_northing);
     inv_trans[insys] = alconinv;
     }
  else
  if (insys == WAGIV)
     {
     /* this is the call to initialize WAGNER IV 
     -----------------------------------------*/
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = wivinvint(radius, center_long,false_easting,false_northing);
     inv_trans[insys] = wivinv;
     }
  else
  if (insys == WAGVII)
     {
     /* this is the call to initialize WAGNER VII 
     ------------------------------------------*/
     center_long = paksz(inparm[4],iflg) * 3600 * S2R;
     if (*iflg != 0)
        return;
     *iflg = wviiinvint(radius, center_long,false_easting,false_northing);
     inv_trans[insys] = wviiinv;
     }
  else
  if (insys == OBEQA)
    {
    /* this is the call to initialize OBLATED EQUAL AREA
    ---------------------------------------------------*/
    center_long = paksz(inparm[4],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    center_lat  = paksz(inparm[5],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    shape_m = inparm[2];
    shape_n = inparm[3];
    angle = paksz(inparm[8],iflg) * 3600 * S2R;
    if (*iflg != 0)
       return;
    *iflg = obleqinvint(radius,center_long,center_lat,shape_m, shape_n,
                angle,false_easting,false_northing);
    inv_trans[insys] = obleqinv;
    }

return;
}
