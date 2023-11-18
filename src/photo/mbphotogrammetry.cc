/*--------------------------------------------------------------------
 *    The MB-system:    mbphotogrammetry.cpp    11/12/2014
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
 * mbphotogrammetry generates bathymetry from stereo pair photographs
 * taken from a submerged survey platform.
 *
 * Author:    D. W. Caress
 * Date:    November 12, 2014
 *
 * Integrated into MB-System July 2020
 *
 */

/* standard include files */
#include <algorithm>
#include <ctime>
#include <ctype.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

/* MB-System include files */
extern "C"
{
#include "mb_define.h"
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_aux.h"
#include "mbsys_stereopair.h"
}

/* OpenCV include files */
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

std::string s;
std::stringstream out;

#define MBPG_ALLOC_SMALL_NUM    64
#define MBPG_ALLOC_LARGE_NUM    262144

#define MBPG_BIN_FILTER_MEAN    0
#define MBPG_BIN_FILTER_MEDIAN    1

// #define DEBUG 1
char program_name[] = "mbphotogrammetry";
char help_message[] = "mbphotogrammetry generates bathymetry from stereo pairs of photographs through photogrammetry.";
char usage_message[] = "mbphotogrammetry \n"
                        "\t--verbose\n"
                        "\t--help\n"
                        "\t--threads=nthreads\n"
                        "\t--show-images\n"
                        "\t--input=imagelist\n"
                        "\t--fov-fudgefactor=factor\n"
                        "\t--navigation-file=file\n"
                        "\t--survey-line-file=file\n"
                        "\t--tide-file=file\n"
                        "\t--image-quality-file=file\n"
                        "\t--image-quality-threshold=value\n"
                        "\t--image-quality-filter-length=value\n"
                        "\t--output=fileroot\n"
                        "\t--output-number-pairs=value\n"
                        "\t--camera-calibration-file=file\n"
                        "\t--calibration-file=file\n"
                        "\t--platform-file=platform.plf\n"
                        "\t--camera-sensor=camera_sensor_id\n"
                        "\t--nav-sensor=nav_sensor_id\n"
                        "\t--sensordepth-sensor=sensordepth_sensor_id\n"
                        "\t--heading-sensor=heading_sensor_id\n"
                        "\t--altitude-sensor=altitude_sensor_id\n"
                        "\t--attitude-sensor=attitude_sensor_id\n"
                        "\t--altitude-min=value\n"
                        "\t--altitude-max=value\n"
                        "\t--trim=value\n"
                        "\t--bin-size=value\n"
                        "\t--bin-filter=value (0=mean, 1=median)\n"
                        "\t--downsample=value\n"
                        "\t--algorithm=algorithm (bm, sgbm, hh)\n"
                        "\t--algorithm-pre-filter-cap=value\n"
                        "\t--algorithm-sad-window-size=value\n"
                        "\t--algorithm-smoothing-penalty-1=value\n"
                        "\t--algorithm-smoothing-penalty-2=value\n"
                        "\t--algorithm-min-disparity=value\n"
                        "\t--algorithm-number-disparities=value\n"
                        "\t--algorithm-uniqueness-ratio=value\n"
                        "\t--algorithm-speckle-window-size=value\n"
                        "\t--algorithm-speckle-range=value\n"
                        "\t--algorithm-disp-12-max-diff=value\n"
                        "\t--algorithm-texture-threshold=value\n";

/*--------------------------------------------------------------------*/

struct mbpg_process_struct {

    // input stereo pair and camera pose
    unsigned int thread;
    int pair_count;
    mb_path imageLeftFile;
    double image_left_time_d;
    double image_left_gain;
    double image_left_exposure;
    double image_left_amplitude;
    double image_left_quality;
    double image_left_navlon;
    double image_left_navlat;
    double image_left_sensordepth;
    double image_left_heading;
    double image_left_roll;
    double image_left_pitch;
    mb_path imageRightFile;
    double image_right_time_d;
    double image_right_gain;
    double image_right_exposure;
    double image_right_amplitude;
    double image_right_quality;
    double image_right_navlon;
    double image_right_navlat;
    double image_right_sensordepth;
    double image_right_heading;
    double image_right_roll;
    double image_right_pitch;
    double speed;
    
    // Output formt 251 data structure
    struct mbsys_stereopair_struct store;

};

enum { STEREO_BM=0, STEREO_SGBM=1, STEREO_HH=2 };
struct mbpg_control_struct {

    // Display images
    bool show_images;

    // Camera calibration model
    bool calibrationInitialized;
    bool photogrammetryInitialized;
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
    Rect    roi1, roi2;
    Size imageSize[2];
    double fovx[2];
    double fovy[2];
    double fov_fudgefactor;
    double focalLength[2];
    Point2d principalPoint[2];
    double aspectRatio[2];
    Mat     map11, map12, map21, map22;

    // Input image trimming
    int trimPixels;

    // Photogrammetry algorithm controls
    Ptr<StereoBM> bm;
    Ptr<StereoSGBM> sgbm;
    int algorithm = STEREO_SGBM;
    int preFilterCap = 4;
    int SADWindowSize = 5;
    int SmoothingPenalty1 = 600;
    int SmoothingPenalty2 = 2400;
    int minDisparity = -64;
    int numberOfDisparities = 192;
    int uniquenessRatio = 1;
    int speckleWindowSize = 150;
    int speckleRange = 2;
    int disp12MaxDiff = 10;
    double altitude_min = 1.0;
    double altitude_max = 5.0;
    double trim = 0.0;
    int bin_size = 1;
    int bin_filter = MBPG_BIN_FILTER_MEAN;
    int downsample = 1;
    int textureThreshold = 10;
    double min_disparity;
    double max_disparity;
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
    fprintf(stderr,"\nRead %d navigation records from %s\n", *nnav, NavigationFile);

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
    fprintf(stderr,"\nRead %d tide records from %s\n", *ntide, TideFile);

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
    fprintf(stderr,"\nRead %d image quality records from %s\n", *nquality, ImageQualityFile);

}
/*--------------------------------------------------------------------*/


void load_calibration(int verbose, mb_path StereoCameraCalibrationFile, struct mbpg_control_struct *control, int *error)
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
        control->calibrationInitialized = true;
    }
    else {
        fprintf(stderr,"\nUnable to read camera calibration file %s\n",
            StereoCameraCalibrationFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
        mb_memory_clear(verbose, error);
        exit(MB_ERROR_BAD_PARAMETER);
    }

    /* print out calibration information */
    cerr << endl;
    cerr << "Stereo camera calibration model read from: " << StereoCameraCalibrationFile << endl;
   if (verbose >= 1) {
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
        cerr << endl;
    }
}
/*--------------------------------------------------------------------*/
void process_stereopair(int verbose, struct mbpg_process_struct *process,
                  struct mbpg_control_struct *control, int *status, int *error)
{

    /* output stream (stdout if verbose <= 1, stderr if verbose > 1) */
    FILE *stream = NULL;
    if (verbose <= 1)
        stream = stdout;
    else
        stream = stderr;

