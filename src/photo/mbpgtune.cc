/*--------------------------------------------------------------------
 *    The MB-system:    mbpgtune.cpp    11/27/2025
 *
 *    Copyright (c) 2025-2025 by
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
 * mbpgtune provides an interactive interface to load seafloor 
 * photography stereo pairs from an imagelist structure and tune 
 * photogrammetry parameters that can then be used 
 * for mbphotogrammetry.
 *
 * The idea for this tool and some code have derived from:
 *   https://learnopencv.com/depth-perception-using-stereo-camera-python-c/
 * in article "Stereo Camera Depth Estimation With OpenCV (Python/C++)"
 * by Kaustubh Sadekar dated  April 5, 2021
 *
 * Author:    D. W. Caress
 * Date:    November 27, 2025
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
#include <opencv2/opencv.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

std::string s;
std::stringstream out;

#define MBPG_ALLOC_SMALL_NUM    64
#define MBPG_ALLOC_LARGE_NUM    262144

#define MBPG_BIN_FILTER_MEAN    0
#define MBPG_BIN_FILTER_MEDIAN    1

// #define DEBUG 1
char program_name[] = "mbpgtune";
char help_message[] = "mbpgtune provides an interactive interface to load seafloor "
											"photography stereo pairs from an imagelist structure and tune "
											"photogrammetry parameters that can then be used "
											"for mbphotogrammetry.";
char usage_message[] = "mbpgtune \n"
                        "\t--verbose\n"
                        "\t--help\n"
                        "\t--input=imagelist\n"
                        "\t--fov-fudgefactor=factor\n"
                        "\t--camera-calibration-file=file\n"
                        "\t--calibration-file=file\n"
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
    bool rectified;
    mb_path imageLeftFile;
    double image_left_time_d;
    double image_left_gain;
    double image_left_exposure;
    double image_left_amplitude;
    mb_path imageRightFile;
    double image_right_time_d;
    double image_right_gain;
    double image_right_exposure;
    double image_right_amplitude;
};

enum { STEREO_BM=0, STEREO_SGBM=1, STEREO_HH=2 };
struct mbpg_control_struct {
		struct mbpg_process_struct process;
		
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
    double altitude_min;
    double altitude_max;
    Mat     map11, map12, map21, map22;

    // Input image trimming
    int trimPixels;

    // Photogrammetry algorithm controls
    Ptr<StereoBM> bm;
    Ptr<StereoSGBM> sgbm;
    
    int PG_algorithm;				// Can be STEREO_BM or STEREO_SGBM
		int	PG_mode;						// BM: Not relevant
														// SGBM: Set it to StereoSGBM::MODE_HH to run the full-scale 
														//   two-pass dynamic programming algorithm. It will consume 
														//   O(W*H*numDisparities) bytes, which is large for 640x480 
														//   stereo and huge for HD-size pictures. 
														//   Default for StereoSGBM = StereoSGBM::MODE_SGBM = 0
    
    // StereoSGMB and StereoBM parameters
		int	PG_minDisparity;		// SGBM: Minimum possible disparity value. Normally, it is zero 
														//   but sometimes rectification algorithms can shift images, so 
														//   this parameter needs to be adjusted accordingly.
														//   Default for StereoSGBM = 0
		int	PG_numDisparities; 	// BM: The disparity search range. For each pixel, the algorithm 
														//   will find the best disparity from 0 (default minimum disparity) 
														//   to numDisparities. The search range can be shifted by changing 
														//   the minimum disparity.  This parameter must be divisible 
														//   by 16.
														// SGBM: Maximum disparity minus minimum disparity. The value is 
														//   always greater than zero. This parameter must be divisible 
														//   by 16.
		int	PG_blockSize;				// Also called SADWindowSize
														// BM: The linear size of the blocks compared by the algorithm. 
														//   The size should be odd (as the block is centered at the 
														//   current pixel). Larger block size implies smoother, though 
														//   less accurate disparity map. Smaller block size gives more 
														//   detailed disparity map, but there is a higher chance for 
														//   the algorithm to find a wrong correspondence.
														//   Default for StereoBM = 21
														// SGBM: Matched block size. It must be an odd number >=1 . 
														//   Normally, it should be somewhere in the 3..11 range.
														//   Default for StereoSGBM = 7
		int	PG_P1;							// SGBM: The first parameter controlling the disparity smoothness. 
														//   Default for StereoSGBM = 0
		int	PG_P2;							// SGBM: The second parameter controlling the disparity smoothness. 
														//   The larger the values are, the smoother the disparity is. 
														//   P1 is the penalty on the disparity change by plus or minus 
														//   1 between neighbor pixels. P2 is the penalty on the 
														//   disparity change by more than 1 between neighbor pixels. 
														//   The algorithm requires P2 > P1 . See stereo_match.cpp 
														//   sample where some reasonably good P1 and P2 values are 
														//   shown (like 8*number_of_image_channels*blockSize*blockSize 
														//   and 32*number_of_image_channels*blockSize*blockSize , 
														//   respectively).
														//   Default for StereoSGBM = 0
		int	PG_disp12MaxDiff;		// SGBM: 	Maximum allowed difference (in integer pixel units) 
														//   in the left-right disparity check. Set it to a non-positive 
														//   value to disable the check.
														//   Default for StereoSGBM = 0
		int PG_preFilterCap;		// BM: This value caps the output of the pre-filter to 
														// 	[-preFilterCap, preFilterCap], helping to reduce the impact 
														//   of noise and outliers in the pre-filtered image. Typically 
														//   in the range [1, 63].
														// SGBM: Truncation value for the prefiltered image pixels. 
														//   The algorithm first computes x-derivative at each pixel and 
														//   clips its value by [-preFilterCap, preFilterCap] interval. 
														//   The result values are passed to the Birchfield-Tomasi pixel 
														//   cost function.
														//   Default for StereoSGBM = 0
		int PG_preFilterSize;		// BM: The pre-filter size determines the spatial extent of the 
														//   pre-filtering operation, which prepares the images for 
														//   disparity computation by normalizing brightness and enhancing 
														//   texture. Larger sizes reduce noise but may blur details, 
														//   while smaller sizes preserve details but are more susceptible 
														//   to noise. Must be an odd integer, typically between 5 and 255.
														//   Default for StereoBM = 9
		int PG_preFilterType;		// BM: The pre-filter type affects how the images are prepared 
														//   before computing the disparity map. Different pre-filtering 
														//   methods can enhance specific image features or reduce noise, 
														//   influencing the quality of the disparity map. Possible values are:
														//     - PREFILTER_NORMALIZED_RESPONSE (0): Uses normalized response for pre-filtering.
														//     - PREFILTER_XSOBEL (1): Uses the X-Sobel operator for pre-filtering.
														//   Default for StereoBM = PREFILTER_XSOBEL (1)
		Rect PG_roi1;						// BM: By setting the ROI, the stereo matching computation is 
														//   limited to the specified region, improving performance and 
														//   potentially accuracy by focusing on relevant parts of the 
														//   image. Default for StereoBM = empty rectangle, meaning 
														//   process the entire image.
		Rect PG_roi2;						// BM: Similar to setROI1, this limits the computation to the 
														//   specified region in the right image. Default for 
														//   StereoBM = empty rectangle, meaning process the entire image.
		int PG_textureThreshold;// BM: This parameter filters out regions with low texture, 
														//   where establishing correspondences is difficult, thus 
														//   reducing noise in the disparity map. Higher values filter 
														//   more aggressively but may discard valid information. Must 
														//   be non-negative.
														//   Default for StereoBM = 10
		int PG_uniquenessRatio;	// BM: This parameter ensures that the best match is sufficiently 
														//   better than the next best match, reducing false positives. 
														//   Higher values are stricter but may filter out valid matches 
														//   in difficult regions.  Typically in the range [5, 15], but 
														//   can be from 0 to 100.
														//   Default for StereoBM = 0
														// SGBM: Margin in percentage by which the best (minimum) 
														//   computed cost function value should "win" the second best 
														//   value to consider the found match correct. Normally, a 
														//   value within the 5-15 range is good enough.
														//   Default for StereoSGBM = 0
		int	PG_speckleWindowSize;// SGBM: Maximum size of smooth disparity regions to consider 
														//   their noise speckles and invalidate. Set it to 0 to disable 
														//   speckle filtering. Otherwise, set it somewhere in the 
														//   50-200 range.
														//   Default for StereoSGBM = 0
		int	PG_speckleRange;		// SGBM: Maximum disparity variation within each connected 
														//   component. If you do speckle filtering, set the parameter 
														//   to a positive value, it will be implicitly multiplied by 16.
														//   Normally, 1 or 2 is good enough.
														//   Default for StereoSGBM = 0
};

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
void process_stereopair(int verbose, struct mbpg_control_struct *control)
{

    /* output stream (stdout if verbose <= 1, stderr if verbose > 1) */
    FILE *stream = NULL;
    if (verbose <= 1)
        stream = stdout;
    else
        stream = stderr;

    bool use_this_pair = true;
    
    /* processing structure */
    struct mbpg_process_struct *process = &control->process;

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

        Scalar avgPixelIntensityLeft = mean(img1);
        process->image_left_amplitude = avgPixelIntensityLeft.val[0];
        Scalar avgPixelIntensityRight = mean(img2);
        process->image_right_amplitude = avgPixelIntensityRight.val[0];

        /* initialize cameras, calibration, and stereo algorithm */
        if (!control->photogrammetryInitialized) {
            if (!control->calibrationInitialized && !process->rectified) {
                fprintf(stderr,"\nNo stereo camera calibration has been loaded - aborting...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n", program_name);
                exit(1);
            }
            control->photogrammetryInitialized = true;

            /* set the desired disparity range */
            double min_disparity = (control->Q.at<double>(3,3) + control->Q.at<double>(2,3) / control->altitude_max) / control->Q.at<double>(3,2);
            double max_disparity = (control->Q.at<double>(3,3) + control->Q.at<double>(2,3) / control->altitude_min) / control->Q.at<double>(3,2);
            fprintf(stderr, "%s:%d:%s: Q.at<double>(2,3):%f Q.at<double>(3,2):%f min max disparity: %f %f\n",
                __FILE__, __LINE__, __func__, control->Q.at<double>(2,3), control->Q.at<double>(3,2), min_disparity, max_disparity);

            /* Set parameters for both algorithms 
            		control->PG_algorithm: STEREO_BM, STEREO_SGBM, STEREO_HH */
            
            /* BM algorithm */
						control->bm = StereoBM::create();
						control->bm->setROI1(control->PG_roi1);
						control->bm->setROI2(control->PG_roi2);
						control->bm->setPreFilterCap(control->PG_preFilterCap);
						control->bm->setPreFilterSize(control->PG_preFilterSize);
						control->bm->setPreFilterType(control->PG_preFilterType);
						control->bm->setBlockSize(control->PG_blockSize);
						control->bm->setMinDisparity(control->PG_minDisparity);
						control->bm->setNumDisparities(control->PG_numDisparities);
						control->bm->setTextureThreshold(control->PG_textureThreshold);
						control->bm->setUniquenessRatio(control->PG_uniquenessRatio);
						control->bm->setSpeckleWindowSize(control->PG_speckleWindowSize);
						control->bm->setSpeckleRange(control->PG_speckleRange);
						control->bm->setDisp12MaxDiff(control->PG_disp12MaxDiff);

						control->sgbm = StereoSGBM::create();
						if (control->PG_algorithm == STEREO_SGBM) {
								control->sgbm->setMode( StereoSGBM::MODE_SGBM );
						}
						else { //if (algorithm == STEREO_HH) {
								control->sgbm->setMode( StereoSGBM::MODE_HH );
						}

						control->sgbm->setPreFilterCap(control->PG_preFilterCap);
						control->sgbm->setBlockSize(control->PG_blockSize);
						control->sgbm->setP1(control->PG_P1);
						control->sgbm->setP2(control->PG_P2);
						control->sgbm->setMinDisparity(control->PG_minDisparity);
						control->sgbm->setNumDisparities(control->PG_numDisparities);
						control->sgbm->setUniquenessRatio(control->PG_uniquenessRatio);
						control->sgbm->setSpeckleWindowSize(control->PG_speckleWindowSize);
						control->sgbm->setSpeckleRange(control->PG_speckleRange);
						control->sgbm->setDisp12MaxDiff(control->PG_disp12MaxDiff);

            fprintf(stderr, "%s:%d:%s: algorithms  set\n", __FILE__, __LINE__, __func__);

            /* set up rectification */
            if (!process->rectified) {
								calibrationMatrixValues(control->cameraMatrix[0], control->imageSize[0],
														control->SensorWidthMm, control->SensorHeightMm,
														control->fovx[0], control->fovy[0], control->focalLength[0],
														control->principalPoint[0], control->aspectRatio[0]);
								calibrationMatrixValues(control->cameraMatrix[1], control->imageSize[1],
														control->SensorWidthMm, control->SensorHeightMm,
														control->fovx[1], control->fovy[1], control->focalLength[1],
														control->principalPoint[1], control->aspectRatio[1]);
            }
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
                fprintf(stderr,"\nPhotogrammetry Algorithm Controls:\n");
                fprintf(stderr,"  PG_algorithm:                 %d\n", control->PG_algorithm);
                fprintf(stderr,"  PG_minDisparity:              %d\n", control->PG_minDisparity);
                fprintf(stderr,"  PG_numDisparities:            %d\n", control->PG_numDisparities);
                fprintf(stderr,"  PG_blockSize:                 %d\n", control->PG_blockSize);
                fprintf(stderr,"  PG_P1:                        %d\n", control->PG_P1);
                fprintf(stderr,"  PG_P2:                        %d\n", control->PG_P2);
                fprintf(stderr,"  PG_disp12MaxDiff:             %d\n", control->PG_disp12MaxDiff);
                fprintf(stderr,"  PG_preFilterCap:              %d\n", control->PG_preFilterCap);
                fprintf(stderr,"  PG_preFilterSize:             %d\n", control->PG_preFilterSize);
                fprintf(stderr,"  PG_preFilterType:             %d\n", control->PG_preFilterType);
                fprintf(stderr,"  PG_roi1:                      %d %d %d %d\n", control->PG_roi1.x, control->PG_roi1.y, control->PG_roi1.width, control->PG_roi1.height);
                fprintf(stderr,"  PG_roi2:                      %d %d %d %d\n", control->PG_roi2.x, control->PG_roi2.y, control->PG_roi2.width, control->PG_roi2.height);
                fprintf(stderr,"  PG_textureThreshold:          %d\n", control->PG_textureThreshold);
                fprintf(stderr,"  PG_uniquenessRatio:           %d\n", control->PG_uniquenessRatio);
                fprintf(stderr,"  PG_speckleWindowSize:         %d\n", control->PG_speckleWindowSize);
                fprintf(stderr,"  PG_speckleRange:              %d\n", control->PG_speckleRange);
                fprintf(stderr,"  PG_mode:                      %d\n", control->PG_mode);
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

        /* Print information for stereo pair to be processed */
        int time_i[7];
        mb_get_date(verbose, camera_time_d, time_i);
        fprintf(stderr, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %s %s\n", 
                 time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
       						process->imageLeftFile, process->imageRightFile);

        /* apply stereo calibration to rectify the images */
        Mat img1r, img2r, img1g, img2g, img1gc, img2gc;
        Mat disp, dispf, disp8, dispc;
        if (process->rectified) {
        		img1r = img1.clone();
        		img2r = img2.clone();
        }
        else {
        		remap(img1, img1r, control->map11, control->map12, INTER_LINEAR);
        		remap(img2, img2r, control->map21, control->map22, INTER_LINEAR);
        }

        /* Convert images to CV_8UC1 format */
        cvtColor(img1r, img1g, COLOR_BGR2GRAY);
        cvtColor(img2r, img2g, COLOR_BGR2GRAY);
       
        /* apply CLAHE filter */
				cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
				clahe->setClipLimit(10.0); // Default is 40.0
				clahe->setTilesGridSize(cv::Size(8, 8)); // Default is 8x8
				clahe->apply(img1g, img1gc);
				clahe->apply(img2g, img2gc);
				
				//imwrite("TestImage-R-Raw.jpg", img1);
				//imwrite("TestImage-R-Gray.jpg", img1g);
				//imwrite("TestImage-R-CLAHE.jpg", img1gc);
				//imwrite("TestImage-L-Raw.jpg", img2);
				//imwrite("TestImage-L-Gray.jpg", img2g);
				//imwrite("TestImage-L-CLAHE.jpg", img2gc);
    
        /* do the photogrammetery */
        if (control->PG_algorithm == STEREO_BM) {
            control->bm->compute(img1gc, img2gc, disp);
            disp.convertTo(disp8, CV_8U, 255/(control->PG_numDisparities*16.));
            disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
        }
        else if (control->PG_algorithm == STEREO_SGBM || control->PG_algorithm == STEREO_HH) {
            control->sgbm->compute(img1gc, img2gc, disp);
            disp.convertTo(disp8, CV_8U, 255/(control->PG_numDisparities*16.));
            disp.convertTo(dispf, CV_32FC1, 1/16.0, 0);
        }
 
				// Displaying the disparity map
				cv::applyColorMap(disp8, dispc, cv::COLORMAP_JET);
				cv::imshow("Disparity Gray",disp8);
				cv::imshow("Disparity Color",dispc);
    }

}

