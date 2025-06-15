/*--------------------------------------------------------------------
 *    The MB-system:  mbr_3dwissl2.c  2/11/93
 *
 *    Copyright (c) 1993-2024 by
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
/*
 * mbr_3dwissl2.c contains the functions for reading and writing
 * multibeam data in the MBF_3DWISSLP format.
 * These functions include:
 *   mbr_alm_3dwissl2  - allocate read/write memory
 *   mbr_dem_3dwissl2  - deallocate read/write memory
 *   mbr_rt_3dwissl2  - read and translate data
 *   mbr_wt_3dwissl2  - translate and write data
 *
 * Author:  D. W. Caress
 * Date:  December 27, 2013
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_3ddwissl2.h"

/*#define MBF_3DWISSLP_DEBUG 1 */

/*--------------------------------------------------------------------*/
int mbr_info_3dwissl2
(
  int verbose,
  int *system,
  int *beams_bath_max,
  int *beams_amp_max,
  int *pixels_ss_max,
  char *format_name,
  char *system_name,
  char *format_description,
  int *numfile,
  int *filetype,
  int *variable_beams,
  int *traveltime,
  int *beam_flagging,
  int *platform_source,
  int *nav_source,
  int *sensordepth_source,
  int *heading_source,
  int *attitude_source,
  int *svp_source,
  double *beamwidth_xtrack,
  double *beamwidth_ltrack,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* set format info parameters */
  *error = MB_ERROR_NO_ERROR;
  *system = MB_SYS_3DDWISSL1;
  *beams_bath_max = 0;
  *beams_amp_max = 0;
  *pixels_ss_max = 0;
  strncpy(format_name, "3DWISSL2", MB_NAME_LENGTH);
  strncpy(system_name, "3DDWISSL2", MB_NAME_LENGTH);
  strncpy(format_description,
    "Format name:          MBF_3DWISSL2\nInformal Description: 3D at Depth " "Second Generation Wide Swath Subsea Lidar (WiSSL2) SRIAT format\n" "           Attributes: 3D at Depth lidar, variable pulses, bathymetry and amplitude, \n" "                      binary, MBARI.\n",
    MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_NORMAL;
  *variable_beams = true;
  *traveltime = false;
  *beam_flagging = true;
  *platform_source = MB_DATA_NONE;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_DATA;
  *svp_source = MB_DATA_NONE;
  *beamwidth_xtrack = 0.02;
  *beamwidth_ltrack = 0.02;

  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", *system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
    fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
    }

  return status;
} /* mbr_info_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_alm_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mbsys_3ddwissl2_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set file header read flag */
  int *file_header_readwritten = (int *)&mb_io_ptr->save1;
  *file_header_readwritten = MB_NO;

  /* set saved bytes flag */
  mb_io_ptr->save2 = false;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_alm_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_dem_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  /* deallocate reading/writing buffer */
  if ((mb_io_ptr->data_structure_size > 0) && (mb_io_ptr->raw_data != NULL))
    {
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
    mb_io_ptr->raw_data = NULL;
    mb_io_ptr->data_structure_size = 0;
    }

  /* deallocate memory  */
  status = mbsys_3ddwissl2_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_dem_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_3dwissl2_rd_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  
  int status = MB_SUCCESS;

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* get saved values */
  int *file_header_readwritten = (int *)&mb_io_ptr->save1;

  /* set file position */
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  /* set status */
  *error = MB_ERROR_NO_ERROR;
  bool done = false;

  /* allocate read buffer to at least handle the file header if needed */
  if (mb_io_ptr->data_structure_size < (size_t)(SRIAT_RECORD_SIZE_FILEHEADER))
	{
	if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
						(size_t)SRIAT_RECORD_SIZE_FILEHEADER, 
						(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
	  {
	  mb_io_ptr->data_structure_size = (size_t)SRIAT_RECORD_SIZE_FILEHEADER;
	  }
	}
  char *buffer = mb_io_ptr->raw_data;

  /* if first time through read the fileheader, which is returned as a parameter record */
  if (status == MB_SUCCESS && !(*file_header_readwritten))
  	{
  
	/* read file header and check the first few bytes */
	if (status == MB_SUCCESS)
	  {
	  size_t read_len = (size_t)(SRIAT_RECORD_SIZE_FILEHEADER);
	  status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
	  }
  
	if (status == MB_SUCCESS)
	  {
	  mbsys_3ddwissl2_sriat_fileheader_struct *fileheader = &store->fileheader;
	  
	  int index = 0;
	  fileheader->PacketID = buffer[index]; index++;
	  fileheader->Version = buffer[index]; index++;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->SizeBytes)); index += 2;
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->ScanSizeBytes)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeStart_Sec)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeStart_nSec)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeEnd_Sec)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->TimeEnd_nSec)); index += 4;
	  fileheader->SL_GEN = buffer[index]; index++;
	  fileheader->SL_Letter = buffer[index]; index++;
	  fileheader->SL_X = buffer[index]; index++;
	  fileheader->nPtsToAverage = buffer[index]; index++;
	  memcpy(fileheader->cJobName, &(buffer[index]), (size_t)24); index += 24;
	  memcpy(fileheader->cScanPos, &(buffer[index]), (size_t)24); index += 24;
	  memcpy(fileheader->cfileTag, &(buffer[index]), (size_t)24); index += 24;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->nScanNum)); index += 2;
  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit1)); index += 4;
	  fileheader->nPtsPerScanLine = fileheader->rawbit1 & 0x3FFF;
	  fileheader->AzCmdStart = fileheader->rawbit1 >> 14;
	  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit2)); index += 4;
	  fileheader->AzCmdEnd = fileheader->rawbit2 & 0x3FFFF;
	  fileheader->nScanLinesPerScan = (fileheader->rawbit2 >> 18) & 0xFFF;
	  fileheader->Spare1 = fileheader->rawbit2 >> 30;
	  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit3)); index += 4;
	  fileheader->nPtsPerLine = fileheader->rawbit3 & 0x3FFF;
	  fileheader->Mode = (fileheader->rawbit3 >> 14) & 0x7;
	  fileheader->nTPtsPerScanLine = (fileheader->rawbit3 >> 17) & 0x3FFF;
	  fileheader->Spare2 = fileheader->rawbit3 >> 31;
	  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->ShotCnt)); index += 4;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->WaterSalinity_psu)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->WaterPressure_dbar)); index += 2;
	  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit4)); index += 4;
	  fileheader->WaterTemperature_C = fileheader->rawbit4 & 0x1FFF;
	  fileheader->PRF_Hz = fileheader->rawbit4 >> 13;
  
	  fileheader->DigitizerTemperature_C = buffer[index]; index++;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->RScale_m_per_cnt)); index += 4;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ThBinStart_cnt)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ThBinEnd_cnts)); index += 2;
	  fileheader->TempAzCnt = buffer[index]; index++;
	  fileheader->TempRowCnt = buffer[index]; index++;
  
	  mb_get_binary_int(true, (void *)&buffer[index], &(fileheader->rawbit5)); index += 4;
	  fileheader->TempRCnt_av2 = fileheader->rawbit5 & 0xFF;
	  fileheader->TempRCnt_av4 = (fileheader->rawbit3 >> 8) & 0xFF;
	  fileheader->TempRCnt_av8 = (fileheader->rawbit3 >> 16) & 0xFF;
	  fileheader->TempRCnt_av16 = fileheader->rawbit5 >> 24;
  
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ScannerShift_mDeg)); index += 2;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[0])); index += 4;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[1])); index += 4;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Shift_m[2])); index += 4;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[0])); index += 4;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[1])); index += 4;
	  mb_get_binary_float(true, (void *)&buffer[index], &(fileheader->Rotate_deg[2])); index += 4;
	  memcpy(fileheader->EC_Version, &(buffer[index]), (size_t)4); index += 4;
	  memcpy(fileheader->InstaCloud_Version, &(buffer[index]), (size_t)4); index += 4;
	  mb_get_binary_short(true, (void *)&buffer[index], &(fileheader->ElDeg_cnts)); index += 2;
  fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
  
	  if (verbose >= 0) {
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       fileheader->PacketID:                    %u\n", fileheader->PacketID);
		fprintf(stderr, "dbg5       fileheader->Version:                     %u\n", fileheader->Version);
		fprintf(stderr, "dbg5       fileheader->SizeBytes:                   %u\n", fileheader->SizeBytes);
		fprintf(stderr, "dbg5       fileheader->ScanSizeBytes:               %u\n", fileheader->ScanSizeBytes);
		fprintf(stderr, "dbg5       fileheader->TimeStart_Sec:               %d\n", fileheader->TimeStart_Sec);
		fprintf(stderr, "dbg5       fileheader->TimeStart_nSec:              %d\n", fileheader->TimeStart_nSec);
		fprintf(stderr, "dbg5       fileheader->TimeEnd_Sec:                 %d\n", fileheader->TimeEnd_Sec);
		fprintf(stderr, "dbg5       fileheader->TimeEnd_nSec:                %d\n", fileheader->TimeEnd_nSec);
		fprintf(stderr, "dbg5       fileheader->SL_GEN:                      %u\n", fileheader->SL_GEN);
		fprintf(stderr, "dbg5       fileheader->SL_Letter:                   %u\n", fileheader->SL_Letter);
		fprintf(stderr, "dbg5       fileheader->SL_X:                        %u\n", fileheader->SL_X);
		fprintf(stderr, "dbg5       fileheader->nPtsToAverage:               %u\n", fileheader->nPtsToAverage);
		fprintf(stderr, "dbg5       fileheader->cJobName:                    %s\n", fileheader->cJobName);
		fprintf(stderr, "dbg5       fileheader->cScanPos:                    %s\n", fileheader->cScanPos);
		fprintf(stderr, "dbg5       fileheader->cfileTag:                    %s\n", fileheader->cfileTag);
		fprintf(stderr, "dbg5       fileheader->nScanNum:                    %u\n", fileheader->nScanNum);
		fprintf(stderr, "dbg5       fileheader->rawbit1:                     %u\n", fileheader->rawbit1);
		fprintf(stderr, "dbg5       -fileheader->nPtsPerScanLine:             %u\n", fileheader->nPtsPerScanLine);
		fprintf(stderr, "dbg5       -fileheader->AzCmdStart:                  %u\n", fileheader->AzCmdStart);
		fprintf(stderr, "dbg5       fileheader->rawbit2:                     %u\n", fileheader->rawbit2);
		fprintf(stderr, "dbg5       -fileheader->AzCmdEnd:                    %u\n", fileheader->AzCmdEnd);
		fprintf(stderr, "dbg5       -fileheader->nScanLinesPerScan:           %u\n", fileheader->nScanLinesPerScan);
		fprintf(stderr, "dbg5       -fileheader->Spare1:                      %u\n", fileheader->Spare1);
		fprintf(stderr, "dbg5       fileheader->rawbit3:                     %u\n", fileheader->rawbit3);
		fprintf(stderr, "dbg5       -fileheader->nPtsPerLine:                 %u\n", fileheader->nPtsPerLine);
		fprintf(stderr, "dbg5       -fileheader->Mode:                        %u\n", fileheader->Mode);
		fprintf(stderr, "dbg5       -fileheader->nTPtsPerScanLine:            %u\n", fileheader->nTPtsPerScanLine);
		fprintf(stderr, "dbg5       -fileheader->Spare2:                      %u\n", fileheader->Spare2);
		fprintf(stderr, "dbg5       fileheader->ShotCnt:                     %u\n", fileheader->ShotCnt);
		fprintf(stderr, "dbg5       fileheader->WaterSalinity_psu:           %u  %.3f\n", fileheader->WaterSalinity_psu, fileheader->WaterSalinity_psu * 42.0 / 65535.0 - 2.0);
		fprintf(stderr, "dbg5       fileheader->WaterPressure_dbar:          %u\n", fileheader->WaterPressure_dbar);
		fprintf(stderr, "dbg5       fileheader->rawbit4:                     %u\n", fileheader->rawbit4);
		fprintf(stderr, "dbg5       -fileheader->WaterTemperature_C:          %u  %.3f\n", fileheader->WaterTemperature_C, fileheader->WaterTemperature_C * 37.0 / 8191.0 - 2.0);
		fprintf(stderr, "dbg5       -fileheader->PRF_Hz:                      %u\n", fileheader->PRF_Hz);
		fprintf(stderr, "dbg5       fileheader->DigitizerTemperature_C:      %u  %.3f\n", fileheader->DigitizerTemperature_C, fileheader->DigitizerTemperature_C * 100.0 / 255.0);
		fprintf(stderr, "dbg5       fileheader->RScale_m_per_cnt:            %f\n", fileheader->RScale_m_per_cnt);
		fprintf(stderr, "dbg5       fileheader->ThBinStart_cnt:              %u\n", fileheader->ThBinStart_cnt);
		fprintf(stderr, "dbg5       fileheader->ThBinEnd_cnts:               %u\n", fileheader->ThBinEnd_cnts);
		fprintf(stderr, "dbg5       fileheader->TempAzCnt:                   %u\n", fileheader->TempAzCnt);
		fprintf(stderr, "dbg5       fileheader->TempRowCnt:                  %u\n", fileheader->TempRowCnt);
		fprintf(stderr, "dbg5       fileheader->rawbit5:                     %u\n", fileheader->rawbit5);
		fprintf(stderr, "dbg5       -fileheader->TempRCnt_av2:                %u\n", fileheader->TempRCnt_av2);
		fprintf(stderr, "dbg5       -fileheader->TempRCnt_av4:                %u\n", fileheader->TempRCnt_av4);
		fprintf(stderr, "dbg5       -fileheader->TempRCnt_av8:                %u\n", fileheader->TempRCnt_av8);
		fprintf(stderr, "dbg5       -fileheader->TempRCnt_av16:               %u\n", fileheader->TempRCnt_av16);
		fprintf(stderr, "dbg5       fileheader->ScannerShift_mDeg:           %u\n", fileheader->ScannerShift_mDeg);
		fprintf(stderr, "dbg5       fileheader->Shift_m[0]:                  %f\n", fileheader->Shift_m[0]);
		fprintf(stderr, "dbg5       fileheader->Shift_m[1]:                  %f\n", fileheader->Shift_m[1]);
		fprintf(stderr, "dbg5       fileheader->Shift_m[2]:                  %f\n", fileheader->Shift_m[2]);
		fprintf(stderr, "dbg5       fileheader->Rotate_deg[0]:               %f\n", fileheader->Rotate_deg[0]);
		fprintf(stderr, "dbg5       fileheader->Rotate_deg[1]:               %f\n", fileheader->Rotate_deg[1]);
		fprintf(stderr, "dbg5       fileheader->Rotate_deg[2]:               %f\n", fileheader->Rotate_deg[2]);
		fprintf(stderr, "dbg5       fileheader->EC_Version:                  %d.%d.%d.%d\n", 
						fileheader->EC_Version[0], fileheader->EC_Version[1], 
						fileheader->EC_Version[2], fileheader->EC_Version[3]);
		fprintf(stderr, "dbg5       fileheader->InstaCloud_Version:          %d.%d.%d.%d\n", 
						fileheader->InstaCloud_Version[0], fileheader->InstaCloud_Version[1], 
						fileheader->InstaCloud_Version[2], fileheader->InstaCloud_Version[3]);
		fprintf(stderr, "dbg5       fileheader->ElDeg_cnts:                  %d  %.3f\n", fileheader->ElDeg_cnts, fileheader->ElDeg_cnts * 90.0 / 65535.0);
	  }
	  
	fileheader->SizeBytes = SRIAT_RECORD_SIZE_FILEHEADER;
	*file_header_readwritten = MB_YES;
	status = MB_SUCCESS;
	store->kind = MB_DATA_PARAMETER;
	done = true;
  	}
  }
  
  /* read other records */
  else if (status == MB_SUCCESS) 
    {
    /* read the next four bytes and check record type, record version, and record size */
    unsigned char Packet_ID;
    unsigned char Version;
    unsigned short SizeBytes;
    size_t read_len = 4;
	if ((status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error)) != MB_SUCCESS)
	  {
	  done = MB_YES;
	  store->kind = MB_DATA_NONE;
	  }
	else
	  {
	  Packet_ID = buffer[0];
	  Version = buffer[1];
	  mb_get_binary_short(true, (void *)&buffer[2], &SizeBytes);
	  }
	  
	/* read a raw range record followed by a raw temperature record */
	if (status == MB_SUCCESS && Packet_ID == SRIAT_RECORD_ID_RANGE && Version == 1 && SizeBytes == SRIAT_RECORD_SIZE_RANGE_HEADER)
	  {
	  size_t read_len = SizeBytes - 4;
	  status = mb_fileio_get(verbose, mbio_ptr, &buffer[4], &read_len, error);

	  mbsys_3ddwissl2_sriatrange_struct *sriatrange = &store->sriatrange;

	  int index = 0;
	  sriatrange->PacketID = buffer[index]; index++;
	  sriatrange->Version = buffer[index]; index++;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->SizeBytes)); index += 2;
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->DataSizeBytes)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->TimeStart_Sec)); index += 4;
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->TimeStart_nSec)); index += 4;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->NumPtsRow)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->NumPtsPkt)); index += 2;
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->LineLaserPower)); index += 4;
  
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit1)); index += 4;
	  sriatrange->PRF_Hz = sriatrange->rawbit1 & 0x7FFFF;
	  sriatrange->Spare1 = (sriatrange->rawbit1 >> 19) & 0x7F;
	  sriatrange->Points_per_LOS = (sriatrange->rawbit1 >> 26) & 0x3;
	  sriatrange->ScannerType = sriatrange->rawbit1 >> 28;
	  
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->lineAccelX)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->lineAccelY)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->lineAccelZ)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->lineIndex)); index += 2;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->RowNumber)); index += 2;
  
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit2)); index += 4;
	  sriatrange->R_Max = sriatrange->rawbit2 & 0xFFFFF;
	  sriatrange->I_Max = sriatrange->rawbit2 >> 20;
 
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit3)); index += 4;
	  sriatrange->R_Auto = sriatrange->rawbit3 & 0xFFFFF;
	  sriatrange->I_Auto = sriatrange->rawbit3 >> 20;
 
	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit4)); index += 4;
	  sriatrange->R_Mode = sriatrange->rawbit4 & 0xFFFFF;
	  sriatrange->I_Mode = sriatrange->rawbit4 >> 20;

	  sriatrange->I_Good = (unsigned char) buffer[index]; index++;
	  sriatrange->I_Low = (unsigned char) buffer[index]; index++;
	  sriatrange->I_High = (unsigned char) buffer[index]; index++;
	  mb_get_binary_short(true, (void *)&buffer[index], &(sriatrange->SHGAmplitudeAv)); index += 2;

	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit5)); index += 4;
	  sriatrange->R_offset = sriatrange->rawbit5 & 0xFFFFF;
	  sriatrange->I_offset = sriatrange->rawbit5 >> 20;

	  mb_get_binary_int(true, (void *)&buffer[index], &(sriatrange->rawbit6)); index += 4;
	  sriatrange->AZ_offset = sriatrange->rawbit1 & 0x3FFFF;
	  sriatrange->R_nbits = (sriatrange->rawbit1 >> 18) & 0x1F;
	  sriatrange->I_nbits = (sriatrange->rawbit1 >> 23) & 0xF;
	  sriatrange->AZ_nbits = sriatrange->rawbit1 >> 27;

fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
  
	  if (verbose >= 0) 
		{
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sriatrange->PacketID:                    %u\n", sriatrange->PacketID);
		fprintf(stderr, "dbg5       sriatrange->Version:                     %u\n", sriatrange->Version);
		fprintf(stderr, "dbg5       sriatrange->SizeBytes:                   %u\n", sriatrange->SizeBytes);
		fprintf(stderr, "dbg5       sriatrange->DataSizeBytes:               %u\n", sriatrange->DataSizeBytes);
		fprintf(stderr, "dbg5       sriatrange->TimeStart_Sec:               %d\n", sriatrange->TimeStart_Sec);
		fprintf(stderr, "dbg5       sriatrange->TimeStart_nSec:              %d\n", sriatrange->TimeStart_nSec);
		fprintf(stderr, "dbg5       sriatrange->NumPtsRow:                   %u\n", sriatrange->NumPtsRow);
		fprintf(stderr, "dbg5       sriatrange->NumPtsPkt:                   %d\n", sriatrange->NumPtsPkt);
		fprintf(stderr, "dbg5       sriatrange->LineLaserPower:              %u  %.2f\n", sriatrange->LineLaserPower, 100.0 * sriatrange->LineLaserPower / 1048576.0);
		fprintf(stderr, "dbg5       sriatrange->rawbit1:                     %u\n", sriatrange->rawbit1);
		fprintf(stderr, "dbg5       -sriatrange->PRF_Hz:                      %u\n", sriatrange->PRF_Hz);
		fprintf(stderr, "dbg5       -sriatrange->Spare1:                      %u\n", sriatrange->Spare1);
		fprintf(stderr, "dbg5       -sriatrange->Points_per_LOS:              %u\n", sriatrange->Points_per_LOS);
		fprintf(stderr, "dbg5       -sriatrange->ScannerType:                 %u\n", sriatrange->ScannerType);
		fprintf(stderr, "dbg5       sriatrange->lineAccelX:                  %d\n", sriatrange->lineAccelX);
		fprintf(stderr, "dbg5       sriatrange->lineAccelY:                  %d\n", sriatrange->lineAccelY);
		fprintf(stderr, "dbg5       sriatrange->lineAccelZ:                  %d\n", sriatrange->lineAccelZ);
		fprintf(stderr, "dbg5       sriatrange->lineIndex:                   %u\n", sriatrange->lineIndex);
		fprintf(stderr, "dbg5       sriatrange->RowNumber:                   %u\n", sriatrange->RowNumber);
		fprintf(stderr, "dbg5       sriatrange->rawbit2:                     %u\n", sriatrange->rawbit2);
		fprintf(stderr, "dbg5       -sriatrange->R_Max:                       %u\n", sriatrange->R_Max);
		fprintf(stderr, "dbg5       -sriatrange->I_Max:                       %u\n", sriatrange->I_Max);
		fprintf(stderr, "dbg5       sriatrange->rawbit3:                     %u\n", sriatrange->rawbit3);
		fprintf(stderr, "dbg5       -sriatrange->R_Auto:                      %u\n", sriatrange->R_Auto);
		fprintf(stderr, "dbg5       -sriatrange->I_Auto:                      %u\n", sriatrange->I_Auto);
		fprintf(stderr, "dbg5       sriatrange->rawbit4:                     %u\n", sriatrange->rawbit4);
		fprintf(stderr, "dbg5       -sriatrange->R_Mode:                      %u\n", sriatrange->R_Mode);
		fprintf(stderr, "dbg5       -sriatrange->I_Mode:                      %u\n", sriatrange->I_Mode);
		fprintf(stderr, "dbg5       sriatrange->I_Good:                      %u\n", sriatrange->I_Good);
		fprintf(stderr, "dbg5       sriatrange->I_Low:                       %u\n", sriatrange->I_Good);
		fprintf(stderr, "dbg5       sriatrange->I_High:                      %u\n", sriatrange->I_Good);
		fprintf(stderr, "dbg5       sriatrange->SHGAmplitudeAv:              %u\n", sriatrange->SHGAmplitudeAv);
		fprintf(stderr, "dbg5       sriatrange->rawbit5:                     %u\n", sriatrange->rawbit5);
		fprintf(stderr, "dbg5       -sriatrange->R_offset:                    %u\n", sriatrange->R_offset);
		fprintf(stderr, "dbg5       -sriatrange->I_offset:                    %u\n", sriatrange->I_offset);
		fprintf(stderr, "dbg5       sriatrange->rawbit6:                     %u\n", sriatrange->rawbit6);
		fprintf(stderr, "dbg5       -sriatrange->AZ_offset:                   %u\n", sriatrange->AZ_offset);
		fprintf(stderr, "dbg5       -sriatrange->R_nbits:                     %u\n", sriatrange->R_nbits);
		fprintf(stderr, "dbg5       -sriatrange->I_nbits:                     %u\n", sriatrange->I_nbits);
		fprintf(stderr, "dbg5       -sriatrange->AZ_nbits:                    %u\n", sriatrange->AZ_nbits);
		}
		
	  /* read and parse the data section of the record */
	  
	  /* fix some problems */
	  if (sriatrange->R_nbits == 0)
	  	sriatrange->R_nbits = 20;
	  if (sriatrange->I_nbits == 0)
	  	sriatrange->I_nbits = 12;
	  if (sriatrange->R_nbits == 0)
	  	sriatrange->AZ_nbits = 18;
	
	  /* if needed allocate read buffer to handle the entire data section
		  and also arrays to hole the unpacked data arrays */
	  if (mb_io_ptr->data_structure_size < (size_t)(sriatrange->DataSizeBytes))
		{
		if ((status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)sriatrange->DataSizeBytes, 
							(void **)(&mb_io_ptr->raw_data), error)) == MB_SUCCESS)
		  {
		  mb_io_ptr->data_structure_size = (size_t)sriatrange->DataSizeBytes;
		  }
		else 
		  {
		  done = MB_YES;
		  store->kind = MB_DATA_NONE;
		  }
		}
	  char *buffer = mb_io_ptr->raw_data;
	  if (status == MB_SUCCESS && sriatrange->sriat_num_samples_alloc < (size_t)(sriatrange->NumPtsPkt))
		{
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_Az), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_Range1), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_Range2), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_Intensity1), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_Intensity2), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_ClassR1), error);
		if (status == MB_SUCCESS)
		status = mb_reallocd(verbose, __FILE__, __LINE__, 
							(size_t)(sriatrange->NumPtsPkt * sizeof(unsigned int)), 
							(void **)(&sriatrange->sriat_ClassR2), error);
		if (status == MB_SUCCESS)
		  {
		  sriatrange->sriat_num_samples_alloc = (size_t)sriatrange->NumPtsPkt;
		  }
		else 
		  {
		  done = MB_YES;
		  store->kind = MB_DATA_NONE;
		  int freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_Az), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_Range1), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_Range2), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_Intensity1), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_Intensity2), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_ClassR1), error);
		  freestatus = mb_freed(verbose, __FILE__, __LINE__, 
							(void **)(&sriatrange->sriat_ClassR2), error);
		  }
		}

	  read_len = (size_t)(sriatrange->DataSizeBytes);
	  if ((status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error)) != MB_SUCCESS)
		{
		done = MB_YES;
		store->kind = MB_DATA_NONE;
		}
	  else 
		{
		/* parse out the lidar pulse values from the buffer, which contains bit packed 
		   arrays of the form:
		   
		   
		    */
		char *bpbuffer = NULL;
		void *bitpackedarrayptr = NULL;
		unsigned int bpbuffer_size = 0;
		index = 0;
		
		bitpackedarrayptr = mb_bitpack_new();
		mb_bitpack_setbitsize(bitpackedarrayptr, sriatrange->AZ_nbits);
		status = mb_bitpack_resize(bitpackedarrayptr, sriatrange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
		memcpy(bpbuffer, &buffer[index], bpbuffer_size);
long long *lptr = (long long *) bpbuffer;
fprintf(stderr, "input buffer: %llx\n", *lptr);
for (int i=0;i<16;i++) {
unsigned char c = bpbuffer[i];
int mask = 0x01;
for (int j=0;j<8;j++) {
int k = mask & c;
fprintf(stderr, "%d", k);
c = c>>1;
}
fprintf(stderr, " ");
}
fprintf(stderr, "\n");
		for (int ipulse = 0; ipulse < sriatrange->NumPtsPkt; ipulse++) 
		  {
		  unsigned int i_az;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_az);
		  sriatrange->sriat_Az[ipulse] = i_az;
		  }
		mb_bitpack_delete(&bitpackedarrayptr);
	    index = +bpbuffer_size;
fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
		
		bitpackedarrayptr = mb_bitpack_new();
		mb_bitpack_setbitsize(bitpackedarrayptr, sriatrange->R_nbits);
		status = mb_bitpack_resize(bitpackedarrayptr, 2 * sriatrange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
		memcpy(bpbuffer, &buffer[index], bpbuffer_size);
		for (int ipulse = 0; ipulse < sriatrange->NumPtsPkt; ipulse++) 
		  {
		  unsigned int i_r;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_r);
		  sriatrange->sriat_Range1[ipulse] = i_r;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_r);
		  sriatrange->sriat_Range2[ipulse] = i_r;
		  }
		mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
		
		bitpackedarrayptr = mb_bitpack_new();
		mb_bitpack_setbitsize(bitpackedarrayptr, sriatrange->R_nbits);
		status = mb_bitpack_resize(bitpackedarrayptr, 2 * sriatrange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
		memcpy(bpbuffer, &buffer[index], bpbuffer_size);
		for (int ipulse = 0; ipulse < sriatrange->NumPtsPkt; ipulse++) 
		  {
		  unsigned int i_i;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_i);
		  sriatrange->sriat_Intensity1[ipulse] = i_i;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_i);
		  sriatrange->sriat_Intensity2[ipulse] = i_i;
		  }
		mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
		
		bitpackedarrayptr = mb_bitpack_new();
		mb_bitpack_setbitsize(bitpackedarrayptr, 4);
		status = mb_bitpack_resize(bitpackedarrayptr, 2 * sriatrange->NumPtsPkt, &bpbuffer, &bpbuffer_size);
		memcpy(bpbuffer, &buffer[index], bpbuffer_size);
		for (int ipulse = 0; ipulse < sriatrange->NumPtsPkt; ipulse++) 
		  {
		  unsigned int i_c;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_c);
		  sriatrange->sriat_ClassR1[ipulse] = i_c;
		  status = mb_bitpack_readvalue(bitpackedarrayptr, &i_c);
		  sriatrange->sriat_ClassR2[ipulse] = i_c;
		  }
		mb_bitpack_delete(&bitpackedarrayptr);
	    index += bpbuffer_size;
