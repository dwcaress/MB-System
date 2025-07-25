/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
 * @file
 * @brief Define swath data format integer identifier codes used by 
 * MBIO functions
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 *
 */

#ifndef MB_FORMAT_H_
#define MB_FORMAT_H_

#include "mb_status.h"

/* Supported swath sonar systems */
#define MB_SYS_NONE 0
#define MB_SYS_SB 1
#define MB_SYS_HSDS 2
#define MB_SYS_SB2000 3
#define MB_SYS_SB2100 4
#define MB_SYS_SIMRAD 5
#define MB_SYS_SIMRAD2 6
#define MB_SYS_SIMRAD3 7
#define MB_SYS_MR1 8
#define MB_SYS_MR1B 9
#define MB_SYS_MR1V2001 10
#define MB_SYS_LDEOIH 11
#define MB_SYS_RESON 12
#define MB_SYS_RESON8K 13
#define MB_SYS_ELAC 14
#define MB_SYS_ELACMK2 15
#define MB_SYS_HSMD 16
#define MB_SYS_DSL 17
#define MB_SYS_GSF 18
#define MB_SYS_MSTIFF 19
#define MB_SYS_OIC 20
#define MB_SYS_HDCS 21
#define MB_SYS_SINGLEBEAM 22
#define MB_SYS_XSE 23
#define MB_SYS_HS10 24
#define MB_SYS_NETCDF 25
#define MB_SYS_ATLAS 26
#define MB_SYS_NAVNETCDF 27
#define MB_SYS_SURF 28
#define MB_SYS_RESON7K 29
#define MB_SYS_JSTAR 30
#define MB_SYS_IMAGE83P 31
#define MB_SYS_HYSWEEP 32
#define MB_SYS_BENTHOS 33
#define MB_SYS_SWATHPLUS 34
#define MB_SYS_3DATDEPTHLIDAR 35
#define MB_SYS_3DDWISSL1 36
#define MB_SYS_3DDWISSL2 37
#define MB_SYS_WASSP 38
#define MB_SYS_STEREOPAIR 39
#define MB_SYS_KMBES 40
#define MB_SYS_RESON7K3 41

/* Number of supported MBIO data formats */
#define MB_FORMATS 87

/* Data formats supported by MBIO */

#define MBF_DATALIST -1

#define MBF_IMAGELIST -2

#define MBF_NONE 0

#define MBF_SBSIOMRG 11
/* SeaBeam, 16 beam, bathymetry, \
binary, uncentered, SIO. */

#define MBF_SBSIOCEN 12
/* SeaBeam, 19 beam, bathymetry, \
binary, centered, SIO. */

#define MBF_SBSIOLSI 13
/* SeaBeam, 19 beam, bathymetry, \
binary, centered, obsolete, SIO. */

#define MBF_SBURICEN 14
/* SeaBeam, 19 beam, bathymetry, \
binary, centered, URI. */

#define MBF_SBURIVAX 15
/* SeaBeam, 19 beam, bathymetry, \
binary, centered, \
VAX byte order, URI. */

#define MBF_SBSIOSWB 16
/* SeaBeam, 19 beam, bathymetry, \
binary, centered, \
swath-bathy, SIO. */

#define MBF_SBIFREMR 17
/* SeaBeam, 19 beam, bathymetry, \
ascii, centered, \
sounding-oriented, IFREMER. */

#define MBF_HSATLRAW 21
/* Hydrosweep DS raw format, 59 beam, \
bathymetry and amplitude, \
ascii, Atlas Electronik. */

#define MBF_HSLDEDMB 22
/* Hydrosweep DS, 59 beam, bathymetry, \
binary, NRL. */

#define MBF_HSURICEN 23
/* Hydrosweep DS, 59 beam, bathymetry, \
binary, URI. */

#define MBF_HSLDEOIH 24
/* Hydrosweep DS in-house format, 59 beam, \
bathymetry and amplitude, \
binary, centered, L-DEO. */

#define MBF_HSURIVAX 25
/* Hydrosweep DS, 59 beam, bathymetry, \
binary, VAX byte order, URI. */

#define MBF_HSUNKNWN 26
/* Hydrosweep DS, 59 beam, bathymetry, \
bathymetry and amplitude, \
ascii, unknown origin, SOPAC. */

#define MBF_SB2000RW 31
/* SeaBeam 2000 vender format, \
151 beam bathymetry and amplitude, \
2000 pixel sidescan, ascii + binary, \
SeaBeam Instruments. */