/*--------------------------------------------------------------------*/

// Defining callback functions for the trackbars to update parameter values
 
void buttonCallback_Algorithm_BM(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;

  fprintf(stderr, "%s:%d:%s: buttonCallback_Algorithm_BM\n", __FILE__, __LINE__, __FUNCTION__);
  
	control->PG_algorithm = STEREO_BM;

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void buttonCallback_Algorithm_SGBM(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  fprintf(stderr, "%s:%d:%s: buttonCallback_Algorithm_SGBM\n", __FILE__, __LINE__, __FUNCTION__);
  
	control->PG_algorithm = STEREO_SGBM;
	if (control->sgbm != NULL) 
		{
		control->sgbm->setMode( StereoSGBM::MODE_SGBM );
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void buttonCallback_Algorithm_HH(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  fprintf(stderr, "%s:%d:%s: buttonCallback_Algorithm_SGBM\n", __FILE__, __LINE__, __FUNCTION__);
  
	control->PG_algorithm = STEREO_HH;
	if (control->sgbm != NULL) 
		{
		control->sgbm->setMode( StereoSGBM::MODE_HH );
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_minDisparity(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_minDisparity = value;
  fprintf(stderr, "%s:%d:%s: PG_minDisparity set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_minDisparity);
	if (control->bm != NULL) 
		{
		control->bm->setMinDisparity(control->PG_minDisparity);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setMinDisparity(control->PG_minDisparity);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_numDisparities(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_numDisparities = value;
  fprintf(stderr, "%s:%d:%s: PG_numDisparities set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_numDisparities);
	if (control->bm != NULL) 
		{
		control->bm->setNumDisparities(control->PG_numDisparities);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setNumDisparities(control->PG_numDisparities);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_blockSize(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_blockSize = value;
  fprintf(stderr, "%s:%d:%s: PG_blockSize set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_blockSize);
	if (control->bm != NULL) 
		{
		control->bm->setBlockSize(control->PG_blockSize);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setBlockSize(control->PG_blockSize);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_P1(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_P1 = value;
  fprintf(stderr, "%s:%d:%s: PG_P1 set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_P1);
	if (control->sgbm != NULL) 
		{
		control->sgbm->setP1(control->PG_P1);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_P2(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_P2 = value;
  fprintf(stderr, "%s:%d:%s: PG_P2 set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_P2);
	if (control->sgbm != NULL) 
		{
		control->sgbm->setP2(control->PG_P2);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_disp12MaxDiff(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_disp12MaxDiff = value;
  fprintf(stderr, "%s:%d:%s: PG_disp12MaxDiff set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_disp12MaxDiff);
	if (control->bm != NULL) 
		{
		control->bm->setDisp12MaxDiff(control->PG_disp12MaxDiff);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setDisp12MaxDiff(control->PG_disp12MaxDiff);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_preFilterCap(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_preFilterCap = value;
  fprintf(stderr, "%s:%d:%s: PG_preFilterCap set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_preFilterCap);
	if (control->bm != NULL) 
		{
		control->bm->setPreFilterCap(control->PG_preFilterCap);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setPreFilterCap(control->PG_preFilterCap);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_preFilterSize(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_preFilterSize = value;
  fprintf(stderr, "%s:%d:%s: PG_preFilterSize set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_preFilterSize);
	if (control->bm != NULL) 
		{
		control->bm->setPreFilterSize(control->PG_preFilterSize);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_preFilterType(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_preFilterType = value;
  fprintf(stderr, "%s:%d:%s: PG_preFilterType set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_preFilterType);
	if (control->bm != NULL) 
		{
		control->bm->setPreFilterType(control->PG_preFilterType);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_textureThreshold(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_textureThreshold = value;
  fprintf(stderr, "%s:%d:%s: PG_textureThreshold set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_textureThreshold);
	if (control->bm != NULL) 
		{
		control->bm->setTextureThreshold(control->PG_textureThreshold);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_uniquenessRatio(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_uniquenessRatio = value;
  fprintf(stderr, "%s:%d:%s: PG_uniquenessRatio set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_uniquenessRatio);
	if (control->bm != NULL) 
		{
		control->bm->setUniquenessRatio(control->PG_uniquenessRatio);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setUniquenessRatio(control->PG_uniquenessRatio);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_speckleWindowSize(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_speckleWindowSize = value;
  fprintf(stderr, "%s:%d:%s: PG_speckleWindowSize set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_speckleWindowSize);
	if (control->bm != NULL) 
		{
		control->bm->setSpeckleWindowSize(control->PG_speckleWindowSize);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setSpeckleWindowSize(control->PG_speckleWindowSize);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
  	}
}
 
void trackbarCallback_PG_speckleRange(int value, void * cptr)
{
	struct mbpg_control_struct *control = (struct mbpg_control_struct *) cptr;
  control->PG_speckleRange = value;
  fprintf(stderr, "%s:%d:%s: PG_speckleRange set to %d\n", __FILE__, __LINE__, __FUNCTION__, control->PG_speckleRange);
	if (control->bm != NULL) 
		{
		control->bm->setSpeckleRange(control->PG_speckleRange);
		}
	if (control->sgbm != NULL) 
		{
		control->sgbm->setSpeckleRange(control->PG_speckleRange);
		}

  /* reprocess the current stereo pair with the changed control settings */
	if (control->bm != NULL || control->sgbm != NULL) 
		{
  	process_stereopair(0, control);
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
    mb_path ImageListFile{};

    /* default photogrammetry algorithm parameters */
    struct mbpg_control_struct control;
    memset((void *)&control, 0, sizeof(struct mbpg_control_struct));
    
    /* processing structure */
    struct mbpg_process_struct *process = &control.process;
    
    control.trimPixels = 0;
    control.PG_algorithm = STEREO_SGBM;
		control.PG_minDisparity = 0;
		control.PG_numDisparities = 192;
		control.PG_blockSize = 5;
		control.PG_P1 = 8 * control.PG_blockSize * control.PG_blockSize;
		control.PG_P2 = 32 * control.PG_blockSize * control.PG_blockSize;
		control.PG_disp12MaxDiff = 1;
		control.PG_preFilterCap = 63;
		control.PG_preFilterSize = 9;
		control.PG_preFilterType = cv::StereoBM::PREFILTER_XSOBEL;
		control.PG_roi1 = cv::Rect(0, 0, 0, 0);
		control.PG_roi2 = cv::Rect(0, 0, 0, 0);
		control.PG_textureThreshold = 10;
		control.PG_uniquenessRatio = 10;
		control.PG_speckleWindowSize = 100;
		control.PG_speckleRange = 32;

    /* Input camera parameters */
    bool calibration_specified = false;
    mb_path StereoCameraCalibrationFile{};
    control.calibrationInitialized = false;
    control.SensorWidthMm = 8.789;
    control.SensorHeightMm = 6.610;
    control.SensorCellMm = 0.00454;
    control.fov_fudgefactor = 1.0;
    control.altitude_min = 1.0;
    control.altitude_max = 5.0;

    /* MBIO status variables */
    int status = MB_SUCCESS;
    int verbose = 0;
    int error = MB_ERROR_NO_ERROR;
    char *message;

    /* command line option definitions */
		/* mbpgtune 
     *         --verbose
     *         --help
     *         --input=imagelist
     *         --fov-fudgefactor=factor
     *         --camera-calibration-file=file
     *         --calibration-file=file
     *         --trim=value
     *         --bin-size=value
     *         --bin-filter=value (0=mean, 1=median)
     *         --downsample=value
     *         --algorithm=algorithm (bm, sgbm, hh)
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
        {"verbose",               no_argument,          NULL, 0},
        {"help",                  no_argument,          NULL, 0},
        {"input",                 required_argument,    NULL, 0},
        {"calibration-file",      required_argument,    NULL, 0},
        {"algorithm",             required_argument,    NULL, 0},
        {"algorithm-bm",          no_argument,    			NULL, 0},
        {"algorithm-sgbm",        no_argument,    			NULL, 0},
        {"minDisparity",          required_argument,    NULL, 0},
        {"numDisparities",        required_argument,    NULL, 0},
        {"blockSize",             required_argument,    NULL, 0},
        {"P1",                    required_argument,    NULL, 0},
        {"P2",                    required_argument,    NULL, 0},
        {"disp12MaxDiff",         required_argument,    NULL, 0},
        {"preFilterCap",          required_argument,    NULL, 0},
        {"preFilterSize",         required_argument,    NULL, 0},
        {"preFilterType",         required_argument,    NULL, 0},
        {"roi1",                  required_argument,    NULL, 0},
        {"roi2",                  required_argument,    NULL, 0},
        {"textureThreshold",      required_argument,    NULL, 0},
        {"uniquenessRatio",       required_argument,    NULL, 0},
        {"speckleWindowSize",     required_argument,    NULL, 0},
        {"speckleRange",          required_argument,    NULL, 0},
        {"mode",                  required_argument,    NULL, 0},
        { NULL,                   0,                    NULL, 0}
        };

    /* set default imagelistfile name */
    snprintf(ImageListFile, sizeof(ImageListFile), "imagelist.mb-1");

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
             * Define input image list file */

            /* input */
            else if (strcmp("input", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageListFile);
                if (n == 1)
                    imagelist_specified = true;
                }

            /*-------------------------------------------------------
             * Define stereo camera calibration file */

            /* camera-calibration-file  or calibration-file */
            else if (strcmp("calibration-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", StereoCameraCalibrationFile);
                if (n == 1)
                    calibration_specified = true;
                }

            /*-------------------------------------------------------
             * Define algorithm and parameters */

            /* algorithm */
            else if (strcmp("algorithm", options[option_index].name) == 0)
                {
                if (strcmp(optarg, "bm") == 0 || strcmp(optarg, "BM") == 0)
                    {
                    control.PG_algorithm = STEREO_BM;
                    }
                else if (strcmp(optarg, "sgbm") == 0 || strcmp(optarg, "SGBM") == 0)
                		{
                    control.PG_algorithm = STEREO_SGBM;
                    }
                else if (strcmp(optarg, "hh") == 0 || strcmp(optarg, "HH") == 0)
                		{
                    control.PG_algorithm = STEREO_HH;
                    }
               }

            /* algorithm-bm */
            else if (strcmp("algorithm-bm", options[option_index].name) == 0)
                {
                control.PG_algorithm = STEREO_BM;
                }

            /* algorithm-sgbm */
            else if (strcmp("algorithm-sgbm", options[option_index].name) == 0)
                {
                control.PG_algorithm = STEREO_SGBM;
                }

            /* minDisparity */
            else if (strcmp("minDisparity", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_minDisparity);
                }

            /* numDisparities */
            else if (strcmp("numDisparities", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_numDisparities);
                }

            /* blockSize */
            else if (strcmp("blockSize", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_blockSize);
                }

            /* P1 */
            else if (strcmp("P1", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_P1);
                }

            /* P2 */
            else if (strcmp("P2", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_P2);
                }

            /* disp12MaxDiff */
            else if (strcmp("disp12MaxDiff", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_disp12MaxDiff);
                }

            /* preFilterCap */
            else if (strcmp("preFilterCap", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_preFilterCap);
                }

            /* preFilterSize */
            else if (strcmp("preFilterSize", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_preFilterSize);
                }

            /* preFilterType */
            else if (strcmp("preFilterType", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_preFilterType);
                }

            /* roi1 */
            else if (strcmp("roi1", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d/%d/%d/%d", &control.PG_roi1.x, &control.PG_roi1.y, &control.PG_roi1.width, &control.PG_roi1.height);
                }

            /* roi2 */
            else if (strcmp("roi2", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d/%d/%d/%d", &control.PG_roi2.x, &control.PG_roi2.y, &control.PG_roi2.width, &control.PG_roi2.height);
                }

            /* textureThreshold */
            else if (strcmp("textureThreshold", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_textureThreshold);
                }

            /* uniquenessRatio */
            else if (strcmp("uniquenessRatio", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_uniquenessRatio);
                }

            /* speckleWindowSize */
            else if (strcmp("speckleWindowSize", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_speckleWindowSize);
                }

            /* speckleRange */
            else if (strcmp("speckleRange", options[option_index].name) == 0)
                {
                const int n = sscanf(optarg, "%d", &control.PG_speckleRange);
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
        fprintf(stream,"%s     imagelist_specified:              %d\n", first, imagelist_specified);
        fprintf(stream,"%s     ImageListFile:                    %s\n", first, ImageListFile);
        fprintf(stream,"%s     calibration_specified:            %d\n", first, calibration_specified);
        fprintf(stream,"%s     StereoCameraCalibrationFile:      %s\n", first, StereoCameraCalibrationFile);
			  fprintf(stderr,"%s     control.PG_algorithm:             %d\n", first, control.PG_algorithm);
				fprintf(stderr,"%s     control.PGminDisparity:           %d\n", first, control.PG_minDisparity);
				fprintf(stderr,"%s     control.PGnumDisparities:         %d\n", first, control.PG_numDisparities);
				fprintf(stderr,"%s     control.PGblockSize:              %d\n", first, control.PG_blockSize);
				fprintf(stderr,"%s     control.PGP1:                     %d\n", first, control.PG_P1);
				fprintf(stderr,"%s     control.PGP2:                     %d\n", first, control.PG_P2);
				fprintf(stderr,"%s     control.PGdisp12MaxDiff:          %d\n", first, control.PG_disp12MaxDiff);
				fprintf(stderr,"%s     control.PGpreFilterCap:           %d\n", first, control.PG_preFilterCap);
				fprintf(stderr,"%s     control.PGpreFilterSize:          %d\n", first, control.PG_preFilterSize);
				fprintf(stderr,"%s     control.PGpreFilterType:          %d\n", first, control.PG_preFilterType);
				fprintf(stderr,"%s     control.PGroi1:                   %d %d %d %d\n", 
												first, control.PG_roi1.x, control.PG_roi1.y, 
												control.PG_roi1.width, control.PG_roi1.height);
				fprintf(stderr,"%s     control.PGroi2:                   %d %d %d %d\n", 
												first, control.PG_roi2.x, control.PG_roi2.y, 
												control.PG_roi2.width, control.PG_roi2.height);
				fprintf(stderr,"%s     control.PGtextureThreshold:       %d\n", first, control.PG_textureThreshold);
				fprintf(stderr,"%s     control.PGuniquenessRatio:        %d\n", first, control.PG_uniquenessRatio);
				fprintf(stderr,"%s     control.PGspeckleWindowSize:      %d\n", first, control.PG_speckleWindowSize);
				fprintf(stderr,"%s     control.PGspeckleRange:           %d\n", first, control.PG_speckleRange);
    }

    /* if help desired then print it and exit */
    if (help)
        {
        fprintf(stream,"\n%s\n",help_message);
        fprintf(stream,"\nusage: %s\n", usage_message);
        exit(error);
        }

    /* loop over the list of input stereo image pairs
       - the images must be associated with a stereo camera model */
 
		// Creating a named window to be linked to the trackbars
		cv::namedWindow("Disparity Gray",cv::WINDOW_NORMAL);
		cv::namedWindow("Disparity Color",cv::WINDOW_NORMAL);
    cv::createButton("BM", buttonCallback_Algorithm_BM, (void *)&control, cv::QT_RADIOBOX, 0);
    cv::createButton("SGBM", buttonCallback_Algorithm_SGBM, (void *)&control, cv::QT_RADIOBOX, 1);
    
		cv::createTrackbar("PG_minDisparity", "", nullptr, 16, &trackbarCallback_PG_minDisparity, (void *)&control);
		cv::setTrackbarPos("PG_minDisparity", "", control.PG_minDisparity);
		
		cv::createTrackbar("PG_numDisparities", "", nullptr, 256, &trackbarCallback_PG_numDisparities, (void *)&control);
		cv::setTrackbarPos("PG_numDisparities", "", control.PG_numDisparities);
		
		cv::createTrackbar("PG_blockSize", "", nullptr, 21, &trackbarCallback_PG_blockSize, (void *)&control);
		cv::setTrackbarPos("PG_blockSize", "", control.PG_blockSize);
		
		cv::createTrackbar("PG_P1", "", nullptr, 500, &trackbarCallback_PG_P1, (void *)&control);
		cv::setTrackbarPos("PG_P1", "", control.PG_P1);
		
		cv::createTrackbar("PG_P2", "", nullptr, 2000, &trackbarCallback_PG_P2, (void *)&control);
		cv::setTrackbarPos("PG_P2", "", control.PG_P2);
		
		cv::createTrackbar("PG_disp12MaxDiff", "", nullptr, 10, &trackbarCallback_PG_disp12MaxDiff, (void *)&control);
		cv::setTrackbarPos("PG_disp12MaxDiff", "", control.PG_disp12MaxDiff);
		
		cv::createTrackbar("PG_preFilterCap", "", nullptr, 64, &trackbarCallback_PG_preFilterCap, (void *)&control);
		cv::setTrackbarPos("PG_preFilterCap", "", control.PG_preFilterCap);
		
		cv::createTrackbar("PG_preFilterSize", "", nullptr, 255, &trackbarCallback_PG_preFilterSize, (void *)&control);
		cv::setTrackbarPos("PG_preFilterSize", "", control.PG_preFilterSize);
		
		cv::createTrackbar("PG_preFilterType", "", nullptr, 1, &trackbarCallback_PG_preFilterType, (void *)&control);
		cv::setTrackbarPos("PG_preFilterType", "", control.PG_preFilterType);
		
		cv::createTrackbar("PG_textureThreshold", "", nullptr, 100, &trackbarCallback_PG_textureThreshold, (void *)&control);
		cv::setTrackbarPos("PG_textureThreshold", "", control.PG_textureThreshold);
		
		cv::createTrackbar("PG_uniquenessRatio", "", nullptr, 100, &trackbarCallback_PG_uniquenessRatio, (void *)&control);
		cv::setTrackbarPos("PG_uniquenessRatio", "", control.PG_uniquenessRatio);
		
		cv::createTrackbar("PG_speckleWindowSize", "", nullptr, 1000, &trackbarCallback_PG_speckleWindowSize, (void *)&control);
		cv::setTrackbarPos("PG_speckleWindowSize", "", control.PG_speckleWindowSize);
		
		cv::createTrackbar("PG_speckleRange", "", nullptr, 100, &trackbarCallback_PG_speckleRange, (void *)&control);
		cv::setTrackbarPos("PG_speckleRange", "", control.PG_speckleRange);
		

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
    bool rectified = false;
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
        status = mb_imagelist_read(verbose, imagelist_ptr, &imageStatus, &rectified, 
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

            /* if newly specified load camera calibration model */
            if (calibration_specified) {
                load_calibration(verbose, StereoCameraCalibrationFile, &control, &error);
                control.calibrationInitialized = true;
                calibration_specified = false;
            }
            if (!control.calibrationInitialized && !rectified) {
                fprintf(stderr,"\nCamera calibration not initialized...\n");
                fprintf(stderr,"\nProgram <%s> Terminated\n",
                    program_name);
                error = MB_ERROR_BAD_PARAMETER;
                mb_memory_clear(verbose, &error);
                exit(error);
            }

            /* process the stereo pair */
            if (use_this_pair) {
 
               /* set process structure to use */
                process->pair_count = npairs_process;
                process->rectified = rectified;
                strncpy(process->imageLeftFile, imageLeftFile, sizeof(mb_path));
                process->image_left_time_d = image_left_time_d;
                process->image_left_gain = image_left_gain;
                process->image_left_exposure = image_left_exposure;
                strncpy(process->imageRightFile, imageRightFile, sizeof(mb_path));
                process->image_right_time_d = image_right_time_d;
                process->image_right_gain = image_right_gain;
                process->image_right_exposure = image_right_exposure;

								process_stereopair(verbose, &control);
								 
								// Close window using esc key
								if (cv::waitKey(0) == 'q') exit(0);

                npairs_process++;
            }
        }

        /* handle parameter statements embedded in the recursive imagelist structure */
        else if (!done && imageStatus == MB_IMAGESTATUS_PARAMETER) {

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
        }
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);

    /* end it all */
    exit(status);

}
/*--------------------------------------------------------------------*/