fprintf(stderr, "%s:%d:%s: index:%d\n", __FILE__, __LINE__, __FUNCTION__, index);
	  }
  
	  if (verbose >= 0) 
		{
		fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
		fprintf(stderr, "dbg5       sriatrange->NumPtsPkt:                   %d\n", sriatrange->NumPtsPkt);
		fprintf(stderr, "dbg5       ipulse  Azimuth  Range1  Range2  Intensity1  Intensity2  Class1  Class2\n");
		for (int ipulse=0; ipulse < sriatrange->NumPtsPkt; ipulse++) 
		  {
		  fprintf(stderr, "dbg5       ipulse:%d  %u %f %u %u %u %u %u %u\n", 
				ipulse, sriatrange->sriat_Az[ipulse], (sriatrange->AZ_offset + sriatrange->sriat_Az[ipulse]) * 360.0 / 0x3FFFF, 
				sriatrange->sriat_Range1[ipulse], sriatrange->sriat_Range2[ipulse], 
				sriatrange->sriat_Intensity1[ipulse], sriatrange->sriat_Intensity2[ipulse], 
				sriatrange->sriat_ClassR1[ipulse], sriatrange->sriat_ClassR2[ipulse]);
		  }
		}
    }
  }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
} /* mbr_3dwissl2_rd_data */
/*--------------------------------------------------------------------*/
int mbr_rt_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* get pointers to mbio descriptor and data structure */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl2_struct *store = (struct mbsys_3ddwissl2_struct *)store_ptr;

  /* read next data from file */
  const int status = mbr_3dwissl2_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* set error and kind in mb_io_ptr */
  mb_io_ptr->new_error = *error;
  mb_io_ptr->new_kind = store->kind;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_rt_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_3dwissl2_wr_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  int status = MB_SUCCESS;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_3dwissl2_wr_data */