#define MBF_SB2000SB 32
/* SeaBeam 2000, 121 beam bathymetry, \
binary, swath-bathy, SIO. */

#define MBF_SB2000SS 33
/* SeaBeam 2000, 1000 pixel sidescan, \
binary, swath-bathy, SIO. */

#define MBF_SB2100RW 41
/* SeaBeam 2100 series vender format, \
151 beam bathymetry and amplitude, \
2000 pixel sidescan, ascii + binary, \
centered, SeaBeam Instruments */

#define MBF_SB2100B1 42
/* SeaBeam 2100 series vendor format, \
151 beam bathymetry and amplitude, \
2000 pixel sidescan, binary, centered, \
SeaBeam Instruments and L-DEO */

#define MBF_SB2100B2 43
/* SeaBeam 2100 series vendor format, \
151 beam bathymetry and amplitude, \
binary, centered, \
SeaBeam Instruments and L-DEO */

#define MBF_EMOLDRAW 51
/* Old Simrad vendor multibeam format, \
Simrad EM1000, EM12S, EM12D, \
EM121 multibeam sonars, \
bathymetry, amplitude, and sidescan, \
60 beams for EM1000, 81 beams for EM12S/D, \
121 beams for EM121, variable pixels, \
ascii + binary, Simrad. */

#define MBF_EM12IFRM 53
/* Simrad EM12S/EM12D multibeam archive format, \
81 beam bathymetry and \
amplitude, \
binary, read-only, IFREMER */

#define MBF_EM12DARW 54
/* Simrad EM12 RRS Darwin processed format, \
81 beam, bathymetry and amplitude,  \
binary, centered, Oxford University */

#define MBF_EM300RAW 56
/* Simrad EM300/EM3000 multibeam vendor format, \
up to 254 beam bathymetry and \
amplitude, variable pixel sidescan, \
ascii + binary, Simrad */

#define MBF_EM300MBA 57
/* Simrad EM300/EM3000 multibeam processing format, \
up to 254 beam bathymetry and \
amplitude, variable pixel sidescan, \
ascii + binary, MBARI */

#define MBF_EM710RAW 58
/* Kongsberg EM122, EM302, EM710 multibeam vendor format, \
up to 400 beam bathymetry and \
amplitude, variable pixel sidescan, \
binary, Kongsberg */

#define MBF_EM710MBA 59
/* Kongsberg EM122, EM302, EM710 multibeam processing format,                                    \
up to 400 beam bathymetry and \
amplitude, variable pixel sidescan, \
binary, MBARI */

#define MBF_MR1PRHIG 61
/* Obsolete MR1 post processed format,  \
variable beam bathymetry, variable  \
pixel sidescan, xdr binary, SOEST */

#define MBF_MR1ALDEO 62
/* MR1 Lamont format with travel times, \
variable beam bathymetry, variable  \
pixel sidescan, xdr binary, L-DEO */

#define MBF_MR1BLDEO 63
/* MR1 small Lamont format with travel times, \
variable beam bathymetry, variable  \
pixel sidescan, xdr binary, L-DEO */

#define MBF_MR1PRVR2 64
/* MR1 post processed format, \
variable beam bathymetry, variable  \
pixel sidescan, xdr binary, SOEST */

#define MBF_MBLDEOIH 71
/* Generic in-house multibeam, variable beam, \
bathymetry, amplitude, and sidescan \
binary, centered, L-DEO. */

#define MBF_MBARIMB1 72
/* Generic in-house swath bathymetry, variable beam, \
bathymetry only, binary, centered, MBARI. */

#define MBF_MBNETCDF 75
/* CARAIBES CDF multibeam, variable beam, \
netCDF, IFREMER. */

#define MBF_MBNCDFXT 76
/* CARAIBES CDF multibeam, variable beam, \
netCDF, IFREMER. - extended format */

#define MBF_CBAT9001 81
/* Reson SeaBat 9001 multibeam, 60 beams  \
bathymetry and amplitude,  \
binary, University of New Brunswick. */

#define MBF_CBAT8101 82
/* Reson SeaBat 8101 multibeam, 101 beams \
bathymetry and amplitude,  \
binary, SeaBeam Instruments. */

#define MBF_HYPC8101 83
/* Reson SeaBat 8101 multibeam, 101 beams \
bathymetry,  \
ASCII, read-only,  \
Coastal Oceanographics. */

