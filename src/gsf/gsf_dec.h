/********************************************************************
 *
 * Module Name : GSF_DEC
 *
 * Author/Date : J. S. Byrne / 26 May 1994
 *
 * Description :
 *
 * Restrictions/Limitations :
 *
 * Change Descriptions :
 * who  when      what
 * ---  ----      ----
 * hem  08/20/96  Added gsfDecodeSinglebeam.
 * jsb  09/28/98  Added gsfDecodeHVNavigationError.
 *
 * Classification : Unclassified
 *
 * References :
 *
 *
 * copyright 2014 Leidos, Inc.
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
 *
 ********************************************************************/

#ifndef _GSF_DEC_H_
   #define _GSF_DEC_H_

   #include "gsf.h"
   #include "gsf_ft.h"

   int OPTLK gsfDecodeHeader(gsfHeader *header, unsigned char *sptr);
   int OPTLK gsfDecodeSwathBathySummary(gsfSwathBathySummary *summ, unsigned char *sptr);
   int OPTLK gsfDecodeSwathBathymetryPing(gsfSwathBathyPing *ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size);
   int OPTLK gsfDecodeSoundVelocityProfile(gsfSVP *svp, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeProcessingParameters(gsfProcessingParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeSensorParameters(gsfSensorParameters *param, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeComment(gsfComment *comment, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeHistory(gsfHistory *history, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeNavigationError(gsfNavigationError *nav_error, unsigned char *sptr);
   int OPTLK gsfDecodeHVNavigationError(gsfHVNavigationError *hv_nav_error, GSF_FILE_TABLE *ft, unsigned char *sptr);
   int OPTLK gsfDecodeSinglebeam(gsfSingleBeamPing * ping, unsigned char *sptr, GSF_FILE_TABLE *ft, int handle, int record_size);
   int OPTLK gsfDecodeAttitude(gsfAttitude *attitude, GSF_FILE_TABLE *ft, unsigned char *sptr);

#endif
