/*--------------------------------------------------------------------
 *    The MB-system:    mbgetphotocorrection.cpp    10/17/2013
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
 * mbgetphotocorrection generates a 3D table of image correction values where
 * the variables range over lateral x and y (with respect to the camera image)
 * and standoff, which is z measured with respect to the camera.
 *
 * Author:    D. W. Caress
 * Date:    October 17, 2013
 * Date:    May 25, 2021 (multithreading version)
 *
 * Integrated into MB-System July 2020
 *
 */

/* standard include files */
#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <thread>

/* MB-System include files */
extern "C"
{
#include "mb_define.h"
#include "mb_status.h"
#include "mb_io.h"
#include "mb_aux.h"
}

/* OpenCV include files */
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

#define MBPM_USE_STEREO         1
#define MBPM_USE_LEFT           2
#define MBPM_USE_RIGHT          3
#define MBPM_CAMERA_LEFT        0
#define MBPM_CAMERA_RIGHT       1

#define MBPM_MIN_VALID_COUNT    250

char program_name[] = "mbgetphotocorrection";
char help_message[] =  "mbgetphotocorrection makes a mosaic of navigated downlooking photographs.";
char usage_message[] = "mbgetphotocorrection \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--threads=nthreads\n"
                            "\t--input=imagelist\n"
                            "\t--output=file  [--correction-file=file]\n"
                            "\t--correction-x-dimension=value\n"
                            "\t--correction-y-dimension=value\n"
                            "\t--correction-z-dimension=value\n"
                            "\t--correction-z-minmax=value/value\n"
                            "\t--fov-fudgefactor=factor\n"
                            "\t--projection=projection_pars\n"
                            "\t--trim=trim_pixels\n"
                            "\t--reference-gain=gain\n"
                            "\t--reference-exposure=exposure\n"
                            "\t--platform-file=platform.plf\n"
                            "\t--camera-sensor=camera_sensor_id\n"
                            "\t--nav-sensor=nav_sensor_id\n"
                            "\t--sensordepth-sensor=sensordepth_sensor_id\n"
                            "\t--heading-sensor=heading_sensor_id\n"
                            "\t--altitude-sensor=altitude_sensor_id\n"
                            "\t--attitude-sensor=attitude_sensor_id\n"
                            "\t--use-left-camera\n"
                            "\t--use-right-camera\n"
                            "\t--use-both-cameras\n"
                            "\t--calibration-file=stereocalibration.yaml\n"
                            "\t--navigation-file=file\n"
                            "\t--tide-file=file\n"
                            "\t--image-quality-file=file\n"
                            "\t--image-quality-threshold=value\n"
                            "\t--image-quality-filter-length=value\n"
                            "\t--topography-grid=file";

/*--------------------------------------------------------------------*/

struct mbpm_process_struct {

    // input image and camera pose
    unsigned int thread;
    mb_path imageFile;
    int image_count;
    int image_camera;
    double image_quality;
    double image_gain;
    double image_exposure;
    double time_d;
    double camera_navlon;
    double camera_navlat;
    double camera_sensordepth;
    double camera_heading;
    double camera_roll;
    double camera_pitch;

    // Image correction table
    Mat corr_table_y[2];
    Mat corr_table_cr[2];
    Mat corr_table_cb[2];
    Mat corr_table_count[2];
};

struct mbpm_control_struct {

    // Camera calibration model
    bool calibration_set;
    Mat cameraMatrix[2];
    Mat distCoeffs[2];
    Mat R;
    Mat T;
    Mat E;
    Mat F;
    Mat R1;
    Mat R2;
    Mat P1;
    Mat P2;
    Mat Q;
    double SensorWidthMm;
    double SensorHeightMm;
    double SensorCellMm;
    bool isVerticalStereo;

    // Apply camera calibration model
    Size imageSize;
    double fovx[2];
    double fovy[2];
    double fov_fudgefactor;
    double focalLength[2];
    Point2d principalPoint[2];
    double aspectRatio[2];

    // Topography grid
    bool use_topography;
    void *topogrid_ptr;
    double mtodeglon;
    double mtodeglat;

    // Pixel trim
    unsigned int trimPixels;

    // Image correction
    double reference_gain;
    double reference_exposure;

    // Image correction table
    int ncorr_x;
    int ncorr_y;
    int ncorr_z;
    double corr_xmin;
    double corr_xmax;
    double corr_ymin;
    double corr_ymax;
    double corr_zmin;
    double corr_zmax;
    double bin_dx;
    double bin_dy;
    double bin_dz;
    int ibin_xcen;
    int jbin_ycen;
    int kbin_zcen;
};

/*--------------------------------------------------------------------*/
void load_navigation(int verbose, mb_path NavigationFile, int lonflip,
                      int *nnav, double **nptime, double **nplon, double **nplat,
                      double **npheading, double **npspeed, double **npdraft,
                      double **nproll, double **nppitch, double **npheave, int *error)
{
    int status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    char *message = NULL;
    FILE *tfp = NULL;
    if ((tfp = fopen(NavigationFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",NavigationFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }
    *nnav = 0;
    char *result;
    mb_path buffer;
    while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
        (*nnav)++;
    fclose(tfp);

    /* allocate arrays for nav */
    if (*nnav > 0)
        {
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)nptime, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)nplon, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)nplat, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)npheading, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)npspeed, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)npdraft, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)nproll, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)nppitch, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nnav * sizeof(double), (void **)npheave, error);
        }

    /* if no nav data then set error */
    else
        {
        *error = MB_ERROR_BAD_DATA;
        }

    /* if error initializing memory or reading data then quit */
    if (*error != MB_ERROR_NO_ERROR)
        {
        mb_error(verbose,*error,&message);
        fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }

    /* read the data points in the nav file */
    *nnav = 0;
    double *ntime = *nptime;
    double *nlon = *nplon;
    double *nlat = *nplat;
    double *nheading = *npheading;
    double *nspeed = *npspeed;
    double *ndraft = *npdraft;
    double *nroll = *nproll;
    double *npitch = *nppitch;
    double *nheave = *npheave;

    if ((tfp = fopen(NavigationFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open navigation file <%s> for reading\n",NavigationFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }

    if (tfp != NULL) {
      bool done = false;
      while (!done) {
        memset(buffer, 0, MB_PATH_MAXLINE);
        if ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == NULL) {
          done = true;
        }
        else if (buffer[0] != '#') {
          bool value_ok = false;
          int time_i[7];
          double sec;
          int nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
              &time_i[0], &time_i[1], &time_i[2],
              &time_i[3], &time_i[4], &sec, &ntime[*nnav], &nlon[*nnav], &nlat[*nnav],
              &nheading[*nnav], &nspeed[*nnav], &ndraft[*nnav],
              &nroll[*nnav], &npitch[*nnav], &nheave[*nnav]);
          if (nget >= 15)
              value_ok = true;

          /* make sure longitude is defined according to lonflip */
          if (value_ok) {
            if (lonflip == -1 && nlon[*nnav] > 0.0)
                nlon[*nnav] = nlon[*nnav] - 360.0;
            else if (lonflip == 0 && nlon[*nnav] < -180.0)
                nlon[*nnav] = nlon[*nnav] + 360.0;
            else if (lonflip == 0 && nlon[*nnav] > 180.0)
                nlon[*nnav] = nlon[*nnav] - 360.0;
            else if (lonflip == 1 && nlon[*nnav] < 0.0)
                nlon[*nnav] = nlon[*nnav] + 360.0;
          }

          /* output some debug values */
          if (verbose >= 5 && value_ok) {
            fprintf(stderr,"\ndbg5  New navigation point read in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                *nnav,ntime[*nnav], nlon[*nnav], nlat[*nnav]);
          }
          else if (verbose >= 5) {
            fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       line: %s\n",buffer);
          }

          /* check for reverses or repeats in time */
          if (value_ok) {
            if (*nnav == 0)
              (*nnav)++;
            else if (ntime[*nnav] > ntime[*nnav-1])
              (*nnav)++;
            else if (*nnav > 0 && ntime[*nnav] <= ntime[*nnav-1]
                && verbose >= 5) {
              fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
              fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                  *nnav-1,ntime[*nnav-1], nlon[*nnav-1], nlat[*nnav-1]);
              fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                  *nnav,ntime[*nnav], nlon[*nnav], nlat[*nnav]);
            }
          }
        }
      }
      fclose(tfp);
    }

}