#define MBF_XTFR8101 84
/* Reson SeaBat 8XXX multibeam, 250 beams \
bathymetry,  \
binary, read-only, \
Triton-Elics XTF format. */

#define MBF_RESONS8K 85
/* Reson SeaBat 8XXX multibeam, 250 beams \
bathymetry,  \
binary, read-only, \
Reson 6042 format. */

#define MBF_SBATPROC 86
/* Reson SeaBat 8XXX multibeam, 250 beams \
bathymetry,  \
binary, \
MBARI processing format. */

#define MBF_RESON7KR 88
/* Reson 7K multibeam vendor format \
Reson 7K series multibeam sonars, \
bathymetry, amplitude, three channels sidescan, and subbottom  \
up to 254 beams, variable pixels, binary, Reson. */

#define MBF_RESON7K3 89
/* Teledyne 7k version 3 format  \
Teledyne Reson and Teledyne Atlas multibeam sonars, \
bathymetry, amplitude, three channels sidescan, and subbottom  \
variable beams, variable pixels, binary, Teledyne. */

#define MBF_BCHRTUNB 91
/* Elac BottomChart multibeam, 56 beams \
bathymetry and amplitude,  \
binary, University of New Brunswick. */

#define MBF_ELMK2UNB 92
/* Elac BottomChart multibeam, 56 beams \
bathymetry and amplitude,  \
binary, University of New Brunswick. */

#define MBF_BCHRXUNB 93
/* Elac BottomChart multibeam, 56 beams \
bathymetry and amplitude,  \
binary, University of New Brunswick. */

#define MBF_L3XSERAW 94
/* ELAC/SeaBeam XSE vendor format \
Bottomchart MkII 50 kHz and 180 kHz multibeam, \
SeaBeam 2120 20 KHz multibeam,  \
bathymetry, amplitude and sidescan, \
variable beams and pixels, binary,  \
L3 Communications (Elac Nautik  \
and SeaBeam Instruments). */

#define MBF_HSMDARAW 101
/* Hydroseep MD multibeam raw format, 40 beam  \
bathymetry, 160 pixel sidescan, \
xdr binary, Atlas Electronik. */

#define MBF_HSMDLDIH 102
/* Hydroseep MD multibeam in-house format, \
40 beam bathymetry, 160 pixel sidescan,  \
binary, L-DEO. */

#define MBF_DSL120PF 111
/* WHOI DSL AMS-120 deep-tow,  \
2048 beam bathymetry, 2048 pixel sidescan, \
binary, parallel files, WHOI DSL. */

#define MBF_DSL120SF 112
/* WHOI DSL AMS-120 deep-tow,  \
2048 beam bathymetry, 2048 pixel sidescan, \
binary, single files, WHOI DSL. */

#define MBF_GSFGENMB 121
/* SAIC Generic Sensor Format (GSF), \
variable beams,  bathymetry and amplitude, \
binary, single files, SAIC. */

#define MBF_MSTIFFSS 131
/* MSTIFF sidescan format,  \
variable pixels,  sidescan,  \
binary TIFF variant, single files, Sea Scan */

#define MBF_EDGJSTAR 132
/* Edgetech Jstar format  \
variable pixels, dual frequency sidescan and subbottom,  \
binary SEGY variant, single files, low frequency sidescan  \
returned as survey data, Edgetech. */

#define MBF_EDGJSTR2 133
/* Edgetech Jstar format  \
variable pixels, dual frequency sidescan and subbottom,  \
binary SEGY variant, single files, high frequency sidescan \
returned as survey data, Edgetech. */

#define MBF_OICGEODA 141
/* OIC swath sonar format, variable beam \
bathymetry and amplitude, variable  \
pixel sidescan, binary, Oceanic Imaging  \
Consultants */

#define MBF_OICMBARI 142
/* OIC-style extended swath sonar format, variable \
beam bathymetry and amplitude, variable  \
pixel sidescan, binary, MBARI */

#define MBF_OMGHDCSJ 151
/* UNB OMG HDCS format, variable \
beam bathymetry and amplitude, variable  \
pixel sidescan, binary, UNB */

#define MBF_SEGYSEGY 160
/* SEGY seismic or subbottom trace data, \
single beam bathymetry, nav, \
binary, SEG (SIOSEIS variant) */

#define MBF_MGD77DAT 161
/* NGDC MGD77 underway geophysics format, \
single beam bathymetry, nav, magnetics, gravity,  \
120 byte ascii records with no line breaks, NOAA NGDC */

