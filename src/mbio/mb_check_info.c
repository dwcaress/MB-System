/*--------------------------------------------------------------------
 *    The MB-system:	mb_check_info.c	1/25/93
 *
 *    Copyright (c) 1993-2023 by
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
 * mb_check_info.c checks if a file has data within the specified
 * bounds by reading the mbinfo output of that file. The mbinfo
 * output must be in an ascii file with a name consisting of the
 * data file name followed by a ".inf" suffix. If the ".inf" file
 * does not exist then the file is assumed to have data within the
 * specified bounds.
 *
 * Author:	D. W. Caress
 * Date:	September 3, 1996
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_check_info(int verbose, char *file, int lonflip, double bounds[4], bool *file_in_bounds, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       lonflip:    %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:  %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:  %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:  %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:  %f\n", bounds[3]);
	}

	/* cannot check bounds if input is stdin */
	int status;
	const char *stdin_string = "stdin";
	if (strncmp(file, stdin_string, 5) == 0) {
		*file_in_bounds = true;

		if (verbose >= 4) {
			fprintf(stderr, "dbg4  Cannot check bounds if input is stdin...\n");
		}
	}

	/* check for inf file */
	else {
		/* get info file path */
		char file_inf[MB_PATH_MAXLINE];
		strcpy(file_inf, file);
		strcat(file_inf, ".inf");

		/* open if possible */
		FILE *fp = fopen(file_inf, "r");
		if (fp != NULL) {
			/* initialize the parameters */
			int nrecords = -1;
			double lon_min = 0.0;
			double lon_max = 0.0;
			double lat_min = 0.0;
			double lat_max = 0.0;
			int mask_nx = 0;
			int mask_ny = 0;
		  int *mask = NULL;

			/* read the inf file */
			char line[MB_PATH_MAXLINE];
			while (fgets(line, MB_PATH_MAXLINE, fp) != NULL) {
				if (strncmp(line, "Number of Records:", 18) == 0) {
					int nrecords_read;
					const int nscan = sscanf(line, "Number of Records: %d", &nrecords_read);
					if (nscan == 1)
						nrecords = nrecords_read;
				}
				else if (strncmp(line, "Minimum Longitude:", 18) == 0)
					sscanf(line, "Minimum Longitude: %lf Maximum Longitude: %lf", &lon_min, &lon_max);
				else if (strncmp(line, "Minimum Latitude:", 17) == 0)
					sscanf(line, "Minimum Latitude: %lf Maximum Latitude: %lf", &lat_min, &lat_max);
				else if (strncmp(line, "CM dimensions:", 14) == 0) {
					sscanf(line, "CM dimensions: %d %d", &mask_nx, &mask_ny);
					status = mb_mallocd(verbose, __FILE__, __LINE__, mask_nx * mask_ny * sizeof(int), (void **)&mask, error);
					for (int j = mask_ny - 1; j >= 0; j--) {
						char *startptr = NULL;
						if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
							startptr = &line[6];
							for (int i = 0; i < mask_nx; i++) {
								int k = i + j * mask_nx;
								char *endptr = NULL;
								mask[k] = strtol(startptr, &endptr, 0);
								startptr = endptr;
							}
						}
					}
				}
			}

			/* check bounds if there is data */
			if (nrecords > 0) {
				/* set lon lat min max according to lonflip */
				if (lonflip == -1 && lon_min > 0.0) {
					lon_min -= 360.0;
					lon_max -= 360.0;
				}
				else if (lonflip == 0 && lon_max < -180.0) {
					lon_min += 360.0;
					lon_max += 360.0;
				}
				else if (lonflip == 0 && lon_min > 180.0) {
					lon_min -= 360.0;
					lon_max -= 360.0;
				}
				else if (lonflip == 1 && lon_max < 0.0) {
					lon_min += 360.0;
					lon_max += 360.0;
				}

				/* check for lonflip conflict with bounds */
				if (lon_min > lon_max || lat_min > lat_max)
					*file_in_bounds = true;

				/* else check mask against desired input bounds */
				else if (mask_nx > 0 && mask_ny > 0) {
					*file_in_bounds = false;
					const double mask_dx = (lon_max - lon_min) / mask_nx;
					const double mask_dy = (lat_max - lat_min) / mask_ny;
					for (int i = 0; i < mask_nx && !*file_in_bounds; i++)
						for (int j = 0; j < mask_ny && !*file_in_bounds; j++) {
							int k = i + j * mask_nx;
							const double lonwest = lon_min + i * mask_dx;
							const double loneast = lonwest + mask_dx;
							const double latsouth = lat_min + j * mask_dy;
							const double latnorth = latsouth + mask_dy;
							if (mask[k] == 1 && lonwest < bounds[1] && loneast > bounds[0] && latsouth < bounds[3] &&
							    latnorth > bounds[2])
								*file_in_bounds = true;
						}
				}

				/* else check whole file against desired input bounds */
				else {
					if (lon_min < bounds[1] && lon_max > bounds[0] && lat_min < bounds[3] && lat_max > bounds[2])
						*file_in_bounds = true;
					else
						*file_in_bounds = false;
				}

				if (verbose >= 4) {
					fprintf(stderr, "dbg4  Bounds from inf file:\n");
					fprintf(stderr, "dbg4      lon_min: %f\n", lon_min);
					fprintf(stderr, "dbg4      lon_max: %f\n", lon_max);
					fprintf(stderr, "dbg4      lat_min: %f\n", lat_min);
					fprintf(stderr, "dbg4      lat_max: %f\n", lat_max);
				}
			}

			/* else if no data records in inf file
			    treat file as out of bounds */
			else if (nrecords == 0) {
				*file_in_bounds = false;

				if (verbose >= 4)
					fprintf(stderr, "dbg4  The inf file shows zero records so out of bounds...\n");
			}

			/* else if no data assume inf file is botched so
			assume file has data in bounds */
			else {
				*file_in_bounds = true;

				if (verbose >= 4)
					fprintf(stderr, "dbg4  No data listed in inf file so cannot check bounds...\n");
			}

    /* free the mask array */
    if (mask_nx > 0 && mask_ny > 0 && mask != NULL) {
      	mb_freed(verbose, __FILE__, __LINE__, (void **)&mask, error);
    }

			/* close the file */
			fclose(fp);
		}

		/* if no inf file assume file has data in bounds */
		else {
			*file_in_bounds = true;

			if (verbose >= 4)
				fprintf(stderr, "dbg4  Cannot open inf file so cannot check bounds...\n");
		}
	}

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file_in_bounds: %d\n", *file_in_bounds);
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_info(int verbose, char *file, struct mb_info_struct *mb_info, int lonflip, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       info:       %p\n", (void *)mb_info);
		fprintf(stderr, "dbg2       lonflip:    %d\n", lonflip);
	}

	/* initialize the parameters */
	mb_info_init(verbose, mb_info, error);

	/* get info file path */
	char file_inf[MB_PATH_MAXLINE];
	strcpy(file_inf, file);
	strcat(file_inf, ".inf");

	/* open if possible */
	int status = MB_SUCCESS;
	FILE *fp = fopen(file_inf, "r");
	if (fp == NULL) {
		/* set error */
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;

		if (verbose >= 2) {
			fprintf(stderr, "dbg2  Cannot open requested inf file: %s\n", file_inf);
		}

  	if (verbose >= 2) {
  		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
  		fprintf(stderr, "dbg2  Return values:\n");
  		fprintf(stderr, "dbg2       loaded:                   %d\n", mb_info->loaded);
  		fprintf(stderr, "dbg2       error:                    %d\n", *error);
  		fprintf(stderr, "dbg2  Return status:\n");
  		fprintf(stderr, "dbg2       status:  %d\n", status);
  	}

		/* return in disgrace */
  	return (status);
	}

	/* load information from inf file */
	else {
		/* set file name */
		strcpy(mb_info->file, file);

		/* read the inf file */
		char line[MB_PATH_MAXLINE];
		while (fgets(line, MB_PATH_MAXLINE, fp) != NULL) {
			char *startptr;
			int nscan;
			int nproblem;
			int problemid;
			double speedkts;
			if (strncmp(line, "Number of Records:", 18) == 0) {
				nscan = sscanf(line, "Number of Records: %d", &mb_info->nrecords);
			}
			else if (strncmp(line, "Number of Subbottom Records:", 28) == 0) {
				nscan = sscanf(line, "Number of Subbottom Records: %d", &mb_info->nrecords_sbp);
			}
			else if (strncmp(line, "Number of Secondary Sidescan Records:", 37) == 0) {
				nscan = sscanf(line, "Number of Secondary Sidescan Records: %d", &mb_info->nrecords_ss1);
			}
			else if (strncmp(line, "Number of Tertiary Sidescan Records:", 36) == 0) {
				nscan = sscanf(line, "Number of Tertiary Sidescan Records: %d", &mb_info->nrecords_ss2);
			}

			else if (strncmp(line, "Bathymetry Data (", 17) == 0) {
				nscan = sscanf(line, "Bathymetry Data (%d beams):", &mb_info->nbeams_bath);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Beams: %d", &mb_info->nbeams_bath_total);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Good Beams: %d", &mb_info->nbeams_bath_good);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Zero Beams: %d", &mb_info->nbeams_bath_zero);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Flagged Beams: %d", &mb_info->nbeams_bath_flagged);
			}

			else if (strncmp(line, "Amplitude Data (", 16) == 0) {
				nscan = sscanf(line, "Amplitude Data (%d beams):", &mb_info->nbeams_amp);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Beams: %d", &mb_info->nbeams_amp_total);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Good Beams: %d", &mb_info->nbeams_amp_good);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Zero Beams: %d", &mb_info->nbeams_amp_zero);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Flagged Beams: %d", &mb_info->nbeams_amp_flagged);
			}

			else if (strncmp(line, "Sidescan Data (", 15) == 0) {
				nscan = sscanf(line, "Sidescan Data (%d pixels):", &mb_info->npixels_ss);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Pixels: %d", &mb_info->npixels_ss_total);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Good Pixels: %d", &mb_info->npixels_ss_good);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Zero Pixels: %d", &mb_info->npixels_ss_zero);
				if (fgets(line, MB_PATH_MAXLINE, fp) != NULL)
					nscan = sscanf(line, "  Number of Flagged Pixels: %d", &mb_info->npixels_ss_flagged);
			}

			else if (strncmp(line, "Total Time:", 11) == 0) {
				nscan = sscanf(line, "Total Time: %lf hours", &mb_info->time_total);
			}
			else if (strncmp(line, "Total Track Length:", 19) == 0) {
				nscan = sscanf(line, "Total Track Length: %lf km", &mb_info->dist_total);
			}
			else if (strncmp(line, "Average Speed:", 14) == 0) {
				nscan = sscanf(line, "Average Speed: %lf km/hr", &mb_info->speed_avg);
			}

			else if (strncmp(line, "Start of Data:", 14) == 0) {
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					int time_i[7];
					nscan = sscanf(line, "Time:  %d %d %d %d:%d:%d.%d  JD", &time_i[1], &time_i[2], &time_i[0], &time_i[3],
					               &time_i[4], &time_i[5], &time_i[6]);
					if (nscan == 7)
						mb_get_time(verbose, time_i, &(mb_info->time_start));
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Lon: %lf	 Lat: %lf Depth: %lf meters", &mb_info->lon_start, &mb_info->lat_start,
					               &mb_info->depth_start);
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Speed: %lf km/hr ( %lf knots)  Heading: %lf degrees", &mb_info->speed_start, &speedkts,
					               &mb_info->heading_start);
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Sonar Depth:  %lf m  Sonar Altitude:   %lf m", &mb_info->sensordepth_start,
					               &mb_info->sonaraltitude_start);
				}
			}

			else if (strncmp(line, "End of Data:", 12) == 0) {
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					int time_i[7];
					nscan = sscanf(line, "Time:  %d %d %d %d:%d:%d.%d  JD", &time_i[1], &time_i[2], &time_i[0], &time_i[3],
					               &time_i[4], &time_i[5], &time_i[6]);
					if (nscan == 7)
						mb_get_time(verbose, time_i, &(mb_info->time_end));
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Lon: %lf	 Lat: %lf Depth: %lf meters", &mb_info->lon_end, &mb_info->lat_end,
					               &mb_info->depth_end);
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Speed: %lf km/hr ( %lf knots)  Heading: %lf degrees", &mb_info->speed_end, &speedkts,
					               &mb_info->heading_end);
				}
				if ((startptr = fgets(line, MB_PATH_MAXLINE, fp)) != NULL) {
					nscan = sscanf(line, "Sonar Depth:  %lf m  Sonar Altitude:   %lf m", &mb_info->sensordepth_end,
					               &mb_info->sonaraltitude_end);
				}
			}

			else if (strncmp(line, "Minimum Longitude:", 18) == 0)
				sscanf(line, "Minimum Longitude: %lf Maximum Longitude: %lf", &mb_info->lon_min, &mb_info->lon_max);
			else if (strncmp(line, "Minimum Latitude:", 17) == 0)
				sscanf(line, "Minimum Latitude: %lf Maximum Latitude: %lf", &mb_info->lat_min, &mb_info->lat_max);

			else if (strncmp(line, "Minimum Sonar Depth:", 20) == 0)
				sscanf(line, "Minimum Sonar Depth: %lf Maximum Sonar Depth: %lf", &mb_info->sensordepth_min,
				       &mb_info->sensordepth_max);

			else if (strncmp(line, "Minimum Altitude:", 17) == 0)
				sscanf(line, "Minimum Altitude: %lf Maximum Altitude: %lf", &mb_info->altitude_min, &mb_info->altitude_max);

			else if (strncmp(line, "Minimum Depth:", 14) == 0)
				sscanf(line, "Minimum Depth: %lf Maximum Depth: %lf", &mb_info->depth_min, &mb_info->depth_max);

			else if (strncmp(line, "Minimum Amplitude:", 18) == 0)
				sscanf(line, "Minimum Amplitude: %lf Maximum Amplitude: %lf", &mb_info->amp_min, &mb_info->amp_max);

			else if (strncmp(line, "Minimum Sidescan:", 17) == 0)
				sscanf(line, "Minimum Sidescan: %lf Maximum Sidescan: %lf", &mb_info->ss_min, &mb_info->ss_max);

			else if (strncmp(line, "PN:", 3) == 0) {
				sscanf(line, "PN: %d DATA PROBLEM (ID=%d):", &nproblem, &problemid);
				if (problemid == MB_PROBLEM_NO_DATA)
					mb_info->problem_nodata += nproblem;
				else if (problemid == MB_PROBLEM_ZERO_NAV)
					mb_info->problem_zeronav += nproblem;
				else if (problemid == MB_PROBLEM_TOO_FAST)
					mb_info->problem_toofast += nproblem;
				else if (problemid == MB_PROBLEM_AVG_TOO_FAST)
					mb_info->problem_avgtoofast += nproblem;
				else if (problemid == MB_PROBLEM_TOO_DEEP)
					mb_info->problem_toodeep += nproblem;
				else if (problemid == MB_PROBLEM_BAD_DATAGRAM)
					mb_info->problem_baddatagram += nproblem;
			}

			else if (strncmp(line, "CM dimensions:", 14) == 0) {
				int mask_nx;
				int mask_ny;
				sscanf(line, "CM dimensions: %d %d", &mask_nx, &mask_ny);
				for (int j = 0; j < mask_ny; j++)
					startptr = fgets(line, MB_PATH_MAXLINE, fp);
			}
		}

		fclose(fp);

		/* apply lonflip if needed */
		if (lonflip == -1 && mb_info->lon_min > 0.0) {
			mb_info->lon_min -= 360.0;
			mb_info->lon_max -= 360.0;
			mb_info->lon_start -= 360.0;
			mb_info->lon_end -= 360.0;
		}
		else if (lonflip == 0 && mb_info->lon_max < -180.0) {
			mb_info->lon_min += 360.0;
			mb_info->lon_max += 360.0;
			mb_info->lon_start += 360.0;
			mb_info->lon_end += 360.0;
		}
		else if (lonflip == 0 && mb_info->lon_min > 180.0) {
			mb_info->lon_min -= 360.0;
			mb_info->lon_max -= 360.0;
			mb_info->lon_start -= 360.0;
			mb_info->lon_end -= 360.0;
		}
		else if (lonflip == 1 && mb_info->lon_max < 0.0) {
			mb_info->lon_min += 360.0;
			mb_info->lon_max += 360.0;
			mb_info->lon_start += 360.0;
			mb_info->lon_end += 360.0;
		}
	}

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       loaded:                   %d\n", mb_info->loaded);
		fprintf(stderr, "dbg2       file:                     %s\n", mb_info->file);
		fprintf(stderr, "dbg2       nrecords:                 %d\n", mb_info->nrecords);
		fprintf(stderr, "dbg2       nrecords_sbp:             %d\n", mb_info->nrecords_sbp);
		fprintf(stderr, "dbg2       nrecords_ss1:             %d\n", mb_info->nrecords_ss1);
		fprintf(stderr, "dbg2       nrecords_ss2:             %d\n", mb_info->nrecords_ss2);
		fprintf(stderr, "dbg2       nbeams_bath:              %d\n", mb_info->nbeams_bath);
		fprintf(stderr, "dbg2       nbeams_bath_total:        %d\n", mb_info->nbeams_bath_total);
		fprintf(stderr, "dbg2       nbeams_bath_good:         %d\n", mb_info->nbeams_bath_good);
		fprintf(stderr, "dbg2       nbeams_bath_zero:         %d\n", mb_info->nbeams_bath_zero);
		fprintf(stderr, "dbg2       nbeams_bath_flagged:      %d\n", mb_info->nbeams_bath_flagged);
		fprintf(stderr, "dbg2       nbeams_amp:               %d\n", mb_info->nbeams_amp);
		fprintf(stderr, "dbg2       nbeams_amp_total:         %d\n", mb_info->nbeams_amp_total);
		fprintf(stderr, "dbg2       nbeams_amp_good:          %d\n", mb_info->nbeams_amp_good);
		fprintf(stderr, "dbg2       nbeams_amp_zero:          %d\n", mb_info->nbeams_amp_zero);
		fprintf(stderr, "dbg2       nbeams_amp_flagged:       %d\n", mb_info->nbeams_amp_flagged);
		fprintf(stderr, "dbg2       npixels_ss:               %d\n", mb_info->npixels_ss);
		fprintf(stderr, "dbg2       npixels_ss_total:         %d\n", mb_info->npixels_ss_total);
		fprintf(stderr, "dbg2       npixels_ss_good:          %d\n", mb_info->npixels_ss_good);
		fprintf(stderr, "dbg2       npixels_ss_zero:          %d\n", mb_info->npixels_ss_zero);
		fprintf(stderr, "dbg2       npixels_ss_flagged:       %d\n", mb_info->npixels_ss_flagged);
		fprintf(stderr, "dbg2       time_total:               %f\n", mb_info->time_total);
		fprintf(stderr, "dbg2       dist_total:               %f\n", mb_info->dist_total);
		fprintf(stderr, "dbg2       speed_avg:                %f\n", mb_info->speed_avg);
		fprintf(stderr, "dbg2       time_start:               %f\n", mb_info->time_start);
		fprintf(stderr, "dbg2       lon_start:                %f\n", mb_info->lon_start);
		fprintf(stderr, "dbg2       lat_start:                %f\n", mb_info->lat_start);
		fprintf(stderr, "dbg2       depth_start:              %f\n", mb_info->depth_start);
		fprintf(stderr, "dbg2       heading_start:            %f\n", mb_info->heading_start);
		fprintf(stderr, "dbg2       speed_start:              %f\n", mb_info->speed_start);
		fprintf(stderr, "dbg2       sensordepth_start:         %f\n", mb_info->sensordepth_start);
		fprintf(stderr, "dbg2       sonaraltitude_start:      %f\n", mb_info->sonaraltitude_start);
		fprintf(stderr, "dbg2       time_end:                 %f\n", mb_info->time_end);
		fprintf(stderr, "dbg2       lon_end:                  %f\n", mb_info->lon_end);
		fprintf(stderr, "dbg2       lat_end:                  %f\n", mb_info->lat_end);
		fprintf(stderr, "dbg2       depth_end:                %f\n", mb_info->depth_end);
		fprintf(stderr, "dbg2       heading_end:              %f\n", mb_info->heading_end);
		fprintf(stderr, "dbg2       speed_end:                %f\n", mb_info->speed_end);
		fprintf(stderr, "dbg2       sensordepth_end:           %f\n", mb_info->sensordepth_end);
		fprintf(stderr, "dbg2       sonaraltitude_end:        %f\n", mb_info->sonaraltitude_end);
		fprintf(stderr, "dbg2       lon_min:                  %f\n", mb_info->lon_min);
		fprintf(stderr, "dbg2       lon_max:                  %f\n", mb_info->lon_max);
		fprintf(stderr, "dbg2       lat_min:                  %f\n", mb_info->lat_min);
		fprintf(stderr, "dbg2       lat_max:                  %f\n", mb_info->lat_max);
		fprintf(stderr, "dbg2       sensordepth_min:           %f\n", mb_info->sensordepth_min);
		fprintf(stderr, "dbg2       sensordepth_max:           %f\n", mb_info->sensordepth_max);
		fprintf(stderr, "dbg2       altitude_min:             %f\n", mb_info->altitude_min);
		fprintf(stderr, "dbg2       altitude_max:             %f\n", mb_info->altitude_max);
		fprintf(stderr, "dbg2       depth_min:                %f\n", mb_info->depth_min);
		fprintf(stderr, "dbg2       depth_max:                %f\n", mb_info->depth_max);
		fprintf(stderr, "dbg2       amp_min:                  %f\n", mb_info->amp_min);
		fprintf(stderr, "dbg2       amp_max:                  %f\n", mb_info->amp_max);
		fprintf(stderr, "dbg2       ss_min:                   %f\n", mb_info->ss_min);
		fprintf(stderr, "dbg2       ss_max:                   %f\n", mb_info->ss_max);
		fprintf(stderr, "dbg2       problem_nodata:           %d\n", mb_info->problem_nodata);
		fprintf(stderr, "dbg2       problem_zeronav:          %d\n", mb_info->problem_zeronav);
		fprintf(stderr, "dbg2       problem_toofast:          %d\n", mb_info->problem_toofast);
		fprintf(stderr, "dbg2       problem_avgtoofast:       %d\n", mb_info->problem_avgtoofast);
		fprintf(stderr, "dbg2       problem_toodeep:          %d\n", mb_info->problem_toodeep);
		fprintf(stderr, "dbg2       problem_baddatagram:      %d\n", mb_info->problem_baddatagram);
		fprintf(stderr, "dbg2       error:                    %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

bool mb_should_make_fbt(int verbose, int format) {
  bool result = true;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       format:     %d\n", format);
	}

  if (format <= 0
    || format == MBF_SBSIOMRG || format == MBF_SBSIOCEN || format == MBF_SBSIOLSI
    || format == MBF_SBURICEN || format == MBF_SBURIVAX || format == MBF_SBSIOSWB
    || format == MBF_HSLDEDMB || format == MBF_HSURICEN || format == MBF_HSURIVAX
    || format == MBF_SB2000SS || format == MBF_SB2000SB || format == MBF_MSTIFFSS
    || format == MBF_MBLDEOIH || format == MBF_MBNETCDF || format == MBF_ASCIIXYZ
    || format == MBF_ASCIIYXZ || format == MBF_ASCIIXYT || format == MBF_ASCIIYXT
    || format == MBF_HYDROB93 || format == MBF_SEGYSEGY || format == MBF_MGD77DAT
    || format == MBF_MBARIROV || format == MBF_MBARROV2 || format == MBF_MBPRONAV)
    result = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return result:\n");
		fprintf(stderr, "dbg2       result:     %d\n", result);
	}

	return (result);
}
/*--------------------------------------------------------------------*/