/*--------------------------------------------------------------------*/
int mbr_wt_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* write next data to file */
  const int status = mbr_3dwissl2_wr_data(verbose, mbio_ptr, store_ptr, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_wt_3dwissl2 */
/*--------------------------------------------------------------------*/
int mbr_register_3dwissl2
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* check for non-null structure pointers */
  assert(mbio_ptr != NULL);

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_3dwissl2(verbose,
    &mb_io_ptr->system,
    &mb_io_ptr->beams_bath_max,
    &mb_io_ptr->beams_amp_max,
    &mb_io_ptr->pixels_ss_max,
    mb_io_ptr->format_name,
    mb_io_ptr->system_name,
    mb_io_ptr->format_description,
    &mb_io_ptr->numfile,
    &mb_io_ptr->filetype,
    &mb_io_ptr->variable_beams,
    &mb_io_ptr->traveltime,
    &mb_io_ptr->beam_flagging,
    &mb_io_ptr->platform_source,
    &mb_io_ptr->nav_source,
    &mb_io_ptr->sensordepth_source,
    &mb_io_ptr->heading_source,
    &mb_io_ptr->attitude_source,
    &mb_io_ptr->svp_source,
    &mb_io_ptr->beamwidth_xtrack,
    &mb_io_ptr->beamwidth_ltrack,
    error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_3dwissl2;
  mb_io_ptr->mb_io_format_free = &mbr_dem_3dwissl2;
  mb_io_ptr->mb_io_store_alloc = &mbsys_3ddwissl2_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_3ddwissl2_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_3dwissl2;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_3dwissl2;
  mb_io_ptr->mb_io_dimensions = &mbsys_3ddwissl2_dimensions;
  mb_io_ptr->mb_io_preprocess = &mbsys_3ddwissl2_preprocess;
  mb_io_ptr->mb_io_sensorhead = &mbsys_3ddwissl2_sensorhead;
  mb_io_ptr->mb_io_extract = &mbsys_3ddwissl2_extract;
  mb_io_ptr->mb_io_insert = &mbsys_3ddwissl2_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_3ddwissl2_extract_nav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_3ddwissl2_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_3ddwissl2_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_3ddwissl2_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_3ddwissl2_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_3ddwissl2_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_3ddwissl2_detects;
  mb_io_ptr->mb_io_copyrecord = &mbsys_3ddwissl2_copy;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
    fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
    fprintf(stderr, "dbg2       format_alloc:       %p\n",
      (void *)mb_io_ptr->mb_io_format_alloc);
    fprintf(stderr, "dbg2       format_free:        %p\n",
      (void *)mb_io_ptr->mb_io_format_free);
    fprintf(stderr, "dbg2       store_alloc:        %p\n",
      (void *)mb_io_ptr->mb_io_store_alloc);
    fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
    fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
    fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
    fprintf(stderr, "dbg2       preprocess:         %p\n", (void *)mb_io_ptr->mb_io_preprocess);
    fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
    fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
    fprintf(stderr, "dbg2       extract_nav:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_nav);
    fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
    fprintf(stderr,
      "dbg2       extract_altitude:   %p\n",
      (void *)mb_io_ptr->mb_io_extract_altitude);
    fprintf(stderr,
      "dbg2       insert_altitude:    %p\n",
      (void *)mb_io_ptr->mb_io_insert_altitude);
    fprintf(stderr, "dbg2       extract_svp:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_svp);
    fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
    fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
    fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
    fprintf(stderr,
      "dbg2       extract_rawss:      %p\n",
      (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n",
      (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
    }

  return status;
} /* mbr_register_3dwissl2 */
/*--------------------------------------------------------------------*/