#define MBF_ASCIIXYZ 162
/* XYZ (lon lat depth) soundings, ascii, generic */

#define MBF_ASCIIYXZ 163
/* YXZ (lat lon depth) soundings, ascii, generic */

#define MBF_HYDROB93 164
/* NGDC hydrographic soundings, binary */

#define MBF_MBARIROV 165
/* MBARI ROV navigation format, ascii, MBARI */

#define MBF_MBPRONAV 166
/* MB-System simple navigation format, ascii, MBARI */

#define MBF_NVNETCDF 167
/* CARAIBES CDF navigation, netCDF, IFREMER */

#define MBF_ASCIIXYT 168
/* XYT (lon lat topography) soundings, ascii, generic */

#define MBF_ASCIIYXT 169
/* YXT (lat lon topography) soundings, ascii, generic */

#define MBF_MBARROV2 170
/* MBARI ROV navigation format, ascii, MBARI */

#define MBF_HS10JAMS 171
/* Furuno HS10 multibeam format, 45 beams,                                     \
             bathymetry and amplitude, ascii, JAMSTEC */

#define MBF_HIR2RNAV 172
/* SIO GDC R2R navigation format, ascii, navigation, SIO */

#define MBF_MGD77TXT 173
/* NGDC MGD77 underway geophysics format,  \
single beam bathymetry, nav, magnetics, gravity, \
122 byte ascii records with CRLF line breaks, NOAA NGDC */

#define MBF_MGD77TAB 174
/* NGDC MGD77T underway geophysics format, \
single beam bathymetry, nav, magnetics, gravity, \
tab delimited ascii records with CRLF line breaks, NOAA NGDC */

#define MBF_SOIUSBLN 175
/* NGDC MGD77T underway geophysics format, \
single beam bathymetry, nav, magnetics, gravity, \
tab delimited ascii records with CRLF line breaks, NOAA NGDC */

#define MBF_SOIROVNV 176
/* NGDC MGD77T underway geophysics format, \
single beam bathymetry, nav, magnetics, gravity, \
tab delimited ascii records with CRLF line breaks, NOAA NGDC */

#define MBF_SAMESURF 181
/* STN Atlas processing multibeam format,  \
Hydrosweep DS2, Hydrosweep MD,  \
Fansweep 10, Fansweep 20,  \
bathymetry, amplitude, and sidescan, \
up to 1440 beams and 4096 pixels, \
XDR binary, STN Atlas. */

#define MBF_HSDS2RAW 182
/* STN Atlas raw multibeam format, \
Hydrosweep DS2, Hydrosweep MD,  \
Fansweep 10, Fansweep 20,  \
bathymetry, amplitude, and sidescan, \
up to 1440 beams and 4096 pixels, \
XDR binary, STN Atlas. */

#define MBF_HSDS2LAM 183
/* L-DEO HSDS2 processing format,  \
STN Atlas multibeam sonars,  \
Hydrosweep DS2, Hydrosweep MD,  \
Fansweep 10, Fansweep 20,  \
bathymetry, amplitude, and sidescan, \
up to 1440 beams and 4096 pixels, \
XDR binary, L-DEO. */

#define MBF_IMAGE83P 191
/* Imagenex 83p vendor format for DeltaT multibeam \
480 beams bathymetry */

#define MBF_IMAGEMBA 192
/* MBARI processing format for DeltaT multibeam  \
480 beams bathymetry */

#define MBF_HYSWEEP1 201
/* HYSWEEP format for multibeam data \
variable beams,  bathymetry, amplitude, and sidescan,  \
ascii text, single files, Hypack. */

#define MBF_XTFB1624 211
/* XTF format Benthos Sidescan SIS1624 \
variable pixels, dual frequency sidescan and subbottom,                             \
xtf variant, single files, \
low frequency sidescan returned as survey data, Benthos. */

#define MBF_SWPLSSXI 221
/* SEA intermediate format for SWATHplus interferometric sonar,                                 \
variable beams, bathymetry, amplitude, \
binary, single files, SEA. */

#define MBF_SWPLSSXP 222
/* SEA processed format for SWATHplus interferometric sonar,                                    \
variable beams, bathymetry, amplitude, \
binary, single files, SEA. */

#define MBF_3DDEPTHP 231
/* 3DatDepth processed format for 3DatDepth LIDAR, \
variable beams, bathymetry, amplitude, \
binary, single files, 3DatDepth. */

