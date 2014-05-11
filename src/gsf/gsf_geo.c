/********************************************************************
 *
 * File Name : gsf_geo.c
 *
 * Author/Date : R.K.Wells / 26 Feb 2014
 *
 * Description : Some hopefully useful functions to compute auxiliary
 *               information about points on a ship.
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 *
 * Classification : Unclassified
 *
 * References :
 *
 * © 2014 Leidos, Inc.
 * There is no charge to use the library, and it may be accessed at:
 * https://www.leidos.com/maritime/gsf.
 * This library may be redistributed and/or modified under the terms of
 * the GNU Lesser General Public License version 2.1, as published by the
 * Free Software Foundation.  A copy of the LGPL 2.1 license is included with
 * the GSF distribution and is avaialbe at: http://opensource.org/licenses/LGPL-2.1.
 *
 * Leidos, Inc. configuration manages GSF, and provides GSF releases. Users are
 * strongly encouraged to communicate change requests and change proposals to Leidos, Inc.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 ********************************************************************/

#include <math.h>

/* rely on the network type definitions of (u_short, and u_int) */
#include <sys/types.h>
#if !defined WIN32 && !defined WIN64
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

#include <math.h>

/* gsf library interface description */
#include "gsf.h"

/* Global external data defined in this module */
extern int      gsfError;                               /* defined in gsf.c */

#define SQR(x) ((x)*(x))
#define Everest_1830        0
#define Bessel_1841         1
#define Clarke_1866         2
#define Clarke_1880         3
#define International_1909  4
#define Australian_National 5
#define Airy                6
#define Fischer_1960        7
#define WGS_1966            8
#define Fischer_1968        9
#define WGS_1972           10
#define WGS_1984           11

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef DTOR
#define DTOR(f) ((f)*PI/180.0)
#endif

#ifndef RTOD
#define RTOD(f) ((f)*180.0/PI)
#endif

#ifndef DEG_TO_METERS
#define DEG_TO_METERS   (1852.0 * 60.0)
#endif

typedef struct {
    double a0;
    double b0;
    char *name;
} GSF_GP_INFO, *pGSF_GP_INFO;

static GSF_GP_INFO gp_info[] = {
    {6377276.345, 6356075.413, "Everest 1830"},
    {6377397.155, 6356078.963, "Bessel 1841"},
    {6378206.400, 6356583.800, "Clarke 1866"},
    {6378249.145, 6356514.869, "Clarke 1880"},
    {6378388.000, 6356911.946, "International 1909"},
    {6378160.000, 6356774.719, "Australian National"},
    {6377563.396, 6356256.910, "Airy"},
    {6378166.000, 6356774.283, "Fischer 1960"},
    {6378145.000, 6356759.769, "WGS 1966"},
    {6378150.000, 6356768.337, "Fischer 1968"},
    {6378135.000, 6356750.520, "WGS 1972"},
    {6378137.000, 6356752.314, "WGS 1984"},
};

static double a0 (int k)
{
    return gp_info[k].a0;
}

static double b0 (int k)
{
    return gp_info[k].b0;
}

static void metric (double phi, int k, double *gx, double *gy)
{
        double beta;

        beta = atan (b0 (k) * tan (phi) / a0 (k));
        *gx = a0 (k) * cos (beta);
        *gy = a0 (k) * SQR (b0 (k) / a0 (k)) * pow (fabs (cos (beta) / cos (phi)), 3.0);
}

/********************************************************************
 *
 * Function Name : gsfGetPositionDestination
 *
 * Description : compute a new position from an existing one.
 *
 * Inputs : ref pos, offsets from ref (+x forward, +y starboard, + z down), ref heading (+hdg cw from north), maximum distance step.
 *
 * latitude, longitude (in GSF_POSITION), and heading are in degrees.  Distances and offsests are in meters.
 *
 * Returns : new position
 *
 * Error Conditions :
 *
 ********************************************************************/

