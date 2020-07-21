/*--------------------------------------------------------------------
 *    The MB-system:    mbphotogrammetry.cpp    11/12/2014
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
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <getopt.h>

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

#define MBPG_ALLOC_SMALL_NUM    64
#define MBPG_ALLOC_LARGE_NUM    262144

#define MBPG_BIN_FILTER_MEAN    0
#define MBPG_BIN_FILTER_MEDIAN    1

std::string s;
std::stringstream out;

using namespace std;
using namespace cv;

int main(int argc, char** argv)
{
    char program_name[] = "mbphotogrammetry";
    char help_message[] = "mbphotogrammetry generates bathymetry from stereo pairs of photographs through photogrammetry.";
    char usage_message[] = "mbphotogrammetry \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--show-images\n"
                            "\t--input=imagelist\n"
                            "\t--navigation-file=file\n"
                            "\t--survey-line-file=file\n"
                            "\t--tide-file=file\n"
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
    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    int    help = 0;
    int    flag = 0;

    /* command line option definitions */
    /* mbphotogrammetry
     *         --verbose
     *         --help
     *         --show-images
     *         --input=imagelist
     *         --image-quality-threshold=value
     *         --navigation-file=file
     *         --survey-line-file=file
     *         --tide-file=file
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
        {"show-images",                     no_argument,          NULL, 0},
        {"input",                           required_argument,    NULL, 0},
        {"image-quality-threshold",         required_argument,      NULL,         0},
        {"navigation-file",                 required_argument,    NULL, 0},
        {"survey-line-file",                required_argument,    NULL, 0},
        {"tide-file",                       required_argument,    NULL, 0},
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
        {"good-fraction-threshold",                  required_argument,    NULL, 0},
        { NULL,                             0,                    NULL, 0}
        };

    /* Input image variables */
    bool show_images = false;
    mb_path ImageListFile;
    mb_path imageLeftFile;
    mb_path imageRightFile;
    double left_time_d;
    double time_diff;
    double time_d;
    int time_i[7];
    double navlon;
    double navlat;
    double speed;
    double heading;
    double distance;
    double altitude;
    double sensordepth, sensordepth_use;
    double draft;
    double roll;
    double pitch;
    double heave;
    double tide;

    /* Output bathymetry variables */
    mb_path OutputFileRoot;
    mb_path OutputFile;
    int output_number_pairs = 0;
    void *mbio_ptr = NULL;
    void *store_ptr = NULL;
    struct mb_io_struct *mb_io_ptr;
    struct mbsys_stereopair_struct *store;
    struct mbsys_stereopair_sounding_struct *sounding;

    /* Input navigation variables */
    int use_navigation = MB_NO;
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

    /* Input auto-line data */
    mb_path    SurveyLineTimeFile;
    int    use_surveylinetimefile = MB_NO;
    int    ntimepoint = 0;
    int    ntimepointalloc = 0;
    double    *routetime_d = NULL;
    double    *routelon = NULL;
    double    *routelat = NULL;
    double    *routeheading = NULL;
    int    *routewaypoint = NULL;
    int    waypoint = 0;
    int    active_waypoint = 0;
    int    new_output_file = MB_NO;

    /* Input tide variables */
    int    use_tide = MB_NO;
    mb_path    TideFile;
    int    ntide = 0;
    double    *ttime = NULL;
    double    *ttide = NULL;

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
    double imageQualityThreshold = 0.0;
    int    camerasInitialized = MB_NO;
    mb_path    StereoCameraCalibrationFile;
    int    use_calibration = MB_NO;
    Mat     cameraMatrix[2], distCoeffs[2];
    Mat     R, T, R1, P1, R2, P2, Q;
    Rect    roi1, roi2;
    Size    imageSize[2];
    Mat     map11, map12, map21, map22;
    Mat     img1g, img2g, img1r, img2r;
    Mat     disp, dispf, disp8;
    double    SensorWidthMm = 8.789;
    double    SensorHeightMm = 6.610;
    double    SensorCellMm =0.00454;
    double    fovx[2], fov_x;
    double    fovy[2], fov_y;
    double    focalLength[2];
    Point2d    principalPoint[2];
    double    center_x, center_y;
    double    aspectRatio[2];
    Mat     rmap[2][2];
    double camera_navlon;
    double camera_navlat;
    double camera_sensordepth;
    double camera_heading;
    double camera_roll;
    double camera_pitch;

        /* algorithm parameters */
    enum { STEREO_BM=0, STEREO_SGBM=1, STEREO_HH=2, STEREO_VAR=3 };
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
    Ptr<StereoBM> bm = StereoBM::create();
    Ptr<StereoSGBM> sgbm = StereoSGBM::create();
    //StereoVar     var;  // StereoVar is no more available in opencv>3, use StereoBM or StereoSGBM instead
    Mat img1, img2;
    double min_disparity;
    double max_disparity;
    double disparity;
    Vec3f  point, direction;
    double pw;

    /* binning variables */
    double    *disparity_bin = NULL;
    size_t    num_bin = 0;
    size_t    num_bin_alloc = 0;

    /* MBIO status variables */
    int    status = MB_SUCCESS;
    int    verbose = 0;
    int    error = MB_ERROR_NO_ERROR;
    char    *message;

    /* output stream for basic stuff (stdout if verbose <= 1,
        stderr if verbose > 1) */
    FILE    *stream = NULL;
    FILE    *tfp;

    int    use_this_pair;
    mb_path buffer;
    char    *result;
    int    size, len, nget, value_ok;
    double    xlon, ylat;
    int    lonflip;
    double    sec;
    double    headingx, headingy;
    double    mtodeglon, mtodeglat;
    double    range;
    double    alphar, betar;
    double    bathr, bathacrosstrack, bathalongtrack, bath, bathlon, bathlat;
    int    obeams_bath, obeams_amp, opixels_ss;

    FileStorage    fstorage;
    bool    isVerticalStereo;
    int    npairs = 0;
    int    output_count = 0;
    int    nimages, ngood, nbad;
    int    isensor;
    int    image_camera;
    int    i, j, k;
    int    ii, jj;
    int    ib, jb, iuse, juse, n;
    int    istart, iend, jstart, jend;

    /* set default imagelistfile name */
    sprintf(ImageListFile, "imagelist.txt");

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
             * Define input image list file */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", ImageListFile);
                }

            /*-------------------------------------------------------
             * Set input image quality threshold  (0 <= imageQualityThreshold <= 1) */

            /* image-quality-threshold */
            else if (strcmp("image-quality-threshold", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%lf", &imageQualityThreshold);
                }

            /*-------------------------------------------------------
             * Define navigation file */

            /* navigation-file */
            else if (strcmp("navigation-file", options[option_index].name) == 0)
                {
                strcpy(NavigationFile, optarg);
                use_navigation = MB_YES;
                }

            /*-------------------------------------------------------
             * Define survey line file */

            /* survey-line-file */
            else if (strcmp("survey-line-file", options[option_index].name) == 0)
                {
                strcpy(SurveyLineTimeFile, optarg);
                use_surveylinetimefile = MB_YES;
                }

            /*-------------------------------------------------------
             * Define tide file */

            /* tide-file */
            else if (strcmp("tide-file", options[option_index].name) == 0)
                {
                strcpy(TideFile, optarg);
                use_tide = MB_YES;
                }

            /*-------------------------------------------------------
             * Define output */

            /* output */
            else if (strcmp("output", options[option_index].name) == 0)
                {
                n = sscanf (optarg,"%s", OutputFileRoot);
                }

            /* output-number-pairs */
            else if (strcmp("output-number-pairs", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &output_number_pairs);
                }

            /*-------------------------------------------------------
             * Define stereo camera calibration file */

            /* camera-calibration-file  or calibration-file */
            else if (strcmp("camera-calibration-file", options[option_index].name) == 0
                      || strcmp("calibration-file", options[option_index].name) == 0)
                {
                strcpy(StereoCameraCalibrationFile, optarg);
                use_calibration = MB_YES;
                }

            /*-------------------------------------------------------
             * Define platform */

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

            /*-------------------------------------------------------
             * Define target altitude and altitude range */

            /* altitude-min */
            else if (strcmp("altitude-min", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%lf", &altitude);
                }

            /* altitude-max */
            else if (strcmp("altitude-max", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%lf", &altitude_max);
                }

            /* trim */
            else if (strcmp("trim", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%lf", &trim);
                }

            /* bin-size */
            else if (strcmp("bin-size", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &bin_size);
                }

            /* bin-filter */
            else if (strcmp("bin-filter", options[option_index].name) == 0)
                {
                if (strcmp(optarg, "mean") == 0 || strcmp(optarg, "MEAN") == 0)
                    bin_filter = MBPG_BIN_FILTER_MEAN;
                else if (strcmp(optarg, "median") == 0 || strcmp(optarg, "MEDIAN") == 0)
                    bin_filter = MBPG_BIN_FILTER_MEDIAN;
                else
                    n = sscanf(optarg, "%d", &bin_filter);
                }

            /* downsample */
            else if (strcmp("downsample", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &downsample);
                }

            /*-------------------------------------------------------
             * Define algorithm and parameters */

            /* algorithm */
            else if (strcmp("algorithm", options[option_index].name) == 0)
                {
                if (strcmp(optarg, "bm") == 0 || strcmp(optarg, "BM") == 0)
                    algorithm = STEREO_BM;
                else if (strcmp(optarg, "sgbm") == 0 || strcmp(optarg, "SGBM") == 0)
                    algorithm = STEREO_SGBM;
                else if (strcmp(optarg, "hh") == 0 || strcmp(optarg, "HH") == 0)
                    algorithm = STEREO_HH;
                else if (strcmp(optarg, "var") == 0 || strcmp(optarg, "VAR") == 0)
                    algorithm = STEREO_VAR;
                }

            /* algorithm-pre-filter-cap=value */
            else if (strcmp("algorithm-pre-filter-cap", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &preFilterCap);
                }

            /* algorithm-sad-window-size */
            else if (strcmp("algorithm-sad-window-size", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &SADWindowSize);
                }

            /* algorithm-smoothing-penalty-1 */
            else if (strcmp("algorithm-smoothing-penalty-1", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &SmoothingPenalty1);
                }

            /* algorithm-smoothing-penalty-2 */
            else if (strcmp("algorithm-smoothing-penalty-2", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &SmoothingPenalty2);
                }

            /* algorithm-min-disparity */
            else if (strcmp("algorithm-min-disparity", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &minDisparity);
                }

            /* algorithm-number-disparities */
            else if (strcmp("algorithm-number-disparities", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &numberOfDisparities);
                }

            /* algorithm-uniqueness-ratio */
            else if (strcmp("algorithm-uniqueness-ratio", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &uniquenessRatio);
                }

            /* algorithm-speckle-window-size */
            else if (strcmp("algorithm-speckle-window-size", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &speckleWindowSize);
                }

            /* algorithm-speckle-range */
            else if (strcmp("algorithm-speckle-range", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &speckleRange);
                }

            /* algorithm-disp-12-max-diff */
            else if (strcmp("algorithm-disp-12-max-diff", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &disp12MaxDiff);
                }

            /* algorithm-texture-threshold */
            else if (strcmp("algorithm-texture-threshold", options[option_index].name) == 0)
                {
                n = sscanf(optarg, "%d", &textureThreshold);
                }

            break;
        case '?':
            errflg++;
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
        fprintf(stream,"dbg2       show_images:                 %d\n",show_images);
        fprintf(stream,"dbg2       ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"dbg2       imageQualityThreshold:       %f\n",imageQualityThreshold);
        fprintf(stream,"dbg2       use_navigation:              %d\n",use_navigation);
        fprintf(stream,"dbg2       NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"dbg2       use_surveylinetimefile:      %d\n",use_surveylinetimefile);
        fprintf(stream,"dbg2       SurveyLineTimeFile:          %s\n",SurveyLineTimeFile);
        fprintf(stream,"dbg2       use_tide:                    %d\n",use_tide);
        fprintf(stream,"dbg2       TideFile:                    %s\n",TideFile);
        fprintf(stream,"dbg2       OutputFileRoot:              %s\n",OutputFileRoot);
        fprintf(stream,"dbg2       output_number_pairs:         %d\n",output_number_pairs);
        fprintf(stream,"dbg2       use_calibration:             %d\n",use_calibration);
        fprintf(stream,"dbg2       StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"dbg2       PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"dbg2       platform_specified:          %d\n",platform_specified);
        fprintf(stream,"dbg2       camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"dbg2       nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"dbg2       sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"dbg2       heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"dbg2       altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"dbg2       attitude_sensor:             %d\n",attitude_sensor);
        fprintf(stream,"dbg2       algorithm:                   %d\n",algorithm);
        fprintf(stream,"dbg2       altitude_min:                %f\n",altitude_min);
        fprintf(stream,"dbg2       altitude_max:                %f\n",altitude_max);
        fprintf(stream,"dbg2       trim:                        %f\n",trim);
        fprintf(stream,"dbg2       bin_size:                    %d\n",bin_size);
        fprintf(stream,"dbg2       bin_filter:                  %d\n",bin_filter);
        fprintf(stream,"dbg2       downsample:                  %d\n",downsample);
        fprintf(stream,"dbg2       preFilterCap:                %d\n",preFilterCap);
        fprintf(stream,"dbg2       SADWindowSize:               %d\n",SADWindowSize);
        fprintf(stream,"dbg2       SmoothingPenalty1:           %d\n",SmoothingPenalty1);
        fprintf(stream,"dbg2       SmoothingPenalty2:           %d\n",SmoothingPenalty2);
        fprintf(stream,"dbg2       minDisparity:                %d\n",minDisparity);
        fprintf(stream,"dbg2       numberOfDisparities:         %d\n",numberOfDisparities);
        fprintf(stream,"dbg2       uniquenessRatio:             %d\n",uniquenessRatio);
        fprintf(stream,"dbg2       speckleWindowSize:           %d\n",speckleWindowSize);
        fprintf(stream,"dbg2       speckleRange:                %d\n",speckleRange);
        fprintf(stream,"dbg2       disp12MaxDiff:               %d\n",disp12MaxDiff);
        fprintf(stream,"dbg2       textureThreshold:            %d\n",textureThreshold);
        }
    else if (verbose == 1)
        {
        fprintf(stream,"\nProgram <%s>\n",program_name);
        fprintf(stream,"MB-system Version %s\n",MB_VERSION);
        fprintf(stream,"Control Parameters:\n");
        fprintf(stream,"     verbose:                     %d\n",verbose);
        fprintf(stream,"     help:                        %d\n",help);
        fprintf(stream,"     show_images:                 %d\n",show_images);
        fprintf(stream,"     ImageListFile:               %s\n",ImageListFile);
        fprintf(stream,"     imageQualityThreshold:       %f\n",imageQualityThreshold);
        fprintf(stream,"     use_navigation:              %d\n",use_navigation);
        fprintf(stream,"     NavigationFile:              %s\n",NavigationFile);
        fprintf(stream,"     use_surveylinetimefile:      %d\n",use_surveylinetimefile);
        fprintf(stream,"     SurveyLineTimeFile:          %s\n",SurveyLineTimeFile);
        fprintf(stream,"     use_tide:                    %d\n",use_tide);
        fprintf(stream,"     TideFile:                    %s\n",TideFile);
        fprintf(stream,"     OutputFileRoot:              %s\n",OutputFileRoot);
        fprintf(stream,"     output_number_pairs:         %d\n",output_number_pairs);
        fprintf(stream,"     use_calibration:             %d\n",use_calibration);
        fprintf(stream,"     StereoCameraCalibrationFile: %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"     PlatformFile:                %s\n",PlatformFile);
        fprintf(stream,"     platform_specified:          %d\n",platform_specified);
        fprintf(stream,"     camera_sensor:               %d\n",camera_sensor);
        fprintf(stream,"     nav_sensor:                  %d\n",nav_sensor);
        fprintf(stream,"     sensordepth_sensor:          %d\n",sensordepth_sensor);
        fprintf(stream,"     heading_sensor:              %d\n",heading_sensor);
        fprintf(stream,"     altitude_sensor:             %d\n",altitude_sensor);
        fprintf(stream,"     attitude_sensor:             %d\n",attitude_sensor);
        fprintf(stream,"     algorithm:                   %d\n",algorithm);
        fprintf(stream,"     altitude_min:                %f\n",altitude_min);
        fprintf(stream,"     altitude_max:                %f\n",altitude_max);
        fprintf(stream,"     trim:                        %f\n",trim);
        fprintf(stream,"     bin_size:                    %d\n",bin_size);
        fprintf(stream,"     bin_filter:                  %d\n",bin_filter);
        fprintf(stream,"     downsample:                  %d\n",downsample);
        fprintf(stream,"     preFilterCap:                %d\n",preFilterCap);
        fprintf(stream,"     SADWindowSize:               %d\n",SADWindowSize);
        fprintf(stream,"     SmoothingPenalty1:           %d\n",SmoothingPenalty1);
        fprintf(stream,"     SmoothingPenalty2:           %d\n",SmoothingPenalty2);
        fprintf(stream,"     minDisparity:                %d\n",minDisparity);
        fprintf(stream,"     numberOfDisparities:         %d\n",numberOfDisparities);
        fprintf(stream,"     uniquenessRatio:             %d\n",uniquenessRatio);
        fprintf(stream,"     speckleWindowSize:           %d\n",speckleWindowSize);
        fprintf(stream,"     speckleRange:                %d\n",speckleRange);
        fprintf(stream,"     disp12MaxDiff:               %d\n",disp12MaxDiff);
        fprintf(stream,"     textureThreshold:            %d\n",textureThreshold);
        }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
        }

    /* if navigation not specified then quit */
    if (use_navigation == MB_NO)
        {
        fprintf(stream,"\nNavigation file not specified:....\n");
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
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
    if (use_calibration == MB_YES)
        {
        if (verbose > 0) {
        fprintf(stream,"\nAbout to read stereo camera calibration file: %s\n",StereoCameraCalibrationFile);

        }
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

    /* read in navigation */
    if (use_navigation == MB_YES)
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
        }

    /* if specified read route time list file */
    if (use_surveylinetimefile == MB_YES)
        {
        /* open the input file */
        if ((tfp = fopen(SurveyLineTimeFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            status = MB_FAILURE;
            fprintf(stderr,"\nUnable to open survey line time file <%s> for reading\n",SurveyLineTimeFile);
            exit(status);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
                {
            if (buffer[0] != '#')
                {
                nget = sscanf(buffer,"%d %d %lf %lf %lf %lf",
                    &i, &waypoint, &navlon, &navlat, &heading, &time_d);

                /* if good data check for need to allocate more space */
                if (ntimepoint + 1 > ntimepointalloc)
                        {
                        ntimepointalloc += MBPG_ALLOC_SMALL_NUM;
                    status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
                                (void **)&routelon, &error);
                    status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
                                (void **)&routelat, &error);
                    status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
                                (void **)&routeheading, &error);
                    status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(int),
                                (void **)&routewaypoint, &error);
                    status = mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double),
                                (void **)&routetime_d, &error);
                        if (status != MB_SUCCESS)
                            {
                        mb_error(verbose,error,&message);
                        fprintf(stderr,"\nMBIO Error allocating data arrays:\n%s\n",
                            message);
                        fprintf(stderr,"\nProgram <%s> Terminated\n",
                            program_name);
                        exit(error);
                            }
                        }

                /* add good point to route */
                if (ntimepointalloc > ntimepoint)
                    {
                    routewaypoint[ntimepoint] = waypoint;
                    routelon[ntimepoint] = navlon;
                    routelat[ntimepoint] = navlat;
                    routeheading[ntimepoint] = heading;
                    routetime_d[ntimepoint] = time_d;
                    ntimepoint++;
                    }
                }
            }

        /* close the file */
        fclose(tfp);
        tfp = NULL;
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
            fprintf(stderr,"\nUnable to Open Navigation File <%s> for reading\n", TideFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            value_ok = MB_NO;

            /* read the tide data */
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

    /* if needed set output file name root - output files need to have
        *.mb251 suffix whether specified or not */
    if (strlen(OutputFileRoot) == 0)
        {
        strcpy(OutputFileRoot, ImageListFile);
        len = strlen(OutputFileRoot);
        if (len > 5 && strcmp(".mb-2", &OutputFileRoot[len-5]) == 0)
            OutputFileRoot[len-5] = '\0';
        }

    /* Open output good imagelist including good disparity fraction values */
    mb_path OutputImagelist;
    FILE *oilfp = NULL;
    sprintf(OutputImagelist, "%s_ImagePairs.mb-2", OutputFileRoot);
    if ((oilfp = fopen(OutputImagelist, "w")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to open output imagelist file <%s> for writing\n",OutputImagelist);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }

    /* loop over single images or stereo pairs in the imagelist file */
    npairs = 0;
    nimages = 0;
    waypoint = 0;
    int imagestatus = MB_IMAGESTATUS_NONE;
    mb_path dpath;
    double imageQuality = 0.0;
    fprintf(stderr,"About to read ImageListFile: %s\n", ImageListFile);

    while ((status = mb_imagelist_read(verbose, imagelist_ptr, &imagestatus,
                                imageLeftFile, imageRightFile, dpath,
                                &left_time_d, &time_diff, &imageQuality, &error)) == MB_SUCCESS) {
        use_this_pair = MB_NO;
        if (imagestatus == MB_IMAGESTATUS_STEREO) {
            use_this_pair = MB_YES;

            /* check imageQuality value against threshold to see if this image should be used */
            if (use_this_pair == MB_YES && imageQuality < imageQualityThreshold) {
              use_this_pair = MB_NO;
            }

            /* check that navigation is available for this stereo pair */
            if (!(nnav > 0 && left_time_d >= ntime[0] && left_time_d <= ntime[nnav-1])) {
                use_this_pair = MB_NO;
            }

            /* Read the stereo pair */
            img1 = imread(imageLeftFile, -1);
            img2 = imread(imageRightFile, -1);
            if (img1.empty()) {
                fprintf(stderr,"Unable to read left file %s\n", imageLeftFile);
                use_this_pair = MB_NO;
            }
            else if (img2.empty()) {
                fprintf(stderr,"Unable to read right file %s\n", imageRightFile);
                use_this_pair = MB_NO;
            }
            else {
                imageSize[0] = img1.size();
                imageSize[1] = img2.size();
                if (imageSize[0] != imageSize[1]) {
                  fprintf(stderr,"Right and left images not the same size: %d:%d != %d:%d\n",
                          imageSize[0].width, imageSize[0].height, imageSize[1].width, imageSize[1].height);
                  use_this_pair = MB_NO;
                }
            }
        }

        /* process the stereo pair */
        if (use_this_pair == MB_YES) {

            /* display images */
            if (show_images) {
                String windowNameLeft = "Left";
                namedWindow(windowNameLeft, 0);
                imshow(windowNameLeft, img1);
                waitKey(1000);
                destroyWindow(windowNameLeft);
                String windowNameRight = "Right";
                namedWindow(windowNameRight, 0);
                imshow(windowNameRight, img2);
                waitKey(1000);
                destroyWindow(windowNameRight);
            }

            /* get navigation attitude and tide for this stereo pair */
            time_d = left_time_d;
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
            tide = 0.0;
            if (ntide > 1) {
                intstat = mb_linear_interp(verbose,
                            ttime-1, ttide-1,
                            ntide, time_d, &tide, &iitime,
                            &error);
            }
            sensordepth = sensordepth - tide;

            /* get coordinate scaling */
            mb_coor_scale(verbose,navlat,&mtodeglon,&mtodeglat);

            /* calculate target sensor position - this is a stereo pair and we
              want to navigate the center or average of the two cameras */
            double cnavlon[2];
            double cnavlat[2];
            double csensordepth[2];
            double cheading[2];
            double croll[2];
            double cpitch[2];
            for (image_camera = 0; image_camera < 2; image_camera++) {
              status = mb_platform_position(verbose, (void *)platform,
                              camera_sensor, image_camera,
                              navlon, navlat, sensordepth,
                              heading, roll, pitch,
                              &cnavlon[image_camera], &cnavlat[image_camera], &csensordepth[image_camera],
                              &error);
              status = mb_platform_orientation_target(verbose, (void *)platform,
                              camera_sensor, image_camera,
                              heading, roll, pitch,
                              &cheading[image_camera], &croll[image_camera], &cpitch[image_camera],
                              &error);
            }
            camera_navlon = 0.5 * (cnavlon[0] + cnavlon[1]);
            camera_navlat = 0.5 * (cnavlat[0] + cnavlat[1]);
            camera_sensordepth = 0.5 * (csensordepth[0] + csensordepth[1]);
            camera_heading = 0.5 * (cheading[0] + cheading[1]);
            camera_roll = 0.5 * (croll[0] + croll[1]);
            camera_pitch = 0.5 * (cpitch[0] + cpitch[1]);

            // The photogrammetry is calculated relative to the first camera (camera 0)
            // so we need to add the offset between the first camera and the navigated
            // center of the rig to individual sounding positions - the first
            // contribution is from the platform model and the second is from the
            // T vector of the stereo camera calibration. This offset is in the
            // frame of the camera rig, i.e. in acrosstrack and alongtrack distance.
            // Generally if the camera rig is calibrated then the platform model
            // has no offset between the two cameras, and so the calculation is
            // entirely from the translation or T vector of the stereo calibration.
            double headingx = sin(DTR * camera_heading);
            double headingy = cos(DTR * camera_heading);
            double pg_xtrack_offset = headingx * (cnavlon[0] - camera_navlon) / mtodeglon
                                        + headingx * (cnavlon[0] - camera_navlon) / mtodeglon;
            double pg_ltrack_offset = (cnavlat[0] - camera_navlat) / mtodeglat;
            double pg_z_offset = csensordepth[0] - camera_sensordepth;
            pg_z_offset = csensordepth[0] - camera_sensordepth;
            pg_xtrack_offset += 0.5 * (T.at<double>(0));
            pg_ltrack_offset += 0.5 * (T.at<double>(1));
            pg_z_offset += 0.5 * (T.at<double>(2));

            /* initialize cameras, calibration, and stereo algorithm */
            if (camerasInitialized == MB_NO) {
                camerasInitialized = MB_YES;

                /* set the desired disparity range */
                min_disparity = (Q.at<double>(3,3) + Q.at<double>(2,3) / altitude_max) / Q.at<double>(3,2);
                max_disparity = (Q.at<double>(3,3) + Q.at<double>(2,3) / altitude_min) / Q.at<double>(3,2);
fprintf(stderr, "%s:%d:%s: Q.at<double>(2,3):%f Q.at<double>(3,2):%f min max disparity: %f %f\n",
__FILE__, __LINE__, __func__, Q.at<double>(2,3), Q.at<double>(3,2), min_disparity, max_disparity);

                /* Set parameters for algorithms */
                if (algorithm == STEREO_BM) {
fprintf(stderr, "%s:%d:%s: algorithm == STEREO_BM\n", __FILE__, __LINE__, __func__);
                    bm->setROI1(roi1);
                    bm->setROI2(roi2);
                    bm->setPreFilterCap(preFilterCap);
                    bm->setBlockSize(SADWindowSize);
                    bm->setMinDisparity(minDisparity);
                    bm->setNumDisparities(numberOfDisparities);
                    bm->setTextureThreshold(textureThreshold);
                    bm->setUniquenessRatio(uniquenessRatio);
                    bm->setSpeckleWindowSize(speckleWindowSize);
                    bm->setSpeckleRange(speckleRange);
                    bm->setDisp12MaxDiff(disp12MaxDiff);
                }
                else if (algorithm == STEREO_SGBM || algorithm == STEREO_HH) {
fprintf(stderr, "%s:%d:%s: algorithm == STEREO_SGBM || algorithm == STEREO_HH\n", __FILE__, __LINE__, __func__);
                    if (algorithm == STEREO_SGBM) {
                        sgbm->setMode( StereoSGBM::MODE_SGBM );
fprintf(stderr, "%s:%d:%s: algorithm == STEREO_SGBM\n", __FILE__, __LINE__, __func__);
                    }
                    else { //if (algorithm == STEREO_HH) {
                        sgbm->setMode( StereoSGBM::MODE_HH );
fprintf(stderr, "%s:%d:%s: algorithm == STEREO_HH\n", __FILE__, __LINE__, __func__);
                    }

                    sgbm->setPreFilterCap(preFilterCap);
                    sgbm->setBlockSize(SADWindowSize);
                    sgbm->setP1(SmoothingPenalty1);
                    sgbm->setP2(SmoothingPenalty2);
                    sgbm->setMinDisparity(minDisparity);
                    sgbm->setNumDisparities(numberOfDisparities);
                    sgbm->setUniquenessRatio(uniquenessRatio);
                    sgbm->setSpeckleWindowSize(speckleWindowSize);
                    sgbm->setSpeckleRange(speckleRange);
                    sgbm->setDisp12MaxDiff(speckleRange);
                } else {
fprintf(stderr, "%s:%d:%s: no algorithm\n", __FILE__, __LINE__, __func__);
                }
//                else if (algorithm == STEREO_VAR) {
//                    var.levels = 3;                                 // ignored with USE_AUTO_PARAMS
//                    var.pyrScale = 0.5;                             // ignored with USE_AUTO_PARAMS
//                    var.nIt = 25;
//                    var.minDisp = -numberOfDisparities;
//                    var.maxDisp = 0;
//                    var.poly_n = 3;
//                    var.poly_sigma = 0.0;
//                    var.fi = 15.0f;
//                    var.lambda = 0.03f;
//                    var.penalization = var.PENALIZATION_TICHONOV;   // ignored with USE_AUTO_PARAMS
//                    var.cycle = var.CYCLE_V;                        // ignored with USE_AUTO_PARAMS
//                    var.flags = var.USE_SMART_ID | var.USE_AUTO_PARAMS | var.USE_INITIAL_DISPARITY | var.USE_MEDIAN_FILTERING ;
//                }

                /* set up rectification */
                if( use_calibration == MB_YES ) {
                    calibrationMatrixValues(cameraMatrix[0], imageSize[0],
                                SensorWidthMm, SensorHeightMm,
                                fovx[0], fovy[0], focalLength[0],
                                principalPoint[0], aspectRatio[0]);
                    calibrationMatrixValues(cameraMatrix[1], imageSize[1],
                                SensorWidthMm, SensorHeightMm,
                                fovx[1], fovy[1], focalLength[1],
                                principalPoint[1], aspectRatio[1]);
                    if (verbose > 0) {
                        fprintf(stderr,"\nLeft Camera Characteristics:\n");
                        fprintf(stderr,"  Image width (pixels):         %d\n", imageSize[0].width);
                        fprintf(stderr,"  Image height (pixels):        %d\n", imageSize[0].height);
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
                        fprintf(stderr,"  Image width (pixels):         %d\n", imageSize[1].width);
                        fprintf(stderr,"  Image height (pixels):        %d\n", imageSize[1].height);
                        fprintf(stderr,"  Sensor width (mm):            %f\n", SensorWidthMm);
                        fprintf(stderr,"  Sensor height (mm):           %f\n", SensorHeightMm);
                        fprintf(stderr,"  Horizontal field of view:     %f\n", fovx[1]);
                        fprintf(stderr,"  Vertical field of view:       %f\n", fovy[1]);
                        fprintf(stderr,"  Focal length (sensor pixels): %f\n", focalLength[1]);
                        fprintf(stderr,"  Focal length (mm):            %f\n", focalLength[1] * SensorCellMm);
                        fprintf(stderr,"  Principal point x:            %f\n", principalPoint[1].x);
                        fprintf(stderr,"  Principal point y:            %f\n", principalPoint[1].y);
                        fprintf(stderr,"  Principal point x (pixels):   %f\n", principalPoint[1].x / SensorCellMm);
                        fprintf(stderr,"  Principal point y (pixels):   %f\n", principalPoint[1].y / SensorCellMm);
                        fprintf(stderr,"  Aspect ratio:                 %f\n", aspectRatio[1]);
                        fprintf(stderr,"\nStereo depth resolution:\n");
                        fprintf(stderr,"  minDisparity:                 %d\n", minDisparity);
                        fprintf(stderr,"  Number of disparities:        %d\n", numberOfDisparities);
                        fprintf(stderr,"  altitude_min:                 %f\n", altitude_min);
                        fprintf(stderr,"  altitude_max:                 %f\n", altitude_max);
                        fprintf(stderr,"  min_disparity:                %f\n", min_disparity);
                        fprintf(stderr,"  max_disparity:                %f\n\n", max_disparity);
                        fprintf(stderr,"  trim:                         %f\n", trim);
                        fprintf(stderr,"  bin_size:                     %d\n", bin_size);
                        fprintf(stderr,"  bin_filter:                   %d\n", bin_filter);
                        fprintf(stderr,"  downsample:                   %d\n\n", downsample);
                    }

                    stereoRectify( cameraMatrix[0], distCoeffs[0], cameraMatrix[1], distCoeffs[1], imageSize[0], R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, imageSize[0], &roi1, &roi2 );
                    initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], R1, P1, imageSize[0], CV_16SC2, map11, map12);
                    initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], R2, P2, imageSize[1], CV_16SC2, map21, map22);

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
            }

            Scalar avgPixelIntensityLeft = mean(img1);
            Scalar avgPixelIntensityRight = mean(img2);
            mb_get_date(verbose, time_d, time_i);
            fprintf(stderr,"%5d Left:%s Right:%s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d LLZ: %.10f %.10f %8.3f Tide:%7.3f H:%6.2f R:%6.2f P:%6.2f Avg Intensities:%.3f %3f\n",
                    npairs, imageLeftFile, imageRightFile,
                    time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                    camera_navlon, camera_navlat, camera_sensordepth, tide, camera_heading, camera_roll, camera_pitch,
                    avgPixelIntensityLeft.val[0], avgPixelIntensityRight.val[0]);

            /* If specified apply stereo calibration to rectify the images */
            if (use_calibration == MB_YES) {
                remap(img1, img1r, map11, map12, INTER_LINEAR);
                remap(img2, img2r, map21, map22, INTER_LINEAR);

                /* Downsample if specified */
                if (downsample > 1) {
                    pyrDown( img1r, img1, Size(img1r.cols/downsample, img1r.rows/downsample));
                    pyrDown( img2r, img2, Size(img2r.cols/downsample, img2r.rows/downsample));
                } else {
                    img1 = img1r;
                    img2 = img2r;
                }
            }

            /* Else if not calibrated but downsample specified */
            else if (downsample > 1) {
                pyrDown( img1, img1r, Size(img1r.cols/downsample, img1r.rows/downsample));
                pyrDown( img2, img2r, Size(img2r.cols/downsample, img2r.rows/downsample));
                img1 = img1r;
                img2 = img2r;
            }

            /* Convert images to CV_8UC1 format */
            cvtColor(img1, img1g, COLOR_BGR2GRAY);
            cvtColor(img2, img2g, COLOR_BGR2GRAY);

            /* do the photogrammetery */
            if (algorithm == STEREO_BM) {
                bm->compute(img1g, img2g, disp);
//            Scalar avgPixelIntensityLeft = mean(img1);
//            Scalar avgPixelIntensityRight = mean(img2);
//            Scalar avgPixelIntensityDisparity = mean(disp);
//fprintf(stderr, "%s:%d:%s: avgPixelIntensity: %f %f %f\n", __FILE__, __LINE__, __func__, avgPixelIntensityLeft.val[0], avgPixelIntensityRight.val[0], avgPixelIntensityDisparity.val[0]);
                disp.convertTo(disp8, CV_8U, 255/(numberOfDisparities*16.));
//avgPixelIntensityDisparity = mean(disp8);
//fprintf(stderr, "%s:%d:%s: avgPixelIntensity: %f %f %f\n", __FILE__, __LINE__, __func__, avgPixelIntensityLeft.val[0], avgPixelIntensityRight.val[0], avgPixelIntensityDisparity.val[0]);

                /* display the disparity map */
                if (show_images) {
                    String windowNameDisparity = "Disparity";
                    namedWindow(windowNameDisparity, 0);
                    imshow(windowNameDisparity, disp);
                    waitKey(1000);
                    destroyWindow(windowNameDisparity);
                }

                disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
            }
            else if (algorithm == STEREO_SGBM || algorithm == STEREO_HH) {
                sgbm->compute(img1g, img2g, disp);
//            Scalar avgPixelIntensityLeft = mean(img1g);
//            Scalar avgPixelIntensityRight = mean(img2g);
//            Scalar avgPixelIntensityDisparity = mean(disp);
//fprintf(stderr, "%s:%d:%s: avgPixelIntensity: %f %f %f\n", __FILE__, __LINE__, __func__,
//avgPixelIntensityLeft.val[0], avgPixelIntensityRight.val[0], avgPixelIntensityDisparity.val[0]);
                disp.convertTo(disp8, CV_8U, 255/(numberOfDisparities*16.));
//avgPixelIntensityDisparity = mean(disp8);
//fprintf(stderr, "%s:%d:%s: avgPixelIntensity: %f %f %f\n", __FILE__, __LINE__, __func__,
//avgPixelIntensityLeft.val[0], avgPixelIntensityRight.val[0], avgPixelIntensityDisparity.val[0]);

                /* display the disparity map */
                if (show_images) {
                    String windowNameDisparity = "Disparity";
                    namedWindow(windowNameDisparity, 0);
                    imshow(windowNameDisparity, disp);
                    waitKey(1000);
                    destroyWindow(windowNameDisparity);
                }

                disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
//for (i = 0; i < dispf.rows; i++) {
//for (j = 0; j < dispf.cols; j++) {
//fprintf(stream, "DISPARITY: %d %d  %d %d %f\n",
//i, j, disp.at<short>(i,j),
//disp8.at<uchar>(i,j),
//dispf.at<float>(i,j));
//}
//}
            }
//            else if (algorithm == STEREO_VAR) {
//                var(img1g, img2g, disp);
//
//
//                /* display the disparity map */
//                if (show_images) {
//                    String windowNameDisparity = "Disparity";
//                    namedWindow(windowNameDisparity, 0);
//                    imshow(windowNameDisparity, disp);
//                    waitKey(1000);
//                    destroyWindow(windowNameDisparity);
//                }
//
//                /* convert to a float disparity */
//                int MaxD = MAX(labs(var.minDisp), labs(var.maxDisp));
//                int SignD = 1; if (MIN(var.minDisp, var.maxDisp) < 0) SignD = -1;
//                if (var.minDisp >= var.maxDisp)
//                    {
//                    MaxD = 256; SignD = 1;
//                    }
//                disp.convertTo(dispf, CV_32FC1, 256 / MaxD, 0);
//                for (i = 0; i < dispf.rows; i++)
//                    {
//                    for (j = 0; j < dispf.cols; j++)
//                        {
//                        fprintf(stream, "DISPARITY: %d %d  %d %d %f\n",
//                            i, j, disp.at<short>(i,j), disp8.at<uchar>(i,j), dispf.at<float>(i,j));
//                        }
//                    }
//                }
//fprintf(stderr,"disp depth:%d channels:%d\n",disp.depth(),disp.channels());

            /* check to see if a new output file is required */
            if (use_surveylinetimefile == MB_YES) {
                if ((time_d > routetime_d[waypoint] || waypoint == 0)
                    && waypoint < ntimepoint - 1) {
                    new_output_file = MB_YES;
                    sprintf(OutputFile, "%s_%3.3d.mb251", OutputFileRoot, waypoint);
                    waypoint++;
                }
            }
            else if (output_number_pairs > 0) {
                if (mbio_ptr == NULL || output_count >= output_number_pairs) {
                    new_output_file = MB_YES;
                    sprintf(OutputFile, "%s_%3.3d.mb251", OutputFileRoot, waypoint);
                    waypoint++;
                }
            }
            else if (mbio_ptr == NULL) {
                new_output_file = MB_YES;
                sprintf(OutputFile, "%s.mb251", OutputFileRoot);
            }

            /* open output format *.mb251 file */
            if (new_output_file == MB_YES) {
                /* if needed close the previous output file */
                if (mbio_ptr != NULL)
                    status = mb_close(verbose, &mbio_ptr, &error);

                /* open the new output file */
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
                new_output_file = MB_NO;
                output_count = 0;
            }

            /* set the timestamp, navigation and attitude for the stereo pair */
            store->kind = MB_DATA_DATA;
            store->time_d = time_d;
            store->longitude = camera_navlon;
            store->latitude = camera_navlat;
            store->sensordepth = camera_sensordepth;
            store->heading = camera_heading;
            store->roll = camera_roll;
            store->pitch = camera_pitch;
            store->speed = speed / 3.6;
            store->altitude = altitude;
            store->num_soundings = 0;

            /* loop over the disparity map, outputting valid values */
            ngood = 0;
            nbad = 0;
            istart = (int)(trim * dispf.rows);
            iend = dispf.rows - (int)(trim * dispf.rows);
            iend = MIN(iend, dispf.rows - bin_size);
            jstart = (int)(trim * dispf.cols);
            jend = dispf.cols - (int)(trim * dispf.cols);
            jend = MIN(jend, dispf.cols - bin_size);
            for (i = istart; i < iend; i+=bin_size) {
                for (j = jstart; j < jend; j+=bin_size) {
                    num_bin = 0;
                    for (ii=i;ii<i+bin_size;ii++)
                    for (jj=j;jj<j+bin_size;jj++) {
                        disparity = downsample * static_cast<double>(dispf.at<float>(ii,jj));
//fprintf(stderr, "%s:%d:%s: ii:%d jj:%d disparity: %f   %f %f",
//__FILE__, __LINE__, __func__, ii, jj, disparity, min_disparity, max_disparity);
//if (disparity > min_disparity && disparity < max_disparity) fprintf(stderr, " *******");
//fprintf(stderr, "\n");
                        /* only accept disparities within the desired altitude range */
                        if (disparity > min_disparity
                            && disparity < max_disparity) {
                            /* allocate more memory for binning if needed */
                            if (num_bin >= num_bin_alloc) {
                                /* allocate memory for data structure */
                                num_bin_alloc += MBPG_ALLOC_SMALL_NUM;
                                status = mb_reallocd(verbose, __FILE__, __LINE__,
                                            num_bin_alloc
                                                * sizeof(double),
                                            (void **)(&disparity_bin), &error);
                                if (error != MB_ERROR_NO_ERROR) {
                                    mb_error(verbose,error,&message);
                                    fprintf(stderr,"\nMBIO Error allocating array:\n%s\n",message);
                                    fprintf(stderr,"\nProgram <%s> Terminated\n",
                                        program_name);
                                    exit(error);
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
                            status = mb_reallocd(verbose, __FILE__, __LINE__,
                                        store->num_soundings_alloc
                                            * sizeof(struct mbsys_stereopair_sounding_struct),
                                        (void **)(&store->soundings), &error);
                            if (error != MB_ERROR_NO_ERROR) {
                                mb_error(verbose,error,&message);
                                fprintf(stderr,"\nMBIO Error allocating array:\n%s\n",message);
                                fprintf(stderr,"\nProgram <%s> Terminated\n",
                                    program_name);
                                exit(error);
                            }
                        }
                        sounding = &store->soundings[store->num_soundings];

                        /* if median filter then sort the bin and use the median */
                        if (bin_filter == MBPG_BIN_FILTER_MEDIAN) {
                            qsort((void *)disparity_bin, num_bin, sizeof(double), mb_double_compare);
                            disparity = disparity_bin[num_bin/2];
                        }

                        /* else use simple mean */
                        else {
                            disparity = 0.0;
                            for (k=0;k<num_bin;k++) {
                                disparity += disparity_bin[k];
                            }
                            disparity /= (double)num_bin;
                        }

                        /* use the position at the center of the bin */
                        ii = downsample * (i + bin_size/2);
                        jj = downsample * (j + bin_size/2);

                        /* calculate position relative to the camera rig */
                        pw = disparity * Q.at<double>(3,2) - Q.at<double>(3,3);
                        point[0] = (static_cast<double>(jj) + Q.at<double>(0,3)) / pw;;
                        point[1] = (-(static_cast<double>(ii) + Q.at<double>(1,3))) / pw;;
                        point[2] = (Q.at<double>(2,3)) / pw;;
//fprintf(stream, "%d %d disparity:%f pw:%f  point: %f %f %f\n", ii, jj, disparity, pw, point[0], point[1], point[2]);

                        /* get range and angles in
                            roll-pitch frame */
                        range = norm(point);
                        direction = normalize(point);
                        if (fabs(range) < 0.001) {
                            alphar = 0.0;
                            betar = 0.5 * M_PI;
                        }
                        else {
                            alphar = asin(MAX(-1.0, MIN(1.0, direction[1])));
                            betar = acos(MAX(-1.0, MIN(1.0, (direction[0] / cos(alphar)))));
                        }
                        if (direction[2] < 0.0)
                            betar = 2.0 * M_PI - betar;

                        /* apply roll pitch corrections */
                        betar += DTR * roll;
                        alphar += DTR * pitch;

                        /* calculate bathymetry */
                        sounding->depth = range * cos(alphar) * sin(betar) + pg_z_offset;
                        sounding->alongtrack = range * sin(alphar) + pg_xtrack_offset;
                        sounding->acrosstrack = range * cos(alphar) * cos(betar) + pg_ltrack_offset;
                        sounding->beamflag = MB_FLAG_NONE;
                        sounding->red = 0;
                        sounding->green = 0;
                        sounding->blue = 0;

                        /* apply navigation and heading */
                        //bathlon = navlon
                        //    + headingy * mtodeglon * bathacrosstrack
                        //    + headingx * mtodeglon * bathalongtrack;
                        //bathlat = navlat
                        //    - headingx * mtodeglat * bathacrosstrack
                        //    + headingy * mtodeglat * bathalongtrack;

                        //fprintf(stream,"%d   %.4f %.4f %.4f   %.4f %.4f %.4f   %.10f %.10f %.4f\n",
                        //    ngood, point[0], point[1], point[2],
                        //    sounding->acrosstrack[store->num_soundings],
                        //    sounding->alongtrack[store->num_soundings],
                        //    sounding->bath[store->num_soundings],
                        //    bathlon, bathlat, bath);

                        //fprintf(stream, "%.10f %.10f %.4f\n", bathlon, bathlat, bath);

                        store->num_soundings++;
                        ngood++;
                    }
                    else {
                        nbad++;
                    }
                }
            }

            // output image list entry and bathymetry
            double good_fraction = ((double)ngood / ((double)(ngood + nbad)));
            fprintf(stream, "      --> Disparity calculations: good:%d  bad:%d  fraction:%.3f\n",
                    ngood, nbad, good_fraction);
            fprintf(oilfp, "%s %s %.6f %.6f  %.2f\n", imageLeftFile, imageRightFile, left_time_d, time_diff, good_fraction);

            mb_write_ping(verbose, mbio_ptr, (void *)store, &error);
            output_count++;
            if (status != MB_SUCCESS) {
                mb_error(verbose,error,&message);
                fprintf(stderr,"\nMBIO Error returned from function <mb_put>:\n%s\n",message);
                fprintf(stderr,"\nMapping Data Not Written To File <%s>\n",OutputFile);
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                exit(error);
            }

            // update counts
            npairs++;
            nimages += 2;
        }

        // else not used add to imagelist with zero good fraction
        else {
            fprintf(oilfp, "%s %s %.6f %.6f  %.2f\n", imageLeftFile, imageRightFile, left_time_d, time_diff, 0.0);
        }
    }

    /* close imagelist files */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);
    if (oilfp != NULL) {
      fclose(oilfp);
      oilfp = NULL;
    }

    /* close the output file */
    if (mbio_ptr != NULL)
        status = mb_close(verbose, &mbio_ptr, &error);
    fprintf(stderr, "\nOutput count: %d\n", output_count);

    /* deallocate survey line arrays if necessary */
    if (use_surveylinetimefile == MB_YES) {
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&routetime_d,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&routelon,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&routelat,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&routeheading,&error);
        status = mb_freed(verbose,__FILE__,__LINE__,(void **)&routewaypoint,&error);
    }

    /* deallocate navigation arrays if necessary */
    if (use_navigation == MB_YES) {
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