#define MBF_3DWISSLR 232
/* 3D at Depth vendor format for 3D at Depth WiSSL
(Wide Swath Subsea Lidar, \
variable beams, bathymetry, amplitude, \
binary, single files, 3D at Depth. */

#define MBF_3DWISSLP 233
/* 3D at Depth processed format for 3D at Depth WiSSL
(Wide Swath Subsea Lidar, \
variable beams, bathymetry, amplitude, \
binary, single files, MBARI. */

#define MBF_3DWISSL2 234
/* 3D at Depth vendor format for 3D at Depth WiSSL2
(Wide Swath Subsea Lidar, \
variable beams, bathymetry, amplitude, \
binary, single files, 3D at Depth. */

#define MBF_WASSPENL 241
/* WASSP Multibeam Vendor Format,  \
WASSP multibeams, \
bathymetry and amplitude,  \
122 or 244 beams, binary, Electronic Navigation Ltd. */

#define MBF_PHOTGRAM 251
/* Stereo Photogrammetry format, \
stereo camera rigs, \
bathymetry,  \
variable soundings, binary, MBARI. */

#define MBF_KEMKMALL 261
/* Kongsberg multibeam echosounder system kmall datagram format, \
Kongsberg fourth generation multibeam sonars (EM2040, EM712, EM304, EM124), \
variable beams, bathymetry, amplitude, \
binary. */