    bool use_this_pair = true;

    /* Read the stereo pair */
    Mat img1 = imread(process->imageLeftFile, -1);
    Mat img2 = imread(process->imageRightFile, -1);
    if (img1.empty()) {
        fprintf(stderr,"Unable to read left file %s\n", process->imageLeftFile);
        use_this_pair = false;
    }
    else if (img2.empty()) {
        fprintf(stderr,"Unable to read right file %s\n", process->imageRightFile);
        use_this_pair = false;
    }
    else {
        control->imageSize[0] = img1.size();
        control->imageSize[1] = img2.size();
        if (control->imageSize[0] != control->imageSize[1]) {
          fprintf(stderr,"Right and left images not the same size: %d:%d != %d:%d\n",
                  control->imageSize[0].width, control->imageSize[0].height, control->imageSize[1].width, control->imageSize[1].height);
          use_this_pair = false;
        }
    }

    if (use_this_pair) {
        // The photogrammetry is calculated relative to the first camera (camera 0)
        // so we need to add the offset between the first camera and the navigated
        // center of the rig to individual sounding positions - the first
        // contribution is from the platform model and the second is from the
        // T vector of the stereo camera calibration. This offset is in the
        // frame of the camera rig, i.e. in acrosstrack and alongtrack distance.
        // Generally if the camera rig is calibrated then the platform model
        // has no offset between the two cameras, and so the calculation is
        // entirely from the translation or T vector of the stereo calibration.
        double camera_time_d = process->image_left_time_d;
        double camera_navlon = 0.5 * (process->image_left_navlon + process->image_right_navlon);
        double camera_navlat = 0.5 * (process->image_left_navlat + process->image_right_navlat);
        double camera_sensordepth = 0.5 * (process->image_left_sensordepth + process->image_right_sensordepth);
        double camera_heading = 0.5 * (process->image_left_heading + process->image_right_heading);
        double camera_roll = 0.5 * (process->image_left_roll + process->image_right_roll);
        double camera_pitch = 0.5 * (process->image_left_pitch + process->image_right_pitch);
        double headingx = sin(DTR * camera_heading);
        double headingy = cos(DTR * camera_heading);
        double mtodeglon;
        double mtodeglat;
        mb_coor_scale(verbose, camera_navlat, &mtodeglon, &mtodeglat);
        double pg_xtrack_offset = headingx * (process->image_left_navlon - camera_navlon) / mtodeglon
                                    + headingx * (process->image_left_navlon - camera_navlon) / mtodeglon;
        double pg_ltrack_offset = (process->image_left_navlat - camera_navlat) / mtodeglat;
        double pg_z_offset = process->image_left_sensordepth - camera_sensordepth;
        pg_z_offset = process->image_left_sensordepth - camera_sensordepth;
        pg_xtrack_offset += 0.5 * (control->T.at<double>(0));
        pg_ltrack_offset += 0.5 * (control->T.at<double>(1));
        pg_z_offset += 0.5 * (control->T.at<double>(2));

        Scalar avgPixelIntensityLeft = mean(img1);
        process->image_left_amplitude = avgPixelIntensityLeft.val[0];
        Scalar avgPixelIntensityRight = mean(img2);
        process->image_right_amplitude = avgPixelIntensityRight.val[0];

        /* initialize cameras, calibration, and stereo algorithm */
        if (!control->photogrammetryInitialized) {
            if (!control->calibrationInitialized) {
                fprintf(stderr,"\nNo stereo camera calibration has been loaded - aborting...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
                exit(1);
            }
            control->photogrammetryInitialized = true;

            /* set the desired disparity range */
            control->min_disparity = (control->Q.at<double>(3,3) + control->Q.at<double>(2,3) / control->altitude_max) / control->Q.at<double>(3,2);
            control->max_disparity = (control->Q.at<double>(3,3) + control->Q.at<double>(2,3) / control->altitude_min) / control->Q.at<double>(3,2);
            fprintf(stderr, "%s:%d:%s: Q.at<double>(2,3):%f Q.at<double>(3,2):%f min max disparity: %f %f\n",
                __FILE__, __LINE__, __func__, control->Q.at<double>(2,3), control->Q.at<double>(3,2), control->min_disparity, control->max_disparity);

            /* Set parameters for algorithms */
            if (control->algorithm == STEREO_BM) {
                fprintf(stderr, "%s:%d:%s: algorithm == STEREO_BM\n", __FILE__, __LINE__, __func__);
                control->bm = StereoBM::create();
                control->bm->setROI1(control->roi1);
                control->bm->setROI2(control->roi2);
                control->bm->setPreFilterCap(control->preFilterCap);
                control->bm->setBlockSize(control->SADWindowSize);
                control->bm->setMinDisparity(control->minDisparity);
                control->bm->setNumDisparities(control->numberOfDisparities);
                control->bm->setTextureThreshold(control->textureThreshold);
                control->bm->setUniquenessRatio(control->uniquenessRatio);
                control->bm->setSpeckleWindowSize(control->speckleWindowSize);
                control->bm->setSpeckleRange(control->speckleRange);
                control->bm->setDisp12MaxDiff(control->disp12MaxDiff);
            }
            else if (control->algorithm == STEREO_SGBM || control->algorithm == STEREO_HH) {
                fprintf(stderr, "%s:%d:%s: algorithm == STEREO_SGBM || algorithm == STEREO_HH\n", __FILE__, __LINE__, __func__);
                control->sgbm = StereoSGBM::create();
                if (control->algorithm == STEREO_SGBM) {
                    fprintf(stderr, "%s:%d:%s: algorithm == STEREO_SGBM\n", __FILE__, __LINE__, __func__);
                    control->sgbm->setMode( StereoSGBM::MODE_SGBM );
                }
                else { //if (algorithm == STEREO_HH) {
                    fprintf(stderr, "%s:%d:%s: algorithm == STEREO_HH\n", __FILE__, __LINE__, __func__);
                    control->sgbm->setMode( StereoSGBM::MODE_HH );
                }

                control->sgbm->setPreFilterCap(control->preFilterCap);
                control->sgbm->setBlockSize(control->SADWindowSize);
                control->sgbm->setP1(control->SmoothingPenalty1);
                control->sgbm->setP2(control->SmoothingPenalty2);
                control->sgbm->setMinDisparity(control->minDisparity);
                control->sgbm->setNumDisparities(control->numberOfDisparities);
                control->sgbm->setUniquenessRatio(control->uniquenessRatio);
                control->sgbm->setSpeckleWindowSize(control->speckleWindowSize);
                control->sgbm->setSpeckleRange(control->speckleRange);
                control->sgbm->setDisp12MaxDiff(control->speckleRange);
            } else {
                fprintf(stderr, "%s:%d:%s: no algorithm\n", __FILE__, __LINE__, __func__);
            }
            fprintf(stderr, "%s:%d:%s: algorithm set\n", __FILE__, __LINE__, __func__);

            /* set up rectification */
            calibrationMatrixValues(control->cameraMatrix[0], control->imageSize[0],
                        control->SensorWidthMm, control->SensorHeightMm,
                        control->fovx[0], control->fovy[0], control->focalLength[0],
                        control->principalPoint[0], control->aspectRatio[0]);
            calibrationMatrixValues(control->cameraMatrix[1], control->imageSize[1],
                        control->SensorWidthMm, control->SensorHeightMm,
                        control->fovx[1], control->fovy[1], control->focalLength[1],
                        control->principalPoint[1], control->aspectRatio[1]);
            if (verbose > 0) {
                fprintf(stderr,"\nLeft Camera Characteristics:\n");
                fprintf(stderr,"  Image width (pixels):         %d\n", control->imageSize[0].width);
                fprintf(stderr,"  Image height (pixels):        %d\n", control->imageSize[0].height);
                fprintf(stderr,"  Sensor width (mm):            %f\n", control->SensorWidthMm);
                fprintf(stderr,"  Sensor height (mm):           %f\n", control->SensorHeightMm);
                fprintf(stderr,"  Horizontal field of view:     %f\n", control->fovx[0]);
                fprintf(stderr,"  Vertical field of view:       %f\n", control->fovy[0]);
                fprintf(stderr,"  Focal length (sensor pixels): %f\n", control->focalLength[0]);
                fprintf(stderr,"  Focal length (mm):            %f\n", control->focalLength[0] * control->SensorCellMm);
                fprintf(stderr,"  Principal point x:            %f\n", control->principalPoint[0].x);
                fprintf(stderr,"  Principal point y:            %f\n", control->principalPoint[0].y);
                fprintf(stderr,"  Principal point x (pixels):   %f\n", control->principalPoint[0].x / control->SensorCellMm);
                fprintf(stderr,"  Principal point y (pixels):   %f\n", control->principalPoint[0].y / control->SensorCellMm);
                fprintf(stderr,"  Aspect ratio:                 %f\n", control->aspectRatio[0]);
                fprintf(stderr,"\nRight Camera Characteristics:\n");
                fprintf(stderr,"  Image width (pixels):         %d\n", control->imageSize[1].width);
                fprintf(stderr,"  Image height (pixels):        %d\n", control->imageSize[1].height);
                fprintf(stderr,"  Sensor width (mm):            %f\n", control->SensorWidthMm);
                fprintf(stderr,"  Sensor height (mm):           %f\n", control->SensorHeightMm);
                fprintf(stderr,"  Horizontal field of view:     %f\n", control->fovx[1]);
                fprintf(stderr,"  Vertical field of view:       %f\n", control->fovy[1]);
                fprintf(stderr,"  Focal length (sensor pixels): %f\n", control->focalLength[1]);
                fprintf(stderr,"  Focal length (mm):            %f\n", control->focalLength[1] * control->SensorCellMm);
                fprintf(stderr,"  Principal point x:            %f\n", control->principalPoint[1].x);
                fprintf(stderr,"  Principal point y:            %f\n", control->principalPoint[1].y);
                fprintf(stderr,"  Principal point x (pixels):   %f\n", control->principalPoint[1].x / control->SensorCellMm);
                fprintf(stderr,"  Principal point y (pixels):   %f\n", control->principalPoint[1].y / control->SensorCellMm);
                fprintf(stderr,"  Aspect ratio:                 %f\n", control->aspectRatio[1]);
                fprintf(stderr,"\nStereo depth resolution:\n");
                fprintf(stderr,"  minDisparity:                 %d\n", control->minDisparity);
                fprintf(stderr,"  Number of disparities:        %d\n", control->numberOfDisparities);
                fprintf(stderr,"  altitude_min:                 %f\n", control->altitude_min);
                fprintf(stderr,"  altitude_max:                 %f\n", control->altitude_max);
                fprintf(stderr,"  min_disparity:                %f\n", control->min_disparity);
                fprintf(stderr,"  max_disparity:                %f\n\n", control->max_disparity);
                fprintf(stderr,"  trim:                         %f\n", control->trim);
                fprintf(stderr,"  bin_size:                     %d\n", control->bin_size);
                fprintf(stderr,"  bin_filter:                   %d\n", control->bin_filter);
                fprintf(stderr,"  downsample:                   %d\n\n", control->downsample);
            }

            stereoRectify( control->cameraMatrix[0], control->distCoeffs[0], control->cameraMatrix[1], control->distCoeffs[1], 
                            control->imageSize[0], control->R, control->T, control->R1, control->R2, control->P1, control->P2, control->Q, 
                            CALIB_ZERO_DISPARITY, -1, control->imageSize[0], &control->roi1, &control->roi2 );
            initUndistortRectifyMap(control->cameraMatrix[0], control->distCoeffs[0], control->R1, control->P1, control->imageSize[0], CV_16SC2, control->map11, control->map12);
            initUndistortRectifyMap(control->cameraMatrix[1], control->distCoeffs[1], control->R2, control->P2, control->imageSize[1], CV_16SC2, control->map21, control->map22);

            /* print out calibration values */
            if (verbose > 0) {
                fprintf(stderr,"\nStereo Camera Calibration Parameters:\n");
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

        /* apply stereo calibration to rectify the images */
        Mat img1g, img2g, img1r, img2r;
        Mat disp, dispf, disp8;
        remap(img1, img1r, control->map11, control->map12, INTER_LINEAR);
        remap(img2, img2r, control->map21, control->map22, INTER_LINEAR);

        /* Downsample if specified */
        if (control->downsample > 1) {
            pyrDown(img1r, img1, Size(img1r.cols/control->downsample, img1r.rows/control->downsample));
            pyrDown(img2r, img2, Size(img2r.cols/control->downsample, img2r.rows/control->downsample));
        } else {
            img1 = img1r;
            img2 = img2r;
        }

        /* Convert images to CV_8UC1 format */
        cvtColor(img1, img1g, COLOR_BGR2GRAY);
        cvtColor(img2, img2g, COLOR_BGR2GRAY);

        /* do the photogrammetery */
        if (control->algorithm == STEREO_BM) {
            control->bm->compute(img1g, img2g, disp);
            disp.convertTo(disp8, CV_8U, 255/(control->numberOfDisparities*16.));
            disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
        }
        else if (control->algorithm == STEREO_SGBM || control->algorithm == STEREO_HH) {
            control->sgbm->compute(img1g, img2g, disp);
            disp.convertTo(disp8, CV_8U, 255/(control->numberOfDisparities*16.));
            disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
        }

        /* MB-System format 251 data are stored in the structure process->store */
        struct mbsys_stereopair_struct *store = &process->store;
        store->kind = MB_DATA_DATA;
        store->time_d = camera_time_d;
        store->longitude = camera_navlon;
        store->latitude = camera_navlat;
        store->sensordepth = camera_sensordepth;
        store->heading = camera_heading;
        store->roll = camera_roll;
        store->pitch = camera_pitch;
        store->speed = process->speed / 3.6;
        store->altitude = 0.0;
        store->num_soundings = 0;


        /* binning variables */
        double    *disparity_bin = NULL;
        size_t    num_bin = 0;
        size_t    num_bin_alloc = 0;

        /* loop over the disparity map, outputting valid values */
        int ngood = 0;
        int nbad = 0;
        int istart = (int)(control->trim * dispf.rows);
        int iend = dispf.rows - (int)(control->trim * dispf.rows);
        iend = MIN(iend, dispf.rows - control->bin_size);
        int jstart = (int)(control->trim * dispf.cols);
        int jend = dispf.cols - (int)(control->trim * dispf.cols);
        jend = MIN(jend, dispf.cols - control->bin_size);
       for (int i = istart; i < iend; i+=control->bin_size) {
            for (int j = jstart; j < jend; j+=control->bin_size) {
                int num_bin = 0;
                for (int ii=i;ii<i+control->bin_size;ii++)
                for (int jj=j;jj<j+control->bin_size;jj++) {
                    double disparity = control->downsample * static_cast<double>(dispf.at<float>(ii,jj));

                    /* only accept disparities within the desired altitude range */
                    if (disparity > control->min_disparity
                        && disparity < control->max_disparity) {
                        /* allocate more memory for binning if needed */
                        if (num_bin >= num_bin_alloc) {
                            /* allocate memory for data structure */
                            num_bin_alloc += MBPG_ALLOC_SMALL_NUM;
                           *status = mb_reallocd(verbose, __FILE__, __LINE__,
                                        num_bin_alloc
                                            * sizeof(double),
                                        (void **)(&disparity_bin), error);
                           if (*error != MB_ERROR_NO_ERROR) {
                                char *message = NULL;
                                mb_error(verbose,*error,&message);
                                fprintf(stderr,"\nMBIO Error allocating array:\n%s\n",message);
                                fprintf(stderr,"\nProgram <%s> Terminated\n",
                                    program_name);
                                exit(*error);
                            }
                        }

                        /* add disparity to the bin */
                        disparity_bin[num_bin] = disparity;
                        num_bin++;
                    }
                }

                /* use the median disparity */
                if (num_bin > 0) {
                    /* allocate more memory for soundings if needed */
                    if (store->num_soundings >= store->num_soundings_alloc) {
                        /* allocate memory for data structure */
                        store->num_soundings_alloc += MBPG_ALLOC_LARGE_NUM;
                        *status = mb_reallocd(verbose, __FILE__, __LINE__,
                                    store->num_soundings_alloc
                                        * sizeof(struct mbsys_stereopair_sounding_struct),
                                    (void **)(&store->soundings), error);
                        if (*error != MB_ERROR_NO_ERROR) {
                            char *message = NULL;
                            mb_error(verbose,*error,&message);
                            fprintf(stderr,"\nMBIO Error allocating array:\n%s\n",message);
                            fprintf(stderr,"\nProgram <%s> Terminated\n",
                                program_name);
                            exit(*error);
                        }
                    }
                    struct mbsys_stereopair_sounding_struct *sounding = &store->soundings[store->num_soundings];

                    /* if median filter then sort the bin and use the median */
                    double disparity = 0.0;
                    if (control->bin_filter == MBPG_BIN_FILTER_MEDIAN) {
                        qsort((void *)disparity_bin, num_bin, sizeof(double), mb_double_compare);
                        disparity = disparity_bin[num_bin/2];
                    }

                    /* else use simple mean */
                    else {
                        disparity = 0.0;
                        for (int k=0;k<num_bin;k++) {
                            disparity += disparity_bin[k];
                        }
                        disparity /= (double)num_bin;
                    }

                    /* use the position at the center of the bin */
                    int ii = control->downsample * (i + control->bin_size/2);
                    int jj = control->downsample * (j + control->bin_size/2);

                    /* calculate position relative to the camera rig */
                    double pw = disparity * control->Q.at<double>(3,2) - control->Q.at<double>(3,3);
                    Vec3f point;
                    point[0] = (static_cast<double>(jj) + control->Q.at<double>(0,3)) / pw;;
                    point[1] = (-(static_cast<double>(ii) + control->Q.at<double>(1,3))) / pw;;
                    point[2] = (control->Q.at<double>(2,3)) / pw;;

                    /* get range and angles in
                        roll-pitch frame */
                    double range = norm(point);
                    Vec3f direction = normalize(point);
                    double alphar = 0.0;
                    double betar = 0.5 * M_PI;
                    if (fabs(range) >= 0.001) {
                        alphar = asin(MAX(-1.0, MIN(1.0, direction[1])));
                        betar = acos(MAX(-1.0, MIN(1.0, (direction[0] / cos(alphar)))));
                    }
                    if (direction[2] < 0.0)
                        betar = 2.0 * M_PI - betar;

                    /* apply roll pitch corrections */
                    betar += DTR * camera_roll;
                    alphar += DTR * camera_pitch;

                    /* calculate bathymetry */
                    sounding->depth = range * cos(alphar) * sin(betar) + pg_z_offset;
                    sounding->alongtrack = range * sin(alphar) + pg_xtrack_offset;
                    sounding->acrosstrack = range * cos(alphar) * cos(betar) + pg_ltrack_offset;
                    sounding->beamflag = MB_FLAG_NONE;
                    sounding->red = 0;
                    sounding->green = 0;
                    sounding->blue = 0;

                    store->num_soundings++;
                    ngood++;
                }
                else {
                    nbad++;
                }
            }
        }
        mb_freed(verbose, __FILE__, __LINE__, (void **)&disparity_bin, error);
        num_bin_alloc = 0;

    }

}
/*--------------------------------------------------------------------*/

int main(int argc, char** argv)
{

    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    bool   help = false;
    int    flag = 0;

    /* Input imagelist */
    bool imagelist_specified = false;
    mb_path ImageListFile;

    /* parameter controls */
    struct mbpg_process_struct processData[MB_THREAD_MAX];
    for (int ithread=0; ithread<MB_THREAD_MAX; ithread++)
        memset((void *)&processData[ithread].store, 0, sizeof(struct mbsys_stereopair_struct));
    struct mbpg_control_struct control;
    control.show_images = false;
    control.algorithm = STEREO_SGBM;
    control.preFilterCap = 4;
    control.SADWindowSize = 5;
    control.SmoothingPenalty1 = 600;
    control.SmoothingPenalty2 = 2400;
    control.minDisparity = -64;
    control.numberOfDisparities = 192;
    control.uniquenessRatio = 1;
    control.speckleWindowSize = 150;
    control.speckleRange = 2;
    control.disp12MaxDiff = 10;
    control.altitude_min = 1.0;
    control.altitude_max = 5.0;
    control.trim = 0.0;
    control.bin_size = 1;
    control.bin_filter = MBPG_BIN_FILTER_MEAN;
    control.downsample = 1;
    control.textureThreshold = 10;
    control.min_disparity = 0.0;
    control.max_disparity = 0.0;

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
    mb_path StereoCameraCalibrationFile;
    control.calibrationInitialized = false;
    control.SensorWidthMm = 8.789;
    control.SensorHeightMm = 6.610;
    control.SensorCellMm = 0.00454;
    control.fov_fudgefactor = 1.0;

    /* Input navigation variables */
    bool navigation_specified = false;
    bool navigation_initialized = false;
    mb_path NavigationFile;
    int intstat;
    int intime = 0;
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

    /* Survey lines */
    bool surveylines_specified = false;
    bool surveylines_initialized = false;
    mb_path    SurveyLinesFile;
    int    ntimepoint = 0;
    int    ntimepointalloc = 0;
    double    *routetime_d = NULL;
    double    *routelon = NULL;
    double    *routelat = NULL;
    double    *routeheading = NULL;
    int    *routewaypoint = NULL;

    /* Input tide variables */
    bool tide_specified = false;
    bool tide_initialized = false;
    mb_path TideFile;
    int ittime = 0;
    int ntide = 0;
    double *ttime = NULL;
    double *ttide = NULL;

    /* Input quality variables */
    bool imagequality_specified = false;
    bool imagequality_initialized = false;
    double imageQualityThreshold = 0.0;
    double imageQualityFilterLength = 0.0;
    mb_path ImageQualityFile;
    int iqtime = 0;
    int nquality = 0;
    double *qtime = NULL;
    double *qquality = NULL;

    /* Output bathymetry variables */
    bool outputfileroot_specified = false;
    mb_path OutputFileRoot;
    mb_pathplus OutputFile;
    int output_number_pairs = 0;
    void *mbio_ptr = NULL;
    void *store_ptr = NULL;
    struct mb_io_struct *mb_io_ptr;
    struct mbsys_stereopair_struct *store;
    struct mbsys_stereopair_sounding_struct *sounding;

    /* MBIO status variables */
    int status = MB_SUCCESS;
    int verbose = 0;
    int error = MB_ERROR_NO_ERROR;
    char *message;

    /* command line option definitions */
    /* mbphotogrammetry
     *         --verbose
     *         --help
     *         --threads=nthreads
     *         --show-images
     *         --input=imagelist
     *         --fov-fudgefactor=factor
     *         --navigation-file=file
     *         --survey-line-file=file
     *         --tide-file=file
     *         --image-quality-file=file
     *         --image-quality-threshold=value
     *         --image-quality-filter-length=value
     *         --output=fileroot
     *         --output-number-pairs=value
     *         --camera-calibration-file=file
     *         --calibration-file=file
     *         --platform-file=platform.plf
     *         --camera-sensor=camera_sensor_id
     *         --nav-sensor=nav_sensor_id
     *         --sensordepth-sensor=sensordepth_sensor_id
     *         --heading-sensor=heading_sensor_id
     *         --altitude-sensor=altitude_sensor_id
     *         --attitude-sensor=attitude_sensor_id
     *         --altitude-min=value
     *         --altitude-max=value
     *         --trim=value
     *         --bin-size=value
     *         --bin-filter=value (0=mean, 1=median)
     *         --downsample=value
     *         --algorithm=algorithm ("bm", "sgbm", "hh", "var")
     *         --algorithm-pre-filter-cap=value
     *         --algorithm-sad-window-size=value
     *         --algorithm-smoothing-penalty-1=value
     *         --algorithm-smoothing-penalty-2=value
     *         --algorithm-min-disparity=value
     *         --algorithm-number-disparities=value
     *         --algorithm-uniqueness-ratio=value
     *         --algorithm-speckle-window-size=value
     *         --algorithm-speckle-range=value
     *         --algorithm-disp-12-max-diff=value
     *         --algorithm-texture-threshold=value
     */
    static struct option options[] =
        {
        {"verbose",                         no_argument,          NULL, 0},
        {"help",                            no_argument,          NULL, 0},
        {"threads",                         required_argument,    NULL, 0},
        {"show-images",                     no_argument,          NULL, 0},
        {"input",                           required_argument,    NULL, 0},
        {"fov-fudgefactor",                 required_argument,    NULL, 0},
        {"navigation-file",                 required_argument,    NULL, 0},
        {"survey-line-file",                required_argument,    NULL, 0},
        {"tide-file",                       required_argument,    NULL, 0},
        {"image-quality-file",              required_argument,    NULL, 0},
        {"image-quality-threshold",         required_argument,    NULL, 0},
        {"image-quality-filter-length",     required_argument,    NULL, 0},
        {"output",                          required_argument,    NULL, 0},
        {"output-number-pairs",             required_argument,    NULL, 0},
        {"camera-calibration-file",         required_argument,    NULL, 0},
        {"calibration-file",                required_argument,    NULL, 0},
        {"platform-file",                   required_argument,    NULL, 0},
        {"camera-sensor",                   required_argument,    NULL, 0},
        {"nav-sensor",                      required_argument,    NULL, 0},
        {"sensordepth-sensor",              required_argument,    NULL, 0},
        {"heading-sensor",                  required_argument,    NULL, 0},
        {"altitude-sensor",                 required_argument,    NULL, 0},
        {"attitude-sensor",                 required_argument,    NULL, 0},
        {"altitude-min",                    required_argument,    NULL, 0},
        {"altitude-max",                    required_argument,    NULL, 0},
        {"trim",                            required_argument,    NULL, 0},
        {"bin-size",                        required_argument,    NULL, 0},
        {"bin-filter",                      required_argument,    NULL, 0},
        {"downsample",                      required_argument,    NULL, 0},
        {"algorithm",                       required_argument,    NULL, 0},
        {"algorithm-pre-filter-cap",        required_argument,    NULL, 0},
        {"algorithm-sad-window-size",       required_argument,    NULL, 0},
        {"algorithm-smoothing-penalty-1",   required_argument,    NULL, 0},
        {"algorithm-smoothing-penalty-2",   required_argument,    NULL, 0},
        {"algorithm-min-disparity",         required_argument,    NULL, 0},
        {"algorithm-number-disparities",    required_argument,    NULL, 0},
        {"algorithm-uniqueness-ratio",      required_argument,    NULL, 0},
        {"algorithm-speckle-window-size",   required_argument,    NULL, 0},
        {"algorithm-speckle-range",         required_argument,    NULL, 0},
        {"algorithm-disp-12-max-diff",      required_argument,    NULL, 0},
        {"algorithm-texture-threshold",     required_argument,    NULL, 0},
        {"good-fraction-threshold",         required_argument,    NULL, 0},
        { NULL,                             0,                    NULL, 0}
        };

    /* set default imagelistfile name */
    snprintf(ImageListFile, sizeof(ImageListFile), "imagelist.mb-1");

    /* initialize some other things */
    memset(StereoCameraCalibrationFile, 0, sizeof(mb_path));
    memset(PlatformFile, 0, sizeof(mb_path));
    memset(NavigationFile, 0, sizeof(mb_path));
    memset(TideFile, 0, sizeof(mb_path));
    memset(ImageQualityFile, 0, sizeof(mb_path));

    /* Thread handling */
    unsigned int numThreads = 1;
    unsigned int numConcurrency = std::thread::hardware_concurrency();
    std::thread mbphotogrammetryThreads[MB_THREAD_MAX];
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

            /* show-images */
            else if (strcmp("show-images", options[option_index].name) == 0)
                {
                control.show_images = true;
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
             * Define input image list file */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageListFile);
                if (n == 1)
                    imagelist_specified = true;
                }

            /*-------------------------------------------------------
             * Define navigation file */

            /* navigation-file */
            else if (strcmp("navigation-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", NavigationFile);
                if (n == 1)
                    navigation_specified = true;
               }

            /*-------------------------------------------------------
             * Define survey line file */

            /* survey-line-file */
            else if (strcmp("survey-line-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", SurveyLinesFile);
                if (n == 1)
                    surveylines_specified = true;
                }

            /*-------------------------------------------------------
             * Define tide file */

            /* tide-file */
            else if (strcmp("tide-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", TideFile);
                if (n == 1)
                    tide_specified = true;
               }

            /*-------------------------------------------------------
             * Define image quality file */

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
                const int n = sscanf (optarg,"%lf", &imageQualityThreshold);
                }

            /* image-quality-filter-length */
            else if (strcmp("image-quality-filter-length", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf", &imageQualityFilterLength);
                }

            /*-------------------------------------------------------
             * Define output */

            /* output */
            else if (strcmp("output", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", OutputFileRoot);
                }

            /* output-number-pairs */
            else if (strcmp("output-number-pairs", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &output_number_pairs);
                }

            /*-------------------------------------------------------
             * Define stereo camera calibration file */

            /* camera-calibration-file  or calibration-file */
            else if (strcmp("camera-calibration-file", options[option_index].name) == 0
                      || strcmp("calibration-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", StereoCameraCalibrationFile);
                if (n == 1)
                    calibration_specified = true;
                }

            /*-------------------------------------------------------
             * Define platform */

            /* platform-file */
            else if (strcmp("platform-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", PlatformFile);
                if (n == 1)
                    platform_specified = true;
                }

            /* camera-sensor */
            else if (strcmp("camera-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &camera_sensor);
                }

            /* nav-sensor */
            else if (strcmp("nav-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &nav_sensor);
                }

            /* sensordepth-sensor */
            else if (strcmp("sensordepth-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &sensordepth_sensor);
                }

            /* heading-sensor */
            else if (strcmp("heading-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &heading_sensor);
                }

            /* altitude-sensor */
            else if (strcmp("altitude-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &altitude_sensor);
                }

            /* attitude-sensor */
            else if (strcmp("attitude-sensor", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &attitude_sensor);
                }

            /*-------------------------------------------------------
             * Define target altitude and altitude range */

            /* altitude-min */
            else if (strcmp("altitude-min", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%lf", &control.altitude_min);
                }

            /* altitude-max */
            else if (strcmp("altitude-max", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%lf", &control.altitude_max);
                }

            /* trim */
            else if (strcmp("trim", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%lf", &control.trim);
                }

            /* fov-fudgefactor */
            else if (strcmp("fov-fudgefactor", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf", &control.fov_fudgefactor);
                }

            /* bin-size */
            else if (strcmp("bin-size", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.bin_size);
                }

            /* bin-filter */
            else if (strcmp("bin-filter", options[option_index].name) == 0)
                {
                if (strcmp(optarg, "mean") == 0 || strcmp(optarg, "MEAN") == 0)
                    control.bin_filter = MBPG_BIN_FILTER_MEAN;
                else if (strcmp(optarg, "median") == 0 || strcmp(optarg, "MEDIAN") == 0)
                    control.bin_filter = MBPG_BIN_FILTER_MEDIAN;
                else
                    const int n = sscanf(optarg, "%d", &control.bin_filter);
                }

            /* downsample */
            else if (strcmp("downsample", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.downsample);
                }

            /*-------------------------------------------------------
             * Define algorithm and parameters */

            /* algorithm */
            else if (strcmp("algorithm", options[option_index].name) == 0)
                {
                if (strcmp(optarg, "bm") == 0 || strcmp(optarg, "BM") == 0)
                    control.algorithm = STEREO_BM;
                else if (strcmp(optarg, "sgbm") == 0 || strcmp(optarg, "SGBM") == 0)
                    control.algorithm = STEREO_SGBM;
                else if (strcmp(optarg, "hh") == 0 || strcmp(optarg, "HH") == 0)
                    control.algorithm = STEREO_HH;
                }

            /* algorithm-pre-filter-cap=value */
            else if (strcmp("algorithm-pre-filter-cap", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.preFilterCap);
                }

            /* algorithm-sad-window-size */
            else if (strcmp("algorithm-sad-window-size", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.SADWindowSize);
                }

            /* algorithm-smoothing-penalty-1 */
            else if (strcmp("algorithm-smoothing-penalty-1", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.SmoothingPenalty1);
                }

            /* algorithm-smoothing-penalty-2 */
            else if (strcmp("algorithm-smoothing-penalty-2", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.SmoothingPenalty2);
                }

            /* algorithm-min-disparity */
            else if (strcmp("algorithm-min-disparity", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.minDisparity);
                }

            /* algorithm-number-disparities */
            else if (strcmp("algorithm-number-disparities", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.numberOfDisparities);
                }

            /* algorithm-uniqueness-ratio */
            else if (strcmp("algorithm-uniqueness-ratio", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.uniquenessRatio);
                }

            /* algorithm-speckle-window-size */
            else if (strcmp("algorithm-speckle-window-size", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.speckleWindowSize);
                }

            /* algorithm-speckle-range */
            else if (strcmp("algorithm-speckle-range", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.speckleRange);
                }

            /* algorithm-disp-12-max-diff */
            else if (strcmp("algorithm-disp-12-max-diff", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.disp12MaxDiff);
                }

            /* algorithm-texture-threshold */
            else if (strcmp("algorithm-texture-threshold", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.textureThreshold);
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

    /* output stream (stdout if verbose <= 1, stderr if verbose > 1) */
    FILE *stream = NULL;
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
    if (verbose > 0) {
        fprintf(stream,"%sMB-system Version %s\n", first, MB_VERSION);
        fprintf(stream,"%sControl Parameters:\n", first);
        fprintf(stream,"%s     verbose:                          %d\n", first, verbose);
        fprintf(stream,"%s     help:                             %d\n", first, help);
        fprintf(stream,"%s     numThreads:                       %d\n", first, numThreads);
        fprintf(stream,"%s     imagelist_specified:              %d\n", first, imagelist_specified);
        fprintf(stream,"%s     ImageListFile:                    %s\n", first, ImageListFile);
        fprintf(stream,"%s     platform_specified:               %d\n", first, platform_specified);
        fprintf(stream,"%s     PlatformFile:                     %s\n", first, PlatformFile);
        fprintf(stream,"%s     calibration_specified:            %d\n", first, calibration_specified);
        fprintf(stream,"%s     StereoCameraCalibrationFile:      %s\n", first, StereoCameraCalibrationFile);
        fprintf(stream,"%s     navigation_specified:             %d\n", first, navigation_specified);
        fprintf(stream,"%s     NavigationFile:                   %s\n", first, NavigationFile);
        fprintf(stream,"%s     surveylines_specified:            %d\n", first, surveylines_specified);
        fprintf(stream,"%s     SurveyLinesFile:                  %s\n", first, SurveyLinesFile);
        fprintf(stream,"%s     tide_specified:                   %d\n", first, tide_specified);
        fprintf(stream,"%s     TideFile:                         %s\n", first, TideFile);
        fprintf(stream,"%s     imagequality_specified:           %d\n", first, imagequality_specified);
        fprintf(stream,"%s     ImageQualityFile:                 %s\n", first, ImageQualityFile);
        fprintf(stream,"%s     outputfileroot_specified:         %d\n", first, outputfileroot_specified);
        fprintf(stream,"%s     OutputFileRoot:                   %s\n", first, OutputFileRoot);
        fprintf(stream,"%s     control.show_images:              %d\n", first, control.show_images);
        fprintf(stream,"%s     control.algorithm:                %d\n", first, control.algorithm);
        fprintf(stream,"%s     control.preFilterCap:             %d\n", first, control.preFilterCap);
        fprintf(stream,"%s     control.SADWindowSize:            %d\n", first, control.SADWindowSize);
        fprintf(stream,"%s     control.SmoothingPenalty1:        %d\n", first, control.SmoothingPenalty1);
        fprintf(stream,"%s     control.minDisparity:             %d\n", first, control.minDisparity);
        fprintf(stream,"%s     control.numberOfDisparities:      %d\n", first, control.numberOfDisparities);
        fprintf(stream,"%s     control.uniquenessRatio:          %d\n", first, control.uniquenessRatio);
        fprintf(stream,"%s     control.speckleWindowSize:        %d\n", first, control.speckleWindowSize);
        fprintf(stream,"%s     control.speckleRange:             %d\n", first, control.speckleRange);
        fprintf(stream,"%s     control.disp12MaxDiff:            %d\n", first, control.disp12MaxDiff);
        fprintf(stream,"%s     control.altitude_min:             %f\n", first, control.altitude_min);
        fprintf(stream,"%s     control.altitude_max:             %f\n", first, control.altitude_max);
        fprintf(stream,"%s     control.trim:                     %f\n", first, control.trim);
        fprintf(stream,"%s     control.bin_size:                 %d\n", first, control.bin_size);
        fprintf(stream,"%s     control.bin_filter:               %d\n", first, control.bin_filter);
        fprintf(stream,"%s     control.downsample:               %d\n", first, control.downsample);
        fprintf(stream,"%s     control.textureThreshold:         %d\n", first, control.textureThreshold);
    }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
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
    int npairs_process = 0;
    int npairs_output = 0;
    int npairs_output_tot = 0;
    int imageStatus = MB_IMAGESTATUS_NONE;
    mb_path dpath;
    unsigned int numThreadsSet = 0;
    mb_path imageLeftFile;
    double image_left_time_d;
    double image_left_gain;
    double image_left_exposure;
    mb_path imageRightFile;
    double image_right_time_d;
    double image_right_gain;
    double image_right_exposure;
    int itime = 0;
    int iitime = 0;
    int waypoint = 0;

    fprintf(stream,"\nAbout to read ImageListFile: %s\n\n", ImageListFile);

    bool done = false;
    bool firstprocess = true;
    while (!done) {

        /* get next entry from the recursive imagelist structure */
        status = mb_imagelist_read(verbose, imagelist_ptr, &imageStatus,
                                imageLeftFile, imageRightFile, dpath,
                                &image_left_time_d, &image_right_time_d,
                                &image_left_gain, &image_right_gain,
                                &image_left_exposure, &image_right_exposure, &error);
        bool use_this_pair = false;
        if (status != MB_SUCCESS)
            done = true;
        
        /* Handle a valid stereo pair - embedded parameter commands are handled below */
        if (!done && imageStatus == MB_IMAGESTATUS_STEREO) {

            use_this_pair = true;

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
                control.calibrationInitialized = true;
                calibration_specified = false;
            }
            if (!control.calibrationInitialized) {
                fprintf(stderr,"\nCamera calibration not initialized...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                error = MB_ERROR_BAD_PARAMETER;
                mb_memory_clear(verbose, &error);
                exit(error);
            }

            /* read in navigation if desired */
            if (navigation_specified) {
                int lonflip = 0;
                load_navigation(verbose, NavigationFile, 0,
                                &nnav, &ntime, &nlon, &nlat, &nheading, &nspeed, &ndraft,
                                &nroll, &npitch, &nheave, &error);
                if (nnav > 0) {
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

            /* check imageQuality value against threshold */
            if (use_this_pair && imagequality_initialized) {
                double image_quality = 1.0;
                if (nquality > 1) {
                    intstat = mb_linear_interp(verbose, qtime-1, qquality-1, nquality,
                                                image_left_time_d, &image_quality, &iqtime, &error);
                }
                if (image_quality < imageQualityThreshold) {
                    use_this_pair = false;
                }
            }

            /* check that navigation is available for this stereo pair */
            if (use_this_pair && !(nnav > 0 && image_left_time_d >= ntime[0] && image_left_time_d <= ntime[nnav-1])) {
                use_this_pair = false;
            }

            /* process the stereo pair */
            if (use_this_pair) {
 
                /* get navigation attitude and tide for this stereo pair */
                double time_d = image_left_time_d;
                double navlon;
                double navlat;
                double heading;
                double speed;
                double draft;
                double roll;
                double pitch;
                double heave;
                double sensordepth;
                intstat = mb_linear_interp_longitude(verbose,
                            ntime-1, nlon-1,
                            nnav, time_d, &navlon, &itime,
                            &error);
                intstat = mb_linear_interp_latitude(verbose,
                            ntime-1, nlat-1,
                            nnav, time_d, &navlat, &itime,
                            &error);
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

                /* get tide for this stereo pair */
                if (tide_initialized) {
                    double tide = 0.0;
                    if (ntide > 1) {
                        intstat = mb_linear_interp(verbose, ttime-1, ttide-1, ntide, time_d, &tide, &iitime, &error);
                        sensordepth = sensordepth - tide;
                    }
                }

               /* set process structure to use */
                struct mbpg_process_struct *process = &processData[numThreadsSet];
                process->thread = numThreadsSet;
                process->pair_count = npairs_process;
                strncpy(process->imageLeftFile, imageLeftFile, sizeof(mb_path));
                process->image_left_time_d = image_left_time_d;
                process->image_left_gain = image_left_gain;
                process->image_left_exposure = image_left_exposure;
                strncpy(process->imageRightFile, imageRightFile, sizeof(mb_path));
                process->image_right_time_d = image_right_time_d;
                process->image_right_gain = image_right_gain;
                process->image_right_exposure = image_right_exposure;
                process->speed = speed;

                /* calculate target sensor position - this is a stereo pair and we
                  want to navigate the center or average of the two cameras */
                status = mb_platform_position(verbose, (void *)platform, camera_sensor, 0,
                                  navlon, navlat, sensordepth, heading, roll, pitch,
                                  &process->image_left_navlon, &process->image_left_navlat, &process->image_left_sensordepth,
                                  &error);
                status = mb_platform_orientation_target(verbose, (void *)platform, camera_sensor, 0,
                                  heading, roll, pitch,
                                  &process->image_left_heading, &process->image_left_roll, &process->image_left_pitch,
                                  &error);

                status = mb_platform_position(verbose, (void *)platform, camera_sensor, 1,
                                  navlon, navlat, sensordepth, heading, roll, pitch,
                                  &process->image_right_navlon, &process->image_right_navlat, &process->image_right_sensordepth,
                                  &error);
                status = mb_platform_orientation_target(verbose, (void *)platform, camera_sensor, 1,
                                  heading, roll, pitch,
                                  &process->image_right_heading, &process->image_right_roll, &process->image_right_pitch,
                                  &error);

                mbphotogrammetryThreads[numThreadsSet]
                        = std::thread(process_stereopair, verbose, process, &control,
                                        &thread_status[numThreadsSet], &thread_error[numThreadsSet]);
                numThreadsSet++;
                npairs_process++;
            }
        }

        /* If done or a full set of threads has been started wait for all to finish using join() 
            before writing output data and then continuing */
        if (numThreadsSet > 0 && (done || firstprocess || numThreadsSet == numThreads || imageStatus == MB_IMAGESTATUS_PARAMETER)) {
            for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
                /* join the thread (wait until it completes) */
                mbphotogrammetryThreads[ithread].join();
            }

            bool new_output_file = false;
            for (int ithread=0; ithread < numThreadsSet; ithread++) {
                struct mbpg_process_struct *process = &processData[ithread];

               if (surveylines_initialized) {
                    if ((process->image_left_time_d > routetime_d[waypoint] || waypoint == 0)
                        && waypoint < ntimepoint - 1) {
                        new_output_file = true;
                        snprintf(OutputFile, sizeof(OutputFile), "%s_%3.3d.mb251", OutputFileRoot, waypoint);
                        waypoint++;
                    }
                }
                else if (output_number_pairs > 0) {
                    if (mbio_ptr == NULL || npairs_output >= output_number_pairs) {
                        new_output_file = true;
                        snprintf(OutputFile, sizeof(OutputFile), "%s_%3.3d.mb251", OutputFileRoot, waypoint);
                        waypoint++;
                    }
                }
                else if (mbio_ptr == NULL) {
                    new_output_file = true;
                    snprintf(OutputFile, sizeof(OutputFile), "%s.mb251", OutputFileRoot);
                }

                /* open output format *.mb251 file */
                if (new_output_file) {
                    /* if needed close the previous output file */
                    if (mbio_ptr != NULL)
                        status = mb_close(verbose, &mbio_ptr, &error);

                    /* open the new output file */
                    int obeams_bath, obeams_amp, opixels_ss;
                    if ((status = mb_write_init(
                        verbose, OutputFile, MBF_PHOTGRAM, &mbio_ptr,
                        &obeams_bath, &obeams_amp, &opixels_ss, &error)) != MB_SUCCESS) {
                        mb_error(verbose,error,&message);
                        fprintf(stderr,"\nMBIO Error returned from function <mb_write_init>:\n%s\n",message);
                        fprintf(stderr,"\nOutput fbt file <%s> not initialized for writing\n",OutputFile);
                        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
                        exit(error);
                    }
                    if (verbose > 0)
                        fprintf(stderr,"      --> Opened output file: %s\n", OutputFile);
                    mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
                    store_ptr = mb_io_ptr->store_data;
                    store = (struct mbsys_stereopair_struct *) store_ptr;
                    new_output_file = false;
                    npairs_output = 0;
                }

                /* write the output structure */
                int time_i[7];
                mb_get_date(verbose, process->image_left_time_d, time_i);
                fprintf(stderr,"%d %s %s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d LLZ: %.8f %.8f %8.3f HRP:%6.2f %5.2f %5.2f A:%.3f %3f Q:%.2f %.2f\n",
                        process->pair_count, process->imageLeftFile, process->imageRightFile,
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                        process->image_left_navlon, process->image_left_navlat, process->image_left_sensordepth, 
                        process->image_left_heading, process->image_left_roll, process->image_left_pitch,
                        process->image_left_amplitude, process->image_right_amplitude,
                        process->image_left_quality, process->image_right_quality);
                mb_write_ping(verbose, mbio_ptr, (void *)&process->store, &error);
                npairs_output++;
                npairs_output_tot++;
                if (error != MB_ERROR_NO_ERROR) {
                    mb_error(verbose,error,&message);
                    fprintf(stderr,"\nMBIO Error returned from function <mb_write_ping>:\n%s\n",message);
                    fprintf(stderr,"\nMapping Data Not Written To File <%s>\n",OutputFile);
                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                        program_name);
                    exit(error);
                }

            }
            numThreadsSet = 0;
            firstprocess = false;
        }
        if (done && mbio_ptr != NULL)
             status = mb_close(verbose, &mbio_ptr, &error);

        /* handle parameter statements embedded in the recursive imagelist structure */
        if (!done && imageStatus == MB_IMAGESTATUS_PARAMETER) {

            firstprocess = true;

            fprintf(stream, "  ->Processing parameter: %s\n",imageLeftFile);
            mb_path tmp;

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
                if (sscanf(imageLeftFile, "--trim=%lf", &control.trim) == 1) {
                    if (verbose > 0)
                        fprintf(stream, "    Parameter reset: trim:%f\n",
                            control.trim);
                } else {
                    if (verbose > 0)
                        fprintf(stream, "\nFailure to reset parameter: trim:%f\n",
                            control.trim);
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
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);

    /* deallocate platform */
    if (platform != NULL) {
fprintf(stderr, "%s:%d:%s: About to deallocate platform model\n", __FILE__, __LINE__, __FUNCTION__);
        status = mb_platform_deall(verbose, (void **)&platform, &error);
    }

    /* deallocate navigation arrays if necessary */
    if (nnav > 0) {
fprintf(stderr, "%s:%d:%s: About to deallocate navigation arrays\n", __FILE__, __LINE__, __FUNCTION__);
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
    if (ntide > 0) {
fprintf(stderr, "%s:%d:%s: About to deallocate tide arrays\n", __FILE__, __LINE__, __FUNCTION__);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ttime,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&ttide,&error);
        ntide = 0;
    }

    /* deallocate image quality arrays if necessary */
    if (nquality > 0) {
fprintf(stderr, "%s:%d:%s: About to deallocate image quality arrays\n", __FILE__, __LINE__, __FUNCTION__);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&qtime,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&qquality,&error);
        nquality = 0;
    }

    /* end it all */
    exit(status);

}
/*--------------------------------------------------------------------*/