bool mb_should_make_fnv(int verbose, int format) {
  bool result = true;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       format:     %d\n", format);
	}

  if (format <= 0
    || format == MBF_ASCIIXYZ || format == MBF_ASCIIYXZ || format == MBF_ASCIIXYT
    || format == MBF_ASCIIYXT || format == MBF_HYDROB93 || format == MBF_SEGYSEGY
    || format == MBF_MGD77DAT || format == MBF_MBARIROV || format == MBF_MBARROV2
    || format == MBF_NVNETCDF || format == MBF_MBPRONAV)
    result = false;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return result:\n");
		fprintf(stderr, "dbg2       result:     %d\n", result);
	}

	return (result);
}
/*--------------------------------------------------------------------*/

int mb_make_info(int verbose, bool force, char *file, int format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       force:      %d\n", force);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", format);
	}

	/* check for existing ancillary files */
	char inffile[MB_PATH_MAXLINE];
	sprintf(inffile, "%s.inf", file);
	char fbtfile[MB_PATH_MAXLINE];
	sprintf(fbtfile, "%s.fbt", file);
	char fnvfile[MB_PATH_MAXLINE];
	sprintf(fnvfile, "%s.fnv", file);

	int fstat;
	struct stat file_status;
	int datmodtime = 0;
	if ((fstat = stat(file, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	int infmodtime = 0;
	if ((fstat = stat(inffile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR && file_status.st_size > 0) {
		infmodtime = file_status.st_mtime;
	}
	int fbtmodtime = 0;
	if ((fstat = stat(fbtfile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR && file_status.st_size > 0) {
		fbtmodtime = file_status.st_mtime;
	}
	int fnvmodtime = 0;
	if ((fstat = stat(fnvfile, &file_status)) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR && file_status.st_size > 0) {
		fnvmodtime = file_status.st_mtime;
	}

	int status = MB_SUCCESS;
	int shellstatus = 0;

	/* make new inf file if not there or out of date */
	if (force || (datmodtime > 0 && datmodtime > infmodtime)) {
		if (verbose >= 1)
			fprintf(stderr, "\nGenerating inf file for %s\n", file);
		char command[MB_PATH_MAXLINE];
		sprintf(command, "mbinfo -F %d -I %s -G -N -O -M10/10", format, file);
		if (verbose >= 2)
			fprintf(stderr, "\t%s\n", command);
		if ((shellstatus = system(command)) != 0)
      status = MB_FAILURE;
	}

	/* make new fbt file if not there or out of date */
	if ((force || (datmodtime > 0 && datmodtime > fbtmodtime)) && mb_should_make_fbt(verbose, format)) {
		if (verbose >= 1)
			fprintf(stderr, "Generating fbt file for %s\n", file);
		  char command[MB_PATH_MAXLINE];
		  sprintf(command, "mbcopy -F %d/71 -I %s -D -O %s.fbt", format, file, file);
		  if ((shellstatus = system(command)) != 0)
        status = MB_FAILURE;
	}

	/* make new fnv file if not there or out of date */
	if ((force || (datmodtime > 0 && datmodtime > fnvmodtime)) && mb_should_make_fnv(verbose, format)) {
		if (verbose >= 1)
			fprintf(stderr, "Generating fnv file for %s\n", file);
	  char command[MB_PATH_MAXLINE];
    sprintf(command,  "echo \"## <yyyy mm dd hh mm ss.ssssss> <epoch seconds> "
                      "<longitude (deg)> <latitude (deg)> <heading (deg)> <speed (km/hr)> "
                      "<draft (m)> <roll (deg)> <pitch (deg)> <heave (m)> <portlon (deg)> "
                      "<portlat (deg)> <stbdlon (deg)> <stbdlat (deg)>\"  > %s.fnv", file);
	  if ((shellstatus = system(command)) != 0)
      status = MB_FAILURE;
	  sprintf(command, "mblist -F %d -I %s -O tMXYHScRPr=X=Y+X+Y -UN >> %s.fnv", format, file, file);
	  if ((shellstatus = system(command)) != 0)
      status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_fbt(int verbose, char *file, int *format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
	}

	/* check for existing fbt file */
	char fbtfile[MB_PATH_MAXLINE];
	sprintf(fbtfile, "%s.fbt", file);
	int datmodtime = 0;
	struct stat file_status;
	int fstat = stat(file, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	int fbtmodtime = 0;
	fstat = stat(fbtfile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		fbtmodtime = file_status.st_mtime;
	}

	/* replace file with fbt file if fbt file exists */
	if (datmodtime > 0 && fbtmodtime > 0) {
		strcpy(file, fbtfile);
		*format = 71;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_fnv(int verbose, char *file, int *format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
	}

	/* check for existing fnv file */
	char fnvfile[MB_PATH_MAXLINE];
	sprintf(fnvfile, "%s.fnv", file);
	struct stat file_status;
	int datmodtime = 0;
	int fstat = stat(file, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	int fnvmodtime = 0;
	fstat = stat(fnvfile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		fnvmodtime = file_status.st_mtime;
	}

	/* replace file with fnv file if fnv file exists */
	if (datmodtime > 0 && fnvmodtime > 0) {
		strcpy(file, fnvfile);
		*format = 166;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_ffa(int verbose, char *file, int *format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
	}

	/* check for existing ffa file */
	char ffafile[MB_PATH_MAXLINE];
	sprintf(ffafile, "%s.ffa", file);
	struct stat file_status;
	int fstat = stat(file, &file_status);
	int datmodtime = 0;
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	fstat = stat(ffafile, &file_status);
	int ffamodtime = 0;
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		ffamodtime = file_status.st_mtime;
	}

	/* replace file with ffa file if ffa file exists */
	int status = MB_SUCCESS;
	if (datmodtime > 0 && ffamodtime > 0) {
		strcpy(file, ffafile);
		*format = 71;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_FILE_NOT_FOUND;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_ffs(int verbose, char *file, int *format, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
	}

	/* check for existing ffs file */
	char ffsfile[MB_PATH_MAXLINE];
	sprintf(ffsfile, "%s.ffs", file);
	int datmodtime = 0;
	struct stat file_status;
	int fstat = stat(file, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	int ffsmodtime = 0;
	fstat = stat(ffsfile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		ffsmodtime = file_status.st_mtime;
	}

	/* replace file with ffs file if ffs file exists */
	int status = MB_SUCCESS;
	if (datmodtime > 0 && ffsmodtime > 0) {
		strcpy(file, ffsfile);
		*format = 71;
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_FILE_NOT_FOUND;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       file:       %s\n", file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_swathbounds(int verbose, int checkgood, int nbath, int nss,
                    char *beamflag, double *bathacrosstrack,
                    double *ss, double *ssacrosstrack,
                    int *ibeamport, int *ibeamcntr, int *ibeamstbd,
                    int *ipixelport, int *ipixelcntr, int *ipixelstbd,
                    int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       checkgood:     %d\n", checkgood);
		fprintf(stderr, "dbg2       nbath:         %d\n", nbath);
		if (verbose >= 3 && nbath > 0) {
			fprintf(stderr, "dbg3       beam   flag   crosstrack\n");
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       %4d   %3d     %f\n", i, beamflag[i], bathacrosstrack[i]);
		}
		fprintf(stderr, "dbg2       nss:      %d\n", nss);
		if (verbose >= 3 && nss > 0) {
			fprintf(stderr, "dbg3       pixel sidescan crosstrack\n");
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3       %4d   %f    %f\n", i, ss[i], ssacrosstrack[i]);
		}
	}

	/* get min max of non-null beams */
	*ibeamport = nbath / 2;
	*ibeamcntr = *ibeamport;
	*ibeamstbd = *ibeamport;
  if (nbath > 0 && beamflag != NULL && bathacrosstrack != NULL) {
  	double xtrackmin = 0.0;
  	double xtrackmax = 0.0;
  	double distmin = 0.0;
  	bool found = false;
  	for (int i = 0; i < nbath; i++) {
  		if ((checkgood && mb_beam_ok(beamflag[i])) || !mb_beam_check_flag_unusable(beamflag[i])) {
  			if (!found) {
  				*ibeamport = i;
  				*ibeamcntr = i;
  				*ibeamstbd = i;
  				xtrackmin = bathacrosstrack[i];
  				distmin = fabs(bathacrosstrack[i]);
  				xtrackmax = bathacrosstrack[i];
  				found = true;
  			}
  			else {
  				if (fabs(bathacrosstrack[i]) < distmin) {
  					*ibeamcntr = i;
  					distmin = fabs(bathacrosstrack[i]);
  				}
  				if (bathacrosstrack[i] < xtrackmin) {
  					*ibeamport = i;
  					xtrackmin = bathacrosstrack[i];
  				}
  				else if (bathacrosstrack[i] > xtrackmax) {
  					*ibeamstbd = i;
  					xtrackmax = bathacrosstrack[i];
  				}
  			}
  		}
  	}
  }

	/* get min max of non-null pixels */
	*ipixelport = nss / 2;
	*ipixelcntr = *ipixelport;
	*ipixelstbd = *ipixelport;
  if (nss > 0 && ss != NULL && ssacrosstrack != NULL) {
  	double xtrackmin = 0.0;
  	double xtrackmax = 0.0;
  	double distmin = 0.0;
  	bool found = false;
  	for (int i = 0; i < nss; i++) {
  		if (ss[i] > 0.0) {
  			if (!found) {
  				*ipixelport = i;
  				*ipixelcntr = i;
  				*ipixelstbd = i;
  				xtrackmin = ssacrosstrack[i];
  				distmin = fabs(ssacrosstrack[i]);
  				xtrackmax = ssacrosstrack[i];
  				found = true;
  			}
  			else {
  				if (fabs(ssacrosstrack[i]) < distmin) {
  					*ipixelcntr = i;
  					distmin = fabs(ssacrosstrack[i]);
  				}
  				if (ssacrosstrack[i] < xtrackmin) {
  					*ipixelport = i;
  					xtrackmin = ssacrosstrack[i];
  				}
  				else if (ssacrosstrack[i] > xtrackmax) {
  					*ipixelstbd = i;
  					xtrackmax = ssacrosstrack[i];
  				}
  			}
  		}
  	}
  }

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       ibeamport:     %d\n", *ibeamport);
		fprintf(stderr, "dbg2       ibeamport:     %d\n", *ibeamcntr);
		fprintf(stderr, "dbg2       ibeamstbd:     %d\n", *ibeamstbd);
		fprintf(stderr, "dbg2       ipixelport:    %d\n", *ipixelport);
		fprintf(stderr, "dbg2       ipixelport:    %d\n", *ipixelcntr);
		fprintf(stderr, "dbg2       ipixelstbd:    %d\n", *ipixelstbd);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}

/*--------------------------------------------------------------------*/
int mb_info_init(int verbose, struct mb_info_struct *mb_info, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_info:    %p\n", (void *)mb_info);
	}

	/* initialize mb_info_struct */
	mb_info->loaded = false;
	mb_info->file[0] = '\0';

	mb_info->nrecords = 0;
	mb_info->nrecords_ss1 = 0;
	mb_info->nrecords_ss2 = 0;
	mb_info->nrecords_sbp = 0;
	mb_info->nbeams_bath = 0;
	mb_info->nbeams_bath_total = 0;
	mb_info->nbeams_bath_good = 0;
	mb_info->nbeams_bath_zero = 0;
	mb_info->nbeams_bath_flagged = 0;
	mb_info->nbeams_amp = 0;
	mb_info->nbeams_amp_total = 0;
	mb_info->nbeams_amp_good = 0;
	mb_info->nbeams_amp_zero = 0;
	mb_info->nbeams_amp_flagged = 0;
	mb_info->npixels_ss = 0;
	mb_info->npixels_ss_total = 0;
	mb_info->npixels_ss_good = 0;
	mb_info->npixels_ss_zero = 0;
	mb_info->npixels_ss_flagged = 0;

	mb_info->time_total = 0.0;
	mb_info->dist_total = 0.0;
	mb_info->speed_avg = 0.0;

	mb_info->time_start = 0.0;
	mb_info->lon_start = 0.0;
	mb_info->lat_start = 0.0;
	mb_info->depth_start = 0.0;
	mb_info->heading_start = 0.0;
	mb_info->speed_start = 0.0;
	mb_info->sensordepth_start = 0.0;
	mb_info->sonaraltitude_start = 0.0;

	mb_info->time_end = 0.0;
	mb_info->lon_end = 0.0;
	mb_info->lat_end = 0.0;
	mb_info->depth_end = 0.0;
	mb_info->heading_end = 0.0;
	mb_info->speed_end = 0.0;
	mb_info->sensordepth_end = 0.0;
	mb_info->sonaraltitude_end = 0.0;

	mb_info->lon_min = 0.0;
	mb_info->lon_max = 0.0;
	mb_info->lat_min = 0.0;
	mb_info->lat_max = 0.0;
	mb_info->sensordepth_min = 0.0;
	mb_info->sensordepth_max = 0.0;
	mb_info->altitude_min = 0.0;
	mb_info->altitude_max = 0.0;
	mb_info->depth_min = 0.0;
	mb_info->depth_max = 0.0;
	mb_info->amp_min = 0.0;
	mb_info->amp_max = 0.0;
	mb_info->ss_min = 0.0;
	mb_info->ss_max = 0.0;

	mb_info->problem_nodata = 0;
	mb_info->problem_zeronav = 0;
	mb_info->problem_toofast = 0;
	mb_info->problem_avgtoofast = 0;
	mb_info->problem_toodeep = 0;
	mb_info->problem_baddatagram = 0;

	// mb_info->mask_nx = 0;
	// mb_info->mask_ny = 0;
	// mb_info->mask_dx = 0;
	// mb_info->mask_dy = 0;
	// mb_info->mask_alloc = 0;
	// mb_info->mask = NULL;

	*error = MB_ERROR_NO_ERROR;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_get_info_datalist(int verbose, char *read_file, int *format, struct mb_info_struct *mb_info, int lonflip, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       read_file:  %s\n", read_file);
		fprintf(stderr, "dbg2       format:     %d\n", *format);
		fprintf(stderr, "dbg2       lonflip:    %d\n", lonflip);
	}

	/* initialize mb_info_struct */
	mb_info_init(verbose, mb_info, error);
	strcpy(mb_info->file, read_file);

	/* get format if required */
	if (*format == 0)
		mb_get_format(verbose, read_file, NULL, format, error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = *format < 0;

	/* open file list */
	char swathfile[MB_PATH_MAXLINE];
	void *datalist = NULL;
	bool read_data;
	char dfile[MB_PATH_MAXLINE];
	int status = MB_SUCCESS;
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, error)) != MB_SUCCESS) {
			*error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
			exit(status);
		}
		double file_weight;
		if ((status = mb_datalist_read(verbose, datalist, swathfile, dfile, format, &file_weight, error)) == MB_SUCCESS)
			read_data = true;
		else
			read_data = false;
	}
	/* else copy single filename to be read */
	else {
		strcpy(swathfile, read_file);
		read_data = true;
	}

	/* loop over all files to be read */
	int nfile = 0;
	while (read_data) {
		/* read inf file */
		struct mb_info_struct mb_info_file;
		status = mb_get_info(verbose, swathfile, &mb_info_file, lonflip, error);

		/* only use if there are data */
		if (mb_info_file.nrecords > 0) {

			/* add in the results */
			mb_info->nrecords += mb_info_file.nrecords;
			mb_info->nrecords_ss1 += mb_info_file.nrecords_ss1;
			mb_info->nrecords_ss2 += mb_info_file.nrecords_ss2;
			mb_info->nrecords_sbp += mb_info_file.nrecords_sbp;
			mb_info->nbeams_bath = MAX(mb_info->nbeams_bath, mb_info_file.nbeams_bath);
			mb_info->nbeams_bath_total += mb_info_file.nbeams_bath_total;
			mb_info->nbeams_bath_good += mb_info_file.nbeams_bath_good;
			mb_info->nbeams_bath_zero += mb_info_file.nbeams_bath_zero;
			mb_info->nbeams_bath_flagged += mb_info_file.nbeams_bath_flagged;
			mb_info->nbeams_amp = MAX(mb_info->nbeams_amp, mb_info_file.nbeams_amp);
			mb_info->nbeams_amp_total += mb_info_file.nbeams_amp_total;
			mb_info->nbeams_amp_good += mb_info_file.nbeams_amp_good;
			mb_info->nbeams_amp_zero += mb_info_file.nbeams_amp_zero;
			mb_info->nbeams_amp_flagged += mb_info_file.nbeams_amp_flagged;
			mb_info->npixels_ss = MAX(mb_info->npixels_ss, mb_info_file.npixels_ss);
			mb_info->npixels_ss_total += mb_info_file.npixels_ss_total;
			mb_info->npixels_ss_good += mb_info_file.npixels_ss_good;
			mb_info->npixels_ss_zero += mb_info_file.npixels_ss_zero;
			mb_info->npixels_ss_flagged += mb_info_file.npixels_ss_flagged;

			mb_info->time_total += mb_info_file.time_total;
			mb_info->dist_total += mb_info_file.dist_total;
			/* mb_info.speed_avg += mb_info_file.speed_avg;*/

			if (nfile == 0) {
				mb_info->time_start += mb_info_file.time_start;
				mb_info->lon_start += mb_info_file.lon_start;
				mb_info->lat_start += mb_info_file.lat_start;
				mb_info->depth_start += mb_info_file.depth_start;
				mb_info->heading_start += mb_info_file.heading_start;
				mb_info->speed_start += mb_info_file.speed_start;
				mb_info->sensordepth_start += mb_info_file.sensordepth_start;
				mb_info->sonaraltitude_start += mb_info_file.sonaraltitude_start;
			}

			mb_info->time_end += mb_info_file.time_end;
			mb_info->lon_end += mb_info_file.lon_end;
			mb_info->lat_end += mb_info_file.lat_end;
			mb_info->depth_end += mb_info_file.depth_end;
			mb_info->heading_end += mb_info_file.heading_end;
			mb_info->speed_end += mb_info_file.speed_end;
			mb_info->sensordepth_end += mb_info_file.sensordepth_end;
			mb_info->sonaraltitude_end += mb_info_file.sonaraltitude_end;

			if (nfile == 0) {
				mb_info->lon_min = mb_info_file.lon_min;
				mb_info->lon_max = mb_info_file.lon_max;
				mb_info->lat_min = mb_info_file.lat_min;
				mb_info->lat_max = mb_info_file.lat_max;
				mb_info->sensordepth_min = mb_info_file.sensordepth_min;
				mb_info->sensordepth_max = mb_info_file.sensordepth_max;
				mb_info->altitude_min = mb_info_file.altitude_min;
				mb_info->altitude_max = mb_info_file.altitude_max;
				mb_info->depth_min = mb_info_file.depth_min;
				mb_info->depth_max = mb_info_file.depth_max;
				mb_info->amp_min = mb_info_file.amp_min;
				mb_info->amp_max = mb_info_file.amp_max;
				mb_info->ss_min = mb_info_file.ss_min;
				mb_info->ss_max = mb_info_file.ss_max;
			}
			else {
				if (mb_info->lon_min == 0.0)
					mb_info->lon_min = mb_info_file.lon_min;
				else
					mb_info->lon_min = MIN(mb_info_file.lon_min, mb_info->lon_min);
				if (mb_info->lon_max == 0.0)
					mb_info->lon_max = mb_info_file.lon_max;
				else
					mb_info->lon_max = MAX(mb_info_file.lon_max, mb_info->lon_max);
				if (mb_info->lat_min == 0.0)
					mb_info->lat_min = mb_info_file.lat_min;
				else
					mb_info->lat_min = MIN(mb_info_file.lat_min, mb_info->lat_min);
				if (mb_info->lat_max == 0.0)
					mb_info->lat_max = mb_info_file.lat_max;
				else
					mb_info->lat_max = MAX(mb_info_file.lat_max, mb_info->lat_max);
				mb_info->sensordepth_min = MIN(mb_info_file.sensordepth_min, mb_info->sensordepth_min);
				mb_info->sensordepth_max = MAX(mb_info_file.sensordepth_max, mb_info->sensordepth_max);
				mb_info->altitude_min = MIN(mb_info_file.altitude_min, mb_info->altitude_min);
				mb_info->altitude_max = MAX(mb_info_file.altitude_max, mb_info->altitude_max);
				mb_info->depth_min = MIN(mb_info_file.depth_min, mb_info->depth_min);
				mb_info->depth_max = MAX(mb_info_file.depth_max, mb_info->depth_max);
				mb_info->amp_min = MIN(mb_info_file.amp_min, mb_info->amp_min);
				mb_info->amp_max = MAX(mb_info_file.amp_max, mb_info->amp_max);
				mb_info->ss_min = MIN(mb_info_file.ss_min, mb_info->ss_min);
				mb_info->ss_max = MAX(mb_info_file.ss_max, mb_info->ss_max);
			}

			mb_info->problem_nodata += mb_info_file.problem_nodata;
			mb_info->problem_zeronav += mb_info_file.problem_zeronav;
			mb_info->problem_toofast += mb_info_file.problem_toofast;
			mb_info->problem_avgtoofast += mb_info_file.problem_avgtoofast;
			mb_info->problem_toodeep += mb_info_file.problem_toodeep;
			mb_info->problem_baddatagram += mb_info_file.problem_baddatagram;

			nfile++;
		}

		/* check memory */
		if (verbose >= 4)
			status = mb_memory_list(verbose, error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			double file_weight;
			if ((status = mb_datalist_read(verbose, datalist, swathfile, dfile, format, &file_weight, error)) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, error);

	/* check memory */
	if (verbose >= 4)
		/* status = */ mb_memory_list(verbose, error);

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       loaded:                   %d\n", mb_info->loaded);
		fprintf(stderr, "dbg2       file:                     %s\n", mb_info->file);
		fprintf(stderr, "dbg2       nrecords:                 %d\n", mb_info->nrecords);
		fprintf(stderr, "dbg2       nrecords_sbp:             %d\n", mb_info->nrecords_sbp);
		fprintf(stderr, "dbg2       nrecords_ss1:             %d\n", mb_info->nrecords_ss1);
		fprintf(stderr, "dbg2       nrecords_ss2:             %d\n", mb_info->nrecords_ss2);
		fprintf(stderr, "dbg2       nbeams_bath:              %d\n", mb_info->nbeams_bath);
		fprintf(stderr, "dbg2       nbeams_bath_total:        %d\n", mb_info->nbeams_bath_total);
		fprintf(stderr, "dbg2       nbeams_bath_good:         %d\n", mb_info->nbeams_bath_good);
		fprintf(stderr, "dbg2       nbeams_bath_zero:         %d\n", mb_info->nbeams_bath_zero);
		fprintf(stderr, "dbg2       nbeams_bath_flagged:      %d\n", mb_info->nbeams_bath_flagged);
		fprintf(stderr, "dbg2       nbeams_amp:               %d\n", mb_info->nbeams_amp);
		fprintf(stderr, "dbg2       nbeams_amp_total:         %d\n", mb_info->nbeams_amp_total);
		fprintf(stderr, "dbg2       nbeams_amp_good:          %d\n", mb_info->nbeams_amp_good);
		fprintf(stderr, "dbg2       nbeams_amp_zero:          %d\n", mb_info->nbeams_amp_zero);
		fprintf(stderr, "dbg2       nbeams_amp_flagged:       %d\n", mb_info->nbeams_amp_flagged);
		fprintf(stderr, "dbg2       npixels_ss:               %d\n", mb_info->npixels_ss);
		fprintf(stderr, "dbg2       npixels_ss_total:         %d\n", mb_info->npixels_ss_total);
		fprintf(stderr, "dbg2       npixels_ss_good:          %d\n", mb_info->npixels_ss_good);
		fprintf(stderr, "dbg2       npixels_ss_zero:          %d\n", mb_info->npixels_ss_zero);
		fprintf(stderr, "dbg2       npixels_ss_flagged:       %d\n", mb_info->npixels_ss_flagged);
		fprintf(stderr, "dbg2       time_total:               %f\n", mb_info->time_total);
		fprintf(stderr, "dbg2       dist_total:               %f\n", mb_info->dist_total);
		fprintf(stderr, "dbg2       speed_avg:                %f\n", mb_info->speed_avg);
		fprintf(stderr, "dbg2       time_start:               %f\n", mb_info->time_start);
		fprintf(stderr, "dbg2       lon_start:                %f\n", mb_info->lon_start);
		fprintf(stderr, "dbg2       lat_start:                %f\n", mb_info->lat_start);
		fprintf(stderr, "dbg2       depth_start:              %f\n", mb_info->depth_start);
		fprintf(stderr, "dbg2       heading_start:            %f\n", mb_info->heading_start);
		fprintf(stderr, "dbg2       speed_start:              %f\n", mb_info->speed_start);
		fprintf(stderr, "dbg2       sensordepth_start:         %f\n", mb_info->sensordepth_start);
		fprintf(stderr, "dbg2       sonaraltitude_start:      %f\n", mb_info->sonaraltitude_start);
		fprintf(stderr, "dbg2       time_end:                 %f\n", mb_info->time_end);
		fprintf(stderr, "dbg2       lon_end:                  %f\n", mb_info->lon_end);
		fprintf(stderr, "dbg2       lat_end:                  %f\n", mb_info->lat_end);
		fprintf(stderr, "dbg2       depth_end:                %f\n", mb_info->depth_end);
		fprintf(stderr, "dbg2       heading_end:              %f\n", mb_info->heading_end);
		fprintf(stderr, "dbg2       speed_end:                %f\n", mb_info->speed_end);
		fprintf(stderr, "dbg2       sensordepth_end:           %f\n", mb_info->sensordepth_end);
		fprintf(stderr, "dbg2       sonaraltitude_end:        %f\n", mb_info->sonaraltitude_end);
		fprintf(stderr, "dbg2       lon_min:                  %f\n", mb_info->lon_min);
		fprintf(stderr, "dbg2       lon_max:                  %f\n", mb_info->lon_max);
		fprintf(stderr, "dbg2       lat_min:                  %f\n", mb_info->lat_min);
		fprintf(stderr, "dbg2       lat_max:                  %f\n", mb_info->lat_max);
		fprintf(stderr, "dbg2       sensordepth_min:           %f\n", mb_info->sensordepth_min);
		fprintf(stderr, "dbg2       sensordepth_max:           %f\n", mb_info->sensordepth_max);
		fprintf(stderr, "dbg2       altitude_min:             %f\n", mb_info->altitude_min);
		fprintf(stderr, "dbg2       altitude_max:             %f\n", mb_info->altitude_max);
		fprintf(stderr, "dbg2       depth_min:                %f\n", mb_info->depth_min);
		fprintf(stderr, "dbg2       depth_max:                %f\n", mb_info->depth_max);
		fprintf(stderr, "dbg2       amp_min:                  %f\n", mb_info->amp_min);
		fprintf(stderr, "dbg2       amp_max:                  %f\n", mb_info->amp_max);
		fprintf(stderr, "dbg2       ss_min:                   %f\n", mb_info->ss_min);
		fprintf(stderr, "dbg2       ss_max:                   %f\n", mb_info->ss_max);
		fprintf(stderr, "dbg2       problem_nodata:           %d\n", mb_info->problem_nodata);
		fprintf(stderr, "dbg2       problem_zeronav:          %d\n", mb_info->problem_zeronav);
		fprintf(stderr, "dbg2       problem_toofast:          %d\n", mb_info->problem_toofast);
		fprintf(stderr, "dbg2       problem_avgtoofast:       %d\n", mb_info->problem_avgtoofast);
		fprintf(stderr, "dbg2       problem_toodeep:          %d\n", mb_info->problem_toodeep);
		fprintf(stderr, "dbg2       problem_baddatagram:      %d\n", mb_info->problem_baddatagram);
		fprintf(stderr, "dbg2       error:          %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
