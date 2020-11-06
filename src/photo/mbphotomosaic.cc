/*--------------------------------------------------------------------
 *    The MB-system:    mbphotomosaic.cpp    10/17/2013
 *
 *    Copyright (c) 1993-2020 by
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
 * mbphotomosaic makes a mosaic of navigated downlooking photographs.
 *
 * Author:    D. W. Caress
 * Date:    October 17, 2013
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

#define MBPM_PRIORITY_CENTRALITY_ONLY               1
#define MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF      2

int main(int argc, char** argv)
{
    char program_name[] = "mbphotomosaic";
    char help_message[] =  "mbphotomosaic makes a mosaic of navigated downlooking photographs.";
    char usage_message[] = "mbphotomosaic \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--show-images\n"
                            "\t--input=imagelist\n"
                            "\t--output=file\n"
                            "\t--image-dimensions=width/height\n"
                            "\t--image-spacing=dx/dy[/units]\n"
                            "\t--fov-fudgefactor=factor\n"
                            "\t--projection=projection_pars\n"
                            "\t--altitude=standoff_target/standoff_range\n"
                            "\t--standoff=standoff_target/standoff_range\n"
                            "\t--rangemax=range_max\n"
                            "\t--bounds=lonmin/lonmax/latmin/latmax | west/east/south/north\n"
                            "\t--bounds-buffer=bounds_buffer\n"
                            "\t--correction-file=imagecorrection.yaml\n"
                            "\t--brightness-correction\n"
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
                            "\t--image-quality-threshold=value\n"
                            "\t--calibration-file=stereocalibration.yaml\n"
                            "\t--navigation-file=file\n"
                            "\t--tide-file=file\n"
                            "\t--topography-grid=file";
    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    int    help = 0;
    int    flag = 0;

    /* Output image variables */
    double  bounds[4];
    int    bounds_specified = MB_NO;
    double bounds_buffer = 6.0;
    int    xdim = 0;
    int    ydim = 0;
    int    spacing_priority = MB_NO;
    int    set_dimensions = MB_NO;
    int    set_spacing = MB_NO;
    double    dx_set = 0.0;
    double    dy_set = 0.0;
    double    dx = 0.0;
    double    dy = 0.0;
    char    units[MB_PATH_MAXLINE];
    Mat     OutputImage;
    mb_path    OutputImageFile;
    int        outputimage_specified = MB_NO;
    mb_path    OutputWorldFile;
    int    priority_mode = MBPM_PRIORITY_CENTRALITY_ONLY;
    float    *priority;
    float    pixel_priority, standoff_priority, pixel_priority_use;
    double    standoff_target = 3.0;
    double    standoff_range = 1.0;
    double    range_max = 200.0;

    /* Input image variables */
    mb_path    ImageListFile;
    mb_path    imageLeftFile;
    mb_path    imageRightFile;
    mb_path    imageFile;
    Mat     imageProcess;
    Mat    imageUndistort;
    Mat    imageUndistortYCrCb;
    Mat     imagePriority;
    int    undistort_initialized = MB_NO;
    double    left_time_d;
    double    time_diff;
    double    time_d;
    int    time_i[7];
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
    mb_path PlatformFile;
    int platform_specified = MB_NO;
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
    int imagelist_camera;
    int image_camera = MBPM_CAMERA_LEFT;
    int use_camera_mode = MBPM_USE_STEREO;
    double imageQualityThreshold = 0.0;
    mb_path StereoCameraCalibrationFile;
    int    calibration_specified = MB_NO;
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
    double center_x, center_y;
    double aspectRatio[2];
    Mat rmap[2][2];
    int use_simple_brightness_correction = MB_NO;
    Scalar avgPixelIntensity;
    float table_intensity, table_intensity_ref;
    double referenceIntensityCorrection[2];
    double avgImageIntensityCorrection;
    double pixelIntensityCorrection;
    double intensityCorrection;
    double intensityChange;
    double fov_fudgefactor = 1.0;
    double camera_navlon;
    double camera_navlat;
    double camera_sensordepth;
    double camera_heading;
    double camera_roll;
    double camera_pitch;

    /* Input image correction */
    mb_path ImageCorrectionFile;
    int correction_specified = MB_NO;
    int ncorr_x = 21;
    int ncorr_y = 21;
    int ncorr_z = 100;
    double corr_xmin = 0.0;
    double corr_xmax = 10.0;
    double corr_ymin = 0.0;
    double corr_ymax = 10.0;
    double corr_zmin = 0.0;
    double corr_zmax = 10.0;
    Mat corr_bounds;
    Mat corr_table[2];
    double bin_dx, bin_dy, bin_dz;
    double vavg;
    int nvavg;
    int ibin_xcen, jbin_ycen, kbin_zcen;
    double v000, v100, v010, v110, v001, v101, v011, v111;
    int ibin_x1, ibin_x2, jbin_y1, jbin_y2, kbin_z1, kbin_z2;
    double factor_x, factor_y, factor_z;

    /* Input navigation variables */
    int navigation_specified = MB_NO;
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
    int use_tide = MB_NO;
    mb_path TideFile;
    int ntide = 0;
    double *ttime = NULL;
    double *ttide = NULL;

    /* topography parameters */
    int use_topography = MB_NO;
    mb_path TopographyGridFile;
    void *topogrid_ptr = NULL;

    /* projected image parameters */
    int use_projection = MB_NO;
    double reference_lon, reference_lat;
    int utm_zone = 1;
    char projection_pars[MB_PATH_MAXLINE];
    char projection_id[MB_PATH_MAXLINE];
    void *pjptr;

    /* image display options */
    bool show_images = false;
    bool show_priority_map = false;

    /* MBIO status variables */
    int status = MB_SUCCESS;
    int verbose = 0;
    int error = MB_ERROR_NO_ERROR;
    char *message;

    /* output stream for basic stuff (stdout if verbose <= 1,
        stderr if verbose > 1) */
    FILE *stream = NULL;
    FILE *tfp;

    int use_this_image;
    mb_path buffer;
    char *result;
    int lonflip;
    double sec;
    double pbounds[4];
    double factor, scale;

    FileStorage fstorage;
    bool isVerticalStereo;
    int npairs, nimages, currentimages;

    /* command line option definitions */
    /* mbphotomosaic
     *    --verbose
     *    --help
     *    --show-image
     *    --show-images
     *    --input=imagelist
     *    --output=file
     *    --image-dimensions=width/height
     *    --image-spacing=dx/dy[/units]
     *    --fov-fudgefactor=factor
     *    --projection=projection_pars
     *    --altitude=standoff_target/standoff_range
     *    --standoff=standoff_target/standoff_range
     *    --rangemax=range_max
     *    --bounds=lonmin/lonmax/latmin/latmax | west/east/south/north
     *    --bounds-buffer=bounds_buffer
     *    --correction-file=imagecorrection.yaml
     *    --brightness-correction
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
     *    --image-quality-threshold=value
     *    --calibration-file=stereocalibration.yaml
     *    --navigation-file=file
     *    --tide-file=file
     *    --topography-grid=file
     *
     */
    static struct option options[] =
        {
        {"verbose",                     no_argument,            NULL,         0},
        {"help",                        no_argument,            NULL,         0},
        {"show-image",                  no_argument,          NULL, 0},
        {"show-images",                 no_argument,          NULL, 0},
        {"input",                       required_argument,      NULL,         0},
        {"output",                      required_argument,      NULL,         0},
        {"image-file",                  required_argument,      NULL,         0},
        {"image-dimensions",            required_argument,      NULL,         0},
        {"image-spacing",               required_argument,      NULL,         0},
        {"fov-fudgefactor",             required_argument,      NULL,         0},
        {"projection",                  required_argument,      NULL,         0},
        {"altitude",                    required_argument,      NULL,         0},
        {"standoff",                    required_argument,      NULL,         0},
        {"rangemax",                    required_argument,      NULL,         0},
        {"bounds",                      required_argument,      NULL,         0},
        {"bounds-buffer",               required_argument,      NULL,         0},
        {"correction-file",             required_argument,      NULL,         0},
        {"brightness-correction",       no_argument,            NULL,         0},
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
        {"image-quality-threshold",     required_argument,      NULL,         0},
        {"calibration-file",            required_argument,      NULL,         0},
        {"navigation-file",             required_argument,      NULL,         0},
        {"tide-file",                   required_argument,      NULL,         0},
        {"topography-grid",             required_argument,      NULL,         0},
        {NULL,                          0,                      NULL,         0}
        };

    /* set default imagelistfile name */
    sprintf(ImageListFile, "imagelist.mb-1");
    sprintf(ImageCorrectionFile, "imagelist_cameracorrection.yml");
    sprintf(OutputImageFile, "testimage.tiff");
    xdim = 1000;
    ydim = 1000;

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

            /*-------------------------------------------------------
             * Image display options */

            /* show-images */
            else if ((strcmp("show-image", options[option_index].name) == 0)
                     || (strcmp("show-images", options[option_index].name) == 0))
                {
                show_images = true;
                //show_priority_map = true;
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
                    || (strcmp("image-file", options[option_index].name) == 0))
                {
                const int n = sscanf (optarg,"%s", OutputImageFile);
                if (n == 1)
                    {
                    outputimage_specified = MB_YES;
                    if (strlen(OutputImageFile) < 6
                        || ((strncmp(".tif", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0)
                            && (strncmp(".tiff", &OutputImageFile[strlen(OutputImageFile)-5], 5) != 0)))
                        {
                        strcat(OutputImageFile, ".tiff");
                        }
                    }
                }

            /* image-dimensions */
            else if (strcmp("image-dimensions", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%d/%d", &xdim, &ydim);
                if (n == 2 && xdim > 0 && ydim > 0)
                  set_dimensions = MB_YES;
                }

            /* image-spacing */
            else if (strcmp("image-spacing", options[option_index].name) == 0)
                {
                if (optarg[strlen(optarg)-1] == '!')
                    {
                    spacing_priority = MB_YES;
                    optarg[strlen(optarg)-1] = '\0';
                    }
                const int n = sscanf (optarg,"%lf/%lf/%s", &dx_set, &dy_set, units);
                if (n > 1 && dx_set > 0.0 && dy_set > 0.0)
                    {
                    set_spacing = MB_YES;
                    if (n < 3)
                        strcpy(units, "meters");
                    }
                }

            /* fov-fudgefactor */
            else if (strcmp("fov-fudgefactor", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &fov_fudgefactor);
                }

            /* projection */
            else if (strcmp("projection", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", projection_pars);
                use_projection = MB_YES;
                }

            /* altitude */
            else if (strcmp("altitude", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf/%lf", &standoff_target, &standoff_range);
                if (n ==2 && standoff_target > 0.0 && standoff_range > 0.0)
                    priority_mode = MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF;
                }

            /* standoff */
            else if (strcmp("standoff", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf/%lf", &standoff_target, &standoff_range);
                if (n ==2 && standoff_target > 0.0 && standoff_range > 0.0)
                    priority_mode = MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF;
                }

            /* rangemax */
            else if (strcmp("rangemax", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &range_max);
                }

            /* bounds */
            else if (strcmp("bounds", options[option_index].name) == 0)
                {
                bounds_specified = mb_get_bounds(optarg, bounds);
                }

            /* bounds-buffer */
            else if (strcmp("bounds-buffer", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &bounds_buffer);
                }

            /* correction-file */
            else if (strcmp("correction-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageCorrectionFile);
                if (n == 1)
                    {
                    correction_specified = MB_YES;
                    if (strlen(ImageCorrectionFile) < 5
                        || strncmp(".yml", &ImageCorrectionFile[strlen(ImageCorrectionFile)-4], 4) != 0)
                        {
                        strcat(ImageCorrectionFile, ".yml");
                        }
                    }
                }

            /* brightness-correction */
            else if (strcmp("brightness-correction", options[option_index].name) == 0)
                {
                use_simple_brightness_correction = MB_YES;
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

            /* image-quality-threshold  (0 <= imageQualityThreshold <= 1) */
            else if (strcmp("image-quality-threshold", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &imageQualityThreshold);
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
                const int n = sscanf (optarg,"%s", NavigationFile);
                if (n == 1)
                    navigation_specified = MB_YES;
                }

            /* tide-file */
            else if (strcmp("tide-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", TideFile);
                if (n == 1)
                    use_tide = MB_YES;
                }

            /* topography-grid */
            else if (strcmp("topography-grid", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", TopographyGridFile);
                if (n == 1)
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

    /* print starting message */
    if (verbose == 1 || help)
        {
        fprintf(stream,"\nProgram %s\n",program_name);
        fprintf(stream,"MB-system Version %s\n",MB_VERSION);
        }

    /* print starting debug statements */
    if (verbose >= 2)
        {
        fprintf(stream,"\ndbg2  Program <%s>\n",program_name);
        fprintf(stream,"dbg2  MB-system Version %s\n",MB_VERSION);
        fprintf(stream,"dbg2  Control Parameters:\n");
        fprintf(stream,"dbg2       verbose:                     %d\n",verbose);
        fprintf(stream,"dbg2       help:                        %d\n",help);
        fprintf(stream,"dbg2       ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"dbg2       use_camera_mode:             %d\n",use_camera_mode);
        fprintf(stream,"dbg2       imageQualityThreshold:       %f\n",imageQualityThreshold);
        fprintf(stream,"dbg2       show_images:                 %d\n",show_images);
        fprintf(stream,"dbg2       OutputImageFile:             %s\n",OutputImageFile);
        fprintf(stream,"dbg2       bounds_specified:            %d\n",bounds_specified);
        fprintf(stream,"dbg2       Bounds: west:                %f\n",bounds[0]);
        fprintf(stream,"dbg2       Bounds: east:                %f\n",bounds[1]);
        fprintf(stream,"dbg2       Bounds: south:               %f\n",bounds[2]);
        fprintf(stream,"dbg2       Bounds: north:               %f\n",bounds[3]);
        fprintf(stream,"dbg2       Bounds buffer:               %f\n",bounds_buffer);
        fprintf(stream,"dbg2       set_spacing:                 %d\n",set_spacing);
        fprintf(stream,"dbg2       spacing_priority:            %d\n",spacing_priority);
        fprintf(stream,"dbg2       dx_set:                      %f\n",dx_set);
        fprintf(stream,"dbg2       dy_set:                      %f\n",dy_set);
        fprintf(stream,"dbg2       set_dimensions:              %d\n",set_dimensions);
        fprintf(stream,"dbg2       xdim:                        %d\n",xdim);
        fprintf(stream,"dbg2       ydim:                        %d\n",ydim);
        fprintf(stream,"dbg2       use_projection:              %d\n",use_projection);
        fprintf(stream,"dbg2       projection_pars:             %s\n",projection_pars);
        fprintf(stream,"dbg2       navigation_specified:              %d\n",navigation_specified);
        fprintf(stream,"dbg2       NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"dbg2       use_tide:                    %d\n",use_tide);
        fprintf(stream,"dbg2       TideFile:                    %s\n",TideFile);
        fprintf(stream,"dbg2       use_topography:              %d\n",use_topography);
        fprintf(stream,"dbg2       TopographyGridFile:          %s\n",TopographyGridFile);
        fprintf(stream,"dbg2       calibration_specified:       %d\n",calibration_specified);
        fprintf(stream,"dbg2       StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"dbg2       correction_specified:              %d\n",correction_specified);
        fprintf(stream,"dbg2       ImageCorrectionFile:         %s\n",ImageCorrectionFile);
        fprintf(stream,"dbg2       fov_fudgefactor:             %f\n",fov_fudgefactor);
        fprintf(stream,"dbg2       PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"dbg2       platform_specified:          %d\n",platform_specified);
        fprintf(stream,"dbg2       camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"dbg2       nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"dbg2       sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"dbg2       heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"dbg2       altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"dbg2       attitude_sensor:             %d\n",attitude_sensor);
        if (priority_mode == MBPM_PRIORITY_CENTRALITY_ONLY)
            fprintf(stream,"dbg2       priority_mode:               %d (priority by centrality in source image only)\n",priority_mode);
        else
            {
            fprintf(stream,"dbg2       priority_mode:               %d (priority by centrality in source image and difference from target standoff)\n",priority_mode);
            fprintf(stream,"dbg2       standoff_target:             %f\n",standoff_target);
            fprintf(stream,"dbg2       standoff_range:              %f\n",standoff_range);
            }
        }
    else if (verbose == 1)
        {
        fprintf(stream,"\nProgram <%s>\n",program_name);
        fprintf(stream,"Control Parameters:\n");
        fprintf(stream,"  ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"  use_camera_mode:             %d\n",use_camera_mode);
        fprintf(stream,"  imageQualityThreshold:       %f\n",imageQualityThreshold);
        fprintf(stream,"  show_images:                 %d\n",show_images);
        fprintf(stream,"  OutputImageFile:             %s\n",OutputImageFile);
        fprintf(stream,"  bounds_specified:            %d\n",bounds_specified);
        fprintf(stream,"  Bounds: west:                %f\n",bounds[0]);
        fprintf(stream,"  Bounds: east:                %f\n",bounds[1]);
        fprintf(stream,"  Bounds: south:               %f\n",bounds[2]);
        fprintf(stream,"  Bounds: north:               %f\n",bounds[3]);
        fprintf(stream,"  Bounds buffer:               %f\n",bounds_buffer);
        fprintf(stream,"  set_spacing:                 %d\n",set_spacing);
        fprintf(stream,"  spacing_priority:            %d\n",spacing_priority);
        fprintf(stream,"  dx_set:                      %f\n",dx_set);
        fprintf(stream,"  dy_set:                      %f\n",dy_set);
        fprintf(stream,"  set_dimensions:              %d\n",set_dimensions);
        fprintf(stream,"  xdim:                        %d\n",xdim);
        fprintf(stream,"  ydim:                        %d\n",ydim);
        fprintf(stream,"  use_projection:              %d\n",use_projection);
        fprintf(stream,"  projection_pars:             %s\n",projection_pars);
        fprintf(stream,"  navigation_specified:              %d\n",navigation_specified);
        fprintf(stream,"  NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"  use_tide:                    %d\n",use_tide);
        fprintf(stream,"  TideFile:                    %s\n",TideFile);
        fprintf(stream,"  use_topography:              %d\n",use_topography);
        fprintf(stream,"  TopographyGridFile:          %s\n",TopographyGridFile);
        fprintf(stream,"  calibration_specified:       %d\n",calibration_specified);
        fprintf(stream,"  StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"  correction_specified:        %d\n",correction_specified);
        fprintf(stream,"  ImageCorrectionFile:         %s\n",ImageCorrectionFile);
        fprintf(stream,"  fov_fudgefactor:             %f\n",fov_fudgefactor);
        fprintf(stream,"  PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"  platform_specified:          %d\n",platform_specified);
        fprintf(stream,"  camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"  nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"  sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"  heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"  altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"  attitude_sensor:             %d\n",attitude_sensor);
        if (priority_mode == MBPM_PRIORITY_CENTRALITY_ONLY)
            fprintf(stream,"  priority_mode:               %d (priority by centrality in source image only)\n",priority_mode);
        else
            {
            fprintf(stream,"  priority_mode:               %d (priority by centrality in source image and difference from target standoff)\n",priority_mode);
            fprintf(stream,"  standoff_target:             %f\n",standoff_target);
            fprintf(stream,"  standoff_range:              %f\n",standoff_range);
            }
        fprintf(stream,"  range_max:                   %f\n",range_max);
        }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
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

        /* if photomosaic bounds not specified use grid bounds */
        if (bounds_specified == MB_NO)
            {
            mb_topogrid_bounds(verbose, topogrid_ptr, bounds, &error);
            }
        }

    /* if bounds not specified then quit */
    if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3])
        {
        fprintf(stream,"\nGrid bounds not properly specified:\n\t%f %f %f %f\n",bounds[0],bounds[1],bounds[2],bounds[3]);
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
        }

    /* if stereo calibration not specified then quit */
    if (calibration_specified == MB_NO)
        {
        fprintf(stream,"\nNo camera calibration file specified\n");
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
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
        for (int isensor=0;isensor<platform->num_sensors;isensor++)
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
    if (calibration_specified == MB_YES)
        {
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
        }

    /* image correction table */
    if (correction_specified == MB_YES) {
        /* read in the image correction table - this is expected to be a stereo correction table */
        fstorage.open(ImageCorrectionFile, FileStorage::READ);
        if(fstorage.isOpened() )
            {
            fstorage["ImageCorrectionBounds"] >> corr_bounds;
            fstorage["ImageCorrectionTable1"] >> corr_table[0];
            fstorage["ImageCorrectionTable2"] >> corr_table[1];
            fstorage.release();
            }
        else
            {
            fprintf(stream,"\nUnable to read image correction file %s\n",
                ImageCorrectionFile);
            fprintf(stream,"\nProgram <%s> Terminated\n",
                program_name);
            error = MB_ERROR_BAD_PARAMETER;
            mb_memory_clear(verbose, &error);
            exit(error);
            }
        ncorr_x = corr_table[0].size[0];
        ncorr_y = corr_table[0].size[1];
        ncorr_z = corr_table[0].size[2];
        corr_xmin = corr_bounds.at<float>(0, 0);
        corr_xmax = corr_bounds.at<float>(0, 1);
        bin_dx = corr_bounds.at<float>(0, 2);
        corr_ymin = corr_bounds.at<float>(1, 0);
        corr_ymax = corr_bounds.at<float>(1, 1);
        bin_dy = corr_bounds.at<float>(1, 2);
        corr_zmin = corr_bounds.at<float>(2, 0);
        corr_zmax = corr_bounds.at<float>(2, 1);
        bin_dz = corr_bounds.at<float>(2, 2);
fprintf(stderr, "\nImage correction:\n");
fprintf(stderr, "x: %d %f %f %f\n", ncorr_x, corr_xmin, corr_xmax, bin_dx);
fprintf(stderr, "y: %d %f %f %f\n", ncorr_y, corr_ymin, corr_ymax, bin_dy);
fprintf(stderr, "z: %d %f %f %f\n", ncorr_z, corr_zmin, corr_zmax, bin_dz);

        /* Get reference intensity value for each camera.
         * If the target standoff value is specified use the correction table value
         * at the center x and y and the target standoff z. If no target standoff
         * value is specified use the correction table value at the x y center
         * which is located halfway between the first and last nonzero values */
        ibin_xcen = ncorr_x / 2;
        jbin_ycen = ncorr_y / 2;
        for (int icamera = 0; icamera < 2; icamera++) {
            if (priority_mode == MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF
                && standoff_target > corr_zmin && standoff_target < corr_zmax) {
                kbin_zcen = (standoff_target - corr_zmin) / bin_dz;
            } else {
                kbin_zcen = ncorr_z / 2;
                int k0 = ncorr_z;
                int k1 = -1;
                for (int k=0;k<ncorr_z;k++) {
                    if (corr_table[icamera].at<float>(ibin_xcen, jbin_ycen, k) > 0.0) {
                        if (k0 > k)
                            k0 = k;
                        k1 = k;
                    }

                    /* reset k0 because there is an isolated nonzero value at k == 0 */
                    else if (k == 1 && k0 == 0) {
                      k0 = ncorr_z;
                      k1 = -1;
                    }
                }
                if (k1 >= k0) {
                  kbin_zcen = (k0 + k1) / 2;
                }
            }
            double referenceIntensity =  corr_table[icamera].at<float>(ibin_xcen, jbin_ycen, kbin_zcen);

            /* Get image correction factor required to bring a pixel intensity equal
             * to the reference value up to a value of 70.0. */
            if (referenceIntensity > 0.0)
                referenceIntensityCorrection[icamera] = 70.0 / referenceIntensity;
            else
                referenceIntensityCorrection[icamera] = 1.0;

fprintf(stderr, "\nImage correction camera: %d\n", icamera);
fprintf(stderr, "center: %d %d %d\n", ibin_xcen, jbin_ycen, kbin_zcen);
fprintf(stderr, "referenceIntensity: %f\n", referenceIntensity);
fprintf(stderr, "referenceIntensityCorrection[%d]: %f\n", icamera, referenceIntensityCorrection[icamera]);
//fprintf(stderr, "\nCorrection Table[%d]:\n", icamera);
//for (int i=0;i<ncorr_x;i++) {
//for (int j=0;j<ncorr_y;j++) {
//for (int k=0;k<ncorr_z;k++) {
//fprintf(stderr,"    %d %d %d   %f\n", i, j, k, corr_table[icamera].at<float>(i, j, k));
//}
//}
//}

        }
    }

    /* else the reference intensity correction is unity, e.g. neutral */
    else {
        referenceIntensityCorrection[0] = 1.0;
        referenceIntensityCorrection[1] = 1.0;
    }

    /* deal with projected gridding */
    double mtodeglon, mtodeglat, deglontokm, deglattokm;
    if (use_projection == MB_YES)
        {
        /* check for UTM with undefined zone */
        if (strcmp(projection_pars, "UTM") == 0
            || strcmp(projection_pars, "U") == 0
            || strcmp(projection_pars, "utm") == 0
            || strcmp(projection_pars, "u") == 0)
            {
            reference_lon = 0.5 * (bounds[0] + bounds[1]);
            if (reference_lon < 180.0)
                reference_lon += 360.0;
            if (reference_lon >= 180.0)
                reference_lon -= 360.0;
            utm_zone = (int)(((reference_lon + 183.0)
                / 6.0) + 0.5);
            reference_lat = 0.5 * (bounds[2] + bounds[3]);
            if (reference_lat >= 0.0)
                sprintf(projection_id, "UTM%2.2dN", utm_zone);
            else
                sprintf(projection_id, "UTM%2.2dS", utm_zone);
            }
        else
            strcpy(projection_id, projection_pars);

        /* set projection flag */
        int proj_status = mb_proj_init(verbose,projection_id,
            &(pjptr), &error);

        /* if projection not successfully initialized then quit */
        if (proj_status != MB_SUCCESS)
            {
            fprintf(stream,"\nOutput projection %s not found in database\n",
                projection_id);
            fprintf(stream,"\nProgram <%s> Terminated\n",
                program_name);
            error = MB_ERROR_BAD_PARAMETER;
            mb_memory_clear(verbose, &error);
            exit(error);
            }

        /* tranlate lon lat bounds from UTM if required */
        if (bounds[0] < -360.0 || bounds[0] > 360.0
            || bounds[1] < -360.0 || bounds[1] > 360.0
            || bounds[2] < -90.0 || bounds[2] > 90.0
            || bounds[3] < -90.0 || bounds[3] > 90.0)
            {
            /* first point */
            double xx = bounds[0];
            double yy = bounds[2];
            double xlon, ylat;
            mb_proj_inverse(verbose, pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = xlon;
            pbounds[1] = xlon;
            pbounds[2] = ylat;
            pbounds[3] = ylat;

            /* second point */
            xx = bounds[1];
            yy = bounds[2];
            mb_proj_inverse(verbose, pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = MIN(pbounds[0], xlon);
            pbounds[1] = MAX(pbounds[1], xlon);
            pbounds[2] = MIN(pbounds[2], ylat);
            pbounds[3] = MAX(pbounds[3], ylat);

            /* third point */
            xx = bounds[0];
            yy = bounds[3];
            mb_proj_inverse(verbose, pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = MIN(pbounds[0], xlon);
            pbounds[1] = MAX(pbounds[1], xlon);
            pbounds[2] = MIN(pbounds[2], ylat);
            pbounds[3] = MAX(pbounds[3], ylat);

            /* fourth point */
            xx = bounds[1];
            yy = bounds[3];
            mb_proj_inverse(verbose, pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = MIN(pbounds[0], xlon);
            pbounds[1] = MAX(pbounds[1], xlon);
            pbounds[2] = MIN(pbounds[2], ylat);
            pbounds[3] = MAX(pbounds[3], ylat);
            }

        /* else translate bounds to UTM */
        else
            {
            /* copy bounds to pbounds */
            pbounds[0] = bounds[0];
            pbounds[1] = bounds[1];
            pbounds[2] = bounds[2];
            pbounds[3] = bounds[3];

            /* first point */
            double xx, yy;
            double xlon = pbounds[0];
            double ylat = pbounds[2];
            mb_proj_forward(verbose, pjptr, xlon, ylat,
                    &xx, &yy, &error);
            bounds[0] = xx;
            bounds[1] = xx;
            bounds[2] = yy;
            bounds[3] = yy;

            /* second point */
            xlon = pbounds[1];
            ylat = pbounds[2];
            mb_proj_forward(verbose, pjptr, xlon, ylat,
                    &xx, &yy, &error);
            bounds[0] = MIN(bounds[0], xx);
            bounds[1] = MAX(bounds[1], xx);
            bounds[2] = MIN(bounds[2], yy);
            bounds[3] = MAX(bounds[3], yy);

            /* third point */
            xlon = pbounds[0];
            ylat = pbounds[3];
            mb_proj_forward(verbose, pjptr, xlon, ylat,
                    &xx, &yy, &error);
            bounds[0] = MIN(bounds[0], xx);
            bounds[1] = MAX(bounds[1], xx);
            bounds[2] = MIN(bounds[2], yy);
            bounds[3] = MAX(bounds[3], yy);

            /* fourth point */
            xlon = pbounds[1];
            ylat = pbounds[3];
            mb_proj_forward(verbose, pjptr, xlon, ylat,
                    &xx, &yy, &error);
            bounds[0] = MIN(bounds[0], xx);
            bounds[1] = MAX(bounds[1], xx);
            bounds[2] = MIN(bounds[2], yy);
            bounds[3] = MAX(bounds[3], yy);
            }

        /* calculate grid properties */
        if (set_spacing == MB_YES)
            {
            xdim = (bounds[1] - bounds[0])/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = dx_set;
            ydim = (bounds[3] - bounds[2])/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                bounds[1] = bounds[0] + dx_set * (xdim - 1);
                bounds[3] = bounds[2] + dy_set * (ydim - 1);
                }
            if (units[0] == 'M' || units[0] == 'm')
                strcpy(units, "meters");
            else if (units[0] == 'K' || units[0] == 'k')
                strcpy(units, "km");
            else if (units[0] == 'F' || units[0] == 'f')
                strcpy(units, "feet");
            else
                strcpy(units, "unknown");
            }

        mb_coor_scale(verbose,0.5*(pbounds[2]+pbounds[3]),&mtodeglon,&mtodeglat);

/* fprintf(stream," Projected coordinates on: proj_status:%d  projection:%s\n",
proj_status, projection_id);
fprintf(stream," Lon Lat Bounds: %f %f %f %f\n",
pbounds[0], pbounds[1], pbounds[2], pbounds[3]);
fprintf(stream," XY Bounds: %f %f %f %f\n",
bounds[0], bounds[1], bounds[2], bounds[3]);*/
        }

    /* deal with no projection */
    else
        {
        /* calculate grid properties */
        mb_coor_scale(verbose,0.5*(bounds[2]+bounds[3]),&mtodeglon,&mtodeglat);
        deglontokm = 0.001/mtodeglon;
        deglattokm = 0.001/mtodeglat;
        if (set_spacing == MB_YES
            && (units[0] == 'M' || units[0] == 'm'))
            {
            xdim = (bounds[1] - bounds[0])/(mtodeglon*dx_set) + 1;
            if (dy_set <= 0.0)
                dy_set = mtodeglon * dx_set / mtodeglat;
            ydim = (bounds[3] - bounds[2])/(mtodeglat*dy_set) + 1;
            if (spacing_priority == MB_YES)
                {
                bounds[1] = bounds[0] + mtodeglon * dx_set * (xdim - 1);
                bounds[3] = bounds[2] + mtodeglat * dy_set * (ydim - 1);
                }
            strcpy(units, "meters");
            }
        else if (set_spacing == MB_YES
            && (units[0] == 'K' || units[0] == 'k'))
            {
            xdim = (bounds[1] - bounds[0])*deglontokm/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = deglattokm * dx_set / deglontokm;
            ydim = (bounds[3] - bounds[2])*deglattokm/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                bounds[1] = bounds[0] + dx_set * (xdim - 1) / deglontokm;
                bounds[3] = bounds[2] + dy_set * (ydim - 1) / deglattokm;
                }
            strcpy(units, "km");
            }
        else if (set_spacing == MB_YES
            && (units[0] == 'F' || units[0] == 'f'))
            {
            xdim = (bounds[1] - bounds[0])/(mtodeglon*0.3048*dx_set) + 1;
            if (dy_set <= 0.0)
                dy_set = mtodeglon * dx_set / mtodeglat;
            ydim = (bounds[3] - bounds[2])/(mtodeglat*0.3048*dy_set) + 1;
            if (spacing_priority == MB_YES)
                {
                bounds[1] = bounds[0] + mtodeglon * 0.3048 * dx_set * (xdim - 1);
                bounds[3] = bounds[2] + mtodeglat * 0.3048 * dy_set * (ydim - 1);
                }
            strcpy(units, "feet");
            }
        else if (set_spacing == MB_YES)
            {
            xdim = (bounds[1] - bounds[0])/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = dx_set;
            ydim = (bounds[3] - bounds[2])/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                bounds[1] = bounds[0] + dx_set * (xdim - 1);
                bounds[3] = bounds[2] + dy_set * (ydim - 1);
                }
            strcpy(units, "degrees");
            }

        /* copy bounds to pbounds */
        pbounds[0] = bounds[0];
        pbounds[1] = bounds[1];
        pbounds[2] = bounds[2];
        pbounds[3] = bounds[3];
        }

    /* calculate other grid properties */
    dx = (bounds[1] - bounds[0])/(xdim-1);
    dy = (bounds[3] - bounds[2])/(ydim-1);

    /* pbounds is used to check for images of interest using navigation
      - expand it a little to account for image size */
    pbounds[0] -= mtodeglon * bounds_buffer;
    pbounds[1] += mtodeglon * bounds_buffer;
    pbounds[2] -= mtodeglat * bounds_buffer;
    pbounds[3] += mtodeglat * bounds_buffer;

    /* output information */
    if (verbose >= 1)
        {
        fprintf(stdout,"\nOutput Image Parameters:\n");
        fprintf(stream,"  OutputImageFile:    %s\n",OutputImageFile);
        if (use_projection == MB_YES)
            fprintf(stream,"  projection:         %s\n",projection_id);
        else
            fprintf(stream,"  projection:         Geographic\n");
        fprintf(stream,"  Bounds: west:       %.9f\n",bounds[0]);
        fprintf(stream,"  Bounds: east:       %.9f\n",bounds[1]);
        fprintf(stream,"  Bounds: south:      %.9f\n",bounds[2]);
        fprintf(stream,"  Bounds: north:      %.9f\n",bounds[3]);
        fprintf(stream,"  dx:                 %.9f\n",dx);
        fprintf(stream,"  dy:                 %.9f\n",dy);
        fprintf(stream,"  xdim:               %d\n",xdim);
        fprintf(stream,"  ydim:               %d\n",ydim);
        }

        /* Create an image */
    OutputImage.create(ydim, xdim, CV_8UC3);
    size_t size = xdim*ydim*sizeof(float);
    status = mb_mallocd(verbose,__FILE__,__LINE__,size,(void **)&priority,&error);
    if (status == MB_FAILURE)
        {
        fprintf(stderr,"\nUnable to allocate %.2f MBytes memory for mosaic priority array\n",((xdim*ydim*sizeof(float))/1048576.0));
        fprintf(stderr,"\nProgram <%s> Terminated\n",
            program_name);
        exit(error);
        }
    for (unsigned int kkk=0;kkk<xdim*ydim;kkk++)
        priority[kkk] = 0.0;

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
            bool value_ok = false;

            /* read the navigation from an fnv file */
            int nget = sscanf(buffer,"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                &time_i[0],&time_i[1],&time_i[2],
                &time_i[3],&time_i[4],&sec,
                &ntime[nnav],
                &nlon[nnav],&nlat[nnav],
                &nheading[nnav],&nspeed[nnav],&ndraft[nnav],
                &nroll[nnav],&npitch[nnav],&nheave[nnav]);
            if (nget >= 15)
                value_ok = true;

            /* make sure longitude is defined according to lonflip */
            if (value_ok)
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
            if (verbose >= 5 && value_ok)
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
            if (value_ok)
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
            fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n",NavigationFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            bool value_ok = false;

            /* read the navigation from an fnv file */
            int nget = sscanf(buffer,"%lf %lf",
                &ttime[ntide], &ttide[ntide]);
            if (nget >= 2)
                value_ok = true;

            /* output some debug values */
            if (verbose >= 5 && value_ok)
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
            if (value_ok)
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
fprintf(stderr,"About to read TopographyGridFile: %s\n", TopographyGridFile);
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
fprintf(stderr,"Done reading TopographyGridFile: %s\n", TopographyGridFile);

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

    /* prepare to display images */
    String windowNameImage = "Source Image & RGB Histograms";
    if (show_images) {
        namedWindow(windowNameImage, 0);
    }
    String windowNamePriority = "Priority Map";
    if (show_priority_map) {
        namedWindow(windowNamePriority, 0);
    }

    /* loop over single images or stereo pairs in the imagelist file */
    npairs = 0;
    nimages = 0;
    int imageStatus = MB_IMAGESTATUS_NONE;
    double imageQuality = 0.0;
    mb_path dpath;
    fprintf(stderr,"About to read ImageListFile: %s\n", ImageListFile);

    while ((status = mb_imagelist_read(verbose, imagelist_ptr, &imageStatus,
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &time_diff, &imageQuality, &error)) == MB_SUCCESS) {
        if (imageStatus == MB_IMAGESTATUS_STEREO) {
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

            /* check imageQuality value against threshold to see if this image should be used */
            if (use_this_image == MB_YES && imageQuality < imageQualityThreshold) {
              use_this_image = MB_NO;
            }

            /* check navigation for location close to or inside destination image bounds */
            if (use_this_image == MB_YES) {
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
                    if (navlon < pbounds[0] || navlon > pbounds[1]
                        || navlat < pbounds[2] || navlat > pbounds[3]) {
                        use_this_image = MB_NO;
                    }
                }
                else {
                    use_this_image = MB_NO;
                }
            }

            /* read the image */
            if (use_this_image == MB_YES) {
                /* read the image */
                imageProcess = imread(imageFile);
                if (imageProcess.empty()) {
                    use_this_image = MB_NO;
                }
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
                            fprintf(stderr,"  Image width (pixels):         %d\n", imageSize.width);
                            fprintf(stderr,"  Image height (pixels):        %d\n", imageSize.height);
                            fprintf(stderr,"  Sensor width (mm):            %f\n", SensorWidthMm);
                            fprintf(stderr,"  Sensor height (mm):           %f\n", SensorHeightMm);
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
                            fprintf(stderr,"  Image width (pixels):         %d\n", imageSize.width);
                            fprintf(stderr,"  Image height (pixels):        %d\n", imageSize.height);
                            fprintf(stderr,"  Sensor width (mm):            %f\n", SensorWidthMm);
                            fprintf(stderr,"  Sensor height (mm):           %f\n", SensorHeightMm);
                            fprintf(stderr,"  Horizontal field of view:     %f\n", fovx[1]);
                            fprintf(stderr,"  Vertical field of view:       %f\n", fovy[1]);
                            fprintf(stderr,"  Focal length (sensor pixels): %f\n", focalLength[1]);
                            fprintf(stderr,"  Focal length (mm):            %f\n", focalLength[1] * SensorCellMm);
                            fprintf(stderr,"  Principal point x (mm):       %f\n", principalPoint[1].x);
                            fprintf(stderr,"  Principal point y (mm):       %f\n", principalPoint[1].y);
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
                        center_x = principalPoint[0].x / SensorCellMm;
                        center_y = principalPoint[0].y / SensorCellMm;
                    }
                    else {
                        fov_x = fovx[1];
                        fov_y = fovy[1];
                        center_x = principalPoint[1].x / SensorCellMm;
                        center_y = principalPoint[1].y / SensorCellMm;
                    }
                }

                /* else leave the image alone, treat the center as the principal point,
                    and use specified offsets */
                else {
                    imageUndistort = imageProcess.clone();
                    if (image_camera == 0) {
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
                double zzref = 0.5 * (0.5 * imageSize.width / tan(DTR * 0.5 * fov_x * fov_fudgefactor)
                        + 0.5 * imageSize.height / tan(DTR * 0.5 * fov_y * fov_fudgefactor));

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

                /* Apply camera model translation */
                double dlon, dlat, dz;
                double headingx = sin(DTR * camera_heading);
                double headingy = cos(DTR * camera_heading);
                if (image_camera == 0) {
                    dlon = 0.5 * (T.at<double>(0)) * mtodeglon;
                    dlat = 0.5 * (T.at<double>(1)) * mtodeglat;
                    dz = 0.5 * (T.at<double>(2));
                } else {
                    dlon = -0.5 * (T.at<double>(0)) * mtodeglon;
                    dlat = -0.5 * (T.at<double>(1)) * mtodeglat;
                    dz = -0.5 * (T.at<double>(2));
                }
                camera_navlon += (headingy * dlon + headingx * dlat);
                camera_navlat += (-headingx * dlon + headingy * dlat);
                camera_sensordepth += dz;
//fprintf(stderr,"%s:%d:%s: Camera %d: %.9f %.9f %.3f %.3f %.3f   %.3f %.3f %.3f\n",
//__FILE__, __LINE__, __func__, image_camera,
//camera_navlon, camera_navlat, (camera_navlon - navlon) / mtodeglon, (camera_navlat - navlat) / mtodeglat, camera_sensordepth,
//camera_heading, camera_roll, camera_pitch);

                /* Process this image */

                if (show_images) {
                    /* get some statistics on this image */
                    vector<Mat> bgr_planes;
                    split( imageUndistort, bgr_planes );
                    int histSize = 256;
                    float range[] = { 0, 256 }; //the upper boundary is exclusive
                    const float* histRange = { range };
                    bool uniform = true, accumulate = false;
                    Mat b_hist, g_hist, r_hist;
                    calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
                    calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
                    calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );
                    int hist_w = imageUndistort.cols, hist_h = imageUndistort.rows;
                    int bin_w = cvRound( (double) hist_w/histSize );
                    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
                    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
                    normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
                    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
                    for ( int i = 1; i < histSize; i++ ) {
                        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ),
                              Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                              Scalar( 255, 0, 0), 2, 8, 0  );
                        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ),
                              Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                              Scalar( 0, 255, 0), 2, 8, 0  );
                        line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ),
                              Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                              Scalar( 0, 0, 255), 2, 8, 0  );
                    }
                    Mat imgConcat;
                    hconcat(imageUndistort, histImage, imgConcat);
                    imshow(windowNameImage, imgConcat );
                    waitKey(1);
                }

                /* calculate the largest distance from center for this image for use
                    in calculating pixel priority */
                double xx = MAX(center_x, imageUndistort.cols - center_x);
                double yy = MAX(center_y, imageUndistort.rows - center_y);
                double rrxymax = sqrt(xx * xx + yy * yy);

                /* make and display a priority map */
                if (show_priority_map) {
                    imagePriority = imageUndistort.clone();
                    for (int i=0; i<imageUndistort.cols; i++) {
                        for (int j=0; j<imageUndistort.rows; j++) {
                            /* calculate the pixel priority based on the distance
                                from the image center */
                            xx = i - center_x;
                            yy = center_y - j;
                            double rrxy = sqrt(xx * xx + yy * yy);
                            double pixel_priority = (rrxymax - rrxy) / rrxymax;
                            unsigned char r = (unsigned char)(pixel_priority * 255);
                            imagePriority.at<Vec3b>(j,i)[0] = r;
                            imagePriority.at<Vec3b>(j,i)[1] = r;
                            imagePriority.at<Vec3b>(j,i)[2] = r;
//if (xx == yy)fprintf(stderr,"PRIORITY xx:%7.1f yy:%7.1f rrxy:%10.5f rrxymax:%10.5f pixel_priority:%10.8f %3u\n",
//            xx,yy,rrxy,rrxymax,pixel_priority,r);
                        }
                    }
                    imshow(windowNamePriority, imagePriority);
                    waitKey(500);
                }

                /* calculate the average intensity of the image
                 * if specified calculate the correction required to bring the
                 * average intensity to a value of 70.0
                 */
                avgPixelIntensity = mean(imageUndistortYCrCb);
                if (use_simple_brightness_correction == MB_YES) {
                    avgImageIntensityCorrection = 70.0 / avgPixelIntensity.val[0];
                }
                else {
                     avgImageIntensityCorrection = 1.0;
                }
                mb_get_date(verbose, time_d, time_i);
                fprintf(stderr,"%4d Camera:%d Image:%s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d LLZ: %.10f %.10f %8.3f Tide:%7.3f H:%6.2f R:%6.2f P:%6.2f Avg Intensity:%.3f\n",
                        (nimages - currentimages + iimage), image_camera, imageFile,
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                        camera_navlon, camera_navlat, camera_sensordepth, tide,
                        camera_heading, camera_roll, camera_pitch, avgPixelIntensity.val[0]);

                /* get unit vector for direction camera is pointing */

                /* rotate center pixel location using attitude and zzref */
                double zz;
                mb_platform_math_attitude_rotate_beam(verbose,
                    0.0, 0.0, zzref,
                    camera_roll, camera_pitch, 0.0,
                    &xx, &yy, &zz,
                    &error);
                double rr = sqrt(xx * xx + yy * yy + zz * zz);
                double phi = RTD * atan2(yy, xx);
                double theta = RTD * acos(zz / rr);

                /* calculate unit vector relative to the camera rig */
                double vx = sin(DTR * theta) * cos(DTR * phi);
                double vy = sin(DTR * theta) * sin(DTR * phi);
                double vz = cos(DTR * theta);

                /* apply rotation of each camera relative to the rig */
                double vxx, vyy, vzz;
                if (image_camera == 1) {
                    vxx = vx * (R.at<double>(0,0)) + vy * (R.at<double>(0,1)) + vz * (R.at<double>(0,2));
                    vyy = vx * (R.at<double>(1,0)) + vy * (R.at<double>(1,1)) + vz * (R.at<double>(1,2));
                    vzz = vx * (R.at<double>(2,0)) + vy * (R.at<double>(2,1)) + vz * (R.at<double>(2,2));
                }
                else {
                    vxx = vx;
                    vyy = vy;
                    vzz = vz;
                }

                /* rotate unit vector by camera rig heading */
                double cx = vxx * cos(DTR * camera_heading) + vyy * sin(DTR * camera_heading);
                double cy = -vxx * sin(DTR * camera_heading) + vyy * cos(DTR * camera_heading);
                double cz = vzz;

                /* loop over the pixels */
                for (int i=0; i<imageUndistort.cols; i++) {
                    for (int j=0; j<imageUndistort.rows; j++) {
                        /* calculate the pixel priority based on the distance
                            from the image center */
                        double xx = i - center_x;
                        double yy = center_y - j;
//int debugprint = MB_NO;
//int icenter_x = (int)center_x;
//int icenter_y = (int)center_y;
//if (i == icenter_x && j == icenter_y) {
//debugprint = MB_YES;
//}
                        double rrxysq = xx * xx + yy * yy;
                        double rrxy = sqrt(rrxysq);
                        double rr = sqrt(rrxysq + zzref * zzref);
                        double pixel_priority = (rrxymax - rrxy) / rrxymax;
//if (debugprint == MB_YES) {
//fprintf(stderr,"\nPRIORITY xx:%f yy:%f zzref:%f rr:%f rrxy:%f rrxymax:%f pixel_priority:%f\n",
//xx,yy,zzref,rr,rrxy,rrxymax,pixel_priority);
//}

                        /* calculate the pixel takeoff angles relative to the camera rig */
                        double phi = RTD * atan2(yy, xx);
                        double theta = RTD * acos(zzref / rr);

                        /* calculate the angular width of a single pixel */
                        double rrxysq2 = (rrxy + 1.0) * (rrxy + 1.0);
                        double rr2 = sqrt(rrxysq2 + zzref * zzref);
                        double theta2 = RTD * acos(zzref / rr2);
                        double dtheta = theta2 - theta;
//if (debugprint == MB_YES) {
//fprintf(stderr,"Camera: roll:%.3f pitch:%.3f\n", camera_roll, camera_pitch);
//fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d", imageUndistort.rows, imageUndistort.cols, i, j, b, g, r);
//fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zzref, rr, phi, theta);
//}

                        /* rotate pixel location using attitude and zzref */
                        double zz;
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
//if (debugprint == MB_YES) {
//fprintf(stderr,"Rows:%d Cols:%d | %5d %5d BGR:%3.3d|%3.3d|%3.3d", imageUndistort.rows, imageUndistort.cols, i, j, b, g, r);
//fprintf(stderr," xyz:%f %f %f r:%f   phi:%f theta:%f\n", xx, yy, zz, rr, phi, theta);
//}

                        /* calculate unit vector relative to the camera rig */
                        double vz = cos(DTR * theta);
                        double vx = sin(DTR * theta) * cos(DTR * phi);
                        double vy = sin(DTR * theta) * sin(DTR * phi);
//if (debugprint == MB_YES) {
//fprintf(stderr,"camera rig unit vector: %f %f %f\n",vx,vy,vz);
//}
                        /* if takeoff angle is too vertical (this is a 2D photomosaic)
                            then do not use this pixel */
                        double vxx, vyy, vzz;
                        double standoff = 0.0;
                        double lon, lat, topo;
                        if (theta <= 80.0) {

                            /* apply rotation of each camera relative to the rig */
                            if (image_camera == 1) {
                                vxx = vx * (R.at<double>(0,0)) + vy * (R.at<double>(0,1)) + vz * (R.at<double>(0,2));
                                vyy = vx * (R.at<double>(1,0)) + vy * (R.at<double>(1,1)) + vz * (R.at<double>(1,2));
                                vzz = vx * (R.at<double>(2,0)) + vy * (R.at<double>(2,1)) + vz * (R.at<double>(2,2));
//    if (debugprint == MB_YES) {
//    fprintf(stderr,"\nR:      %f %f %f\n",R.at<double>(0,0), R.at<double>(0,1), R.at<double>(0,2));
//    fprintf(stderr,"R:      %f %f %f\n",R.at<double>(1,0), R.at<double>(1,1), R.at<double>(1,2));
//    fprintf(stderr,"R:      %f %f %f\n",R.at<double>(2,0), R.at<double>(2,1), R.at<double>(2,2));
//    fprintf(stderr,"Rotation: camera 1 unit vector: %f %f %f    camera 0 unit vector: %f %f %f\n",vx,vy,vz,vxx,vyy,vzz);
//    }
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
                            vx = vxx * cos(DTR * camera_heading) + vyy * sin(DTR * camera_heading);
                            vy = -vxx * sin(DTR * camera_heading) + vyy * cos(DTR * camera_heading);
                            vz = vzz;
//if (debugprint == MB_YES) {
//fprintf(stderr,"camera unit vector rotated by heading %f:     %f %f %f\n",camera_heading,vx,vy,vz);
//}

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
//if (debugprint == MB_YES) {
//fprintf(stderr," llz: %.10f %.10f %.3f  range:%.3f  standoff:%.3f\n", lon, lat, topo, rr, standoff);
//}

                        }

                        /* use pixel if not too vertical or range too large */
                        if (theta <= 80.0 && rr < range_max) {

                            /* modulate the source pixel priority by the standoff if desired */
                            if (priority_mode == MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF)  {
                                double dstandoff = (standoff - standoff_target) / standoff_range;
                                double standoff_priority = (float) (exp(-dstandoff * dstandoff));
                                pixel_priority *= standoff_priority;
                            }
//if (debugprint == MB_YES) {
//fprintf(stderr," standoff:%f pixel_priority:%.3f\n", standoff, pixel_priority);
//}

                            /* get the intensity correction using interpolation of the correction table */
                            if (correction_specified == MB_YES) {
                                ibin_x1 = MIN(MAX(((int)((i + 0.5 * bin_dx) / bin_dx)), 0), ncorr_x - 2);
                                ibin_x2 = ibin_x1 + 1;
                                factor_x = ((double)i - 0.5 * bin_dx) / bin_dx - (double)ibin_x1;
                                jbin_y1 = MIN(MAX(((int)((j + 0.5 * bin_dy) / bin_dy)), 0), ncorr_y - 2);
                                jbin_y2 = jbin_y1 + 1;
                                factor_y = ((double)j - 0.5 * bin_dy) / bin_dy - (double)jbin_y1;
                                kbin_z1 = MIN(MAX(((int)((standoff + 0.5 * bin_dz) / bin_dz)), 0), ncorr_z - 2);
                                kbin_z2 = kbin_z1 + 1;
                                factor_z = ((double)standoff - 0.5 * bin_dz) / bin_dz - (double)kbin_z1;
                                factor_x = MIN(MAX(factor_x, 0.0), 1.0);
                                factor_y = MIN(MAX(factor_y, 0.0), 1.0);
                                factor_z = MIN(MAX(factor_z, 0.0), 1.0);
//fprintf(stderr,"i:%d j:%d sensordepth:%f tide:%f sensordepth:%f topo:%f standoff:%f kbin_z:%d %d\n",
//i,j,sensordepth,tide,sensordepth,topo,standoff,kbin_z1,kbin_z2);

                                /* get reference intensity from center of image at the current standoff */
                                table_intensity_ref =  (1.0 - factor_z) * corr_table[image_camera].at<float>(ibin_xcen, jbin_ycen, kbin_z1)
                                            + factor_z * corr_table[image_camera].at<float>(ibin_xcen, jbin_ycen, kbin_z2);

                                v000 = corr_table[image_camera].at<float>(ibin_x1, jbin_y1, kbin_z1);
                                v100 = corr_table[image_camera].at<float>(ibin_x2, jbin_y1, kbin_z1);
                                v010 = corr_table[image_camera].at<float>(ibin_x1, jbin_y2, kbin_z1);
                                v001 = corr_table[image_camera].at<float>(ibin_x1, jbin_y1, kbin_z2);
                                v101 = corr_table[image_camera].at<float>(ibin_x2, jbin_y1, kbin_z2);
                                v011 = corr_table[image_camera].at<float>(ibin_x1, jbin_y2, kbin_z2);
                                v110 = corr_table[image_camera].at<float>(ibin_x2, jbin_y2, kbin_z1);
                                v111 = corr_table[image_camera].at<float>(ibin_x2, jbin_y2, kbin_z2);
                                vavg = v000 + v100 + v010 + v110 + v001 + v101 + v011 + v111;
                                nvavg = 0;
                                if (v000 > 0.0) nvavg++;
                                if (v100 > 0.0) nvavg++;
                                if (v010 > 0.0) nvavg++;
                                if (v110 > 0.0) nvavg++;
                                if (v001 > 0.0) nvavg++;
                                if (v101 > 0.0) nvavg++;
                                if (v011 > 0.0) nvavg++;
                                if (v111 > 0.0) nvavg++;
                                if (nvavg > 0)
                                    vavg /= nvavg;
                                if (v000 == 0.0) v000 = vavg;
                                if (v100 == 0.0) v100 = vavg;
                                if (v010 == 0.0) v010 = vavg;
                                if (v110 == 0.0) v110 = vavg;
                                if (v001 == 0.0) v001 = vavg;
                                if (v101 == 0.0) v101 = vavg;
                                if (v011 == 0.0) v011 = vavg;
                                if (v111 == 0.0) v111 = vavg;

                                /* use trilinear interpolation from http://paulbourke.net/miscellaneous/interpolation/ */
                                table_intensity = v000 * (1.0 - factor_x) * (1.0 - factor_y) * (1.0 - factor_x)
                                        + v100 * factor_x * (1.0 - factor_y) * (1.0 - factor_z)
                                        + v010 * (1.0 - factor_x) * factor_y * (1.0 - factor_z)
                                        + v001 * (1.0 - factor_x) * (1.0 - factor_y) * factor_z
                                        + v101 * factor_x * (1.0 - factor_y) * factor_z
                                        + v011 * (1.0 - factor_x) * factor_y * factor_z
                                        + v110 * factor_x * factor_y * (1.0 - factor_z)
                                        + v111 * factor_x * factor_y * factor_z;
                                if (table_intensity > 0.0)
                                    pixelIntensityCorrection = table_intensity_ref / table_intensity;
                                else {
                                    pixelIntensityCorrection = 1.0;
                                }

//fprintf(stderr,"pixelIntensityCorrection:%f table_intensity_ref:%f table_intensity:%f\n",
//pixelIntensityCorrection,table_intensity_ref,table_intensity);
                            }

                            /* else do no correction */
                            else {
                                pixelIntensityCorrection = 1.0;
                            }

                            /* apply intensity correction combining all specified corrections */
                            intensityCorrection = referenceIntensityCorrection[image_camera]
                                                    * avgImageIntensityCorrection
                                                    * pixelIntensityCorrection;

                            /* access the Y value from the YCrCb image */
                            intensityChange = (intensityCorrection - 1.0) * ((double) imageUndistortYCrCb.at<Vec3b>(j,i)[0]);
//fprintf(stderr,"i:%d j:%d BGR: %f %f %f  Correction:%f  Y: %f %f\n",
//i,j,db,dg,dr,intensityCorrection,(double) imageUndistortYCrCb.at<Vec3b>(j,i)[0],dy);

                            /* apply the pixel correction */
                            unsigned char b = saturate_cast<unsigned char>(imageUndistort.at<Vec3b>(j,i)[0] + intensityChange);
                            unsigned char g = saturate_cast<unsigned char>(imageUndistort.at<Vec3b>(j,i)[1] + intensityChange);
                            unsigned char r = saturate_cast<unsigned char>(imageUndistort.at<Vec3b>(j,i)[2] + intensityChange);
//if (r+g+b < 20)
//fprintf(stderr,"%5d %5d  rgb: %3d %3d %3d   %3d %3d %3d   intensityChange:%f\n",
//i,j,imageUndistort.at<Vec3b>(j,i)[0],
//imageUndistort.at<Vec3b>(j,i)[1],
//imageUndistort.at<Vec3b>(j,i)[2],
//r,g,b,intensityChange);

                            /* find the location and footprint of the input pixel
                                in the output image */
                            unsigned int iii, jjj, kkk;
                            double pixel_dx, pixel_dy;
                            if (use_projection == MB_YES) {
                                /* pixel location */
                                mb_proj_forward(verbose, pjptr, lon, lat, &xx, &yy, &error);
                                iii = (xx - bounds[0] + 0.5 * dx) / dx;
                                jjj = (bounds[3] - yy + 0.5 * dy) / dy;
                                kkk = xdim * jjj + iii;

                                /* pixel footprint size */
                                pixel_dx = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) / dx;
                                pixel_dy = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) / dy;
                            }

                            else {
                                /* pixel location */
                                iii = (lon - bounds[0] + 0.5 * dx) / dx;
                                jjj = (bounds[3] - lat + 0.5 * dy) / dy;
                                kkk = xdim * jjj + iii;

                                /* pixel footprint size */
                                pixel_dx = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) * (mtodeglon / dx);
                                pixel_dy = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) * (mtodeglat / dy);
                            }

                            /* figure out the "footprint" extent of the mapped pixel */
                            unsigned int iii1 = iii - floor(pixel_dx);
                            unsigned int iii2 = iii + floor(pixel_dx);
                            unsigned int jjj1 = jjj - floor(pixel_dy);
                            unsigned int jjj2 = jjj + floor(pixel_dy);
//fprintf(stderr,"i:%d j:%d iii:%d:%d:%d jjj:%d:%d:%d theta:%f dtheta:%f pixel_dx:%f pixel_dy:%f dx:%g dy:%g mtodeglon:%g mtodeglat:%g\n",
//i,j,iii1,iii,iii2,jjj1,jjj,jjj2,theta,dtheta,pixel_dx,pixel_dy,dx,dy,mtodeglon,mtodeglat);
//fprintf(stderr,"iii:%d jjj:%d\n",iii,jjj);
//if (debugprint == MB_YES) {
//fprintf(stderr,"       standoff:%f pixel_priority:%.3f\n", standoff, pixel_priority);
//}

                            for (unsigned int ipix=iii1;ipix<=iii2;ipix++) {
                                for (unsigned int jpix=jjj1;jpix<=jjj2;jpix++) {
                                    unsigned int kpix = xdim * jpix + ipix;
                                    if (ipix == iii && jpix == jjj)
                                        pixel_priority_use = pixel_priority;
                                    else if (ipix > iii-2 && iii < iii+2 && jpix > jjj-2 && jpix < jjj+2)
                                        pixel_priority_use = 0.99 * pixel_priority;
                                    else
                                        pixel_priority_use = 0.98 * pixel_priority;
                                    if (ipix >= 0 && ipix < xdim
                                        && jpix >= 0 && jpix < ydim
                                        && pixel_priority_use > priority[kpix]) {
//if (debugprint == MB_YES) {
//fprintf(stderr,"              Pixel used: i:%d j:%d  ipix:%d jpix:%d  BGR:%d %d %d Priority:%f %f\n",
//i,j,ipix,jpix,OutputImage.at<Vec3b>(jpix,ipix)[0],OutputImage.at<Vec3b>(jpix,ipix)[1],OutputImage.at<Vec3b>(jpix,ipix)[2],
//pixel_priority_use, priority[kpix]);
//}
                                        OutputImage.at<Vec3b>(jpix,ipix)[0] = b;
                                        OutputImage.at<Vec3b>(jpix,ipix)[1] = g;
                                        OutputImage.at<Vec3b>(jpix,ipix)[2] = r;
                                        priority[kpix] = pixel_priority_use;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    /* end display images */
    if (show_images) {
        destroyWindow(windowNameImage);
    }
    if (show_images) {
        destroyWindow(windowNamePriority);
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);

    /* Write out the ouput image */
    status = imwrite(OutputImageFile,OutputImage);
    if (status == MB_SUCCESS)
        {
        /* output world file */
        strcpy(OutputWorldFile, OutputImageFile);
        OutputWorldFile[strlen(OutputImageFile)-5] = '\0';
        strcat(OutputWorldFile,".tfw");
        if ((tfp = fopen(OutputWorldFile,"w")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to open output world file: %s\n",
                  OutputWorldFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                  program_name);
            exit(error);
            }

        /* write out world file contents */
        fprintf(tfp, "%.10g\r\n0.0\r\n0.0\r\n%.10g\r\n%.10g\r\n%.10g\r\n",
            dx, -dy,
            bounds[0] - 0.5 * dx,
            bounds[3] + 0.5 * dy);

        /* close the world file */
        fclose(tfp);

        /* announce it */
        fprintf(stderr, "\nOutput photomosaic: %s\n",OutputImageFile);

        }
    else
        {
        fprintf(stderr, "Could not save: %s\n",OutputImageFile);
        }

    /* deallocate priority array */
    status = mb_freed(verbose,__FILE__,__LINE__,(void **)&priority,&error);

    /* deallocate topography grid array if necessary */
    if (use_topography == MB_YES)
        status = mb_topogrid_deall(verbose, &topogrid_ptr, &error);

    /* deallocate navigation arrays if necessary */
    if (navigation_specified == MB_YES)
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

    /* end it all */
    exit(status);

}
