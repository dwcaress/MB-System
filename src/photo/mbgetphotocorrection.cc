/*--------------------------------------------------------------------
 *    The MB-system:    mbgetphotocorrection.cpp    10/25/2013
 *
 *    Copyright (c) 1993-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbgetphotocorrection makes a mosaic of navigated downlooking photographs.
 *
 * Author:    D. W. Caress
 * Date:    October 25, 2013
 *
 * Integrated into MB-System July 2020
 *
 */

/* source file version string */
static char version_id[] = "$Id: mbpreprocess.c 2261 2016-01-07 01:49:22Z caress $";

/* standard include files */
#include <iostream>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>

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

std::string s;
std::stringstream out;

using namespace std;
using namespace cv;

#define MBPM_USE_STEREO         1
#define MBPM_USE_LEFT           2
#define MBPM_USE_RIGHT          3
#define MBPM_CAMERA_LEFT        0
#define MBPM_CAMERA_RIGHT       1

int main(int argc, char** argv)
{
    char program_name[] = "mbgetphotocorrection";
    char help_message[] =  "mbgetphotocorrection generates image intensity corrections from a set of navigated downlooking photographs.";
    char usage_message[] = "mbgetphotocorrection \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--show-images\n"
                            "\t--input=imagelist\n"
                            "\t--output=file  [--correction-file=file]\n"
                            "\t--correction-x-dimension=value\n"
                            "\t--correction-y-dimension=value\n"
                            "\t--correction-z-dimension=value\n"
                            "\t--correction-z-minmax=value/value\n"
                            "\t--platform-file=plffile\n"
                            "\t--camera-sensor=sensor_id\n"
                            "\t--nav-sensor=sensor_id\n"
                            "\t--sensordepth-sensor=sensor_id\n"
                            "\t--heading-sensor=sensor_id\n"
                            "\t--altitude-sensor=sensor_id\n"
                            "\t--attitude-sensor=sensor_id\n"
                            "\t--use-left-camera\n"
                            "\t--use-right-camera\n"
                            "\t--use-both-cameras\n"
                            "\t--calibration-file=file\n"
                            "\t--navigation-file=file\n"
                            "\t--tide-file=file\n"
                            "\t--topography-grid=file";
    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    int    help = 0;
    int    flag = 0;

    /* Output image correction */
    mb_path   ImageCorrectionFile;
    int       correction_specified = MB_NO;
    int       ncorr_x = 21;
    int       ncorr_y = 21;
    int       ncorr_z = 100;
    double    corr_zmin = 0.0;
    double    corr_zmax = 10.0;
    double    standoff_target = 0.5 * (corr_zmin + corr_zmax);
    double    corr_xmin = 0.0;
    double    corr_xmax = 0.0;
    double    corr_ymin = 0.0;
    double    corr_ymax = 0.0;
    Mat       corr_bounds;
    Mat       corr_table[2];
    Mat       corr_table_count[2];
    double    bin_dx = 0.0, bin_dy = 0.0, bin_dz = 0.0;

    /* Input image variables */
    mb_path   ImageListFile;
    int       imagelist_specified = MB_NO;
    mb_path   imageLeftFile;
    mb_path   imageRightFile;
    mb_path   imageFile;
    Mat       imageProcess;
    Mat       imageUndistort;
    Mat       imageUndistortYCrCb;
    int       undistort_initialized = MB_NO;
    double    left_time_d = 0.0;
    double    time_diff = 0.0;
    double    time_d = 0.0;
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
    mb_path    PlatformFile;
    int        platform_specified = MB_NO;
    int        camera_sensor = -1;
    int        nav_sensor = -1;
    int        sensordepth_sensor = -1;
    int        heading_sensor = -1;
    int        altitude_sensor = -1;
    int        attitude_sensor = -1;
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
    bool show_images = false;
    int imagelist_camera = 0;
    int image_camera = MBPM_CAMERA_LEFT;
    int use_camera_mode = MBPM_USE_STEREO;
    mb_path StereoCameraCalibrationFile;
    int calibration_specified = MB_NO;
    Mat cameraMatrix[2], distCoeffs[2];
    Mat R, T, E, F;
    Mat R1, R2, P1, P2, Q;
    Rect validRoi[2];
    Size imageSize;
    double SensorWidthMm = 8.789;
    double SensorHeightMm = 6.610;
    double SensorCellMm =0.00454;
    double fovx[2], fov_x;
    double fovy[2], fov_y;
    double focalLength[2];
    Point2d principalPoint[2];
    double  center_x, center_y;
    double aspectRatio[2];
    Mat rmap[2][2];
    Scalar avgPixelIntensity;
    float table_intensity, table_intensity_ref;
    //int pixelCorrection, avgPixelCorrection;
    double pixelCorrectionM, avgPixelCorrectionM;
    double intensityChange;
    double fov_fudgefactor = 1.0;
    double camera_navlon;
    double camera_navlat;
    double camera_sensordepth;
    double camera_heading;
    double camera_roll;
    double camera_pitch;

    /* Input navigation variables */
    int    navigation_specified = MB_NO;
    mb_path    NavigationFile;
    int    intstat;
    int    itime = 0;
    int    iitime = 0;
    int    nnav = 0;
    double    *ntime = NULL;
    double    *nlon = NULL;
    double    *nlat = NULL;
    double    *nheading = NULL;
    double    *nspeed = NULL;
    double    *ndraft = NULL;
    double    *nroll = NULL;
    double    *npitch = NULL;
    double    *nheave = NULL;

    /* Input tide variables */
    int    use_tide = MB_NO;
    mb_path    TideFile;
    int    ntide = 0;
    double    *ttime = NULL;
    double    *ttide = NULL;

    /* topography parameters */
    int    use_topography = MB_NO;
    mb_path    TopographyGridFile;
    void    *topogrid_ptr = NULL;

    /* projected image parameters */
    double    reference_lon, reference_lat;
    double    mtodeglon = 0.0;
    double    mtodeglat = 0.0;
    unsigned char    b = '\0', g = '\0', r = '\0';
    float    bgr_intensity, ycrcb_intensity;
    double    xx, yy, zz, zzref, rr, rrxymax, rrxy, rrxysq;
    double    phi, theta, theta2, dtheta;
    double    pixel_dx, pixel_dy;
    double    alpha, beta;
    double    vx, vy, vz;
    double    vxx, vyy, vzz;
    double    lon, lat, topo;
    double  cx, cy, cz, standoff;

    /* MBIO status variables */
    int    status = MB_SUCCESS;
    int    verbose = 0;
    int    error = MB_ERROR_NO_ERROR;
    char    *message;

    /* output stream for basic stuff (stdout if verbose <= 1,
        stderr if verbose > 1) */
    FILE    *stream = NULL;
    FILE    *lfp;
    FILE    *tfp;

    int    use_this_image;
    mb_path buffer;
    char    *result;
    int    size, len, nget, value_ok;
    double    xlon, ylat;
    int    lonflip = 0;
    double    sec;
    double  pbounds[4];
    double    factor, scale;
    double    headingx, headingy;
    FileStorage    fstorage;
    bool    isVerticalStereo;
    int    npairs, nimages, currentimages;
    int    iimage;
    int    ibin_x, jbin_y, kbin_z;
    int isensor;
    int    i, j, k, n;

    /* command line option definitions */
    /* mbgetphotocorrection
     *         --verbose
     *         --help
     *         --show-images
     *
     *         --input=imagelist
     *         --output=file  [--correction-file=file]
     *         --correction-x-dimension=value
     *         --correction-y-dimension=value
     *         --correction-z-dimension=value
     *         --correction-z-minmax=value/value
     *
     *         --platform-file=plffile
     *         --camera-sensor=sensor_id
     *         --nav-sensor=sensor_id
     *         --sensordepth-sensor=sensor_id
     *         --heading-sensor=sensor_id
     *         --altitude-sensor=sensor_id
     *         --attitude-sensor=sensor_id
     *
     *         --use-left-camera
     *         --use-right-camera
     *         --use-both-cameras
     *
     *         --calibration-file=file
     *         --navigation-file=file
     *         --tide-file=file
     *         --topography-grid=file
     *
     */
    static struct option options[] =
        {
        {"verbose",                   no_argument,        NULL, 0},
        {"help",                      no_argument,        NULL, 0},
        {"show-images",               no_argument,        NULL, 0},
        {"input",                     required_argument,  NULL, 0},
        {"output",                    required_argument,  NULL, 0},
        {"correction-file",           required_argument,  NULL, 0},
        {"correction-x-dimension",    required_argument,  NULL, 0},
        {"correction-y-dimension",    required_argument,  NULL, 0},
        {"correction-z-dimension",    required_argument,  NULL, 0},
        {"correction-z-minmax",       required_argument,  NULL, 0},
        {"platform-file",             required_argument,  NULL, 0},
        {"camera-sensor",             required_argument,  NULL, 0},
        {"nav-sensor",                required_argument,  NULL, 0},
        {"sensordepth-sensor",        required_argument,  NULL, 0},
        {"heading-sensor",            required_argument,  NULL, 0},
        {"altitude-sensor",           required_argument,  NULL, 0},
        {"attitude-sensor",           required_argument,  NULL, 0},
        {"use-left-camera",           no_argument,        NULL, 0},
        {"use-right-camera",          no_argument,        NULL, 0},
        {"use-both-cameras",          no_argument,        NULL, 0},
        {"calibration-file",          required_argument,  NULL, 0},
        {"navigation-file",           required_argument,  NULL, 0},
        {"tide-file",                 required_argument,  NULL, 0},
        {"topography-grid",           required_argument,  NULL, 0},
        { NULL,                       0,                  NULL, 0}
        };

    /* set default imagelistfile name */
    sprintf(ImageListFile, "imagelist.mb-2");
    sprintf(ImageCorrectionFile, "imagelist_cameracorrection.yml");

    /* initialize some other things */
    memset(StereoCameraCalibrationFile, 0, sizeof(mb_path));
    memset(PlatformFile, 0, sizeof(mb_path));
    memset(NavigationFile, 0, sizeof(mb_path));
    memset(TideFile, 0, sizeof(mb_path));
    memset(TopographyGridFile, 0, sizeof(mb_path));

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
                show_images = true;
                }

            /*-------------------------------------------------------
             * Define input file and format (usually a datalist) */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", ImageListFile);
                imagelist_specified = MB_YES;
                }

            /* output */
            else if ((strcmp("output", options[option_index].name) == 0)
                    || (strcmp("correction-file", options[option_index].name) == 0))
                {
                n = sscanf (optarg,"%s", ImageCorrectionFile);
                if (strlen(ImageCorrectionFile) < 5
                    || strncmp(".yml", &ImageCorrectionFile[strlen(ImageCorrectionFile)-4], 4) != 0)
                    {
                    strcat(ImageCorrectionFile, ".yml");
                    }
                correction_specified = MB_YES;
                }

            /* correction-x-dimension */
            else if (strcmp("correction-x-dimension", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &ncorr_x);
                }

            /* correction-y-dimension */
            else if (strcmp("correction-y-dimension", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &ncorr_y);
                }

            /* correction-z-dimension */
            else if (strcmp("correction-z-dimension", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &ncorr_z);
                }

            /* correction-z-minmax */
            else if (strcmp("correction-z-minmax", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%lf/%lf", &corr_zmin, &corr_zmax);
                }

            /* fov-fudgefactor */
            else if (strcmp("fov-fudgefactor", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%lf", &fov_fudgefactor);
                }

            /* platform-file */
            else if (strcmp("platform-file", options[option_index].name) == 0)
                {
                strcpy(PlatformFile, optarg);
                platform_specified = MB_YES;
                }

            /* camera-sensor */
            else if (strcmp("camera-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &camera_sensor);
                }

            /* nav-sensor */
            else if (strcmp("nav-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &nav_sensor);
                }

            /* sensordepth-sensor */
            else if (strcmp("sensordepth-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &sensordepth_sensor);
                }

            /* heading-sensor */
            else if (strcmp("heading-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &heading_sensor);
                }

            /* altitude-sensor */
            else if (strcmp("altitude-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &altitude_sensor);
                }

            /* attitude-sensor */
            else if (strcmp("attitude-sensor", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &attitude_sensor);
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
                calibration_specified = MB_YES;
                }

            /* navigation-file */
            else if (strcmp("navigation-file", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", NavigationFile);
                navigation_specified = MB_YES;
                }

            /* tide-file */
            else if (strcmp("tide-file", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", TideFile);
                use_tide = MB_YES;
                }

            /* topography-grid */
            else if (strcmp("topography-grid", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", TopographyGridFile);
                use_topography = MB_YES;
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

    /* if error flagged then print it and exit */
    if (errflg)
        {
        fprintf(stream,"usage: %s\n", usage_message);
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_USAGE;
        exit(error);
        }

    /* print starting debug statements */
    if (verbose >= 2)
        {
        fprintf(stream,"\ndbg2  Program <%s>\n",program_name);
        fprintf(stream,"dbg2  MB-system Version %s\n",MB_VERSION);
        fprintf(stream,"dbg2  Control Parameters:\n");
        fprintf(stream,"dbg2       verbose:                     %d\n",verbose);
        fprintf(stream,"dbg2       help:                        %d\n",help);
        fprintf(stream,"dbg2       show_images:                 %d\n",show_images);
        fprintf(stream,"dbg2       ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"dbg2       imagelist_specified:         %d\n",imagelist_specified);
        fprintf(stream,"dbg2       ImageCorrectionFile:         %s\n",ImageCorrectionFile);
        fprintf(stream,"dbg2       correction_specified:        %d\n",correction_specified);
        fprintf(stream,"dbg2       ncorr_x:                     %d\n",ncorr_x);
        fprintf(stream,"dbg2       ncorr_y:                     %d\n",ncorr_y);
        fprintf(stream,"dbg2       ncorr_z:                     %d\n",ncorr_z);
        fprintf(stream,"dbg2       corr_zmin:                   %f\n",corr_zmin);
        fprintf(stream,"dbg2       corr_zmax:                   %f\n",corr_zmax);
        fprintf(stream,"dbg2       fov_fudgefactor:             %f\n",fov_fudgefactor);
        fprintf(stream,"dbg2       PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"dbg2       platform_specified:          %d\n",platform_specified);
        fprintf(stream,"dbg2       camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"dbg2       nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"dbg2       sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"dbg2       heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"dbg2       altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"dbg2       attitude_sensor:             %d\n",attitude_sensor);
        fprintf(stream,"dbg2       use_camera_mode:             %d\n",use_camera_mode);
        fprintf(stream,"dbg2       StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"dbg2       calibration_specified:       %d\n",calibration_specified);
        fprintf(stream,"dbg2       NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"dbg2       navigation_specified:        %d\n",navigation_specified);
        fprintf(stream,"dbg2       TideFile:                    %s\n",TideFile);
        fprintf(stream,"dbg2       use_tide:                    %d\n",use_tide);
        fprintf(stream,"dbg2       TopographyGridFile:          %s\n",TopographyGridFile);
        fprintf(stream,"dbg2       use_topography:              %d\n",use_topography);
        }
    else if (verbose == 1)
        {
        fprintf(stdout,"\nProgram <%s>\n",program_name);
        fprintf(stdout,"Control Parameters:\n");
        fprintf(stream,"    verbose:                     %d\n",verbose);
        fprintf(stream,"    help:                        %d\n",help);
        fprintf(stream,"    show_images:                 %d\n",show_images);
        fprintf(stream,"    ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"    imagelist_specified:         %d\n",imagelist_specified);
        fprintf(stream,"    ImageCorrectionFile:         %s\n",ImageCorrectionFile);
        fprintf(stream,"    correction_specified:        %d\n",correction_specified);
        fprintf(stream,"    ncorr_x:                     %d\n",ncorr_x);
        fprintf(stream,"    ncorr_y:                     %d\n",ncorr_y);
        fprintf(stream,"    ncorr_z:                     %d\n",ncorr_z);
        fprintf(stream,"    corr_zmin:                   %f\n",corr_zmin);
        fprintf(stream,"    corr_zmax:                   %f\n",corr_zmax);
        fprintf(stream,"    fov_fudgefactor:             %f\n",fov_fudgefactor);
        fprintf(stream,"    PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"    platform_specified:          %d\n",platform_specified);
        fprintf(stream,"    camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"    nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"    sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"    heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"    altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"    attitude_sensor:             %d\n",attitude_sensor);
        fprintf(stream,"    use_camera_mode:             %d\n",use_camera_mode);
        fprintf(stream,"    StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"    calibration_specified:       %d\n",calibration_specified);
        fprintf(stream,"    NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"    navigation_specified:        %d\n",navigation_specified);
        fprintf(stream,"    TideFile:                    %s\n",TideFile);
        fprintf(stream,"    use_tide:                    %d\n",use_tide);
        fprintf(stream,"    TopographyGridFile:          %s\n",TopographyGridFile);
        fprintf(stream,"    use_topography:              %d\n",use_topography);
        }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
        }

    /* if stereo calibration not specified then quit */
    if (calibration_specified == MB_NO)
        {
        fprintf(stream,"\nNo camera calibration file specified\n");
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        }

    /* if platform not specified then quit */
    if (platform_specified == MB_NO)
        {
        fprintf(stream,"\nNo platform file specified\n");
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        }

    /* if navigation not specified then quit */
    if (navigation_specified == MB_NO)
        {
        fprintf(stream,"\nNo navigation file specified\n");
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        }

    if (error != MB_ERROR_NO_ERROR)
        {
        mb_memory_clear(verbose, &error);
        exit(error);
        }

    /* read in platform offsets */
    status = mb_platform_read(verbose, PlatformFile, (void **)&platform, &error);
    if (status == MB_FAILURE)
        {
        error = MB_ERROR_OPEN_FAIL;
        fprintf(stderr,"\nUnable to open and parse platform file: %s\n", PlatformFile);
        fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
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
    if (camera_sensor < 0)
        {
        for (isensor=0;isensor<platform->num_sensors;isensor++)
            {
            if (platform->sensors[isensor].type == MB_SENSOR_TYPE_CAMERA_STEREO)
                {
                camera_sensor = isensor;
                }
            }
        }
    if (camera_sensor >= 0)
        sensor_camera = &(platform->sensors[camera_sensor]);

    /* read intrinsic and extrinsic stereo camera calibration parameters */
    fstorage.open(StereoCameraCalibrationFile, FileStorage::READ);
    if(fstorage.isOpened() )
        {
        fstorage["M1"] >> cameraMatrix[0];
        fstorage["D1"] >> distCoeffs[0];
        fstorage["M2"] >> cameraMatrix[1];
        fstorage["D2"] >> distCoeffs[1];
        fstorage["R"] >> R;
        fstorage["T"] >> T;
        fstorage["R1"] >> R1;
        fstorage["R2"] >> R2;
        fstorage["P1"] >> P1;
        fstorage["P2"] >> P2;
        fstorage["Q"] >> Q;
        fstorage.release();
        isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));
        }
    else
        {
        fprintf(stream,"\nUnable to read calibration file %s\n",
            StereoCameraCalibrationFile);
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        mb_memory_clear(verbose, &error);
        exit(error);
        }

    /* print out calibration values */
    if (verbose > 0)
        {
        fprintf(stderr,"\nStereo Camera Calibration Parameters:\n");
        cerr << "M1:" << endl << cameraMatrix[0] << endl << endl;
        cerr << "D1:" << endl << distCoeffs[0] << endl << endl;
        cerr << "M2:" << endl << cameraMatrix[1] << endl << endl;
        cerr << "D2:" << endl << distCoeffs[1] << endl << endl;
        cerr << "R:" << endl << R << endl << endl;
        cerr << "T:" << endl << T << endl << endl;
        cerr << "R1:" << endl << R1 << endl << endl;
        cerr << "R2:" << endl << R2 << endl << endl;
        cerr << "P1:" << endl << P1 << endl << endl;
        cerr << "P2:" << endl << P2 << endl << endl;
        cerr << "Q:" << endl << Q << endl << endl;
        }

    /* read in navigation if desired */
    if (navigation_specified == MB_YES)
        {
        if ((tfp = fopen(NavigationFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",NavigationFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        nnav = 0;
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            nnav++;
        fclose(tfp);

        /* allocate arrays for nav */
        if (nnav > 1)
            {
            size = (nnav+1)*sizeof(double);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&ntime,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlon,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nlat,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nheading,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nspeed,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&ndraft,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nroll,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&npitch,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nnav*sizeof(double),(void **)&nheave,&error);

            /* if error initializing memory then quit */
            if (error != MB_ERROR_NO_ERROR)
                {
                fclose(tfp);
                mb_error(verbose,error,&message);
                fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                exit(error);
                }
            }

        /* if no nav data then quit */
        else
            {
            fclose(tfp);
            error = MB_ERROR_BAD_DATA;
            fprintf(stderr,"\nUnable to read data from navigation file <%s>\n",NavigationFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }

        /* read the data points in the nav file */
        nnav = 0;
        if ((tfp = fopen(NavigationFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",NavigationFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            value_ok = MB_NO;

            /* read the navigation from an fnv file */
            nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                &time_i[0],&time_i[1],&time_i[2],
                &time_i[3],&time_i[4],&sec,
                &ntime[nnav],
                &nlon[nnav],&nlat[nnav],
                &nheading[nnav],&nspeed[nnav],&ndraft[nnav],
                &nroll[nnav],&npitch[nnav],&nheave[nnav]);
            if (nget >= 15)
                value_ok = MB_YES;

            /* make sure longitude is defined according to lonflip */
            if (value_ok == MB_YES)
                {
                if (lonflip == -1 && nlon[nnav] > 0.0)
                    nlon[nnav] = nlon[nnav] - 360.0;
                else if (lonflip == 0 && nlon[nnav] < -180.0)
                    nlon[nnav] = nlon[nnav] + 360.0;
                else if (lonflip == 0 && nlon[nnav] > 180.0)
                    nlon[nnav] = nlon[nnav] - 360.0;
                else if (lonflip == 1 && nlon[nnav] < 0.0)
                    nlon[nnav] = nlon[nnav] + 360.0;
                }

            /* output some debug values */
            if (verbose >= 5 && value_ok == MB_YES)
                {
                fprintf(stderr,"\ndbg5  New navigation point read in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                    nnav,ntime[nnav],nlon[nnav],nlat[nnav]);
                }
            else if (verbose >= 5)
                {
                fprintf(stderr,"\ndbg5  Error parsing line in navigation file in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       line: %s\n",buffer);
                }

            /* check for reverses or repeats in time */
            if (value_ok == MB_YES)
                {
                if (nnav == 0)
                    nnav++;
                else if (ntime[nnav] > ntime[nnav-1])
                    nnav++;
                else if (nnav > 0 && ntime[nnav] <= ntime[nnav-1]
                    && verbose >= 5)
                    {
                    fprintf(stderr,"\ndbg5  Navigation time error in program <%s>\n",program_name);
                    fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                        nnav-1,ntime[nnav-1],nlon[nnav-1],
                        nlat[nnav-1]);
                    fprintf(stderr,"dbg5       nav[%d]: %f %f %f\n",
                        nnav,ntime[nnav],nlon[nnav],
                        nlat[nnav]);
                    }
                }
            strncpy(buffer,"\0",sizeof(buffer));
            }
        fclose(tfp);

        /* output information */
        if (verbose >= 1)
            {
            fprintf(stdout,"\nNavigation Parameters:\n");
            fprintf(stream,"  NavigationFile:     %s\n",NavigationFile);
            fprintf(stream,"  nnav:               %d\n",nnav);
            }

        /* set mtodeglon, mtodeglat */
        mb_coor_scale(verbose,nlat[nnav/2],&mtodeglon,&mtodeglat);

        }

    /* read in tide if desired */
    if (use_tide == MB_YES)
        {
        if ((tfp = fopen(TideFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to Open Tide File <%s> for reading\n",TideFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        ntide = 0;
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            ntide++;
        fclose(tfp);

        /* allocate arrays for tide */
        if (ntide > 1)
            {
            size = (ntide+1)*sizeof(double);
            status = mb_mallocd(verbose,__FILE__,__LINE__,ntide*sizeof(double),(void **)&ttime,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,ntide*sizeof(double),(void **)&ttide,&error);

            /* if error initializing memory then quit */
            if (error != MB_ERROR_NO_ERROR)
                {
                fclose(tfp);
                mb_error(verbose,error,&message);
                fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",message);
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                exit(error);
                }
            }

        /* if no tide data then quit */
        else
            {
            fclose(tfp);
            error = MB_ERROR_BAD_DATA;
            fprintf(stderr,"\nUnable to read data from tide file <%s>\n",TideFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }

        /* read the data points in the tide file */
        ntide = 0;
        if ((tfp = fopen(TideFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to Open tide File <%s> for reading\n",NavigationFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            value_ok = MB_NO;

            /* read the navigation from an fnv file */
            nget = sscanf(buffer,"%lf %lf",
                &ttime[ntide], &ttide[ntide]);
            if (nget >= 2)
                value_ok = MB_YES;

            /* output some debug values */
            if (verbose >= 5 && value_ok == MB_YES)
                {
                fprintf(stderr,"\ndbg5  New tide point read in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
                    ntide,ttime[ntide],ttide[ntide]);
                }
            else if (verbose >= 5)
                {
                fprintf(stderr,"\ndbg5  Error parsing line in tide file in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       line: %s\n",buffer);
                }

            /* check for reverses or repeats in time */
            if (value_ok == MB_YES)
                {
                if (ntide == 0)
                    ntide++;
                else if (ttime[ntide] > ttime[ntide-1])
                    ntide++;
                else if (ntide > 0 && ttime[ntide] <= ttime[ntide-1]
                    && verbose >= 5)
                    {
                    fprintf(stderr,"\ndbg5  Tide time error in program <%s>\n",program_name);
                    fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
                        ntide-1,ttime[ntide-1],ttide[ntide-1]);
                    fprintf(stderr,"dbg5       tide[%d]: %f %f\n",
                        ntide,ttime[ntide],ttide[ntide]);
                    }
                }
            strncpy(buffer,"\0",sizeof(buffer));
            }
        fclose(tfp);

        /* output information */
        if (verbose >= 1)
            {
            fprintf(stdout,"\nTide Parameters:\n");
            fprintf(stream,"  TideFile:     %s\n",TideFile);
            fprintf(stream,"  ntide:        %d\n",ntide);
            }
        }

    /* Load topography grid if desired */
    if (use_topography == MB_YES)
        {
        status = mb_topogrid_init(verbose, TopographyGridFile, &lonflip, &topogrid_ptr, &error);
        if (error != MB_ERROR_NO_ERROR)
            {
            mb_error(verbose,error,&message);
            fprintf(stderr,"\nMBIO Error loading topography grid: %s\n%s\n",TopographyGridFile,message);
            fprintf(stderr,"\nProgram <%s> Terminated\n",program_name);
            mb_memory_clear(verbose, &error);
            exit(error);
            }
        }

    /* Initialize camera intensity correction tables */

    const int corr_table_dims[3] = {ncorr_x, ncorr_y, ncorr_z};
    corr_bounds = Mat(3, 3, CV_32FC(1), Scalar(0.0));
    corr_table[0] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
    corr_table_count[0] = Mat(3, corr_table_dims, CV_32SC(1), Scalar(0));
    corr_table[1] = Mat(3, corr_table_dims, CV_32FC(1), Scalar(0.0));
    corr_table_count[1] = Mat(3, corr_table_dims, CV_32SC(1), Scalar(0));

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
    int imagestatus = MB_IMAGESTATUS_NONE;
    mb_path dpath;
    fprintf(stderr,"About to read ImageListFile: %s\n", ImageListFile);
    while ((status = mb_imagelist_read(verbose, imagelist_ptr, &imagestatus,
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &time_diff, &error)) == MB_SUCCESS) {
        if (imagestatus == MB_IMAGESTATUS_STEREO) {
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
        } else if (imagestatus == MB_IMAGESTATUS_LEFT) {
          if (use_camera_mode == MBPM_USE_LEFT) {
            image_camera = MBPM_CAMERA_LEFT;
            currentimages = 1;
            nimages++;
          } else {
            currentimages = 0;
          }
        } else if (imagestatus == MB_IMAGESTATUS_RIGHT) {
          if (use_camera_mode == MBPM_USE_RIGHT) {
            image_camera = MBPM_CAMERA_RIGHT;
            currentimages = 1;
            nimages++;
          } else {
            currentimages = 0;
          }
        } else if (imagestatus == MB_IMAGESTATUS_SINGLE) {
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
        for (iimage=0;iimage<currentimages;iimage++) {
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
            use_this_image = MB_NO;
            if (image_camera == MBPM_CAMERA_LEFT
                && (use_camera_mode == MBPM_USE_LEFT || use_camera_mode == MBPM_USE_STEREO)) {
                     time_d = left_time_d;
                    strcpy(imageFile, imageLeftFile);
                    use_this_image = MB_YES;
            }
            else if (image_camera == MBPM_CAMERA_RIGHT
                && (use_camera_mode == MBPM_USE_RIGHT || use_camera_mode == MBPM_USE_STEREO)) {
                     time_d = left_time_d + time_diff;
                    strcpy(imageFile, imageRightFile);
                    use_this_image = MB_YES;
            }

            /* check that navigation is available for this image */
            if (use_this_image == MB_YES) {
                if (!(nnav > 0 && time_d >= ntime[0] && time_d <= ntime[nnav-1])) {
                    use_this_image = MB_NO;
                }
            }

            /* read the image */
            if (use_this_image == MB_YES) {
                /* read the image */
                imageProcess = imread(imageFile);
                if (imageProcess.empty())
                    use_this_image = MB_NO;
            }

            /* process the image */
            if (use_this_image == MB_YES) {
                /* if calibration loaded, undistort the image */
                if (calibration_specified == MB_YES) {
                    if (undistort_initialized == MB_NO) {
                        imageSize = imageProcess.size();
                        calibrationMatrixValues(cameraMatrix[0], imageSize,
                                    SensorWidthMm, SensorHeightMm,
                                    fovx[0], fovy[0], focalLength[0],
                                    principalPoint[0], aspectRatio[0]);
                        calibrationMatrixValues(cameraMatrix[1], imageSize,
                                    SensorWidthMm, SensorHeightMm,
                                    fovx[1], fovy[1], focalLength[1],
                                    principalPoint[1], aspectRatio[1]);
                        undistort_initialized = MB_YES;
                        if (verbose > 0) {
                            fprintf(stderr,"\nLeft Camera Characteristics:\n");
                            fprintf(stderr,"  Image width:                  %d\n", imageSize.width);
                            fprintf(stderr,"  Image height:                 %d\n", imageSize.height);
                            fprintf(stderr,"  Horizontal field of view:     %f\n", fovx[0]);
                            fprintf(stderr,"  Vertical field of view:       %f\n", fovy[0]);
                            fprintf(stderr,"  Focal length (sensor pixels): %f\n", focalLength[0]);
                            fprintf(stderr,"  Focal length (mm):            %f\n", focalLength[0] * SensorCellMm);
                            fprintf(stderr,"  Principal point x:            %f\n", principalPoint[0].x);
                            fprintf(stderr,"  Principal point y:            %f\n", principalPoint[0].y);
                            fprintf(stderr,"  Principal point x (pixels):   %f\n", principalPoint[0].x / SensorCellMm);
                            fprintf(stderr,"  Principal point y (pixels):   %f\n", principalPoint[0].y / SensorCellMm);
                            fprintf(stderr,"  Aspect ratio:                 %f\n", aspectRatio[0]);
                            fprintf(stderr,"\nRight Camera Characteristics:\n");
                            fprintf(stderr,"  Image width:                  %d\n", imageSize.width);
                            fprintf(stderr,"  Image height:                 %d\n", imageSize.height);
                            fprintf(stderr,"  Horizontal field of view:     %f\n", fovx[1]);
                            fprintf(stderr,"  Vertical field of view:       %f\n", fovy[1]);
                            fprintf(stderr,"  Focal length (sensor pixels): %f\n", focalLength[1]);
                            fprintf(stderr,"  Focal length (mm):            %f\n", focalLength[1] * SensorCellMm);
                            fprintf(stderr,"  Principal point x:            %f\n", principalPoint[1].x);
                            fprintf(stderr,"  Principal point y:            %f\n", principalPoint[1].y);
                            fprintf(stderr,"  Principal point x (pixels):   %f\n", principalPoint[1].x / SensorCellMm);
                            fprintf(stderr,"  Principal point y (pixels):   %f\n", principalPoint[1].y / SensorCellMm);
                            fprintf(stderr,"  Aspect ratio:                 %f\n", aspectRatio[1]);
                        }
                    }

                    /* undistort the image */
                    undistort(imageProcess, imageUndistort, cameraMatrix[image_camera], distCoeffs[image_camera], noArray());

                    /* get field of view, offsets, and principal point to use */
                    if (image_camera == 0) {
                        fov_x = fovx[0];
                        fov_y = fovy[0];
                        center_x = imageSize.width / 2 + principalPoint[0].x;
                        center_y = imageSize.height / 2 + principalPoint[0].y;
                    }
                    else {
                        fov_x = fovx[1];
                        fov_y = fovy[1];
                        center_x = imageSize.width / 2 + principalPoint[1].x;
                        center_y = imageSize.height / 2 + principalPoint[1].y;
                    }
                }

                /* else leave the image alone, treat the center as the principal point,
                    and use specified offsets */
                else {
                    imageUndistort = imageProcess.clone();
                    if (iimage == 0) {
                        fov_x = 77.36;
                        fov_y = (fov_x * imageSize.height) / imageSize.width;
                        center_x = imageSize.width / 2;
                        center_y = imageSize.height / 2;
                    }
                    else {
                        fov_x = 77.36;
                        fov_y = (fov_x * imageSize.height) / imageSize.width;
                        center_x = imageSize.width / 2;
                        center_y = imageSize.height / 2;
                    }
                }

                /* convert to YCrCb */
                    cvtColor(imageUndistort, imageUndistortYCrCb, COLOR_BGR2YCrCb);

                /* calculate reference "depth" for use in calculating ray angles for
                    individual pixels */
                zzref = 0.5 * (0.5 * imageSize.width / tan(DTR * 0.5 * fov_x * fov_fudgefactor)
                        + 0.5 * imageSize.height / tan(DTR * 0.5 * fov_y * fov_fudgefactor));

                /* display images */
                if (show_images) {
                    String windowName = "Undistorted Image";
                    namedWindow(windowName, 0);
                    imshow(windowName, imageUndistort);
                    waitKey(1000);
                    destroyWindow(windowName);
                }

                /* get navigation for this image */
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

                /* get tide for this image */
                tide = 0.0;
                if (ntide > 1)
                    {
                    intstat = mb_linear_interp(verbose,
                            ttime-1, ttide-1,
                            ntide, time_d, &tide, &iitime,
                            &error);
                    }
                sensordepth = sensordepth - tide;

                /* calculate target sensor position */
                status = mb_platform_position (verbose, (void *)platform,
                                camera_sensor, iimage,
                                navlon, navlat, sensordepth,
                                heading, roll, pitch,
                                &camera_navlon, &camera_navlat, &camera_sensordepth,
                                &error);
                status = mb_platform_orientation_target (verbose, (void *)platform,
                                camera_sensor, image_camera,
                                heading, roll, pitch,
                                &camera_heading, &camera_roll, &camera_pitch,
                                &error);

                /* Apply camera model translation */
                double dx, dy, dz;
                double headingx = sin(DTR * camera_heading);
                double headingy = cos(DTR * camera_heading);
                if (image_camera == 0) {
                    dx = 0.5 * (T.at<double>(0)) * mtodeglon;
                    dy = 0.5 * (T.at<double>(1)) * mtodeglat;
                    dz = 0.5 * (T.at<double>(2));
                } else {
                    dx = -0.5 * (T.at<double>(0)) * mtodeglon;
                    dy = -0.5 * (T.at<double>(1)) * mtodeglat;
                    dz = -0.5 * (T.at<double>(2));
                }
                camera_navlon += (headingy * dx + headingx * dy);
                camera_navlat += (-headingx * dx + headingy * dy);
                camera_sensordepth += dz;
//fprintf(stderr,"%s:%d:%s: Camera %d: %.9f %.9f %.3f %.3f %.3f   %.3f %.3f %.3f\n",
//__FILE__, __LINE__, __func__, image_camera,
//camera_navlon, camera_navlat, (camera_navlon - navlon) / mtodeglon, (camera_navlat - navlat) / mtodeglat, camera_sensordepth,
//camera_heading, camera_roll, camera_pitch);

                /* Process this image */

                /* calculate the largest distance from center for this image for use
                    in calculating pixel priority */
                xx = MAX(center_x, imageUndistortYCrCb.cols - center_x);
                yy = MAX(center_y, imageUndistortYCrCb.rows - center_y);
                rrxymax = sqrt(xx * xx + yy * yy);

                avgPixelIntensity = mean(imageUndistortYCrCb);
                mb_get_date(verbose, time_d, time_i);
                fprintf(stderr,"%4d Camera:%d Image:%s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d LLZ: %.10f %.10f %8.3f Tide:%7.3f H:%6.2f R:%6.2f P:%6.2f Avg Intensity:%.3f\n",
                        nimages - currentimages + iimage + 1, image_camera, imageFile,
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                        navlon, navlat, sensordepth, tide, heading, roll, pitch, avgPixelIntensity.val[0]);

                /* get unit vector for direction camera is pointing */

                /* rotate center pixel location using attitude and zzref */
                mb_platform_math_attitude_rotate_beam(verbose,
                    0.0, 0.0, zzref,
                    camera_roll, camera_pitch, 0.0,
                    &xx, &yy, &zz,
                    &error);
                rr = sqrt(xx * xx + yy * yy + zz * zz);
                phi = RTD * atan2(yy, xx);
                theta = RTD * acos(zz / rr);

                /* calculate unit vector relative to the camera rig */
                vz = cos(DTR * theta);
                vx = sin(DTR * theta) * cos(DTR * phi);
                vy = sin(DTR * theta) * sin(DTR * phi);

                /* apply rotation of each camera relative to the rig */
                if (image_camera == 1)
                    {
                    vxx = vx * (R.at<double>(0,0)) + vy * (R.at<double>(0,1)) + vz * (R.at<double>(0,2));
                    vyy = vx * (R.at<double>(1,0)) + vy * (R.at<double>(1,1)) + vz * (R.at<double>(1,2));
                    vzz = vx * (R.at<double>(2,0)) + vy * (R.at<double>(2,1)) + vz * (R.at<double>(2,2));
                    }
                else
                    {
                    vxx = vx;
                    vyy = vy;
                    vzz = vz;
                    }

                /* rotate unit vector by camera rig heading */
                cx = vxx * cos(DTR * camera_heading) + vyy * sin(DTR * camera_heading);
                cy = -vxx * sin(DTR * camera_heading) + vyy * cos(DTR * camera_heading);
                cz = vzz;

                /* reset for binning */
                corr_xmin = 0;
                corr_xmax = imageUndistortYCrCb.cols;
                corr_ymin = 0;
                corr_ymax = imageUndistortYCrCb.rows;
                bin_dx = imageUndistortYCrCb.cols / (ncorr_x - 1);
                bin_dy = imageUndistortYCrCb.rows / (ncorr_y - 1);
                bin_dz = (corr_zmax - corr_zmin) / (ncorr_z - 1);

                /* loop over the pixels */
                for(i=0; i<imageUndistortYCrCb.cols; i++) {
                    for(j=0; j<imageUndistortYCrCb.rows; j++) {
                        /* calculate intensity for this pixel
                            - access the pixel value with Vec3b */
                        bgr_intensity = ((float)imageUndistort.at<Vec3b>(j,i)[0]
                                 + (float)imageUndistort.at<Vec3b>(j,i)[1]
                                 + (float)imageUndistort.at<Vec3b>(j,i)[2]) / 3.0;
                        ycrcb_intensity = (float)imageUndistortYCrCb.at<Vec3b>(j,i)[0];
//fprintf(stderr,"i:%d j:%d intensity: bgr:%f hsv:%f\n",i,j,bgr_intensity,ycrcb_intensity);

                        /* only use nonzero intensities */
                        if (ycrcb_intensity > 0.0) {

                            xx = i - center_x;
                            yy = center_y - j;
int debugprint = MB_NO;
//int icenter_x = (int)center_x;
//int icenter_y = (int)center_y;
//if (i == icenter_x && j == icenter_y) {
//debugprint = MB_YES;
//}
                            rrxysq = xx * xx + yy * yy;
                            rrxy = sqrt(rrxysq);
                            rr = sqrt(rrxysq + zzref * zzref);

                            /* calculate the pixel takeoff angles relative to the platform */
                            phi = RTD * atan2(yy,xx);
                            theta = RTD * acos(zzref / rr);
if (debugprint == MB_YES) {
fprintf(stderr,"Camera: roll:%.3f pitch:%.3f\n", camera_roll, camera_pitch);
fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d", imageUndistort.rows, imageUndistort.cols, i, j, b, g, r);
fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zzref, rr, phi, theta);
}

                            /* rotate pixel location using attitude and zzref */
                            mb_platform_math_attitude_rotate_beam(verbose,
                                xx, yy, zzref,
                                camera_roll, camera_pitch, 0.0,
                                &xx, &yy, &zz,
                                &error);

                            /* recalculate the pixel takeoff angles relative to the camera rig */
                            rrxysq = xx * xx + yy * yy;
                            rrxy = sqrt(rrxysq);
                            rr = sqrt(rrxysq + zz * zz);
                            phi = RTD * atan2(yy, xx);
                            theta = RTD * acos(zz / rr);
if (debugprint == MB_YES) {
fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d", imageUndistort.rows, imageUndistort.cols, i, j, b, g, r);
fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zz, rr, phi, theta);
}

                            /* calculate unit vector relative to the camera rig */
                            vz = cos(DTR * theta);
                            vx = sin(DTR * theta) * cos(DTR * phi);
                            vy = sin(DTR * theta) * sin(DTR * phi);
if (debugprint == MB_YES) {
fprintf(stderr,"camera rig unit vector: %f %f %f\n",vx,vy,vz);
}

                            /* if takeoff angle is too vertical (this is for 2D photomosaics)
                                then do not use this pixel */
                            if (theta <= 80.0) {


                                /* apply rotation of each camera relative to the rig */
                                if (image_camera == 1)
                                    {
                                    vxx = vx * (R.at<double>(0,0)) + vy * (R.at<double>(0,1)) + vz * (R.at<double>(0,2));
                                    vyy = vx * (R.at<double>(1,0)) + vy * (R.at<double>(1,1)) + vz * (R.at<double>(1,2));
                                    vzz = vx * (R.at<double>(2,0)) + vy * (R.at<double>(2,1)) + vz * (R.at<double>(2,2));
if (debugprint == MB_YES) {
fprintf(stderr,"\nR:      %f %f %f\n",R.at<double>(0,0), R.at<double>(0,1), R.at<double>(0,2));
fprintf(stderr,"R:      %f %f %f\n",R.at<double>(1,0), R.at<double>(1,1), R.at<double>(1,2));
fprintf(stderr,"R:      %f %f %f\n",R.at<double>(2,0), R.at<double>(2,1), R.at<double>(2,2));
fprintf(stderr,"Rotation: camera 1 unit vector: %f %f %f    camera 0 unit vector: %f %f %f\n",vx,vy,vz,vxx,vyy,vzz);
}
                                    }
                                else
                                    {
                                    vxx = vx;
                                    vyy = vy;
                                    vzz = vz;
                                    }
if (debugprint == MB_YES) {
fprintf(stderr,"camera unit vector:     %f %f %f\n",vx,vy,vz);
}

                                /* rotate unit vector by camera rig heading */
                                vx = vxx * cos(DTR * camera_heading) + vyy * sin(DTR * camera_heading);
                                vy = -vxx * sin(DTR * camera_heading) + vyy * cos(DTR * camera_heading);
                                vz = vzz;
if (debugprint == MB_YES) {
fprintf(stderr,"camera unit vector rotated by heading %f:     %f %f %f\n",camera_heading,vx,vy,vz);
}

                                /* find the location where this vector intersects the grid */
                                if (use_topography == MB_YES) {
                                    status = mb_topogrid_intersect(verbose, topogrid_ptr,
                                                camera_navlon, camera_navlat, 0.0, camera_sensordepth,
                                                mtodeglon, mtodeglat, vx, vy, vz,
                                                &lon, &lat, &topo, &rr, &error);
                                }
                                else {
                                    rr = standoff_target / vz;
                                    lon = camera_navlon + mtodeglon * vx * rr;
                                    lat = camera_navlat + mtodeglon * vy * rr;
                                    topo = -camera_sensordepth -  standoff_target;
                                }
                                zz = -camera_sensordepth - topo;

                                /* standoff is dot product of camera vector with projected pixel vector */
                                standoff = (cx * rr * vx) + (cy * rr * vy) + (cz * rr * vz);
if (debugprint == MB_YES) {
fprintf(stderr," llz: %.10f %.10f %.3f  range:%.3f  standoff:%.3f\n", lon, lat, topo, rr, standoff);
}


                                /* use pixel if standoff in range */
                                if (standoff >= corr_zmin && standoff <= corr_zmax) {

                                    /* save the intensity in the correct pixel and range bin */
                                    ibin_x = (int)((i + 0.5 * bin_dx) / bin_dx);
                                    ibin_x = MIN(MAX(ibin_x, 0), ncorr_x - 1);
                                    jbin_y = (int)((j + 0.5 * bin_dy) / bin_dy);
                                    jbin_y = MIN(MAX(jbin_y, 0), ncorr_y - 1);
                                    kbin_z = (int)((standoff + 0.5 * bin_dz - corr_zmin) / bin_dz);
                                    kbin_z = MIN(MAX(kbin_z, 0), ncorr_z - 1);
                                    corr_table[iimage].at<float>(ibin_x, jbin_y, kbin_z) += ycrcb_intensity;
                                    corr_table_count[iimage].at<int>(ibin_x, jbin_y, kbin_z) += 1;
if (debugprint == MB_YES) {
fprintf(stderr,"i:%d j:%d sensordepth:%f tide:%f sensordepth:%f topo:%f standoff:%f kbin_z:%d\n",
i,j,sensordepth,tide,sensordepth,topo,standoff,kbin_z);
}

                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);

    /* process camera correction tables */
    for (i=0;i<ncorr_x;i++)
        for (j=0;j<ncorr_y;j++)
            for (k=0;k<ncorr_z;k++) {
                if (corr_table_count[0].at<int>(i, j, k) > 0) {
                    corr_table[0].at<float>(i, j, k) /= corr_table_count[0].at<int>(i, j, k);
fprintf(stderr,"Correction Table[0]: %d %d %d   %d %f\n",i,j,k,corr_table_count[0].at<int>(i, j, k), corr_table[0].at<float>(i, j, k));
                }
                if (corr_table_count[1].at<int>(i, j, k) > 0) {
                    corr_table[1].at<float>(i, j, k) /= corr_table_count[1].at<int>(i, j, k);
fprintf(stderr,"Correction Table[1]: %d %d %d   %d %f\n",i,j,k,corr_table_count[1].at<int>(i, j, k), corr_table[1].at<float>(i, j, k));
                }
            }
    corr_bounds.at<float>(0, 0) = corr_xmin;
    corr_bounds.at<float>(0, 1) = corr_xmax;
    corr_bounds.at<float>(0, 2) = bin_dx;
    corr_bounds.at<float>(1, 0) = corr_ymin;
    corr_bounds.at<float>(1, 1) = corr_ymax;
    corr_bounds.at<float>(1, 2) = bin_dy;
    corr_bounds.at<float>(2, 0) = corr_zmin;
    corr_bounds.at<float>(2, 1) = corr_zmax;
    corr_bounds.at<float>(2, 2) = bin_dz;

    /* Write out the ouput camera correction tables */
    fstorage = FileStorage(ImageCorrectionFile, FileStorage::WRITE);
    if( fstorage.isOpened() ) {
        fstorage << "ImageCorrectionBounds" << corr_bounds
            << "ImageCorrectionTable1" << corr_table[0]
            << "ImageCorrectionTable2" << corr_table[1];
        fstorage.release();
    }
    else
        cout << "Error: Cannot save the image correction tables\n";

    /* deallocate topography grid array if necessary */
    if (use_topography == MB_YES)
        status = mb_topogrid_deall(verbose, &topogrid_ptr, &error);

    /* deallocate navigation arrays if necessary */
    if (navigation_specified == MB_YES) {
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

    /* end it all */
    exit(status);

}