/*--------------------------------------------------------------------*/
void load_tide(int verbose, mb_path TideFile, int *ntide, double **tptime, double **tptide, int *error)
{
    int status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    char *message = NULL;
    FILE *tfp = NULL;
    if ((tfp = fopen(TideFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open tide file <%s> for reading\n",TideFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }
    *ntide = 0;
    char *result;
    mb_path buffer;
    while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
        (*ntide)++;
    fclose(tfp);

    /* allocate arrays for nav */
    if (*ntide > 0)
        {
        status = mb_reallocd(verbose, __FILE__, __LINE__, *ntide * sizeof(double), (void **)tptime, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *ntide * sizeof(double), (void **)tptide, error);
        }

    /* if no nav data then set error */
    else
        {
        *error = MB_ERROR_BAD_DATA;
        }

    /* if error initializing memory or reading data then quit */
    if (*error != MB_ERROR_NO_ERROR)
        {
        mb_error(verbose,*error,&message);
        fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }

    /* read the data points in the nav file */
    *ntide = 0;
    double *ttime = *tptime;
    double *ttide = *tptide;
    if ((tfp = fopen(TideFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open tide file <%s> for reading\n",TideFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }
    while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
        {
        bool value_ok = false;
        int time_i[7];
        double sec;

        /* read the navigation from an fnv file */
        int nget = sscanf(buffer,"%lf %lf", &ttime[*ntide], &ttide[*ntide]);
        if (nget == 2)
            value_ok = true;

        /* output some debug values */
        if (verbose >= 5 && value_ok)
            {
            fprintf(stderr,"\ndbg5  New tide point read in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
                *ntide, ttime[*ntide], ttide[*ntide]);
            }
        else if (verbose >= 5)
            {
            fprintf(stderr,"\ndbg5  Error parsing line in tide file in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       line: %s\n", buffer);
            }

        /* check for reverses or repeats in time */
        if (value_ok)
            {
            if (*ntide == 0)
                (*ntide)++;
            else if (ttime[*ntide] > ttime[*ntide-1])
                (*ntide)++;
            else if (*ntide > 0 && ttime[*ntide] <= ttime[*ntide-1] && verbose >= 5)
                {
                fprintf(stderr,"\ndbg5  Tide time error in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       tide[%d]: %f %f\n", *ntide-1, ttime[*ntide-1], ttide[*ntide-1]);
                fprintf(stderr,"dbg5       nav[%d]: %f %f\n", *ntide, ttime[*ntide], ttide[*ntide]);
                }
            }
        strncpy(buffer,"\0",sizeof(buffer));
        }
    fclose(tfp);

}

/*--------------------------------------------------------------------*/
void load_image_quality(int verbose, mb_path ImageQualityFile, int *nquality, double **qptime, double **qpquality, int *error)
{
    int status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    char *message = NULL;
    FILE *tfp = NULL;
    if ((tfp = fopen(ImageQualityFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open image quality file <%s> for reading\n",ImageQualityFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }
    *nquality = 0;
    char *result;
    mb_path buffer;
    while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
        (*nquality)++;
    fclose(tfp);

    /* allocate arrays for quality */
    if (*nquality > 0)
        {
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nquality * sizeof(double), (void **)qptime, error);
        status = mb_reallocd(verbose, __FILE__, __LINE__, *nquality * sizeof(double), (void **)qpquality, error);
        }

    /* if no quality data then set error */
    else
        {
        *error = MB_ERROR_BAD_DATA;
        }

    /* if error initializing memory or reading data then quit */
    if (*error != MB_ERROR_NO_ERROR)
        {
        mb_error(verbose,*error,&message);
        fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }

    /* read the data points in the quality file */
    *nquality = 0;
    double *qtime = *qptime;
    double *qquality = *qpquality;
    if ((tfp = fopen(ImageQualityFile, "r")) == NULL)
        {
        *error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open image quality file <%s> for reading\n",ImageQualityFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        exit(*error);
        }
    while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
        {
        bool value_ok = false;

        /* read the navigation from an fnv file */
        int nget = sscanf(buffer,"%lf %lf", &qtime[*nquality], &qquality[*nquality]);
        if (nget == 2)
            value_ok = true;

        /* output some debug values */
        if (verbose >= 5 && value_ok)
            {
            fprintf(stderr,"\ndbg5  New image quality point read in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       quality[%d]: %f %f\n", *nquality, qtime[*nquality], qquality[*nquality]);
            }
        else if (verbose >= 5)
            {
            fprintf(stderr,"\ndbg5  Error parsing line in image quality file in program <%s>\n",program_name);
            fprintf(stderr,"dbg5       line: %s\n",buffer);
            }

        /* check for reverses or repeats in time */
        if (value_ok)
            {
            if (*nquality == 0)
                (*nquality)++;
            else if (qtime[*nquality] > qtime[*nquality-1])
                (*nquality)++;
            else if (*nquality > 0 && qtime[*nquality] <= qtime[*nquality-1] && verbose >= 5)
                {
                fprintf(stderr,"\ndbg5  Image quality time error in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       quality[%d]: %f %f\n", *nquality-1, qtime[*nquality-1], qquality[*nquality-1]);
                fprintf(stderr,"dbg5       quality[%d]: %f %f\n", *nquality, qtime[*nquality], qquality[*nquality]);
                }
            }
        strncpy(buffer,"\0",sizeof(buffer));
        }
    fclose(tfp);

}
/*--------------------------------------------------------------------*/


void load_calibration(int verbose, mb_path StereoCameraCalibrationFile, struct mbpm_control_struct *control, int *error)
{
    FileStorage fstorage;

    /* read intrinsic and extrinsic stereo camera calibration parameters */
    fstorage.open(StereoCameraCalibrationFile, FileStorage::READ);
    if(fstorage.isOpened() ) {
        fstorage["M1"] >> control->cameraMatrix[0];
        fstorage["D1"] >> control->distCoeffs[0];
        fstorage["M2"] >> control->cameraMatrix[1];
        fstorage["D2"] >> control->distCoeffs[1];
        fstorage["R"] >> control->R;
        fstorage["T"] >> control->T;
        fstorage["R1"] >> control->R1;
        fstorage["R2"] >> control->R2;
        fstorage["P1"] >> control->P1;
        fstorage["P2"] >> control->P2;
        fstorage["Q"] >> control->Q;
        fstorage.release();
        control->isVerticalStereo = fabs(control->P2.at<double>(1, 3)) > fabs(control->P2.at<double>(0, 3));
        control->calibration_set = false;
    }
    else {
        fprintf(stderr,"\nUnable to read camera calibration file %s\n",
            StereoCameraCalibrationFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        mb_memory_clear(verbose, error);
        exit(MB_ERROR_BAD_PARAMETER);
    }

    /* print out calibration information */
    if (verbose >= 0) {
        cerr << endl;
        cerr << "Stereo camera calibration model read from: " << StereoCameraCalibrationFile << endl;
        cerr << "M1:" << endl << control->cameraMatrix[0] << endl << endl;
        cerr << "D1:" << endl << control->distCoeffs[0] << endl << endl;
        cerr << "M2:" << endl << control->cameraMatrix[1] << endl << endl;
        cerr << "D2:" << endl << control->distCoeffs[1] << endl << endl;
        cerr << "R:" << endl << control->R << endl << endl;
        cerr << "T:" << endl << control->T << endl << endl;
        cerr << "R1:" << endl << control->R1 << endl << endl;
        cerr << "R2:" << endl << control->R2 << endl << endl;
        cerr << "P1:" << endl << control->P1 << endl << endl;
        cerr << "P2:" << endl << control->P2 << endl << endl;
        cerr << "Q:" << endl << control->Q << endl << endl;
    }
}

/*--------------------------------------------------------------------*/

void process_image(int verbose, struct mbpm_process_struct *process,
                  struct mbpm_control_struct *control, int *status, int *error)
{
    // Working images
    Mat imageProcess;
    Mat imageUndistort;
    Mat imageUndistortYCrCb;

    /* read the image */
    imageProcess = imread(process->imageFile);
    if (!imageProcess.empty()) {

        double fov_x, fov_y;
        double center_x, center_y;
        double intensityCorrection;

        /* undistort the image */
        undistort(imageProcess, imageUndistort, control->cameraMatrix[process->image_camera], control->distCoeffs[process->image_camera], noArray());
        imageProcess.release();

        /* get field of view, offsets, and principal point to use */
        fov_x = control->fovx[process->image_camera];
        fov_y = control->fovy[process->image_camera];
        center_x = control->principalPoint[process->image_camera].x / control->SensorCellMm;
        center_y = control->principalPoint[process->image_camera].y / control->SensorCellMm;

        /* calculate reference "depth" for use in calculating ray angles for
            individual pixels */
        double zzref = 0.5 * (0.5 * control->imageSize.width / tan(DTR * 0.5 * fov_x * control->fov_fudgefactor)
                + 0.5 * control->imageSize.height / tan(DTR * 0.5 * fov_y * control->fov_fudgefactor));

        /* Apply camera model translation */
        double dlon, dlat, dz;
        double headingx = sin(DTR * process->camera_heading);
        double headingy = cos(DTR * process->camera_heading);
        if (process->image_camera == 0) {
            dlon = 0.5 * (control->T.at<double>(0)) * control->mtodeglon;
            dlat = 0.5 * (control->T.at<double>(1)) * control->mtodeglat;
            dz = 0.5 * (control->T.at<double>(2));
        } else {
            dlon = -0.5 * (control->T.at<double>(0)) * control->mtodeglon;
            dlat = -0.5 * (control->T.at<double>(1)) * control->mtodeglat;
            dz = -0.5 * (control->T.at<double>(2));
        }
        process->camera_navlon += (headingy * dlon + headingx * dlat);
        process->camera_navlat += (-headingx * dlon + headingy * dlat);
        process->camera_sensordepth += dz;

        /* calculate the largest distance from center for this image for use
            in calculating pixel priority */
        double xx = MAX(center_x, imageUndistort.cols - center_x);
        double yy = MAX(center_y, imageUndistort.rows - center_y);
        double rrxymax = sqrt(xx * xx + yy * yy);

        /* Do some calculations relevant to image correction:
         * - calculate the average intensity of the image
         * - if specified calculate the correction required to bring the
         *    average intensity to a value of 70.0
         */
        cvtColor(imageUndistort, imageUndistortYCrCb, COLOR_BGR2YCrCb);
        Scalar avgPixelIntensity = mean(imageUndistortYCrCb);

        /* get correction for embedded camera gain */
        double imageIntensityCorrection = 1.0;
        if (control->reference_gain > 0.0)
            imageIntensityCorrection *= pow(10.0, (control->reference_gain - process->image_gain) / 20.0);

        /* get correction for embedded camera exposure time */
        if (process->image_exposure > 0.0 && control->reference_exposure > 0.0) {
            //imageIntensityCorrection *= control->reference_exposure / process->image_exposure;

            if (process->image_exposure >= 7999.0)
                imageIntensityCorrection *= 1.0;
            else if (process->image_exposure >= 3999.00)
                imageIntensityCorrection *= 1.14;
            else if (process->image_exposure >= 1999.00)
                imageIntensityCorrection *= 1.4;
            else if (process->image_exposure >= 999.00)
                imageIntensityCorrection *= 2.0;
            if (control->reference_exposure >= 7999.0)
                imageIntensityCorrection /= 1.0;
            else if (control->reference_exposure >= 3999.00)
                imageIntensityCorrection /= 1.14;
            else if (control->reference_exposure >= 1999.00)
                imageIntensityCorrection /= 1.4;
            else if (control->reference_exposure >= 999.00)
                imageIntensityCorrection /= 2.0;
        }

        /* Print information for image to be processed */
        int time_i[7];
        mb_get_date(verbose, process->time_d, time_i);
        fprintf(stderr,"%4d Camera:%d %s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d LLZ: %.8f %.8f %8.3f HRP: %6.2f %5.2f %5.2f A:%.3f Q:%.2f\n",
                process->image_count, process->image_camera, process->imageFile,
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                process->camera_navlon, process->camera_navlat, process->camera_sensordepth,
                process->camera_heading, process->camera_roll, process->camera_pitch, avgPixelIntensity.val[0], process->image_quality);

        /* get unit vector for direction camera is pointing */

        /* rotate center pixel location using attitude and zzref */
        double zz;
        mb_platform_math_attitude_rotate_beam(verbose,
            0.0, 0.0, zzref,
            process->camera_roll, process->camera_pitch, 0.0,
            &xx, &yy, &zz,
            error);
        double rr = sqrt(xx * xx + yy * yy + zz * zz);
        double phi = RTD * atan2(yy, xx);
        double theta = RTD * acos(zz / rr);

        /* calculate unit vector relative to the camera rig */
        double vx = sin(DTR * theta) * cos(DTR * phi);
        double vy = sin(DTR * theta) * sin(DTR * phi);
        double vz = cos(DTR * theta);

        /* apply rotation of each camera relative to the rig */
        double vxx, vyy, vzz;
        if (process->image_camera == 1) {
            vxx = vx * (control->R.at<double>(0,0)) + vy * (control->R.at<double>(0,1)) + vz * (control->R.at<double>(0,2));
            vyy = vx * (control->R.at<double>(1,0)) + vy * (control->R.at<double>(1,1)) + vz * (control->R.at<double>(1,2));
            vzz = vx * (control->R.at<double>(2,0)) + vy * (control->R.at<double>(2,1)) + vz * (control->R.at<double>(2,2));
        }
        else {
            vxx = vx;
            vyy = vy;
            vzz = vz;
        }

        /* rotate unit vector by camera rig heading */
        double cx = vxx * cos(DTR * process->camera_heading) + vyy * sin(DTR * process->camera_heading);
        double cy = -vxx * sin(DTR * process->camera_heading) + vyy * cos(DTR * process->camera_heading);
        double cz = vzz;

        /* Loop over the pixels in the undistorted image. If trim is nonzero then
            that number of pixels are ignored around the margins. This solves the
            problem of black pixels being incorporated into the correction tables. If
            trim is not specified then code below will ignore both black pixels
            and pixels that are adjacent to black pixels. */

        for (int i=control->trimPixels; i<imageUndistortYCrCb.cols-control->trimPixels; i++) {
            for (int j=control->trimPixels; j<imageUndistortYCrCb.rows-control->trimPixels; j++) {
                bool use_pixel = true;
                double xx;
                double yy;
                double rrxysq;
                double rrxy;
                double rrxysq2;
                double rr2;
                double theta2;
                double dtheta;
                double standoff = 0.0;
                double lon, lat, topo;
                //float bgr_intensity;
                float ycrcb_y;
                float ycrcb_cr;
                float ycrcb_cb;

                /* Deal with problem of black pixels at the margins of the
                    undistorted images. If the user has not specified a trim
                    range for the image margins, then ignore all purely black
                    pixels and all pixels adjacent to purely black pixels. */
                if (control->trimPixels == 0) {
                    unsigned int sum = imageUndistort.at<Vec3b>(j,i)[0]
                                        + imageUndistort.at<Vec3b>(j,i)[1]
                                        + imageUndistort.at<Vec3b>(j,i)[2];
                    if (sum == 0)
                        use_pixel = false;
                    else {
                        for (int ii = MAX(i-1, 0); ii < MIN(i+2, imageUndistort.cols) && use_pixel; ii++) {
                            for (int jj = MAX(j-1, 0); jj < MIN(j+2, imageUndistort.rows) && use_pixel; jj++) {
                                unsigned int sum2 = imageUndistort.at<Vec3b>(jj,ii)[0]
                                        + imageUndistort.at<Vec3b>(jj,ii)[1]
                                        + imageUndistort.at<Vec3b>(jj,ii)[2];
                                if (sum2 == 0) {
                                    use_pixel = false;
                                }
                            }
                        }
                    }
                }

                /* calculate intensity for this pixel
                    - access the pixel value with Vec3b */
                if (use_pixel) {
                    //bgr_intensity = ((float)imageUndistort.at<Vec3b>(j,i)[0]
                    //         + (float)imageUndistort.at<Vec3b>(j,i)[1]
                    //         + (float)imageUndistort.at<Vec3b>(j,i)[2]) / 3.0;
                    ycrcb_y = (float)(imageIntensityCorrection * imageUndistortYCrCb.at<Vec3b>(j,i)[0]);
                    ycrcb_cr = (float)(imageUndistortYCrCb.at<Vec3b>(j,i)[1]);
                    ycrcb_cb = (float)(imageUndistortYCrCb.at<Vec3b>(j,i)[2]);
//fprintf(stderr,"i:%d j:%d corr:%f  YCrCb intensity:%f\n",
// i, j, imageIntensityCorrection, ycrcb_y);

                    /* only use nonzero intensities */
                    if (ycrcb_y <= 0.0) {
                        use_pixel = false;
                    }
                }

                if (use_pixel) {
                    /* calculate the pixel location and distance
                        from the image center */
                    xx = i - center_x;
                    yy = center_y - j;
                    rrxysq = xx * xx + yy * yy;
                    rrxy = sqrt(rrxysq);
                    rr = sqrt(rrxysq + zzref * zzref);

                    /* calculate the pixel takeoff angles relative to the camera rig */
                    phi = RTD * atan2(yy, xx);
                    theta = RTD * acos(zzref / rr);

                    /* calculate the angular width of a single pixel */
                    rrxysq2 = (rrxy + 1.0) * (rrxy + 1.0);
                    rr2 = sqrt(rrxysq2 + zzref * zzref);
                    theta2 = RTD * acos(zzref / rr2);
                    dtheta = theta2 - theta;
//if (debugprint == MB_YES) {
//fprintf(stderr,"Camera: roll:%.3f pitch:%.3f\n", process->camera_roll, process->camera_pitch);
//fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d",
//imageUndistort.rows, imageUndistort.cols, i, j,
//imageUndistort.at<Vec3b>(j,i)[0], imageUndistort.at<Vec3b>(j,i)[1], imageUndistort.at<Vec3b>(j,i)[2]);
//fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zzref, rr, phi, theta);
//}

                    /* rotate pixel location using attitude and zzref */
                    double zz;
                    mb_platform_math_attitude_rotate_beam(verbose,
                        xx, yy, zzref,
                        process->camera_roll, process->camera_pitch, 0.0,
                        &xx, &yy, &zz,
                        error);

                    /* recalculate the pixel takeoff angles relative to the camera rig */
                    rrxysq = xx * xx + yy * yy;
                    rrxy = sqrt(rrxysq);
                    rr = sqrt(rrxysq + zz * zz);
                    phi = RTD * atan2(yy, xx);
                    theta = RTD * acos(zz / rr);
//if (debugprint == MB_YES) {
//fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d",
//imageUndistort.rows, imageUndistort.cols, i, j,
//imageUndistort.at<Vec3b>(j,i)[0], imageUndistort.at<Vec3b>(j,i)[1], imageUndistort.at<Vec3b>(j,i)[2]);
//fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zz, rr, phi, theta);
//}

                    /* calculate unit vector relative to the camera rig */
                    vz = cos(DTR * theta);
                    vx = sin(DTR * theta) * cos(DTR * phi);
                    vy = sin(DTR * theta) * sin(DTR * phi);
//if (debugprint == MB_YES) {
//fprintf(stderr,"camera rig unit vector: %f %f %f\n",vx,vy,vz);
//}
                    /* if takeoff angle is too vertical (this is a 2D photomosaic)
                        then do not use this pixel */
                    if (theta > 80.0)
                        use_pixel = false;
                }

                if (use_pixel) {

                    /* apply rotation of each camera relative to the rig */
                    if (process->image_camera == 1) {
                        vxx = vx * (control->R.at<double>(0,0)) + vy * (control->R.at<double>(0,1)) + vz * (control->R.at<double>(0,2));
                        vyy = vx * (control->R.at<double>(1,0)) + vy * (control->R.at<double>(1,1)) + vz * (control->R.at<double>(1,2));
                        vzz = vx * (control->R.at<double>(2,0)) + vy * (control->R.at<double>(2,1)) + vz * (control->R.at<double>(2,2));
//if (debugprint == MB_YES) {
//fprintf(stderr,"\nR:      %f %f %f\n",control->R.at<double>(0,0), control->R.at<double>(0,1), control->R.at<double>(0,2));
//fprintf(stderr,"R:      %f %f %f\n",control->R.at<double>(1,0), control->R.at<double>(1,1), control->R.at<double>(1,2));
//fprintf(stderr,"R:      %f %f %f\n",control->R.at<double>(2,0), control->R.at<double>(2,1), control->R.at<double>(2,2));
//fprintf(stderr,"Rotation: camera 1 unit vector: %f %f %f    camera 0 unit vector: %f %f %f\n",vx,vy,vz,vxx,vyy,vzz);
//}
                    }
                    else {
                        vxx = vx;
                        vyy = vy;
                        vzz = vz;
                    }
//if (debugprint == MB_YES) {
//fprintf(stderr,"camera unit vector:     %f %f %f\n",vx,vy,vz);
//}

                    /* rotate unit vector by camera rig heading */
                    vx = vxx * cos(DTR * process->camera_heading) + vyy * sin(DTR * process->camera_heading);
                    vy = -vxx * sin(DTR * process->camera_heading) + vyy * cos(DTR * process->camera_heading);
                    vz = vzz;
//if (debugprint == MB_YES) {
//fprintf(stderr,"camera unit vector rotated by heading %f:     %f %f %f\n",process->camera_heading,vx,vy,vz);
//}

                    /* find the location where this vector intersects the grid */
                    if (control->use_topography) {
                        *status = mb_topogrid_intersect(verbose, control->topogrid_ptr,
                                    process->camera_navlon, process->camera_navlat, 0.0, process->camera_sensordepth,
                                    control->mtodeglon, control->mtodeglat, vx, vy, vz,
                                    &lon, &lat, &topo, &rr, error);
                    }
                    else {
                        rr = 0.5 * (control->corr_zmin + control->corr_zmax) / vz;
                        lon = process->camera_navlon + control->mtodeglon * vx * rr;
                        lat = process->camera_navlat + control->mtodeglon * vy * rr;
                        topo = -process->camera_sensordepth -  0.5 * (control->corr_zmin + control->corr_zmax);
                    }
                    zz = -process->camera_sensordepth - topo;

                    /* standoff is dot product of camera vector with projected pixel vector */
                    standoff = (cx * rr * vx) + (cy * rr * vy) + (cz * rr * vz);
//if (debugprint == MB_YES) {
//fprintf(stderr," llz: %.10f %.10f %.3f  range:%.3f  standoff:%.3f\n", lon, lat, topo, rr, standoff);
//}
                    /* Don't use pixel if too vertical or standoff zero */
                    if (theta > 80.0 || standoff <= 0.0)
                        use_pixel = false;
                }

                if (use_pixel) {

                    /* use pixel if standoff in range */
                    if (standoff >= control->corr_zmin && standoff <= control->corr_zmax) {

                        /* save the intensity in the correct pixel and range bin */
                        int ibin_x = (int)((i + 0.5 * control->bin_dx) / control->bin_dx);
                        ibin_x = MIN(MAX(ibin_x, 0), control->ncorr_x - 1);
                        int jbin_y = (int)((j + 0.5 * control->bin_dy) / control->bin_dy);
                        jbin_y = MIN(MAX(jbin_y, 0), control->ncorr_y - 1);
                        int kbin_z = (int)((standoff + 0.5 * control->bin_dz - control->corr_zmin) / control->bin_dz);
                        kbin_z = MIN(MAX(kbin_z, 0), control->ncorr_z - 1);
                        process->corr_table_y[process->image_camera].at<float>(ibin_x, jbin_y, kbin_z) += ycrcb_y;
                        process->corr_table_cr[process->image_camera].at<float>(ibin_x, jbin_y, kbin_z) += ycrcb_cr;
                        process->corr_table_cb[process->image_camera].at<float>(ibin_x, jbin_y, kbin_z) += ycrcb_cb;
                        process->corr_table_count[process->image_camera].at<int>(ibin_x, jbin_y, kbin_z) += 1;
                    }
                }
            }
        }
        imageUndistortYCrCb.release();
        imageUndistort.release();

    }
}
/*--------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    int    help = 0;
    int    flag = 0;

    /* parameter controls */
    struct mbpm_process_struct processPars[MB_THREAD_MAX];
    struct mbpm_control_struct control;

    /* Output image correction table */
    mb_path ImageCorrectionFile;

    /* Input image variables */
    mb_path    ImageListFile;
    mb_path    imageLeftFile;
    mb_path    imageRightFile;
    mb_path    imageFile;
    double    left_time_d;
    double    right_time_d;
    double    left_gain;
    double    right_gain;
    double    left_exposure;
    double    right_exposure;
    double    time_d;
    int       time_i[7];
    double    navlon;
    double    navlat;
    double    speed;
    double    heading;
    double    distance;
    double    altitude;
    double    sensordepth;
    double    draft;
    double    roll;
    double    pitch;
    double    heave;
    double    tide;

    /* platform offsets */
    bool platform_specified = false;
    bool platform_initialized = false;
    mb_path PlatformFile;
    int camera_sensor = -1;
    int nav_sensor = -1;
    int sensordepth_sensor = -1;
    int heading_sensor = -1;
    int altitude_sensor = -1;
    int attitude_sensor = -1;
    struct mb_platform_struct *platform = NULL;
    struct mb_sensor_struct *sensor_bathymetry = NULL;
    struct mb_sensor_struct *sensor_backscatter = NULL;
    struct mb_sensor_struct *sensor_stereocamera = NULL;
    struct mb_sensor_struct *sensor_position = NULL;
    struct mb_sensor_struct *sensor_depth = NULL;
    struct mb_sensor_struct *sensor_heading = NULL;
    struct mb_sensor_struct *sensor_rollpitch = NULL;
    struct mb_sensor_struct *sensor_heave = NULL;
    struct mb_sensor_struct *sensor_camera = NULL;

    /* Input camera parameters */
    bool calibration_specified = false;
    bool calibration_initialized = false;
    mb_path StereoCameraCalibrationFile;
    int image_camera = MBPM_CAMERA_LEFT;
    int use_camera_mode = MBPM_USE_STEREO;
    control.calibration_set = false;
    control.SensorWidthMm = 8.789;
    control.SensorHeightMm = 6.610;
    control.SensorCellMm =0.00454;
    control.fov_fudgefactor = 1.0;
    
    control.trimPixels = 0;
    bool undistort_initialized = false;

    /* Input image correction */
    control.reference_gain = 14.0;
    control.reference_exposure = 8000.0;
    control.ncorr_x = 11;
    control.ncorr_y = 11;
    control.ncorr_z = 41;
    control.corr_xmin = 0.0;
    control.corr_xmax = 0.0;
    control.corr_ymin = 0.0;
    control.corr_ymax = 0.0;
    control.corr_zmin = 1.0;
    control.corr_zmax = 9.0;
    control.bin_dx = 0.0;
    control.bin_dy = 0.0;
    control.bin_dz = 0.0;
    control.ibin_xcen = 0;
    control.jbin_ycen = 0;
    control.kbin_zcen = 0;

    /* Input navigation variables */
    bool navigation_specified = false;
    bool navigation_initialized = false;
    mb_path NavigationFile;
    int intstat;
    int itime = 0;
    int iitime = 0;
    int nnav = 0;
    double *ntime = NULL;
    double *nlon = NULL;
    double *nlat = NULL;
    double *nheading = NULL;
    double *nspeed = NULL;
    double *ndraft = NULL;
    double *nroll = NULL;
    double *npitch = NULL;
    double *nheave = NULL;

    /* Input tide variables */
    bool tide_specified = false;
    bool tide_initialized = false;
    mb_path TideFile;
    int ntide = 0;
    double *ttime = NULL;
    double *ttide = NULL;

    /* Input quality variables */
    bool imagequality_initialized = false;
    bool imagequality_specified = false;
    double imageQualityThreshold = 0.0;
    double imageQualityFilterLength = 0.0;
    mb_path ImageQualityFile;
    int iqtime = 0;
    int nquality = 0;
    double *qtime = NULL;
    double *qquality = NULL;

    /* topography parameters */
    control.use_topography = false;
    mb_path TopographyGridFile;
    control.topogrid_ptr = NULL;

    /* MBIO status variables */
    int status = MB_SUCCESS;
    int verbose = 0;
    int error = MB_ERROR_NO_ERROR;
    char *message;

    /* output stream for basic stuff (stdout if verbose <= 1,
        stderr if verbose > 1) */
    FILE *stream = NULL;
    FILE *tfp;

    bool use_this_image = false;
    mb_path buffer;
    char *result;
    int lonflip;
    double sec;
    double pbounds[4];
    double factor, scale;
    FileStorage fstorage;

    int npairs, nimages, currentimages;

    /* command line option definitions */
    /* mbgetphotocorrection
     *    --verbose
     *    --help
     *    --threads=nthreads
     *    --input=imagelist
     *    --output=file  [--correction-file=file]
     *    --correction-x-dimension=value
     *    --correction-y-dimension=value
     *    --correction-z-dimension=value
     *    --correction-z-minmax=value/value
     *    --fov-fudgefactor=factor
     *    --trim=trim_pixels
     *    --reference-gain=gain
     *    --reference-exposure=exposure
     *    --platform-file=platform.plf
     *    --camera-sensor=camera_sensor_id
     *    --nav-sensor=nav_sensor_id
     *    --sensordepth-sensor=sensordepth_sensor_id
     *    --heading-sensor=heading_sensor_id
     *    --altitude-sensor=altitude_sensor_id
     *    --attitude-sensor=attitude_sensor_id
     *    --use-left-camera
     *    --use-right-camera
     *    --use-both-cameras
     *    --calibration-file=stereocalibration.yaml
     *    --navigation-file=file
     *    --tide-file=file
     *    --image-quality-file=file
     *    --image-quality-threshold=value
     *    --image-quality-filter-length=value
     *    --topography-grid=file
     *
     */
    static struct option options[] =
        {
        {"verbose",                     no_argument,            NULL,         0},
        {"help",                        no_argument,            NULL,         0},
        {"threads",                     required_argument,      NULL,         0},
        {"input",                       required_argument,      NULL,         0},
        {"output",                      required_argument,      NULL,         0},
        {"correction-file",           required_argument,  NULL, 0},
        {"correction-x-dimension",    required_argument,  NULL, 0},
        {"correction-y-dimension",    required_argument,  NULL, 0},
        {"correction-z-dimension",    required_argument,  NULL, 0},
        {"correction-z-minmax",       required_argument,  NULL, 0},
        {"fov-fudgefactor",             required_argument,      NULL,         0},
        {"trim",                        required_argument,      NULL,         0},
        {"reference-gain",              required_argument,      NULL,         0},
        {"reference-exposure",          required_argument,      NULL,         0},
        {"platform-file",               required_argument,      NULL,         0},
        {"camera-sensor",               required_argument,      NULL,         0},
        {"nav-sensor",                  required_argument,      NULL,         0},
        {"sensordepth-sensor",          required_argument,      NULL,         0},
        {"heading-sensor",              required_argument,      NULL,         0},
        {"altitude-sensor",             required_argument,      NULL,         0},
        {"attitude-sensor",             required_argument,      NULL,         0},
        {"use-left-camera",             no_argument,            NULL,         0},
        {"use-right-camera",            no_argument,            NULL,         0},
        {"use-both-cameras",            no_argument,            NULL,         0},
        {"calibration-file",            required_argument,      NULL,         0},
        {"navigation-file",             required_argument,      NULL,         0},
        {"tide-file",                   required_argument,      NULL,         0},
        {"image-quality-file",          required_argument,      NULL,         0},
        {"image-quality-threshold",     required_argument,      NULL,         0},
        {"image-quality-filter-length", required_argument,      NULL,         0},
        {"topography-grid",             required_argument,      NULL,         0},
        {NULL,                          0,                      NULL,         0}
        };

    /* set default imagelistfile name */
    snprintf(ImageListFile, sizeof(ImageListFile), "imagelist.mb-1");
    snprintf(ImageCorrectionFile, sizeof(ImageListFile), "imagelist_cameracorrection.yml");

    /* initialize some other things */
    memset(StereoCameraCalibrationFile, 0, sizeof(mb_path));
    memset(PlatformFile, 0, sizeof(mb_path));
    memset(NavigationFile, 0, sizeof(mb_path));
    memset(TideFile, 0, sizeof(mb_path));
    memset(ImageQualityFile, 0, sizeof(mb_path));
    memset(TopographyGridFile, 0, sizeof(mb_path));

    /* Thread handling */
    unsigned int numThreads = 1;
    unsigned int numConcurrency = std::thread::hardware_concurrency();
    std::thread mbgetphotocorrectionThreads[MB_THREAD_MAX];
    int thread_status[MB_THREAD_MAX];
    int thread_error[MB_THREAD_MAX];

    /* process argument list */
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
      switch (c)
        {
        /* long options all return c=0 */
        case 0:
            /* verbose */
            if (strcmp("verbose", options[option_index].name) == 0)
                {
                verbose++;
                }

            /* help */
            else if (strcmp("help", options[option_index].name) == 0)
                {
                help = MB_YES;
                }

            /*-------------------------------------------------------
             * Thread handling - desired number of threads */

            /* threads */
            else if (strcmp("threads", options[option_index].name) == 0)
                {
                sscanf (optarg,"%d", &numThreads);
                if (numThreads < 1)
                    numThreads = 1;
                numThreads = MIN(numThreads, MIN(numConcurrency, MB_THREAD_MAX));
                }

            /*-------------------------------------------------------
             * Define input file and format (usually a datalist) */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", ImageListFile);
                }

            /* output */
            else if ((strcmp("output", options[option_index].name) == 0)
                    || (strcmp("correction-file", options[option_index].name) == 0))
                {
                const int n = sscanf (optarg,"%s", ImageCorrectionFile);
                }

            /* correction-x-dimension */
            else if (strcmp("correction-x-dimension", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.ncorr_x);
                }

            /* correction-y-dimension */
            else if (strcmp("correction-y-dimension", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.ncorr_y);
                }

            /* correction-z-dimension */
            else if (strcmp("correction-z-dimension", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.ncorr_z);
                }

            /* correction-z-minmax */
            else if (strcmp("correction-z-minmax", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%lf/%lf", &control.corr_zmin, &control.corr_zmax);
                }

            /* fov-fudgefactor */
            else if (strcmp("fov-fudgefactor", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &control.fov_fudgefactor);
                }

            /* trim */
            else if (strcmp("trim", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%d", &control.trimPixels);
                }

            /* reference-gain */
            else if (strcmp("reference-gain", options[option_index].name) == 0)
                {
                int n = sscanf (optarg,"%lf", &control.reference_gain);
                }

            /* reference-exposure */
            else if (strcmp("reference-exposure", options[option_index].name) == 0)
                {
                int n = sscanf (optarg,"%lf", &control.reference_exposure);
                }

            /* platform-file */
            else if (strcmp("platform-file", options[option_index].name) == 0)
                {
                strcpy(PlatformFile, optarg);
                platform_specified = true;
                }

            /* camera-sensor */
            else if (strcmp("camera-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &camera_sensor);
                }

            /* nav-sensor */
            else if (strcmp("nav-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &nav_sensor);
                }

            /* sensordepth-sensor */
            else if (strcmp("sensordepth-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &sensordepth_sensor);
                }

            /* heading-sensor */
            else if (strcmp("heading-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &heading_sensor);
                }

            /* altitude-sensor */
            else if (strcmp("altitude-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &altitude_sensor);
                }

            /* attitude-sensor */
            else if (strcmp("attitude-sensor", options[option_index].name) == 0)
                {
                sscanf(optarg, "%d", &attitude_sensor);
                }

            /* use-left-camera */
            else if (strcmp("use-left-camera", options[option_index].name) == 0)
                {
                use_camera_mode = MBPM_USE_LEFT;
                }

            /* use-right-camera */
            else if (strcmp("use-right-camera", options[option_index].name) == 0)
                {
                use_camera_mode = MBPM_USE_RIGHT;
                }

            /* use-both-cameras */
            else if (strcmp("use-both-cameras", options[option_index].name) == 0)
                {
                use_camera_mode = MBPM_USE_STEREO;
                }

            /* calibration-file */
            else if (strcmp("calibration-file", options[option_index].name) == 0)
                {
                strcpy(StereoCameraCalibrationFile, optarg);
                calibration_specified = true;
                }

            /* navigation-file */
            else if (strcmp("navigation-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", NavigationFile);
                if (n == 1)
                    navigation_specified = true;
                }

            /* tide-file */
            else if (strcmp("tide-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", TideFile);
                if (n == 1)
                    tide_specified = true;
                }

            /* image-quality-file */
            else if (strcmp("image-quality-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageQualityFile);
                if (n == 1)
                    imagequality_specified = true;
                }

            /* image-quality-threshold  (0 <= imageQualityThreshold <= 1) */
            else if (strcmp("image-quality-threshold", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &imageQualityThreshold);
                }

            /* image-quality-filter-length */
            else if (strcmp("image-quality-filter-length", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &imageQualityFilterLength);
                }

            /* topography-grid */
            else if (strcmp("topography-grid", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", TopographyGridFile);
                if (n == 1)
                    control.use_topography = true;
                }

            break;
        case '?':
            errflg++;
        }

    /* if error flagged then print it and exit */
    if (errflg)
        {
        fprintf(stderr,"usage: %s\n", usage_message);
        fprintf(stderr,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_USAGE;
        exit(error);
        }

    /* set output stream */
    if (verbose <= 1)
        stream = stdout;
    else
        stream = stderr;

    /* print starting message */
    if (verbose == 1 || help)
        {
        fprintf(stream,"\nProgram %s\n",program_name);
        fprintf(stream,"MB-system Version %s\n",MB_VERSION);
        }

    /* print starting debug statements */
    char dbg2[] = "dbg2  ";
    char blank[] = "";
    char *first = NULL;
    if (verbose >= 2)
        first = dbg2;
    else
        first = blank;
    fprintf(stream,"\n%sProgram <%s>\n", first, program_name);
    if (verbose > 0)
        {
        fprintf(stream,"\ndbg2  Program <%s>\n",program_name);
        fprintf(stream,"dbg2  MB-system Version %s\n",MB_VERSION);
        fprintf(stream,"dbg2  Control Parameters:\n");
        fprintf(stream,"%s     verbose:                          %d\n", first, verbose);
        fprintf(stream,"%s     help:                             %d\n", first, help);
        fprintf(stream,"%s     numThreads:                       %d\n", first, numThreads);
        fprintf(stream,"%s     ImageListFile:                    %s\n", first, ImageListFile);
        fprintf(stream,"%s     ImageCorrectionFile:              %s\n", first, ImageCorrectionFile);
        fprintf(stream,"%s     ncorr_x:                          %d\n", first, control.ncorr_x);
        fprintf(stream,"%s     ncorr_y:                          %d\n", first, control.ncorr_y);
        fprintf(stream,"%s     ncorr_z:                          %d\n", first, control.ncorr_z);
        fprintf(stream,"%s     corr_zmin:                        %f\n", first, control.corr_zmin);
        fprintf(stream,"%s     corr_zmax:                        %f\n", first, control.corr_zmax);
        fprintf(stream,"%s     control.fov_fudgefactor:          %f\n", first, control.fov_fudgefactor);
        fprintf(stream,"%s     control.trimPixels:               %u\n", first, control.trimPixels);
        fprintf(stream,"%s     control.reference_gain:           %f\n", first, control.reference_gain);
        fprintf(stream,"%s     control.reference_exposure:       %f\n", first, control.reference_exposure);
        fprintf(stream,"%s     PlatformFile:                     %s\n", first, PlatformFile);
        fprintf(stream,"%s     platform_specified:               %d\n", first, platform_specified);
        fprintf(stream,"%s     camera_sensor:                    %d\n", first, camera_sensor);
        fprintf(stream,"%s     nav_sensor:                       %d\n", first, nav_sensor);
        fprintf(stream,"%s     sensordepth_sensor:               %d\n", first, sensordepth_sensor);
        fprintf(stream,"%s     heading_sensor:                   %d\n", first, heading_sensor);
        fprintf(stream,"%s     altitude_sensor:                  %d\n", first, altitude_sensor);
        fprintf(stream,"%s     attitude_sensor:                  %d\n", first, attitude_sensor);
        fprintf(stream,"%s     use_camera_mode:                  %d\n", first, use_camera_mode);
        fprintf(stream,"%s     control.calibration_set:          %d\n", first, control.calibration_set);
        fprintf(stream,"%s     StereoCameraCalibrationFile:      %s\n", first, StereoCameraCalibrationFile);
        fprintf(stream,"%s     navigation_specified:             %d\n", first, navigation_specified);
        fprintf(stream,"%s     NavigationFile:                   %s\n", first, NavigationFile);
        fprintf(stream,"%s     tide_specified:                   %d\n", first, tide_specified);
        fprintf(stream,"%s     TideFile:                         %s\n", first, TideFile);
        fprintf(stream,"%s     imagequality_specified:           %d\n", first, imagequality_specified);
        fprintf(stream,"%s     ImageQualityFile:                 %s\n", first, ImageQualityFile);
        fprintf(stream,"%s     imagequality_initialized:         %d\n", first, imagequality_initialized);
        fprintf(stream,"%s     ImageQualityFile:                 %s\n", first, ImageQualityFile);
        fprintf(stream,"%s     imageQualityThreshold:            %f\n", first, imageQualityThreshold);
        fprintf(stream,"%s     imageQualityFilterLength:         %f\n", first, imageQualityFilterLength);
        fprintf(stream,"%s     control.use_topography:           %d\n", first, control.use_topography);
        fprintf(stream,"%s     TopographyGridFile:               %s\n", first, TopographyGridFile);
        }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
        }

    /* Load topography grid if desired */
    if (control.use_topography)
        {
        status = mb_topogrid_init(verbose, TopographyGridFile, &lonflip, &control.topogrid_ptr, &error);
        if (error != MB_ERROR_NO_ERROR)
            {
            mb_error(verbose,error,&message);
            fprintf(stderr,"\nMBIO Error loading topography grid: %s\n%s\n",TopographyGridFile,message);
            fprintf(stderr,"\nProgram <%s> Terminated\n",program_name);
            mb_memory_clear(verbose, &error);
            exit(error);
            }
        }

    /* Initialize image correction tables for each thread */

    /* Create an an output image correction table in the processing structure for each
        processing thread. The tables will be combined after all processing is complete. */

    const int corr_table_dims[3] = {control.ncorr_x, control.ncorr_y, control.ncorr_z};
    for (int ithread = 0; ithread < numThreads; ithread++) {
        processPars[ithread].corr_table_y[0] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_cb[0] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_cr[0] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_count[0] = Mat(3, corr_table_dims, CV_32SC(1), Scalar(0));
        processPars[ithread].corr_table_y[1] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_cr[1] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_cb[1] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
        processPars[ithread].corr_table_count[1] = Mat(3, corr_table_dims, CV_32SC(1), Scalar(0));
    }

    /* loop over the list of input images
       - this can be a list of single images or stereo images
       - the images must have navigation and attitude
       - the images must be associated with a camera model, mono or stereo */

    /* open imagelist file */
    void *imagelist_ptr = NULL;
    status = mb_imagelist_open(verbose, &imagelist_ptr, ImageListFile, &error);
    if (error != MB_ERROR_NO_ERROR)
        {
        mb_error(verbose,error,&message);
        fprintf(stderr,"\nMBIO Error opening imagelist: %s\n%s\n", ImageListFile, message);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        mb_memory_clear(verbose, &error);
        exit(error);
        }

    /* loop over single images or stereo pairs in the imagelist file */
    npairs = 0;
    nimages = 0;
    int imageStatus = MB_IMAGESTATUS_NONE;
    double image_quality = 0.0;
    mb_path dpath;
    unsigned int numThreadsSet = 0;
    fprintf(stderr,"About to read ImageListFile: %s\n", ImageListFile);

    while ((status = mb_imagelist_read(verbose, imagelist_ptr, &imageStatus,
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &right_time_d,
                                &left_gain, &right_gain,
                                &left_exposure, &right_exposure, &error)) == MB_SUCCESS) {

        /* handle parameter statements embedded in the recursive imagelist structure */
        if (imageStatus == MB_IMAGESTATUS_PARAMETER) {
            currentimages = 0;
            mb_path tmp;

            /* A parameter change has been encountered while parsing the imagelist
                structure. If any threads have been started, these must be completed
                using the old parameters before parsing any changes. So, wait for
                all current threads to finish using join() before parsing the
                parameter change */
            if (numThreadsSet > 0) {
                for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
                  /* join the thread (wait until it completes) */
                  mbgetphotocorrectionThreads[ithread].join();
                }
                numThreadsSet = 0;
            }

            fprintf(stream, "  ->Processing parameter: %s\n",imageLeftFile);

            /* fov-fudgefactor */
            if (strncmp(imageLeftFile, "--fov-fudgefactor=", 18) == 0) {
                if (sscanf(imageLeftFile, "--fov-fudgefactor=%lf", &control.fov_fudgefactor) == 1) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameter reset: fov-fudgefactor: %f\n", control.fov_fudgefactor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameter: fov_fudgefactor:%f\n", control.fov_fudgefactor);
                }
            }

            /* trim */
            else if (strncmp(imageLeftFile, "--trim=", 7) == 0) {
                if (sscanf(imageLeftFile, "--trim=%d", &control.trimPixels) == 1) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameter reset: trimPixels:%d\n",
                            control.trimPixels);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameter: trimPixels:%d\n",
                            control.trimPixels);
                }
            }

            /* reference-gain */
            else if (strncmp(imageLeftFile, "--reference-gain=", 17) == 0) {
                if (sscanf(imageLeftFile,"--reference-gain=%lf", &control.reference_gain) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: reference_gain:%f\n",
                            control.reference_gain);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: reference_gain:%f\n",
                            control.reference_gain);
                }
            }

            /* reference-exposure */
            else if (strncmp(imageLeftFile, "--reference-exposure=", 21) == 0) {
                if (sscanf(imageLeftFile,"--reference-exposure=%lf", &control.reference_exposure) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: reference_exposure:%f\n",
                            control.reference_exposure);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: reference_exposure:%f\n",
                            control.reference_exposure);
                }
            }

            /* platform-file */
            else if (strncmp(imageLeftFile, "--platform-file=", 16) == 0) {
                if (sscanf(imageLeftFile, "--platform-file=%s", tmp) == 1) {
                	if (strlen(imageRightFile) > 0) {
                		strcpy(PlatformFile, imageRightFile);
                		strcat(PlatformFile, "/");
                		strcat(PlatformFile, tmp);
                	}
                	else {
                    	strcpy(PlatformFile, tmp);
                    }
                    platform_specified = true;
                }
            }

            /* camera-sensor */
            else if (strncmp(imageLeftFile, "--camera-sensor=", 16) == 0) {
                if (sscanf(imageLeftFile,"--camera-sensor=%d", &camera_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: camera_sensor:%d\n", camera_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: camera_sensor:%d\n", camera_sensor);
                }
            }

            /* nav-sensor */
            else if (strncmp(imageLeftFile, "--nav-sensor=", 13) == 0) {
                if (sscanf(imageLeftFile,"--nav-sensor=%d", &nav_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: nav_sensor:%d\n", nav_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: nav_sensor:%d\n", nav_sensor);
                }
            }

            /* sensordepth-sensor */
            else if (strncmp(imageLeftFile, "--sensordepth-sensor=", 16) == 0) {
                if (sscanf(imageLeftFile,"--sensordepth-sensor=%d", &sensordepth_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: sensordepth_sensor:%d\n", sensordepth_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: sensordepth_sensor:%d\n", sensordepth_sensor);
                }
            }

            /* heading-sensor */
            else if (strncmp(imageLeftFile, "--heading-sensor=", 16) == 0) {
                if (sscanf(imageLeftFile,"--heading-sensor=%d", &heading_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: heading_sensor:%d\n", heading_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: heading_sensor:%d\n", heading_sensor);
                }
            }

            /* altitude-sensor */
            else if (strncmp(imageLeftFile, "--altitude-sensor=", 16) == 0) {
                if (sscanf(imageLeftFile,"--altitude-sensor=%d", &altitude_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: altitude_sensor:%d\n", altitude_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: altitude_sensor:%d\n", altitude_sensor);
                }
            }

            /* attitude-sensor */
            else if (strncmp(imageLeftFile, "--attitude-sensor=", 16) == 0) {
                if (sscanf(imageLeftFile,"--attitude-sensor=%d", &attitude_sensor) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: attitude_sensor:%d\n", attitude_sensor);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: attitude_sensor:%d\n", attitude_sensor);
                }
            }

            /* use-left-camera */
            else if (strncmp(imageLeftFile, "--use-left-camera", 17) == 0) {
                use_camera_mode = MBPM_USE_LEFT;
            }

            /* use-right-camera */
            else if (strncmp(imageLeftFile, "--use-right-camera", 18) == 0) {
                use_camera_mode = MBPM_USE_RIGHT;
            }

            /* use-both-cameras */
            else if (strncmp(imageLeftFile, "--use-both-cameras", 18) == 0) {
                use_camera_mode = MBPM_USE_STEREO;
            }

            /* calibration-file */
            else if (strncmp(imageLeftFile, "--calibration-file=", 19) == 0) {
                if (sscanf(imageLeftFile, "--calibration-file=%s", tmp) == 1) {
                	if (strlen(imageRightFile) > 0) {
                		strcpy(StereoCameraCalibrationFile, imageRightFile);
                		strcat(StereoCameraCalibrationFile, "/");
                		strcat(StereoCameraCalibrationFile, tmp);
                	}
                	else {
                    	strcpy(StereoCameraCalibrationFile, tmp);
                    }
                    calibration_specified = true;
                }
            }

            /* navigation-file */
            else if (strncmp(imageLeftFile, "--navigation-file=", 18) == 0) {
                if (sscanf(imageLeftFile, "--navigation-file=%s", tmp) == 1) {
                	if (strlen(imageRightFile) > 0) {
                		strcpy(NavigationFile, imageRightFile);
                		strcat(NavigationFile, "/");
                		strcat(NavigationFile, tmp);
                	}
                	else {
                    	strcpy(NavigationFile, tmp);
                    }
                    navigation_specified = true;
                }
            }

            /* tide-file */
            else if (strncmp(imageLeftFile, "--tide-file=", 12) == 0) {
                if (sscanf(imageLeftFile, "--tide-file=%s", tmp) == 1) {
                	if (strlen(imageRightFile) > 0) {
                		strcpy(TideFile, imageRightFile);
                		strcat(TideFile, "/");
                		strcat(TideFile, tmp);
                	}
                	else {
                    	strcpy(TideFile, tmp);
                    }
                    tide_specified = true;
                }
            }

            /* image-quality-file */
            else if (strncmp(imageLeftFile, "--image-quality-file=", 21) == 0) {
                if (sscanf(imageLeftFile, "--image-quality-file=%s", tmp) == 1) {
                	if (strlen(imageRightFile) > 0) {
                		strcpy(ImageQualityFile, imageRightFile);
                		strcat(ImageQualityFile, "/");
                		strcat(ImageQualityFile, tmp);
                	}
                	else {
                    	strcpy(ImageQualityFile, tmp);
                    }
                    imagequality_specified = true;
                }
            }

            /* image-quality-threshold */
            else if (strncmp(imageLeftFile, "--image-quality-threshold=", 26) == 0) {
                if (sscanf(imageLeftFile,"--image-quality-threshold=%lf", &imageQualityThreshold) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: imageQualityThreshold:%f\n",
                            imageQualityThreshold);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: imageQualityThreshold:%f\n",
                            imageQualityThreshold);
                }
            }

            /* image-quality-filter-length */
            else if (strncmp(imageLeftFile, "--image-quality-filter-length=", 30) == 0) {
                if (sscanf(imageLeftFile,"--image-quality-filter-length=%lf", &imageQualityFilterLength) == 1 ) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameters reset: imageQualityFilterLength:%f\n",
                            imageQualityFilterLength);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameters: imageQualityFilterLength:%f\n",
                            imageQualityFilterLength);
                }
            }

        }
        else if (imageStatus == MB_IMAGESTATUS_STEREO) {
          if (use_camera_mode == MBPM_USE_STEREO) {
            npairs++;
            nimages += 2;
            currentimages = 2;
          } else if (use_camera_mode == MBPM_USE_LEFT) {
            image_camera = MBPM_CAMERA_LEFT;
            currentimages = 1;
            nimages++;
          } else if (use_camera_mode == MBPM_USE_RIGHT) {
            image_camera = MBPM_CAMERA_RIGHT;
            currentimages = 1;
            nimages++;
          }
        } else if (imageStatus == MB_IMAGESTATUS_LEFT) {
          if (use_camera_mode == MBPM_USE_LEFT) {
            image_camera = MBPM_CAMERA_LEFT;
            currentimages = 1;
            nimages++;
          } else {
            currentimages = 0;
          }
        } else if (imageStatus == MB_IMAGESTATUS_RIGHT) {
          if (use_camera_mode == MBPM_USE_RIGHT) {
            image_camera = MBPM_CAMERA_RIGHT;
            currentimages = 1;
            nimages++;
          } else {
            currentimages = 0;
          }
        } else if (imageStatus == MB_IMAGESTATUS_SINGLE) {
          if (use_camera_mode == MBPM_USE_LEFT) {
            image_camera = MBPM_CAMERA_LEFT;
            currentimages = 1;
            nimages++;
          }
          else if (use_camera_mode == MBPM_USE_RIGHT) {
            image_camera = MBPM_CAMERA_RIGHT;
            currentimages = 1;
            nimages++;
          } else {
            currentimages = 0;
          }
        }

        /* process any images */
        for (int iimage=0;iimage<currentimages;iimage++) {
            double time_d;
            double camera_navlon;
            double camera_navlat;
            double camera_sensordepth;
            double camera_heading;
            double camera_roll;
            double camera_pitch;
            double image_gain;
            double image_exposure;

            /* If this is the first image processed since a platform model has
                been specified, then load and initialize the platform model,
                along with any specifications of ancilliary data source sensors. We assume that all
                subsequent images derive from the same camera rig and have the same
                dimensions. */
            if (platform_specified) {
                if (platform_initialized && platform != NULL) {
                    status = mb_platform_deall(verbose, (void **)&platform, &error);
                    platform_initialized = false;
                }
                if (mb_platform_read(verbose, PlatformFile, (void **)&platform, &error) == MB_SUCCESS) {
                    fprintf(stream, "\nRead platform model from: %s\n", PlatformFile);
                    platform_specified = false;
                    platform_initialized = true;
                }
                else {
                    error = MB_ERROR_OPEN_FAIL;
                    fprintf(stderr,"\nUnable to open and parse platform file: %s\n", PlatformFile);
                    fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
                    mb_memory_clear(verbose, &error);
                    exit(error);
                }

                /* reset data sources according to commands */
                if (nav_sensor >= 0)
                    platform->source_position = nav_sensor;
                if (sensordepth_sensor >= 0)
                    platform->source_depth = sensordepth_sensor;
                if (heading_sensor >= 0)
                    platform->source_heading = heading_sensor;
                if (attitude_sensor >= 0)
                    {
                    platform->source_rollpitch = attitude_sensor;
                    platform->source_heave = attitude_sensor;
                    }

                /* get sensor structures */
                if (platform->source_bathymetry >= 0)
                    sensor_bathymetry = &(platform->sensors[platform->source_bathymetry]);
                if (platform->source_backscatter >= 0)
                    sensor_backscatter = &(platform->sensors[platform->source_backscatter]);
                if (platform->source_position >= 0)
                    sensor_position = &(platform->sensors[platform->source_position]);
                if (platform->source_depth >= 0)
                    sensor_depth = &(platform->sensors[platform->source_depth]);
                if (platform->source_heading >= 0)
                    sensor_heading = &(platform->sensors[platform->source_heading]);
                if (platform->source_rollpitch >= 0)
                    sensor_rollpitch = &(platform->sensors[platform->source_rollpitch]);
                if (platform->source_heave >= 0)
                    sensor_heave = &(platform->sensors[platform->source_heave]);
                if (camera_sensor < 0) {
                    for (int isensor=0;isensor<platform->num_sensors;isensor++) {
                        if (platform->sensors[isensor].type == MB_SENSOR_TYPE_CAMERA_STEREO) {
                            camera_sensor = isensor;
                        }
                    }
                }
                if (camera_sensor >= 0)
                    sensor_camera = &(platform->sensors[camera_sensor]);

                if (verbose > 0)
                    fprintf(stream, "    Survey platform model read from: %s\n", PlatformFile);
            }
            if (!platform_initialized) {
                fprintf(stderr,"\nNo platform model file specified, either on command line or in imagelist structure...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                error = MB_ERROR_BAD_PARAMETER;
                mb_memory_clear(verbose, &error);
                exit(error);
            }

            /* if newly specified load camera calibration model */
            if (calibration_specified) {
                load_calibration(verbose, StereoCameraCalibrationFile, &control, &error);
                calibration_initialized = true;
                calibration_specified = false;
                fprintf(stream, "    Stereo camera calibration model read from: %s\n", StereoCameraCalibrationFile);
            }
            if (!calibration_initialized) {
                fprintf(stderr,"\nNo camera calibration file specified, either on command line or in imagelist structure...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                error = MB_ERROR_BAD_PARAMETER;
                mb_memory_clear(verbose, &error);
                exit(error);
            }

            /* read in navigation if desired */
            if (navigation_specified) {
                load_navigation(verbose, NavigationFile, lonflip,
                                &nnav, &ntime, &nlon, &nlat, &nheading, &nspeed, &ndraft,
                                &nroll, &npitch, &nheave, &error);
                if (nnav > 0) {
                    fprintf(stream,"\nRead %d navigation records read %s\n", nnav, NavigationFile);
                    navigation_initialized = true;
                    navigation_specified = false;
                }
            }
            if (!navigation_initialized) {
                fprintf(stderr,"\nNo navigation file specified, either on command line or in imagelist structure...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                error = MB_ERROR_BAD_PARAMETER;
                mb_memory_clear(verbose, &error);
                exit(error);
            }

            /* read in tide if desired */
            if (tide_specified) {
                load_tide(verbose, TideFile, &ntide, &ttime, &ttide, &error);
                if (ntide > 0) {
                    tide_initialized = true;
                    fprintf(stream,"\nRead %d tide records from %s\n", ntide, TideFile);
                } else {
                    tide_initialized = false;
                }
                tide_specified = false;
            }

            /* read in image quality if desired */
            if (imagequality_specified) {
                load_image_quality(verbose, ImageQualityFile, &nquality, &qtime, &qquality, &error);
                if (nquality > 1) {
                    imagequality_initialized = true;
                    fprintf(stream,"    Read %d image quality records from %s\n", nquality, ImageQualityFile);
                }
                else {
                    imagequality_initialized = false;
                }
                imagequality_specified = false;
            }

            /* set camera for stereo image */
            if (currentimages == 2) {
                if (iimage == MBPM_CAMERA_LEFT) {
                    image_camera = MBPM_CAMERA_LEFT;
                }
                else {
                    image_camera = MBPM_CAMERA_RIGHT;
                }
            }

            /* check to see if this image should be used */
            use_this_image = false;
            if (image_camera == MBPM_CAMERA_LEFT
                && (use_camera_mode == MBPM_USE_LEFT || use_camera_mode == MBPM_USE_STEREO)) {
                time_d = left_time_d;
                image_gain = left_gain;
                image_exposure = left_exposure;
                strcpy(imageFile, imageLeftFile);
                use_this_image = true;
            }
            else if (image_camera == MBPM_CAMERA_RIGHT
                && (use_camera_mode == MBPM_USE_RIGHT || use_camera_mode == MBPM_USE_STEREO)) {
                time_d = right_time_d;
                image_gain = right_gain;
                image_exposure = right_exposure;
                strcpy(imageFile, imageRightFile);
                use_this_image = true;
            }

            /* check imageQuality value against threshold to see if this image should be used */
            if (use_this_image && imagequality_initialized) {
                if (nquality > 1) {
                    intstat = mb_linear_interp(verbose, qtime-1, qquality-1, nquality,
                                                time_d, &image_quality, &iqtime, &error);
                }
                if (image_quality < imageQualityThreshold) {
                    use_this_image = false;
                }
            }

            /* check for valid navigation */
            if (use_this_image) {
                if (nnav > 0 && time_d >= ntime[0] && time_d <= ntime[nnav-1]) {
                    /* get navigation for this image */
                    intstat = mb_linear_interp_longitude(verbose,
                            ntime-1, nlon-1,
                            nnav, time_d, &navlon, &itime,
                            &error);
                    intstat = mb_linear_interp_latitude(verbose,
                            ntime-1, nlat-1,
                            nnav, time_d, &navlat, &itime,
                            &error);
                }
                else {
                    use_this_image = false;
                }
            }

            /* calculate camera pose */
            if (use_this_image) {
                intstat = mb_linear_interp_heading(verbose,
                        ntime-1, nheading-1,
                        nnav, time_d, &heading, &itime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nspeed-1,
                        nnav, time_d, &speed, &itime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, ndraft-1,
                        nnav, time_d, &draft, &itime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nroll-1,
                        nnav, time_d, &roll, &itime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, npitch-1,
                        nnav, time_d, &pitch, &itime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nheave-1,
                        nnav, time_d, &heave, &itime,
                        &error);
                if (heading < 0.0)
                    heading += 360.0;
                else if (heading > 360.0)
                    heading -= 360.0;
                sensordepth = draft + heave;

                /* get tide for this image */
                tide = 0.0;
                if (ntide > 1) {
                    intstat = mb_linear_interp(verbose,
                            ttime-1, ttide-1,
                            ntide, time_d, &tide, &iitime,
                            &error);
                }
                sensordepth = sensordepth - tide;

                /* calculate target sensor position */
                status = mb_platform_position (verbose, (void *)platform,
                                camera_sensor, image_camera,
                                navlon, navlat, sensordepth,
                                heading, roll, pitch,
                                &camera_navlon, &camera_navlat, &camera_sensordepth,
                                &error);
                status = mb_platform_orientation_target (verbose, (void *)platform,
                                camera_sensor, image_camera,
                                heading, roll, pitch,
                                &camera_heading, &camera_roll, &camera_pitch,
                                &error);
            }

            // Start processing thread
            if (use_this_image) {

                /* If this is the first image processed, and a camera model has
                    been specified, then read the image here and
                    initialize the use of the camera model. We assume that all
                    images derive from the same camera rig and have the same
                    dimensions. */
                if (!undistort_initialized) {
                    undistort_initialized = true;
                    Mat imageFirst = imread(imageFile);
                    if (!imageFirst.empty()) {
                        control.imageSize = imageFirst.size();
                        calibrationMatrixValues(control.cameraMatrix[0], control.imageSize,
                                    control.SensorWidthMm, control.SensorHeightMm,
                                    control.fovx[0], control.fovy[0], control.focalLength[0],
                                    control.principalPoint[0], control.aspectRatio[0]);
                        calibrationMatrixValues(control.cameraMatrix[1], control.imageSize,
                                    control.SensorWidthMm, control.SensorHeightMm,
                                    control.fovx[1], control.fovy[1], control.focalLength[1],
                                    control.principalPoint[1], control.aspectRatio[1]);

                        /* define correction table dimensions */
                        control.corr_xmin = 0;
                        control.corr_xmax = imageFirst.cols;
                        control.corr_ymin = 0;
                        control.corr_ymax = imageFirst.rows;
                        control.bin_dx = imageFirst.cols / (control.ncorr_x - 1);
                        control.bin_dy = imageFirst.rows / (control.ncorr_y - 1);
                        control.bin_dz = (control.corr_zmax - control.corr_zmin) / (control.ncorr_z - 1);

                        if (verbose > 0) {
                            fprintf(stderr,"\nLeft Camera Characteristics:\n");
                            fprintf(stderr,"  Image width (pixels):         %d\n", control.imageSize.width);
                            fprintf(stderr,"  Image height (pixels):        %d\n", control.imageSize.height);
                            fprintf(stderr,"  Sensor width (mm):            %f\n", control.SensorWidthMm);
                            fprintf(stderr,"  Sensor height (mm):           %f\n", control.SensorHeightMm);
                            fprintf(stderr,"  Horizontal field of view:     %f\n", control.fovx[0]);
                            fprintf(stderr,"  Vertical field of view:       %f\n", control.fovy[0]);
                            fprintf(stderr,"  Focal length (sensor pixels): %f\n", control.focalLength[0]);
                            fprintf(stderr,"  Focal length (mm):            %f\n", control.focalLength[0] * control.SensorCellMm);
                            fprintf(stderr,"  Principal point x:            %f\n", control.principalPoint[0].x);
                            fprintf(stderr,"  Principal point y:            %f\n", control.principalPoint[0].y);
                            fprintf(stderr,"  Principal point x (pixels):   %f\n", control.principalPoint[0].x / control.SensorCellMm);
                            fprintf(stderr,"  Principal point y (pixels):   %f\n", control.principalPoint[0].y / control.SensorCellMm);
                            fprintf(stderr,"  Aspect ratio:                 %f\n", control.aspectRatio[0]);
                            fprintf(stderr,"\nRight Camera Characteristics:\n");
                            fprintf(stderr,"  Image width (pixels):         %d\n", control.imageSize.width);
                            fprintf(stderr,"  Image height (pixels):        %d\n", control.imageSize.height);
                            fprintf(stderr,"  Sensor width (mm):            %f\n", control.SensorWidthMm);
                            fprintf(stderr,"  Sensor height (mm):           %f\n", control.SensorHeightMm);
                            fprintf(stderr,"  Horizontal field of view:     %f\n", control.fovx[1]);
                            fprintf(stderr,"  Vertical field of view:       %f\n", control.fovy[1]);
                            fprintf(stderr,"  Focal length (sensor pixels): %f\n", control.focalLength[1]);
                            fprintf(stderr,"  Focal length (mm):            %f\n", control.focalLength[1] * control.SensorCellMm);
                            fprintf(stderr,"  Principal point x (mm):       %f\n", control.principalPoint[1].x);
                            fprintf(stderr,"  Principal point y (mm):       %f\n", control.principalPoint[1].y);
                            fprintf(stderr,"  Principal point x (pixels):   %f\n", control.principalPoint[1].x / control.SensorCellMm);
                            fprintf(stderr,"  Principal point y (pixels):   %f\n", control.principalPoint[1].y / control.SensorCellMm);
                            fprintf(stderr,"  Aspect ratio:                 %f\n", control.aspectRatio[1]);
                            fprintf(stderr,"\nCorrection Table Dimensions:\n");
                            fprintf(stderr,"  X Dimensions (n min max dx):  %d %f %f %f\n", control.ncorr_x, control.corr_xmin, control.corr_xmax, control.bin_dx);
                            fprintf(stderr,"  Y Dimensions (n min max dy):  %d %f %f %f\n", control.ncorr_y, control.corr_ymin, control.corr_ymax, control.bin_dy);
                            fprintf(stderr,"  Z Dimensions (n min max dz):  %d %f %f %f\n\n", control.ncorr_z, control.corr_zmin, control.corr_zmax, control.bin_dz);
                        }

                        imageFirst.release();
                    }
                }

                /* copy parameters to current processing parameter structure */
                processPars[numThreadsSet].thread = numThreadsSet;
                strcpy(processPars[numThreadsSet].imageFile, imageFile);
                processPars[numThreadsSet].image_count = nimages - currentimages + iimage;
                processPars[numThreadsSet].image_camera = image_camera;
                processPars[numThreadsSet].image_quality = image_quality;
                processPars[numThreadsSet].image_gain = image_gain;
                processPars[numThreadsSet].image_exposure = image_exposure;
                processPars[numThreadsSet].time_d = time_d;
                processPars[numThreadsSet].camera_navlon = camera_navlon;
                processPars[numThreadsSet].camera_navlat = camera_navlat;
                processPars[numThreadsSet].camera_sensordepth = camera_sensordepth;
                processPars[numThreadsSet].camera_heading = camera_heading;
                processPars[numThreadsSet].camera_roll = camera_roll;
                processPars[numThreadsSet].camera_pitch = camera_pitch;

                mbgetphotocorrectionThreads[numThreadsSet]
                    = std::thread(process_image, verbose, &processPars[numThreadsSet], &control,
                                    &thread_status[numThreadsSet], &thread_error[numThreadsSet]);
                numThreadsSet++;
            }

            /* If full set of threads has been started, wait to join them all and then reset */
            if (numThreadsSet == numThreads) {
                for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
                  /* join the thread (wait until it completes) */
                  mbgetphotocorrectionThreads[ithread].join();
                }
                numThreadsSet = 0;
            }
        }
    }

    /* If any threads has been started but not joined, wait to join them all and then reset */
    if (numThreadsSet > 0) {
        for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
          /* join the thread (wait until it completes) */
          mbgetphotocorrectionThreads[ithread].join();
        }
        numThreadsSet = 0;
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);
    fprintf(stderr, "Imagelist structure contained %d images and %d image pairs\n", nimages, npairs);

    /* if more than one thread was used, combine the correction tables into the image from
        the first thread */
    if (numThreads > 1) {
        fprintf(stderr, "\n");
        for (int ithread = 1; ithread < numThreads; ithread++) {
            fprintf(stderr, "Merging correction table from thread %d of %d\n", ithread, numThreads);
            for (int i=0;i<control.ncorr_x;i++) {
                for (int j=0;j<control.ncorr_y;j++) {
                    for (int k=0;k<control.ncorr_z;k++) {
                        if (processPars[ithread].corr_table_count[0].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[0].at<float>(i, j, k) += processPars[ithread].corr_table_y[0].at<float>(i, j, k);
                            processPars[0].corr_table_cr[0].at<float>(i, j, k) += processPars[ithread].corr_table_cr[0].at<float>(i, j, k);
                            processPars[0].corr_table_cb[0].at<float>(i, j, k) += processPars[ithread].corr_table_cb[0].at<float>(i, j, k);
                            processPars[0].corr_table_count[0].at<int>(i, j, k) += processPars[ithread].corr_table_count[0].at<int>(i, j, k);
                        }
                        if (processPars[ithread].corr_table_count[1].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[1].at<float>(i, j, k) += processPars[ithread].corr_table_y[1].at<float>(i, j, k);
                            processPars[0].corr_table_cr[1].at<float>(i, j, k) += processPars[ithread].corr_table_cr[1].at<float>(i, j, k);
                            processPars[0].corr_table_cb[1].at<float>(i, j, k) += processPars[ithread].corr_table_cb[1].at<float>(i, j, k);
                            processPars[0].corr_table_count[1].at<int>(i, j, k) += processPars[ithread].corr_table_count[1].at<int>(i, j, k);
                        }
                    }
                }
            }
        processPars[ithread].corr_table_y[0].release();
        processPars[ithread].corr_table_cr[0].release();
        processPars[ithread].corr_table_cb[0].release();
        processPars[ithread].corr_table_count[0].release();
        processPars[ithread].corr_table_y[1].release();
        processPars[ithread].corr_table_cr[1].release();
        processPars[ithread].corr_table_cb[1].release();
        processPars[ithread].corr_table_count[1].release();
        }
    }

    /* process final camera correction table */
    int count_max = 0;
    for (int i=0;i<control.ncorr_x;i++) {
        for (int j=0;j<control.ncorr_y;j++) {
            for (int k=0;k<control.ncorr_z;k++) {
                count_max = MAX(count_max, processPars[0].corr_table_count[0].at<int>(i, j, k));
                count_max = MAX(count_max, processPars[0].corr_table_count[1].at<int>(i, j, k));
            }
        }
    }
    int count_min = MIN(count_max / 20, MBPM_MIN_VALID_COUNT);
    for (int i=0;i<control.ncorr_x;i++) {
        for (int j=0;j<control.ncorr_y;j++) {
            for (int k=0;k<control.ncorr_z;k++) {
                if (processPars[0].corr_table_count[0].at<int>(i, j, k) > count_min) {
                    processPars[0].corr_table_y[0].at<float>(i, j, k) /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                    processPars[0].corr_table_cr[0].at<float>(i, j, k) /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                    processPars[0].corr_table_cb[0].at<float>(i, j, k) /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                } else {
                    processPars[0].corr_table_y[0].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_cr[0].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_cb[0].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_count[0].at<int>(i, j, k) = 0;
                }
                if (processPars[0].corr_table_count[1].at<int>(i, j, k) > count_min) {
                    processPars[0].corr_table_y[1].at<float>(i, j, k) /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                    processPars[0].corr_table_cr[1].at<float>(i, j, k) /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                    processPars[0].corr_table_cb[1].at<float>(i, j, k) /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                } else {
                    processPars[0].corr_table_y[1].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_cr[1].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_cb[1].at<float>(i, j, k) = 0.0;
                    processPars[0].corr_table_count[1].at<int>(i, j, k) = 0;
                }
            }
        }
    }

    /* extrapolate values over the unoccupied portions of the correction table
        first horizontally then vertically */
    bool done = false;
    while (!done) {
        int num_changes = 0;
        for (int k=0;k<control.ncorr_z;k++) {
            for (int j=0;j<control.ncorr_y;j++) {
                for (int i=0;i<control.ncorr_x;i++) {
                    if (processPars[0].corr_table_y[0].at<float>(i, j, k) == 0.0) {
                        processPars[0].corr_table_count[0].at<int>(i, j, k) = 0;
                        for (int jj=MAX(0,j-1);jj<=MIN(control.ncorr_y-1,j+1);jj++) {
                            for (int ii=MAX(0,i-1);ii<=MIN(control.ncorr_x-1,i+1);ii++) {
                                if (!(ii == i && jj == j) && processPars[0].corr_table_y[0].at<float>(ii, jj, k) > 0.0) {
                                    processPars[0].corr_table_y[0].at<float>(i, j, k)
                                        += processPars[0].corr_table_y[0].at<float>(ii, jj, k);
                                    processPars[0].corr_table_cr[0].at<float>(i, j, k)
                                        += processPars[0].corr_table_cr[0].at<float>(ii, jj, k);
                                    processPars[0].corr_table_cb[0].at<float>(i, j, k)
                                        += processPars[0].corr_table_cb[0].at<float>(ii, jj, k);
                                    processPars[0].corr_table_count[0].at<int>(i, j, k)++;
                                    num_changes++;
                                }
                            }
                        }
                        if (processPars[0].corr_table_count[0].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_cr[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_cb[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_count[0].at<int>(i, j, k) = 0;
                        }
                    }
                    if (processPars[0].corr_table_y[1].at<float>(i, j, k) == 0.0) {
                        processPars[0].corr_table_count[1].at<int>(i, j, k) = 0;
                        for (int jj=MAX(0,j-1);jj<=MIN(control.ncorr_y-1,j+1);jj++) {
                            for (int ii=MAX(0,i-1);ii<=MIN(control.ncorr_x-1,i+1);ii++) {
                                if (!(ii == i && jj == j) && processPars[0].corr_table_y[1].at<float>(ii, jj, k) > 0.0) {
                                    processPars[0].corr_table_y[1].at<float>(i, j, k)
                                        += processPars[0].corr_table_y[1].at<float>(ii, jj, k);
                                    processPars[0].corr_table_cr[1].at<float>(i, j, k)
                                        += processPars[0].corr_table_cr[1].at<float>(ii, jj, k);
                                    processPars[0].corr_table_cb[1].at<float>(i, j, k)
                                        += processPars[0].corr_table_cb[1].at<float>(ii, jj, k);
                                    processPars[0].corr_table_count[1].at<int>(i, j, k)++;
                                    num_changes++;
                                }
                            }
                        }
                        if (processPars[0].corr_table_count[1].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_cr[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_cb[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_count[1].at<int>(i, j, k) = 0;
                        }
                    }
                }
            }
        }
        if (num_changes == 0)
            done = true;
    }
    done = false;
    while (!done) {
        int num_changes = 0;
        for (int j=0;j<control.ncorr_y;j++) {
            for (int i=0;i<control.ncorr_x;i++) {
                for (int k=0;k<control.ncorr_z;k++) {
                    if (processPars[0].corr_table_y[0].at<float>(i, j, k) == 0.0) {
                        processPars[0].corr_table_count[0].at<int>(i, j, k) = 0;
                        for (int kk=MAX(0,k-1);kk<=MIN(control.ncorr_z-1,k+1);kk++) {
                            if (!(kk == k) && processPars[0].corr_table_y[0].at<float>(i, j, kk) > 0.0) {
                                processPars[0].corr_table_y[0].at<float>(i, j, k)
                                    += processPars[0].corr_table_y[0].at<float>(i, j, kk);
                                processPars[0].corr_table_cr[0].at<float>(i, j, k)
                                    += processPars[0].corr_table_cr[0].at<float>(i, j, kk);
                                processPars[0].corr_table_cb[0].at<float>(i, j, k)
                                    += processPars[0].corr_table_cb[0].at<float>(i, j, kk);
                                processPars[0].corr_table_count[0].at<int>(i, j, k)++;
                                num_changes++;
                            }
                        }
                        if (processPars[0].corr_table_count[0].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_cr[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_cb[0].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[0].at<int>(i, j, k);
                            processPars[0].corr_table_count[0].at<int>(i, j, k) = 0;
                        }
                    }
                    if (processPars[0].corr_table_y[1].at<float>(i, j, k) == 0.0) {
                        processPars[0].corr_table_count[1].at<int>(i, j, k) = 0;
                        for (int kk=MAX(0,k-1);kk<=MIN(control.ncorr_z-1,k+1);kk++) {
                            if (!(kk == k) && processPars[0].corr_table_y[1].at<float>(i, j, kk) > 0.0) {
                                processPars[0].corr_table_y[1].at<float>(i, j, k)
                                    += processPars[0].corr_table_y[1].at<float>(i, j, kk);
                                processPars[0].corr_table_cr[1].at<float>(i, j, k)
                                    += processPars[0].corr_table_cr[1].at<float>(i, j, kk);
                                processPars[0].corr_table_cb[1].at<float>(i, j, k)
                                    += processPars[0].corr_table_cb[1].at<float>(i, j, kk);
                                processPars[0].corr_table_count[1].at<int>(i, j, k)++;
                                num_changes++;
                            }
                        }
                        if (processPars[0].corr_table_count[1].at<int>(i, j, k) > 0) {
                            processPars[0].corr_table_y[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_cr[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_cb[1].at<float>(i, j, k)
                                /= processPars[0].corr_table_count[1].at<int>(i, j, k);
                            processPars[0].corr_table_count[1].at<int>(i, j, k) = 0;
                        }
                    }
                }
            }
        }
        if (num_changes == 0)
            done = true;
    }

    /* print out each correction table layer from lowest standoff to largest */
    fprintf(stderr, "\n---------------------\nCamera 0 Image Correction\n--------------------\n");
    for (int k=0;k<control.ncorr_z;k++) {
      fprintf(stderr, "Camera 0 Correction: Standoff %.3f meters +/- %.3f\n", k * control.bin_dz + control.corr_zmin, 0.5 * control.bin_dz);
        for (int j=0;j<control.ncorr_y;j++) {
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_y[0].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_cr[0].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_cb[0].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%7d ", processPars[0].corr_table_count[0].at<int>(i, j, k));
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n---------------------\nCamera 1 Image Correction\n--------------------\n");
    for (int k=0;k<control.ncorr_z;k++) {
      fprintf(stderr, "Camera 1 Correction: Standoff %.3f meters +/- %.3f\n", k * control.bin_dz + control.corr_zmin, 0.5 * control.bin_dz);
        for (int j=0;j<control.ncorr_y;j++) {
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_y[1].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_cr[1].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%5.1f ", processPars[0].corr_table_cb[1].at<float>(i, j, k));
            }
            fprintf(stderr, "   ");
            for (int i=0;i<control.ncorr_x;i++) {
                fprintf(stderr, "%7d ", processPars[0].corr_table_count[1].at<int>(i, j, k));
            }
            fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
    }

    /* Write out the ouput correction table */
    int corr_version = 3;
    Mat corr_bounds = Mat(3, 3, CV_32FC(1), Scalar(0.0));
    corr_bounds.at<float>(0, 0) = control.corr_xmin;
    corr_bounds.at<float>(0, 1) = control.corr_xmax;
    corr_bounds.at<float>(0, 2) = control.bin_dx;
    corr_bounds.at<float>(1, 0) = control.corr_ymin;
    corr_bounds.at<float>(1, 1) = control.corr_ymax;
    corr_bounds.at<float>(1, 2) = control.bin_dy;
    corr_bounds.at<float>(2, 0) = control.corr_zmin;
    corr_bounds.at<float>(2, 1) = control.corr_zmax;
    corr_bounds.at<float>(2, 2) = control.bin_dz;
    fstorage = FileStorage(ImageCorrectionFile, FileStorage::WRITE);
    if( fstorage.isOpened() ) {
        fstorage << "ImageCorrectionVersion" << corr_version
            << "ImageCorrectionBounds" << corr_bounds
            << "ImageCorrectionReferenceGain" << control.reference_gain
            << "ImageCorrectionReferenceExposure" << control.reference_exposure
            << "ImageCorrectionTableY1" << processPars[0].corr_table_y[0]
            << "ImageCorrectionTableCr1" << processPars[0].corr_table_cr[0]
            << "ImageCorrectionTableCb1" << processPars[0].corr_table_cb[0]
            << "ImageCorrectionTableY2" << processPars[0].corr_table_y[1]
            << "ImageCorrectionTableCr2" << processPars[0].corr_table_cr[1]
            << "ImageCorrectionTableCb2" << processPars[0].corr_table_cb[1];
        fstorage.release();
    }
    else
        cout << "Error: Cannot save the image correction tables\n";


    processPars[0].corr_table_y[0].release();
    processPars[0].corr_table_cr[0].release();
    processPars[0].corr_table_cb[0].release();
    processPars[0].corr_table_count[0].release();
    processPars[0].corr_table_y[1].release();
    processPars[0].corr_table_cr[1].release();
    processPars[0].corr_table_cb[1].release();
    processPars[0].corr_table_count[1].release();
    corr_bounds.release();

    /* deallocate topography grid array if necessary */
    if (control.use_topography)
        status = mb_topogrid_deall(verbose, &control.topogrid_ptr, &error);

    /* deallocate navigation arrays if necessary */
    if (navigation_specified)
        {
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ntime,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nlon,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nlat,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nheading,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nspeed,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ndraft,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nroll,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&npitch,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&nheave,&error);
        }

    /* deallocate tide arrays if necessary */
    if (ntide > 0)
        {
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ttime,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ttide,&error);
        ntide = 0;
        }

    /* deallocate image quality arrays if necessary */
    if (nquality > 0)
        {
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&qtime,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&qquality,&error);
        nquality = 0;
        }

    /* end it all */
    exit(status);

}
/*--------------------------------------------------------------------*/