/* format registration function prototypes */
int mbr_register_sbsiomrg(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbsiocen(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbsiolsi(int verbose, void *mbio_ptr, int *error);
int mbr_register_sburicen(int verbose, void *mbio_ptr, int *error);
int mbr_register_sburivax(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbsioswb(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbifremr(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsatlraw(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsldedmb(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsuricen(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsldeoih(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsurivax(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsunknwn(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2000rw(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2000sb(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2000ss(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2100rw(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2100b1(int verbose, void *mbio_ptr, int *error);
int mbr_register_sb2100b2(int verbose, void *mbio_ptr, int *error);
int mbr_register_emoldraw(int verbose, void *mbio_ptr, int *error);
int mbr_register_em12ifrm(int verbose, void *mbio_ptr, int *error);
int mbr_register_em12darw(int verbose, void *mbio_ptr, int *error);
int mbr_register_em300raw(int verbose, void *mbio_ptr, int *error);
int mbr_register_em300mba(int verbose, void *mbio_ptr, int *error);
int mbr_register_em710raw(int verbose, void *mbio_ptr, int *error);
int mbr_register_em710mba(int verbose, void *mbio_ptr, int *error);
int mbr_register_mr1prhig(int verbose, void *mbio_ptr, int *error);
int mbr_register_mr1aldeo(int verbose, void *mbio_ptr, int *error);
int mbr_register_mr1bldeo(int verbose, void *mbio_ptr, int *error);
int mbr_register_mr1prvr2(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbldeoih(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbarimb1(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbnetcdf(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbncdfxt(int verbose, void *mbio_ptr, int *error);
int mbr_register_cbat9001(int verbose, void *mbio_ptr, int *error);
int mbr_register_cbat8101(int verbose, void *mbio_ptr, int *error);
int mbr_register_hypc8101(int verbose, void *mbio_ptr, int *error);
int mbr_register_xtfr8101(int verbose, void *mbio_ptr, int *error);
int mbr_register_resons8k(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbatproc(int verbose, void *mbio_ptr, int *error);
int mbr_register_reson7kr(int verbose, void *mbio_ptr, int *error);
int mbr_register_reson7k3(int verbose, void *mbio_ptr, int *error);
int mbr_register_bchrtunb(int verbose, void *mbio_ptr, int *error);
int mbr_register_elmk2unb(int verbose, void *mbio_ptr, int *error);
int mbr_register_bchrxunb(int verbose, void *mbio_ptr, int *error);
int mbr_register_l3xseraw(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsmdaraw(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsmdldih(int verbose, void *mbio_ptr, int *error);
int mbr_register_dsl120pf(int verbose, void *mbio_ptr, int *error);
int mbr_register_dsl120sf(int verbose, void *mbio_ptr, int *error);
int mbr_register_gsfgenmb(int verbose, void *mbio_ptr, int *error);
int mbr_register_mstiffss(int verbose, void *mbio_ptr, int *error);
int mbr_register_edgjstar(int verbose, void *mbio_ptr, int *error);
int mbr_register_edgjstr2(int verbose, void *mbio_ptr, int *error);
int mbr_register_oicgeoda(int verbose, void *mbio_ptr, int *error);
int mbr_register_oicmbari(int verbose, void *mbio_ptr, int *error);
int mbr_register_omghdcsj(int verbose, void *mbio_ptr, int *error);
int mbr_register_segysegy(int verbose, void *mbio_ptr, int *error);
int mbr_register_mgd77dat(int verbose, void *mbio_ptr, int *error);
int mbr_register_asciixyz(int verbose, void *mbio_ptr, int *error);
int mbr_register_asciiyxz(int verbose, void *mbio_ptr, int *error);
int mbr_register_hydrob93(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbarirov(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbpronav(int verbose, void *mbio_ptr, int *error);
int mbr_register_nvnetcdf(int verbose, void *mbio_ptr, int *error);
int mbr_register_asciixyt(int verbose, void *mbio_ptr, int *error);
int mbr_register_asciiyxt(int verbose, void *mbio_ptr, int *error);
int mbr_register_mbarrov2(int verbose, void *mbio_ptr, int *error);
int mbr_register_hs10jams(int verbose, void *mbio_ptr, int *error);
int mbr_register_hir2rnav(int verbose, void *mbio_ptr, int *error);
int mbr_register_mgd77txt(int verbose, void *mbio_ptr, int *error);
int mbr_register_mgd77tab(int verbose, void *mbio_ptr, int *error);
int mbr_register_soiusbln(int verbose, void *mbio_ptr, int *error);
int mbr_register_soirovnv(int verbose, void *mbio_ptr, int *error);
int mbr_register_samesurf(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsds2raw(int verbose, void *mbio_ptr, int *error);
int mbr_register_hsds2lam(int verbose, void *mbio_ptr, int *error);
int mbr_register_image83p(int verbose, void *mbio_ptr, int *error);
int mbr_register_imagemba(int verbose, void *mbio_ptr, int *error);
int mbr_register_hysweep1(int verbose, void *mbio_ptr, int *error);
int mbr_register_xtfb1624(int verbose, void *mbio_ptr, int *error);
int mbr_register_swplssxi(int verbose, void *mbio_ptr, int *error);
int mbr_register_swplssxp(int verbose, void *mbio_ptr, int *error);
int mbr_register_3ddepthp(int verbose, void *mbio_ptr, int *error);
int mbr_register_3dwisslr(int verbose, void *mbio_ptr, int *error);
int mbr_register_3dwisslp(int verbose, void *mbio_ptr, int *error);
int mbr_register_3dwissl2(int verbose, void *mbio_ptr, int *error);
int mbr_register_wasspenl(int verbose, void *mbio_ptr, int *error);
int mbr_register_photgram(int verbose, void *mbio_ptr, int *error);
int mbr_register_kemkmall(int verbose, void *mbio_ptr, int *error);
int mbr_info_sbsiomrg(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbsiomrg(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbsiocen(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbsiolsi(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sburicen(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sburivax(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbsioswb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbifremr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsatlraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsldedmb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsuricen(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsldeoih(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsurivax(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsunknwn(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2000rw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2000sb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2000ss(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2100rw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2100b1(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sb2100b2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_emoldraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em12ifrm(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em12darw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em300raw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em300mba(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em710raw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_em710mba(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mr1prhig(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mr1aldeo(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mr1bldeo(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mr1prvr2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbldeoih(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbarimb1(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbnetcdf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbncdfxt(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_cbat9001(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_cbat8101(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hypc8101(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_xtfr8101(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_resons8k(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_sbatproc(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_reson7kr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_reson7k3(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_bchrtunb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_elmk2unb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_bchrxunb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_l3xseraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsmdaraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsmdldih(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_dsl120pf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_dsl120sf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_gsfgenmb(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mstiffss(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_edgjstar(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_edgjstr2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_oicgeoda(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_oicmbari(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_omghdcsj(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_segysegy(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mgd77dat(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_asciixyz(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_asciiyxz(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hydrob93(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbarirov(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbpronav(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_nvnetcdf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_asciixyt(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_asciiyxt(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mbarrov2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hs10jams(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hir2rnav(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mgd77txt(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_mgd77tab(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_soiusbln(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_soirovnv(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_samesurf(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsds2raw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hsds2lam(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_image83p(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_imagemba(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_hysweep1(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_xtfb1624(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_swplssxi(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_swplssxp(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_3ddepthp(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_3dwisslr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_3dwisslp(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_3dwissl2(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_wasspenl(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_photgram(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_info_kemkmall(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);

#endif  /* MB_FORMAT_H_ */