GSF_POSITION *gsfGetPositionDestination(GSF_POSITION gp, GSF_POSITION_OFFSETS offsets, double hdg, double dist_step)
{
    static GSF_POSITION     new_gp;
    double                  gx, gy;
    double                  dp, dl;
    double                  dx, dy, dz;
    double                  ilat, ilon, iz;
    int                     n, iter;

    gp.lat   = DTOR(gp.lat);
    gp.lon   = DTOR(gp.lon);
    hdg      = DTOR(hdg);

    if (fabs(offsets.x) > fabs(offsets.y))
        iter = (int)floor(fabs(offsets.x) / dist_step);
    else
        iter = (int)floor(fabs(offsets.y) / dist_step);
    if (iter <= 0)
        iter = 1;

    dx = offsets.x / (double)iter;
    dy = offsets.y / (double)iter;
    dz = offsets.z / (double)iter;
    ilat = gp.lat;
    ilon = gp.lon;
    iz   = gp.z;
    for (n = 0; n < iter; n++) {
        metric(ilat, WGS_1984, &gx, &gy);
        dl = (dy * cos(hdg) + dx * sin(hdg)) / gx;
        dp = (dx * cos(hdg) - dy * sin(hdg)) / gy;
        ilon += dl;
        ilat += dp;
        iz   += dz;
    }
    new_gp.lon = RTOD(ilon);
    new_gp.lat = RTOD(ilat);
    new_gp.z   = iz;

    return &new_gp;
}


/********************************************************************
 *
 * Function Name : gsfGetPositionOffsets
 *
 * Description : compute offsets between two positions.
 *
 * Inputs : ref pos, new pos, ref heading (+hdg cw from north), maximum distance_step.
 *
 * latitude, longitude (in GSF_POSITION), and heading are in degrees.  Distances and offsests are in meters.
 *
 * Returns : offsets from ref (+x forward, +y starboard, + z down)
 *
 * Error Conditions :
 *
 ********************************************************************/

GSF_POSITION_OFFSETS *gsfGetPositionOffsets(GSF_POSITION gp_from, GSF_POSITION gp_to, double hdg, double dist_step)
{
    static GSF_POSITION_OFFSETS   offsets;
    double                  gx, gy;
    double                  dx, dy, dz;
    double                  dlat, dlon, doz;
    double                  ilat;
    double                  ix, iy, iz;
    int                     n, iter;
    double                  lat_diff, lon_diff;

    //  need to determine number of iterations from the dist_step.

    gp_from.lat = DTOR(gp_from.lat);
    gp_from.lon = DTOR(gp_from.lon);
    gp_to.lat   = DTOR(gp_to.lat);
    gp_to.lon   = DTOR(gp_to.lon);
    hdg         = DTOR(hdg);

    if (gp_to.lat > gp_from.lat)
        lat_diff = fabs(gp_to.lat - gp_from.lat);
    else
        lat_diff = fabs(gp_from.lat - gp_to.lat);
    lat_diff = RTOD(lat_diff) * DEG_TO_METERS;

    if (gp_to.lon > gp_from.lon)
        lon_diff = fabs(gp_to.lon - gp_from.lon);
    else
        lon_diff = fabs(gp_from.lon - gp_to.lon);

    if (lon_diff > PI)
        lon_diff = (2.0 * PI) - lon_diff;
    lon_diff = RTOD(lon_diff) * DEG_TO_METERS * cos(gp_to.lat);


    if (lon_diff > lat_diff)
        iter = (int)ceil(lon_diff / dist_step);
    else
        iter = (int)ceil(lat_diff / dist_step);

    //  need to handle crossing the dateline.
    dlon = gp_to.lon - gp_from.lon;
    if (dlon > PI)
        dlon = ((2.0 * PI) - dlon) / (double)iter;
    else if (dlon < -PI)
        dlon = -(dlon + (2.0 * PI)) / (double)iter;
    else
        dlon = dlon / (double)iter;

    dlat = (gp_to.lat - gp_from.lat) / (double)iter;
    doz  = (gp_to.z   - gp_from.z)   / (double)iter;

    ilat = gp_from.lat;
    iz   = gp_from.z;
    ix = iy = 0.0;
    for (n = 0; n < iter; n++) {
        metric(ilat, WGS_1984, &gx, &gy);
        dx = -gx * dlon;
        dy = -gy * dlat;
        dz = -doz;

        ix += dy * cos(hdg) + dx * sin(hdg);
        iy += dx * cos(hdg) - dy * sin(hdg);
        iz += dz;

        ilat += dlat;
    }
    offsets.x = ix;
    offsets.y = iy;
    offsets.z = iz;

    return &offsets;
}

