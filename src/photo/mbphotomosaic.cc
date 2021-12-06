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

#define MBPM_PRIORITY_CENTRALITY_ONLY               1
#define MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF      2

/* Image correction modes: */
/*   MBPM_CORRECTION_NONE: use images as is, no correction at all */
/*   MBPM_CORRECTION_BRIGHTNESS: correct image so that the average magnitude */
/*                               of each pixel is 70 out of 0->255 */
/*   MBPM_CORRECTION_RANGE: correct for pixel range from camera (as projected */
/*                               onto the seafloor) after correcting for gain and */
/*                               exposure */
/*   MBPM_CORRECTION_STANDOFF: correct for pixel standoff from seafloor (as projected */
/*                               onto the seafloor) after correcting for gain and */
/*                               exposure */
/*   MBPM_CORRECTION_FILE: correct using 3D correction table from  */
/*                               mbgetphotocorrection after correcting for gain  */
/*                               and exposure */
#define MBPM_CORRECTION_NONE            0
#define MBPM_CORRECTION_BRIGHTNESS      1
#define MBPM_CORRECTION_CAMERA_SETTINGS 2
#define MBPM_CORRECTION_RANGE           3
#define MBPM_CORRECTION_STANDOFF        4
#define MBPM_CORRECTION_FILE            5

#define MBPM_FORMAT_NONE              0
#define MBPM_FORMAT_TIFF              1
#define MBPM_FORMAT_PNG               2

// #define DEBUG 1

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

    // Output image and priority map
    Mat OutputImage;
    Mat OutputPriority;
#ifdef DEBUG
    Mat OutputIntensityCorrection;
    Mat OutputStandoff;
#endif
};

struct mbpm_control_struct {

    // Output mosaic parameters
    double OutputBounds[4];
    double OutputDx[2];
    int OutputDim[2];
    double mtodeglon;
    double mtodeglat;

    // Projection
    bool use_projection;
    void *pjptr;

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

    // Image correction
    int corr_mode;
    double reference_gain;
    double reference_exposure;
    double corr_range_target;
    double corr_range_coeff;
    double corr_standoff_target;
    double corr_standoff_coeff;

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
    double referenceIntensity[2];
    Mat corr_bounds;
    Mat corr_table[2];

    // Topography grid
    bool use_topography;
    void *topogrid_ptr;

    // Input pixel priority
    int priority_mode;
    double standoff_target;
    double standoff_range;
    double range_max;

    // Input image trimming
    int trimPixels;

    // Input image section size (pixels)
    int sectionPixels;
};

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
        cvtColor(imageUndistort, imageUndistortYCrCb, COLOR_BGR2YCrCb);
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

        /* Calculate the average intensity of the image */
        Scalar avgPixelIntensity = mean(imageUndistortYCrCb);

        /* get unit vector (cx, cy, cz) for direction camera is pointing */
        /* (1) rotate center pixel location using attitude and zzref */
        double zz;
        mb_platform_math_attitude_rotate_beam(verbose,
            0.0, 0.0, zzref,
            process->camera_roll, process->camera_pitch, 0.0,
            &xx, &yy, &zz,
            error);
        double rr = sqrt(xx * xx + yy * yy + zz * zz);
        double phi = RTD * atan2(yy, xx);
        double theta = RTD * acos(zz / rr);

        /* (2) calculate unit vector relative to the camera rig */
        double vx = sin(DTR * theta) * cos(DTR * phi);
        double vy = sin(DTR * theta) * sin(DTR * phi);
        double vz = cos(DTR * theta);

        /* (3) apply rotation of each camera relative to the rig */
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

        /* (4) rotate unit vector by camera rig heading */
        double cx = vxx * cos(DTR * process->camera_heading) + vyy * sin(DTR * process->camera_heading);
        double cy = -vxx * sin(DTR * process->camera_heading) + vyy * cos(DTR * process->camera_heading);
        double cz = vzz;

        /* (5) get standoff for center of image - this is the range of the intersection
               of the camera vector with the grid */
        double lon, lat, topo;
        if (control->use_topography) {
            *status = mb_topogrid_intersect(verbose, control->topogrid_ptr,
                        process->camera_navlon, process->camera_navlat, 0.0, process->camera_sensordepth,
                        control->mtodeglon, control->mtodeglat, cx, cy, cz,
                        &lon, &lat, &topo, &rr, error);
        }
        else {
            rr = control->standoff_target / vz;
            lon = process->camera_navlon + control->mtodeglon * vx * rr;
            lat = process->camera_navlat + control->mtodeglon * vy * rr;
            topo = -process->camera_sensordepth -  control->standoff_target;
        }
        double image_center_standoff = rr;

        /* Calculate intensity correction for this image - this will be modified
            if range, standoff, or lookup table correction is specified
            - Also calculate the final image correction for the center of the
              image by the method used for each section of pixels */
        double imageIntensityCorrection = 1.0;
        double centerIntensityCorrection = 1.0;;

        /* Apply no image correction */
        if (control->corr_mode == MBPM_CORRECTION_NONE) {
            imageIntensityCorrection = 1.0;
            centerIntensityCorrection = imageIntensityCorrection;
        }

        /* Apply brightness correction to each image separately
           - correct each image so the average intensity is a value of 70.0 */
        else if (control->corr_mode == MBPM_CORRECTION_BRIGHTNESS) {
            imageIntensityCorrection = 70.0 / avgPixelIntensity.val[0];
            centerIntensityCorrection = imageIntensityCorrection;
        }

        /* Image correction starts by correcting for camera gain and
            exposure and is followed later by correction for range or standoff or
            using a 3D correction table from mbgetphotocorrection */
        else {
            imageIntensityCorrection = 1.0;

            /* get correction for embedded camera gain */
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

            /* Apply camera setting correction */
            if (control->corr_mode == MBPM_CORRECTION_CAMERA_SETTINGS) {
                centerIntensityCorrection = imageIntensityCorrection;
            }

            /* Apply range based correction */
            else if (control->corr_mode == MBPM_CORRECTION_RANGE) {
                centerIntensityCorrection = imageIntensityCorrection;
                centerIntensityCorrection *= exp(control->corr_range_coeff * (rr - control->corr_range_target));
            }

            /* Apply standoff based correction */
            else if (control->corr_mode == MBPM_CORRECTION_STANDOFF) {
                centerIntensityCorrection = imageIntensityCorrection;
                centerIntensityCorrection *= exp(control->corr_standoff_coeff * (image_center_standoff - control->corr_standoff_target));
            }

            /* Apply correction by interpolation of 3D table generated by mbgetphotocorrection */
            else if (control->corr_mode == MBPM_CORRECTION_FILE) {
                int kbin_z1 = MIN(MAX(((int)((image_center_standoff + 0.5 * control->bin_dz) / control->bin_dz)), 0), control->ncorr_z - 2);
                int kbin_z2 = kbin_z1 + 1;
                double factor_z = ((double)image_center_standoff - 0.5 * control->bin_dz) / control->bin_dz - (double)kbin_z1;
                factor_z = MIN(MAX(factor_z, 0.0), 1.0);

                /* get reference intensity from center of image at the current standoff */
                double table_intensity_ref
                    = (1.0 - factor_z) * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z1)
                            + factor_z * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z2);

                centerIntensityCorrection = imageIntensityCorrection;
                if (table_intensity_ref > 0.0 && control->referenceIntensity[process->image_camera] > 0.0)
                    centerIntensityCorrection *= control->referenceIntensity[process->image_camera]
                                            / table_intensity_ref;
            }
        }

        /* Print information for image to be processed */
        int time_i[7];
        mb_get_date(verbose, process->time_d, time_i);
        fprintf(stderr,"%4d Camera:%d %s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d ",
                process->image_count, process->image_camera, process->imageFile,
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
        fprintf(stderr,"LLZ: %.8f %.8f %8.3f HRP: %6.2f %5.2f %5.2f A:%.3f Q:%.2f ",
                process->camera_navlon, process->camera_navlat, process->camera_sensordepth,
                process->camera_heading, process->camera_roll, process->camera_pitch,
                avgPixelIntensity.val[0], process->image_quality);
        fprintf(stderr,"G:%.0f/%.0f E:%.0f/%.0f S:%.3f C:%.3f %.3f",
                process->image_gain, control->reference_gain,
                process->image_exposure, control->reference_exposure,
                image_center_standoff, imageIntensityCorrection, centerIntensityCorrection);
        fprintf(stderr, "\n");

        /* Loop over the pixels in the undistorted image. If trim is nonzero then
            that number of pixels are ignored around the margins. This solves the
            problem of black pixels being incorporated into the photomosaic. If
            trim is not specified then code below will ignore both black pixels
            and pixels that are adjacent to black pixels. */

        for (int i=control->trimPixels; i<imageUndistort.cols-control->trimPixels; i++) {
            for (int j=control->trimPixels; j<imageUndistort.rows-control->trimPixels; j++) {
                bool use_pixel = true;
                double xx;
                double yy;
                double rrxysq;
                double rrxy;
                double rrxysq2;
                double rr2;
                double theta2;
                double dtheta;
                double pixel_priority;
                double standoff = 0.0;
                double lon, lat, topo;

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
                            for (int jj = MAX(j-1, 0); jj < MIN(i+2, imageUndistort.rows) && use_pixel; jj++) {
                                if (imageUndistort.at<Vec3b>(jj,ii)[0]
                                        + imageUndistort.at<Vec3b>(jj,ii)[1]
                                        + imageUndistort.at<Vec3b>(jj,ii)[2] == 0) {
                                    use_pixel = false;
                                }
                            }
                        }
                    }
                }

                if (use_pixel) {
                    /* calculate the pixel priority based on the distance
                        from the image center */
                    xx = i - center_x;
                    yy = center_y - j;
    //int debugprint = MB_NO;
    //int icenter_x = (int)center_x;
    //int icenter_y = (int)center_y;
    //if (i == icenter_x && j == icenter_y) {
    //debugprint = MB_YES;
    //}
                    rrxysq = xx * xx + yy * yy;
                    rrxy = sqrt(rrxysq);
                    rr = sqrt(rrxysq + zzref * zzref);
                    pixel_priority = (rrxymax - rrxy) / rrxymax;
    //if (debugprint == MB_YES) {
    //fprintf(stderr,"\nPRIORITY xx:%f yy:%f zzref:%f rr:%f rrxy:%f rrxymax:%f pixel_priority:%f\n",
    //xx,yy,zzref,rr,rrxy,rrxymax,pixel_priority);
    //}

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
                        rr = control->standoff_target / vz;
                        lon = process->camera_navlon + control->mtodeglon * vx * rr;
                        lat = process->camera_navlat + control->mtodeglon * vy * rr;
                        topo = -process->camera_sensordepth -  control->standoff_target;
                    }
                    zz = -process->camera_sensordepth - topo;

                    /* standoff is dot product of camera vector with projected pixel vector */
                    standoff = (cx * rr * vx) + (cy * rr * vy) + (cz * rr * vz);
//if (debugprint == MB_YES) {
//fprintf(stderr," llz: %.10f %.10f %.3f  range:%.3f  standoff:%.3f\n", lon, lat, topo, rr, standoff);
//}
                    /* Don't use pixel if too vertical or range too large */
                    if (theta > 80.0 || rr > control->range_max)
                        use_pixel = false;
                }

                if (use_pixel) {

                    /* modulate the source pixel priority by the standoff if desired */
                    if (control->priority_mode == MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF)  {
                        double dstandoff = (standoff - control->standoff_target) / control->standoff_range;
                        double standoff_priority = (float) (exp(-dstandoff * dstandoff));
                        pixel_priority *= standoff_priority;
                    }
//if (debugprint == MB_YES) {
//fprintf(stderr," standoff:%f pixel_priority:%.3f\n", standoff, pixel_priority);
//}

                    /* No correction - use original pixel BGR */
                    unsigned char b, g, r;
                    if (control->corr_mode == MBPM_CORRECTION_NONE) {
                        b = imageUndistort.at<Vec3b>(j,i)[0];
                        g = imageUndistort.at<Vec3b>(j,i)[1];
                        r = imageUndistort.at<Vec3b>(j,i)[2];
                    }

                    /* Apply specified image correction */
                    else {
                        /* For corr_mode == MBPM_CORRECTION_BRIGHTNESS
                           or MBPM_CORRECTION_CAMERA_SETTINGS
                           just use the imageIntensityCorrection calculated earlier.
                           For corr_mode == MBPM_CORRECTION_RANGE
                           or MBPM_CORRECTION_STANDOFF or MBPM_CORRECTION_FILE
                           start with imageIntensityCorrection and modify it */
                        double intensityCorrection = imageIntensityCorrection;

                        /* Apply range based correction to pixels */
                        if (control->corr_mode == MBPM_CORRECTION_RANGE) {
                            intensityCorrection *= exp(control->corr_range_coeff * (rr - control->corr_range_target));
                        }

                        /* Apply standoff based correction to pixels */
                        else if (control->corr_mode == MBPM_CORRECTION_STANDOFF) {
                            intensityCorrection *= exp(control->corr_standoff_coeff * (standoff - control->corr_standoff_target));
                        }

                        /* Apply correction by interpolation of 3D table generated by mbgetphotocorrection */
                        else if (control->corr_mode == MBPM_CORRECTION_FILE) {
                            int ibin_x1 = MIN(MAX(((int)((i + 0.5 * control->bin_dx) / control->bin_dx)), 0), control->ncorr_x - 2);
                            int ibin_x2 = ibin_x1 + 1;
                            double factor_x = ((double)i - 0.5 * control->bin_dx) / control->bin_dx - (double)ibin_x1;
                            int jbin_y1 = MIN(MAX(((int)((j + 0.5 * control->bin_dy) / control->bin_dy)), 0), control->ncorr_y - 2);
                            int jbin_y2 = jbin_y1 + 1;
                            double factor_y = ((double)j - 0.5 * control->bin_dy) / control->bin_dy - (double)jbin_y1;
                            int kbin_z1 = MIN(MAX(((int)((standoff + 0.5 * control->bin_dz) / control->bin_dz)), 0), control->ncorr_z - 2);
                            int kbin_z2 = kbin_z1 + 1;
                            double factor_z = ((double)standoff - 0.5 * control->bin_dz) / control->bin_dz - (double)kbin_z1;
                            factor_x = MIN(MAX(factor_x, 0.0), 1.0);
                            factor_y = MIN(MAX(factor_y, 0.0), 1.0);
                            factor_z = MIN(MAX(factor_z, 0.0), 1.0);

                            /* get reference intensity from center of image at the current standoff */
                            double table_intensity_ref
                                = (1.0 - factor_z) * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z1)
                                        + factor_z * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z2);

                            double v000 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y1, kbin_z1);
                            double v100 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y1, kbin_z1);
                            double v010 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y2, kbin_z1);
                            double v001 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y1, kbin_z2);
                            double v101 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y1, kbin_z2);
                            double v011 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y2, kbin_z2);
                            double v110 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y2, kbin_z1);
                            double v111 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y2, kbin_z2);
                            double vavg = v000 + v100 + v010 + v110 + v001 + v101 + v011 + v111;
                            int nvavg = 0;
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
                            double table_intensity = v000 * (1.0 - factor_x) * (1.0 - factor_y) * (1.0 - factor_x)
                                    + v100 * factor_x * (1.0 - factor_y) * (1.0 - factor_z)
                                    + v010 * (1.0 - factor_x) * factor_y * (1.0 - factor_z)
                                    + v001 * (1.0 - factor_x) * (1.0 - factor_y) * factor_z
                                    + v101 * factor_x * (1.0 - factor_y) * factor_z
                                    + v011 * (1.0 - factor_x) * factor_y * factor_z
                                    + v110 * factor_x * factor_y * (1.0 - factor_z)
                                    + v111 * factor_x * factor_y * factor_z;
                            if (table_intensity > 0.0 && control->referenceIntensity[process->image_camera] > 0.0) {
                                intensityCorrection *= control->referenceIntensity[process->image_camera]
                                                        / table_intensity;
                            }
                            //else {
                            //    intensityCorrection *= 1.0;
                            // }
                        }
                    }

                    /* access the pixel value in YCrCb image */
                    unsigned char Y = imageUndistortYCrCb.at<Vec3b>(j,i)[0];
                    unsigned char Cr = imageUndistortYCrCb.at<Vec3b>(j,i)[1];
                    unsigned char Cb = imageUndistortYCrCb.at<Vec3b>(j,i)[2];

                    /* correct Y (intensity) value */
                    Y = saturate_cast<unsigned char>(intensityCorrection * Y);

                    /* convert back to gbr */
                    b = saturate_cast<unsigned char>(Y + 1.773 * (Cb - 128));
                    g = saturate_cast<unsigned char>(Y - 0.714 * (Cr - 128) - 0.344 * (Cb - 128));
                    r = saturate_cast<unsigned char>(Y + 1.403 * (Cr - 128));

                    /* find the location and footprint of the input pixel
                        in the output image */
                    double diii, djjj;
                    double pixel_dx, pixel_dy;
                    if (control->use_projection) {
                        /* pixel location */
                        mb_proj_forward(verbose, control->pjptr, lon, lat, &xx, &yy, error);
                        diii = (xx - control->OutputBounds[0] + 0.5 * control->OutputDx[0]) / control->OutputDx[0];
                        djjj = (control->OutputBounds[3] - yy + 0.5 * control->OutputDx[1]) / control->OutputDx[1];

                        /* pixel footprint size */
                        pixel_dx = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) / control->OutputDx[0];
                        pixel_dy = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) / control->OutputDx[1];
                    }

                    else {
                        /* pixel location */
                        diii = (lon - control->OutputBounds[0] + 0.5 * control->OutputDx[0]) / control->OutputDx[0];
                        djjj = (control->OutputBounds[3] - lat + 0.5 * control->OutputDx[1]) / control->OutputDx[1];

                        /* pixel footprint size */
                        pixel_dx = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) * (control->mtodeglon / control->OutputDx[0]);
                        pixel_dy = 4.0 * rr * cos(DTR * theta) * tan (DTR * dtheta) * (control->mtodeglat / control->OutputDx[1]);
                    }

                    /* figure out the "footprint" extent of the mapped pixel */
                    int iii1 = (int)(floor(diii) - floor(pixel_dx));
                    int iii2 = (int)(floor(diii) + floor(pixel_dx));
                    int jjj1 = (int)(floor(djjj) - floor(pixel_dy));
                    int jjj2 = (int)(floor(djjj) + floor(pixel_dy));
                    unsigned int uiii1 = (unsigned int)(MAX(iii1, 0));
                    unsigned int uiii2 = (unsigned int)(MAX(MIN(iii2, control->OutputDim[0] - 1), 0));
                    unsigned int ujjj1 = (unsigned int)(MAX(jjj1, 0));
                    unsigned int ujjj2 = (unsigned int)(MAX(MIN(jjj2, control->OutputDim[1] - 1), 0));

                    unsigned int iii = MIN(MAX((unsigned int)floor(diii), 2), control->OutputDim[0] - 3);
                    unsigned int jjj = MIN(MAX((unsigned int)floor(djjj), 2), control->OutputDim[1] - 3);
                    bool out_of_map = true;
                    if (diii >= 2.0 && diii < control->OutputDim[0] - 3.0 && djjj >= 2.0 && djjj < control->OutputDim[1] - 3.0)
                        out_of_map = false;

//fprintf(stderr,"i:%d j:%d iii:%d:%d:%d jjj:%d:%d:%d theta:%f dtheta:%f pixel_dx:%f pixel_dy:%f mtodeglon:%g mtodeglat:%g\n",
//i,j,iii1,iii,iii2,jjj1,jjj,jjj2,theta,dtheta,pixel_dx,pixel_dy,control->mtodeglon,control->mtodeglat);
//fprintf(stderr,"iii:%d jjj:%d\n",iii,jjj);
//if (debugprint == MB_YES) {
//fprintf(stderr,"       standoff:%f pixel_priority:%.3f\n", standoff, pixel_priority);
//}

                    for (unsigned int ipix=uiii1;ipix<=uiii2;ipix++) {
                        for (unsigned int jpix=ujjj1;jpix<=ujjj2;jpix++) {
                            unsigned int kpix = control->OutputDim[0] * jpix + ipix;
                            double pixel_priority_use;
                            if (out_of_map)
                                pixel_priority_use = 0.98 * pixel_priority;
                            else if (ipix == iii && jpix == jjj)
                                pixel_priority_use = pixel_priority;
                            else if (ipix > iii-2 && iii < iii+2 && jpix > jjj-2 && jpix < jjj+2)
                                pixel_priority_use = 0.99 * pixel_priority;
                            else
                                pixel_priority_use = 0.98 * pixel_priority;
                            if (pixel_priority_use > process->OutputPriority.at<float>(jpix,ipix)) {
                                process->OutputImage.at<Vec3b>(jpix,ipix)[0] = b;
                                process->OutputImage.at<Vec3b>(jpix,ipix)[1] = g;
                                process->OutputImage.at<Vec3b>(jpix,ipix)[2] = r;
                                process->OutputPriority.at<float>(jpix,ipix) = pixel_priority_use;
#ifdef DEBUG
                                process->OutputIntensityCorrection.at<float>(jpix,ipix) = intensityCorrection;
                                process->OutputStandoff.at<float>(jpix,ipix) = standoff;
#endif
//if (debugprint == MB_YES) {
//fprintf(stderr,"              Pixel used: i:%d j:%d  ipix:%d jpix:%d  BGR:%d %d %d Priority:%f %f\n",
//i,j,ipix,jpix,
//process->OutputImage.at<Vec3b>(jpix,ipix)[0],
//process->OutputImage.at<Vec3b>(jpix,ipix)[1],
//process->OutputImage.at<Vec3b>(jpix,ipix)[2],
//pixel_priority_use, process->OutputPriority.at<float>(jpix,ipix));
//}
                            }
                        }
                    }
                }
            }
        }
        imageUndistortYCrCb.release();
        imageUndistort.release();

    }
}
/*--------------------------------------------------------------------*/
void process_image_sectioned2(int verbose, struct mbpm_process_struct *process,
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
        bool image_use = false;

        /* undistort the image */
        undistort(imageProcess, imageUndistort, control->cameraMatrix[process->image_camera], control->distCoeffs[process->image_camera], noArray());
        cvtColor(imageUndistort, imageUndistortYCrCb, COLOR_BGR2YCrCb);
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

        /* Calculate the average intensity of the image */
        Scalar avgPixelIntensity = mean(imageUndistortYCrCb);

        /* get unit vector (cx, cy, cz) for direction camera is pointing */
        /* (1) rotate center pixel location using attitude and zzref */
        double zz;
        mb_platform_math_attitude_rotate_beam(verbose,
            0.0, 0.0, zzref,
            process->camera_roll, process->camera_pitch, 0.0,
            &xx, &yy, &zz,
            error);
        double rr = sqrt(xx * xx + yy * yy + zz * zz);
        double phi = RTD * atan2(yy, xx);
        double theta = RTD * acos(zz / rr);

        /* (2) calculate unit vector relative to the camera rig */
        double vx = sin(DTR * theta) * cos(DTR * phi);
        double vy = sin(DTR * theta) * sin(DTR * phi);
        double vz = cos(DTR * theta);

        /* (3) apply rotation of each camera relative to the rig */
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

        /* (4) rotate unit vector by camera rig heading */
        double cx = vxx * cos(DTR * process->camera_heading) + vyy * sin(DTR * process->camera_heading);
        double cy = -vxx * sin(DTR * process->camera_heading) + vyy * cos(DTR * process->camera_heading);
        double cz = vzz;

        /* (5) get standoff for center of image - this is the range of the intersection
               of the camera vector with the grid */
        double lon, lat, topo;
        if (control->use_topography) {
            *status = mb_topogrid_intersect(verbose, control->topogrid_ptr,
                        process->camera_navlon, process->camera_navlat, 0.0, process->camera_sensordepth,
                        control->mtodeglon, control->mtodeglat, cx, cy, cz,
                        &lon, &lat, &topo, &rr, error);
        }
        else {
            rr = control->standoff_target / vz;
            lon = process->camera_navlon + control->mtodeglon * vx * rr;
            lat = process->camera_navlat + control->mtodeglon * vy * rr;
            topo = -process->camera_sensordepth -  control->standoff_target;
        }
        double image_center_standoff = rr;

        /* Calculate intensity correction for this image - this will be modified
            if range, standoff, or lookup table correction is specified
            - Also calculate the final image correction for the center of the
              image by the method used for each section of pixels */
        double imageIntensityCorrection = 1.0;
        double centerIntensityCorrection = 1.0;;

        /* Apply no image correction */
        if (control->corr_mode == MBPM_CORRECTION_NONE) {
            imageIntensityCorrection = 1.0;
            centerIntensityCorrection = imageIntensityCorrection;
        }

        /* Apply brightness correction to each image separately
           - correct each image so the average intensity is a value of 70.0 */
        else if (control->corr_mode == MBPM_CORRECTION_BRIGHTNESS) {
            imageIntensityCorrection = 70.0 / avgPixelIntensity.val[0];
            centerIntensityCorrection = imageIntensityCorrection;
        }

        /* Image correction starts by correcting for camera gain and
            exposure and is followed later by correction for range or standoff or
            using a 3D correction table from mbgetphotocorrection */
        else {
            imageIntensityCorrection = 1.0;

            /* get correction for embedded camera gain */
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

            /* Apply camera setting correction */
            if (control->corr_mode == MBPM_CORRECTION_CAMERA_SETTINGS) {
                centerIntensityCorrection = imageIntensityCorrection;
            }

            /* Apply range based correction */
            else if (control->corr_mode == MBPM_CORRECTION_RANGE) {
                centerIntensityCorrection = imageIntensityCorrection;
                centerIntensityCorrection *= exp(control->corr_range_coeff * (rr - control->corr_range_target));
            }

            /* Apply standoff based correction */
            else if (control->corr_mode == MBPM_CORRECTION_STANDOFF) {
                centerIntensityCorrection = imageIntensityCorrection;
                centerIntensityCorrection *= exp(control->corr_standoff_coeff * (image_center_standoff - control->corr_standoff_target));
            }

            /* Apply correction by interpolation of 3D table generated by mbgetphotocorrection */
            else if (control->corr_mode == MBPM_CORRECTION_FILE) {
                int kbin_z1 = MIN(MAX(((int)((image_center_standoff + 0.5 * control->bin_dz) / control->bin_dz)), 0), control->ncorr_z - 2);
                int kbin_z2 = kbin_z1 + 1;
                double factor_z = ((double)image_center_standoff - 0.5 * control->bin_dz) / control->bin_dz - (double)kbin_z1;
                factor_z = MIN(MAX(factor_z, 0.0), 1.0);

                /* get reference intensity from center of image at the current standoff */
                double table_intensity_ref
                    = (1.0 - factor_z) * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z1)
                            + factor_z * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z2);

                centerIntensityCorrection = imageIntensityCorrection;
                if (table_intensity_ref > 0.0 && control->referenceIntensity[process->image_camera] > 0.0)
                    centerIntensityCorrection *= control->referenceIntensity[process->image_camera]
                                            / table_intensity_ref;
            }
        }

        /* Loop over sections of the undistorted image, and map those onto the
           destination image as continuous quads. The priority determining if the
           section is mapped is calculated for the center pixel of the section. */
        unsigned int nsection_x = (imageUndistort.cols / control->sectionPixels);
        if (imageUndistort.cols % control->sectionPixels)
            nsection_x++;
        unsigned int nsection_y = (imageUndistort.rows / control->sectionPixels);
        if (imageUndistort.rows % control->sectionPixels)
            nsection_y++;
        for (int isection=0; isection<nsection_x; isection++) {
            for (int jsection=0; jsection<nsection_y; jsection++) {
                int i0 = MAX(control->trimPixels, isection * control->sectionPixels);
                int i1 = MIN(((int)imageUndistort.cols - control->trimPixels - 1), ((isection + 1) * control->sectionPixels - 1));
                int j0 = MAX(control->trimPixels, jsection * control->sectionPixels);
                int j1 = MIN(((int)imageUndistort.rows - control->trimPixels - 1), ((jsection + 1) * control->sectionPixels - 1));
                int ic = (i0 + i1) / 2; // center
                int jc = (j0 + j1) / 2; // center
                Point2f srcCorners[5], dstCorners[5];
                Point2f dstCornersWide[5];
                double range[5], standoff[5];
                Point2i dstCornerPixels[5];
                bool use_section = true;
                double section_priority = 0.0;
                double rrxy;
                double dtheta;

                /* if entire section or center point is trimmed ignore */
                if (i1 < i0 || j1 < j0 || ic < i0 || ic > i1 || jc < j0 || jc > j1)
                    use_section = false;

                /* calculate the location and standoff of the section corners and center */
                if (use_section) {
                    srcCorners[0].x = i0 - center_x; // lower left
                    srcCorners[0].y = center_y - j0; // lower left
                    srcCorners[1].x = i0 - center_x; // upper left
                    srcCorners[1].y = center_y - j1; // upper left
                    srcCorners[2].x = i1 - center_x; // upper right
                    srcCorners[2].y = center_y - j1; // upper right
                    srcCorners[3].x = i1 - center_x; // lower right
                    srcCorners[3].y = center_y - j0; // lower right
                    srcCorners[4].x = ic - center_x; // center
                    srcCorners[4].y = center_y - jc; // center

                    for (int icorner = 0; icorner < 5 && use_section; icorner++) {

                        /* rotate pixel location using attitude and zzref */
                        mb_platform_math_attitude_rotate_beam(verbose,
                            srcCorners[icorner].x, srcCorners[icorner].y, zzref,
                            process->camera_roll, process->camera_pitch, 0.0,
                            &xx, &yy, &zz,
                            error);

                        /* calculate the pixel takeoff angles relative
                            to the world frame vertical (but heading not yet applied) */
                        double rrxysq = xx * xx + yy * yy;
                        rrxy = sqrt(rrxysq);
                        rr = sqrt(rrxysq + zz * zz);
                        phi = RTD * atan2(yy, xx);
                        theta = RTD * acos(zz / rr);

                        /* continue only if takeoff angle is not too vertical
                            (this is a 2D photomosaic) */
                        if (theta > 80.0)
                            use_section = false;
                        if (use_section) {

                            /* calculate unit direction vector of pixel */
                            vz = cos(DTR * theta);
                            vx = sin(DTR * theta) * cos(DTR * phi);
                            vy = sin(DTR * theta) * sin(DTR * phi);

                            /* apply rotation of each camera relative to the rig */
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
                            vx = vxx * cos(DTR * process->camera_heading) + vyy * sin(DTR * process->camera_heading);
                            vy = -vxx * sin(DTR * process->camera_heading) + vyy * cos(DTR * process->camera_heading);
                            vz = vzz;

                            /* find the location where this vector intersects the grid */
                            if (control->use_topography) {
                                *status = mb_topogrid_intersect(verbose, control->topogrid_ptr,
                                            process->camera_navlon, process->camera_navlat, 0.0, process->camera_sensordepth,
                                            control->mtodeglon, control->mtodeglat, vx, vy, vz,
                                            &lon, &lat, &topo, &rr, error);
                            }
                            else {
                                rr = control->standoff_target / vz;
                                lon = process->camera_navlon + control->mtodeglon * vx * rr;
                                lat = process->camera_navlat + control->mtodeglon * vy * rr;
                                topo = -process->camera_sensordepth -  control->standoff_target;
                            }
                            zz = -process->camera_sensordepth - topo;
                        }

                        /* continue only if range not too large */
                        if (use_section && rr > control->range_max)
                            use_section = false;
                        if (use_section) {

                            /* standoff is dot product of camera vector with projected pixel vector */
                            range[icorner] = rr;
                            standoff[icorner] = (cx * rr * vx) + (cy * rr * vy) + (cz * rr * vz);

                            /* find  location in the output image */
                            if (control->use_projection) {
                                mb_proj_forward(verbose, control->pjptr, lon, lat, &xx, &yy, error);
                                dstCorners[icorner].x = ((xx - control->OutputBounds[0] + 0.5 * control->OutputDx[0]) / control->OutputDx[0]);
                                dstCorners[icorner].y = ((control->OutputBounds[3] - yy + 0.5 * control->OutputDx[1]) / control->OutputDx[1]);
                            } else {
                                dstCorners[icorner].x = ((lon - control->OutputBounds[0] + 0.5 * control->OutputDx[0]) / control->OutputDx[0]);
                                dstCorners[icorner].y = ((control->OutputBounds[3] - lat + 0.5 * control->OutputDx[1]) / control->OutputDx[1]);
                            }
                            dstCornersWide[icorner].x = dstCorners[icorner].x;
                            dstCornersWide[icorner].y = dstCorners[icorner].y;
                            dstCornerPixels[icorner].x = (int)dstCorners[icorner].x;
                            dstCornerPixels[icorner].y = (int)dstCorners[icorner].y;

                            /* at section center point calculate section priority, then
                                check if section center is in the output image and if the
                                priority of section center is greater than the existing priority
                                at the corresponding point in the output image */
                            if (icorner == 4) {
                                /* calculate the section priority based on the distance
                                    from the image center and if specified the standoff */
                                section_priority = (rrxymax - rrxy) / rrxymax;
                                if (control->priority_mode == MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF)  {
                                    double dstandoff = (standoff[icorner] - control->standoff_target) / control->standoff_range;
                                    double standoff_priority = (float) (exp(-dstandoff * dstandoff));
                                    section_priority *= standoff_priority;
                                }

                                if (dstCornerPixels[icorner].x < 0 || dstCornerPixels[icorner].x >= control->OutputDim[0]
                                    || dstCornerPixels[icorner].y < 0 || dstCornerPixels[icorner].y >= control->OutputDim[1]
                                    || section_priority <= process->OutputPriority.at<float>(dstCornerPixels[icorner].y,dstCornerPixels[icorner].x))
                                    use_section = false;
                            }
                        }
                    }
                }

                /* if ok and higher priority, map entire section onto the output image */
                if (use_section) {

                    /* Print information for image to be processed */
                    if (!image_use) {
                        image_use = true;
                        int time_i[7];
                        mb_get_date(verbose, process->time_d, time_i);
                        fprintf(stderr,"%4d Camera:%d %s %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d ",
                                process->image_count, process->image_camera, process->imageFile,
                                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
                        fprintf(stderr,"LLZ: %.8f %.8f %8.3f HRP: %6.2f %5.2f %5.2f A:%.3f Q:%.2f ",
                                process->camera_navlon, process->camera_navlat, process->camera_sensordepth,
                                process->camera_heading, process->camera_roll, process->camera_pitch,
                                avgPixelIntensity.val[0], process->image_quality);
                        fprintf(stderr,"G:%.0f/%.0f E:%.0f/%.0f S:%.3f C:%.3f %.3f",
                                process->image_gain, control->reference_gain,
                                process->image_exposure, control->reference_exposure,
                                image_center_standoff, imageIntensityCorrection, centerIntensityCorrection);
                        fprintf(stderr, "\n");
                    }

                    /* widen the quad in the destination image to avoid missing some pixels */
                    for (int icorner = 0; icorner < 4; icorner++) {
                        dstCornersWide[icorner].x += (dstCorners[icorner].x > dstCorners[4].x) ? 5.0 : -5.0;
                        dstCornersWide[icorner].y += (dstCorners[icorner].y > dstCorners[4].y) ? 5.0 : -5.0;
                        dstCornerPixels[icorner].x = (int)dstCornersWide[icorner].x;
                        dstCornerPixels[icorner].y = (int)dstCornersWide[icorner].y;
                    }

                    /* Calculate non-affine transformation for this section
                        allowing calculation of the corresponding source location
                        for any pixel in the destination image. */
                    Mat transformation = getPerspectiveTransform(dstCorners, srcCorners, DECOMP_LU);

                    /* get bounds of the region of interest in the destination image */
                    int di0 = dstCornerPixels[4].x;
                    int di1 = dstCornerPixels[4].x;
                    int dj0 = dstCornerPixels[4].y;
                    int dj1 = dstCornerPixels[4].y;
                    for (int icorner = 0; icorner < 4; icorner++) {
                      di0 = MIN(di0, dstCornerPixels[icorner].x);
                      di1 = MAX(di1, dstCornerPixels[icorner].x);
                      dj0 = MIN(dj0, dstCornerPixels[icorner].y);
                      dj1 = MAX(dj1, dstCornerPixels[icorner].y);
                    }
                    di0 = MAX(di0, 0);
                    di1 = MIN(di1, control->OutputDim[0]-1);
                    dj0 = MAX(dj0, 0);
                    dj1 = MIN(dj1, control->OutputDim[1]-1);

                    /* loop over the region of interest in the destination image
                        - for each pixel actually inside the bounds of the
                        projected section, get the color of the corresponding
                        pixel in the source image */
                    for (int di = di0; di <= di1; di++) {
                        for (int dj = dj0; dj <= dj1; dj++) {
                            /* Determine if the pixel di,dj is inside the quad
                                defined by the section corners in the destination
                                image. Since the corner points of the section are ordered
                                counterclockwise, the z-component of the cross
                                product between the vector from corner i to
                                point di,dj with the vector between corner i and i+1
                                will be positive if di,dj is "to the right" of the
                                quad side. If the cross product z-components are
                                positive for all four sides, the point di,dj is
                                inside the section quad. */
                            vector<Point2f> dstPoint(1), srcPoint(1);
                            dstPoint[0].x = di;
                            dstPoint[0].y = dj;
                            bool inside = true;
                            for (int icorner = 0; icorner < 4 && inside; icorner++) {
                                int icorner2 = icorner + 1;
                                if (icorner2 == 4)
                                    icorner2 = 0;
                                float xprod = ((dstPoint[0].x - dstCornersWide[icorner].x) * (dstCornersWide[icorner2].y - dstCornersWide[icorner].y)
                                                - (dstPoint[0].y - dstCornersWide[icorner].y) * (dstCornersWide[icorner2].x - dstCornersWide[icorner].x));

                                if (xprod < 0.0)
                                    inside = false;
                            }

                            /* if pixel is inside the section calculate the corresponding
                                point in the source image */
                            bool use_pixel = false;
                            int si, sj;
                            if (inside) {
                                perspectiveTransform(dstPoint, srcPoint, transformation);
                                si = center_x + srcPoint[0].x;
                                sj = center_y - srcPoint[0].y;

                                /* check that source pixel is inside the source image */
                                if (si < 0 || si >= imageUndistort.cols
                                    || sj < 0 || sj >= imageUndistort.rows)
                                    use_pixel = false;
                                else
                                    use_pixel = true;
                            }

                            if (use_pixel) {

                                /* Deal with problem of black pixels at the margins of the
                                    undistorted images. If the user has not specified a trim
                                    range for the image margins, then ignore all purely black
                                    pixels and all pixels adjacent to purely black pixels. */
                                if (control->trimPixels == 0) {
                                    unsigned int sum = imageUndistort.at<Vec3b>(sj,si)[0]
                                                        + imageUndistort.at<Vec3b>(sj,si)[1]
                                                        + imageUndistort.at<Vec3b>(sj,si)[2];
                                    if (sum == 0)
                                        use_pixel = false;
                                    else {
                                        for (int ii = MAX(si-1, 0); ii < MIN(si+2, imageUndistort.cols) && use_pixel; ii++) {
                                            for (int jj = MAX(sj-1, 0); jj < MIN(sj+2, imageUndistort.rows) && use_pixel; jj++) {
                                                if (imageUndistort.at<Vec3b>(jj,ii)[0]
                                                        + imageUndistort.at<Vec3b>(jj,ii)[1]
                                                        + imageUndistort.at<Vec3b>(jj,ii)[2] == 0) {
                                                    use_pixel = false;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            if (use_pixel) {

                                /* No correction - use original pixel BGR */
                                unsigned char b, g, r;
                                if (control->corr_mode == MBPM_CORRECTION_NONE) {
                                    b = imageUndistort.at<Vec3b>(sj,si)[0];
                                    g = imageUndistort.at<Vec3b>(sj,si)[1];
                                    r = imageUndistort.at<Vec3b>(sj,si)[2];
                                }

                                /* Apply specified image correction */
                                else {
                                    /* For corr_mode == MBPM_CORRECTION_BRIGHTNESS
                                       or MBPM_CORRECTION_CAMERA_SETTINGS
                                       just use the imageIntensityCorrection calculated earlier.
                                       For corr_mode == MBPM_CORRECTION_RANGE
                                       or MBPM_CORRECTION_STANDOFF or MBPM_CORRECTION_FILE
                                       start with imageIntensityCorrection and modify it */
                                    double intensityCorrection = imageIntensityCorrection;

                                    /* Apply range based correction to pixels */
                                    if (control->corr_mode == MBPM_CORRECTION_RANGE) {
                                        intensityCorrection *= exp(control->corr_range_coeff * (range[4] - control->corr_range_target));
                                    }

                                    /* Apply standoff based correction to pixels */
                                    else if (control->corr_mode == MBPM_CORRECTION_STANDOFF) {
                                        intensityCorrection *= exp(control->corr_standoff_coeff * (standoff[4] - control->corr_standoff_target));
                                    }

                                    /* Apply correction by interpolation of 3D table generated by mbgetphotocorrection */
                                    else if (control->corr_mode == MBPM_CORRECTION_FILE) {
                                        int ibin_x1 = MIN(MAX(((int)((si + 0.5 * control->bin_dx) / control->bin_dx)), 0), control->ncorr_x - 2);
                                        int ibin_x2 = ibin_x1 + 1;
                                        double factor_x = ((double)si - 0.5 * control->bin_dx) / control->bin_dx - (double)ibin_x1;
                                        int jbin_y1 = MIN(MAX(((int)((sj + 0.5 * control->bin_dy) / control->bin_dy)), 0), control->ncorr_y - 2);
                                        int jbin_y2 = jbin_y1 + 1;
                                        double factor_y = ((double)sj - 0.5 * control->bin_dy) / control->bin_dy - (double)jbin_y1;
                                        int kbin_z1 = MIN(MAX(((int)((standoff[4] + 0.5 * control->bin_dz) / control->bin_dz)), 0), control->ncorr_z - 2);
                                        int kbin_z2 = kbin_z1 + 1;
                                        double factor_z = ((double)standoff[4] - 0.5 * control->bin_dz) / control->bin_dz - (double)kbin_z1;
                                        factor_x = MIN(MAX(factor_x, 0.0), 1.0);
                                        factor_y = MIN(MAX(factor_y, 0.0), 1.0);
                                        factor_z = MIN(MAX(factor_z, 0.0), 1.0);

                                        /* get reference intensity from center of image at the current standoff */
                                        double table_intensity_ref
                                            = (1.0 - factor_z) * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z1)
                                                    + factor_z * control->corr_table[process->image_camera].at<float>(control->ibin_xcen, control->jbin_ycen, kbin_z2);

                                        double v000 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y1, kbin_z1);
                                        double v100 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y1, kbin_z1);
                                        double v010 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y2, kbin_z1);
                                        double v001 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y1, kbin_z2);
                                        double v101 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y1, kbin_z2);
                                        double v011 = control->corr_table[process->image_camera].at<float>(ibin_x1, jbin_y2, kbin_z2);
                                        double v110 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y2, kbin_z1);
                                        double v111 = control->corr_table[process->image_camera].at<float>(ibin_x2, jbin_y2, kbin_z2);
                                        double vavg = v000 + v100 + v010 + v110 + v001 + v101 + v011 + v111;
                                        int nvavg = 0;
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
                                        double table_intensity = v000 * (1.0 - factor_x) * (1.0 - factor_y) * (1.0 - factor_x)
                                                + v100 * factor_x * (1.0 - factor_y) * (1.0 - factor_z)
                                                + v010 * (1.0 - factor_x) * factor_y * (1.0 - factor_z)
                                                + v001 * (1.0 - factor_x) * (1.0 - factor_y) * factor_z
                                                + v101 * factor_x * (1.0 - factor_y) * factor_z
                                                + v011 * (1.0 - factor_x) * factor_y * factor_z
                                                + v110 * factor_x * factor_y * (1.0 - factor_z)
                                                + v111 * factor_x * factor_y * factor_z;
                                        if (table_intensity > 0.0 && control->referenceIntensity[process->image_camera] > 0.0)
                                            intensityCorrection *= control->referenceIntensity[process->image_camera]
                                                                    / table_intensity;
                                        // else {
                                        //    intensityCorrection *= 1.0;
                                        // }
                                    }

                                    /* access the pixel value in YCrCb image */
                                    unsigned char Y = imageUndistortYCrCb.at<Vec3b>(sj,si)[0];
                                    unsigned char Cr = imageUndistortYCrCb.at<Vec3b>(sj,si)[1];
                                    unsigned char Cb = imageUndistortYCrCb.at<Vec3b>(sj,si)[2];

                                    /* correct Y (intensity) value */
                                    Y = saturate_cast<unsigned char>(intensityCorrection * Y);

                                    /* convert back to gbr */
                                    b = saturate_cast<unsigned char>(Y + 1.773 * (Cb - 128));
                                    g = saturate_cast<unsigned char>(Y - 0.714 * (Cr - 128) - 0.344 * (Cb - 128));
                                    r = saturate_cast<unsigned char>(Y + 1.403 * (Cr - 128));
                                }

                                process->OutputImage.at<Vec3b>(dj,di)[0] = b;
                                process->OutputImage.at<Vec3b>(dj,di)[1] = g;
                                process->OutputImage.at<Vec3b>(dj,di)[2] = r;
                                process->OutputPriority.at<float>(dj,di) = section_priority;
                            }
                        }
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
    char program_name[] = "mbphotomosaic";
    char help_message[] =  "mbphotomosaic makes a mosaic of navigated downlooking photographs.";
    char usage_message[] = "mbphotomosaic \n"
                            "\t--verbose\n"
                            "\t--help\n"
                            "\t--threads=nthreads\n"
                            "\t--input=imagelist\n"
                            "\t--output=file\n"
                            "\t--output-tiff\n"
                            "\t--output-png\n"
                            "\t--image-dimensions=width/height\n"
                            "\t--image-spacing=dx/dy[/units]\n"
                            "\t--fov-fudgefactor=factor\n"
                            "\t--projection=projection_pars\n"
                            "\t--altitude=standoff_target/standoff_range\n"
                            "\t--standoff=standoff_target/standoff_range\n"
                            "\t--rangemax=range_max\n"
                            "\t--trim=trim_pixels\n"
                            "\t--bounds=lonmin/lonmax/latmin/latmax | west/east/south/north\n"
                            "\t--bounds-buffer=bounds_buffer\n"
                            "\t--correction-brightness\n"
                            "\t--correction-camera-settings\n"
                            "\t--correction-range=target/coeff\n"
                            "\t--correction-standoff=target/coeff\n"
                            "\t--correction-file=imagecorrection.yaml\n"
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
    extern char *optarg;
    int    option_index;
    int    errflg = 0;
    int    c;
    int    help = 0;
    int    flag = 0;

    /* parameter controls */
    struct mbpm_process_struct processPars[MB_THREAD_MAX];
    struct mbpm_control_struct control;

    /* Output image variables */
    bool    bounds_specified = false;
    double bounds_buffer = 6.0;
    int    spacing_priority = MB_NO;
    int    set_dimensions = MB_NO;
    int    set_spacing = MB_NO;
    double    dx_set = 0.0;
    double    dy_set = 0.0;
    char      units[MB_PATH_MAXLINE];
    mb_path   OutputImageFile;
    bool      outputimage_specified = false;
    int       output_format = MBPM_FORMAT_NONE;
    control.priority_mode = MBPM_PRIORITY_CENTRALITY_ONLY;
    control.standoff_target = 3.0;
    control.standoff_range = 1.0;
    control.range_max = 200.0;
    control.trimPixels = 0;
    control.sectionPixels = 0;
    control.reference_gain = 15.0;
    control.reference_exposure = 4000.0;

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
    mb_path StereoCameraCalibrationFile;
    control.calibration_set = false;
    control.SensorWidthMm = 8.789;
    control.SensorHeightMm = 6.610;
    control.SensorCellMm =0.00454;
    control.fov_fudgefactor = 1.0;

    bool undistort_initialized = false;

    /* Input image correction */
    control.corr_mode = MBPM_CORRECTION_NONE;
    control.corr_range_target = 3.0;
    control.corr_range_coeff = 1.0;
    control.corr_standoff_target = 3.0;
    control.corr_standoff_coeff = 1.0;
    control.reference_gain = 15.0;
    control.reference_exposure = 4000.0;
    mb_path ImageCorrectionFile;
    control.ncorr_x = 21;
    control.ncorr_y = 21;
    control.ncorr_z = 100;
    control.corr_xmin = 0.0;
    control.corr_xmax = 10.0;
    control.corr_ymin = 0.0;
    control.corr_ymax = 10.0;
    control.corr_zmin = 0.0;
    control.corr_zmax = 10.0;
    control.bin_dx = 0.0;
    control.bin_dy = 0.0;
    control.bin_dz = 0.0;
    control.ibin_xcen = 0;
    control.jbin_ycen = 0;
    control.kbin_zcen = 0;

    /* Input navigation variables */
    bool navigation_specified = false;
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

    /* Input tide variables */
    bool use_tide = false;
    mb_path TideFile;
    int ittime = 0;
    int ntide = 0;
    double *ttime = NULL;
    double *ttide = NULL;

    /* Input quality variables */
    bool use_imagequality = false;
    double imageQualityThreshold = 0.0;
    double imageQualityFilterLength = 0.0;
    bool ImageQualityFile_specified = false;
    mb_path ImageQualityFile;
    int iqtime = 0;
    int nquality = 0;
    double *qtime = NULL;
    double *qquality = NULL;

    /* topography parameters */
    control.use_topography = false;
    mb_path TopographyGridFile;
    control.topogrid_ptr = NULL;

    /* projected image parameters */
    control.use_projection = false;
    double reference_lon, reference_lat;
    int utm_zone = 1;
    char projection_pars[MB_PATH_MAXLINE];
    char projection_id[MB_PATH_MAXLINE] = "Geographic";

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
    /* mbphotomosaic
     *    --verbose
     *    --help
     *    --threads=nthreads
     *    --input=imagelist
     *    --output=file
     *    --output-tiff
     *    --output-png
     *    --image-dimensions=width/height
     *    --image-spacing=dx/dy[/units]
     *    --fov-fudgefactor=factor
     *    --projection=projection_pars
     *    --altitude=standoff_target/standoff_range
     *    --standoff=standoff_target/standoff_range
     *    --rangemax=range_max
     *    --trim=trim_pixels
     *    --section=section_pixels
     *    --bounds=lonmin/lonmax/latmin/latmax | west/east/south/north
     *    --bounds-buffer=bounds_buffer
     *    --correction-brightness
     *    --correction-camera-settings
     *    --correction-range=target/coeff
     *    --correction-standoff=target/coeff
     *    --correction-file=imagecorrection.yaml
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
        {"output-tiff",                 no_argument,            NULL,         0},
        {"output-png",                  no_argument,            NULL,         0},
        {"image-file",                  required_argument,      NULL,         0},
        {"image-dimensions",            required_argument,      NULL,         0},
        {"image-spacing",               required_argument,      NULL,         0},
        {"fov-fudgefactor",             required_argument,      NULL,         0},
        {"projection",                  required_argument,      NULL,         0},
        {"altitude",                    required_argument,      NULL,         0},
        {"standoff",                    required_argument,      NULL,         0},
        {"rangemax",                    required_argument,      NULL,         0},
        {"trim",                        required_argument,      NULL,         0},
        {"section",                     required_argument,      NULL,         0},
        {"bounds",                      required_argument,      NULL,         0},
        {"bounds-buffer",               required_argument,      NULL,         0},
        {"correction-brightness",       no_argument,            NULL,         0},
        {"correction-camera-settings",  no_argument,            NULL,         0},
        {"correction-range",            required_argument,      NULL,         0},
        {"correction-standoff",         required_argument,      NULL,         0},
        {"correction-file",             required_argument,      NULL,         0},
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
    sprintf(ImageListFile, "imagelist.mb-1");
    sprintf(ImageCorrectionFile, "imagelist_cameracorrection.yml");
    control.OutputDim[0] = 1000;
    control.OutputDim[1] = 1000;

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
    std::thread mbphotomosaicThreads[MB_THREAD_MAX];
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
                    || (strcmp("image-file", options[option_index].name) == 0))
                {
                const int n = sscanf (optarg,"%s", OutputImageFile);
                if (n == 1)
                    {
                    outputimage_specified = true;
                    if (output_format == MBPM_FORMAT_NONE) {
                        if (strlen(OutputImageFile) >= 5
                            && (strncmp(".png", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0
                                || strncmp(".PNG", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0))
                            output_format = MBPM_FORMAT_PNG;
                        else if (strlen(OutputImageFile) >= 5
                            && (strncmp(".tif", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0
                                && strncmp(".TIF", &OutputImageFile[strlen(OutputImageFile)-4], 4) == 0))
                            output_format = MBPM_FORMAT_TIFF;
                        else if (strlen(OutputImageFile) >= 6
                            && (strncmp(".tiff", &OutputImageFile[strlen(OutputImageFile)-5], 5) == 0
                                && strncmp(".TIFF", &OutputImageFile[strlen(OutputImageFile)-5], 5) == 0))
                            output_format = MBPM_FORMAT_TIFF;
                        }
                    }
                }

            /* output-tiff */
            else if (strcmp("output-tiff", options[option_index].name) == 0)
                {
                output_format = MBPM_FORMAT_TIFF;
                }

            /* output-png */
            else if (strcmp("output-png", options[option_index].name) == 0)
                {
                output_format = MBPM_FORMAT_PNG;
                }

            /* image-dimensions */
            else if (strcmp("image-dimensions", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%d/%d", &control.OutputDim[0], &control.OutputDim[1]);
                if (n == 2 && control.OutputDim[0] > 0 && control.OutputDim[1] > 0)
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
                sscanf (optarg,"%lf", &control.fov_fudgefactor);
                }

            /* projection */
            else if (strcmp("projection", options[option_index].name) == 0)
                {
                sscanf (optarg,"%s", projection_pars);
                control.use_projection = true;
                }

            /* altitude */
            else if (strcmp("altitude", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf/%lf", &control.standoff_target, &control.standoff_range);
                if (n ==2 && control.standoff_target > 0.0 && control.standoff_range > 0.0)
                    control.priority_mode = MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF;
                }

            /* standoff */
            else if (strcmp("standoff", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%lf/%lf", &control.standoff_target, &control.standoff_range);
                if (n ==2 && control.standoff_target > 0.0 && control.standoff_range > 0.0)
                    control.priority_mode = MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF;
                }

            /* trim */
            else if (strcmp("trim", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%d", &control.trimPixels);
                }

            /* section */
            else if (strcmp("section", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%d", &control.sectionPixels);
                }

            /* rangemax */
            else if (strcmp("rangemax", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &control.range_max);
                }

            /* bounds */
            else if (strcmp("bounds", options[option_index].name) == 0)
                {
                bounds_specified = mb_get_bounds(optarg, control.OutputBounds);
                }

            /* bounds-buffer */
            else if (strcmp("bounds-buffer", options[option_index].name) == 0)
                {
                sscanf (optarg,"%lf", &bounds_buffer);
                }

            /* correction-brightness */
            else if (strcmp("correction-brightness", options[option_index].name) == 0)
                {
                control.corr_mode = MBPM_CORRECTION_BRIGHTNESS;
                }

            /* correction-camera-settings */
            else if (strcmp("correction-camera-settings", options[option_index].name) == 0)
                {
                control.corr_mode = MBPM_CORRECTION_CAMERA_SETTINGS;
                }

            /* correction-range */
            else if (strcmp("correction-range", options[option_index].name) == 0)
                {
                int n = sscanf (optarg,"%lf/%lf", &control.corr_range_target, &control.corr_range_coeff);
                if (n == 2)
                    control.corr_mode = MBPM_CORRECTION_RANGE;
                }

            /* correction-standoff */
            else if (strcmp("correction-standoff", options[option_index].name) == 0)
                {
                int n = sscanf (optarg,"%lf/%lf", &control.corr_standoff_target, &control.corr_standoff_coeff);
                if (n == 2)
                    control.corr_mode = MBPM_CORRECTION_STANDOFF;
                }

            /* correction-file */
            else if (strcmp("correction-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageCorrectionFile);
                if (n == 1)
                    {
                    control.corr_mode = MBPM_CORRECTION_FILE;
                    if (strlen(ImageCorrectionFile) < 5
                        || strncmp(".yml", &ImageCorrectionFile[strlen(ImageCorrectionFile)-4], 4) != 0)
                        {
                        strcat(ImageCorrectionFile, ".yml");
                        }
                    }
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

            /* calibration-file */
            else if (strcmp("calibration-file", options[option_index].name) == 0)
                {
                strcpy(StereoCameraCalibrationFile, optarg);
                control.calibration_set = true;
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
                    use_tide = true;
                }

            /* image-quality-file */
            else if (strcmp("image-quality-file", options[option_index].name) == 0)
                {
                const int n = sscanf (optarg,"%s", ImageQualityFile);
                if (n == 1)
                    ImageQualityFile_specified = true;
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

    /* check output name and format - set format and suffix if necessary */
    if (outputimage_specified) {
        if (output_format == MBPM_FORMAT_NONE) {
            output_format = MBPM_FORMAT_PNG;
        }
        if (output_format == MBPM_FORMAT_TIFF
            && (strlen(OutputImageFile) < 5
                || (strncmp(".tif", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0
                    && strncmp(".tiff", &OutputImageFile[strlen(OutputImageFile)-5], 5) != 0
                    && strncmp(".TIF", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0
                    && strncmp(".TIFF", &OutputImageFile[strlen(OutputImageFile)-5], 5) != 0))) {
            strcat(OutputImageFile, ".tif");
        }
        else if (output_format == MBPM_FORMAT_PNG
            && (strlen(OutputImageFile) < 5
                || (strncmp(".png", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0
                    && strncmp(".PNG", &OutputImageFile[strlen(OutputImageFile)-4], 4) != 0))) {
            strcat(OutputImageFile, ".png");
        }
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
        fprintf(stream,"dbg2       verbose:                       %d\n",verbose);
        fprintf(stream,"dbg2       help:                          %d\n",help);
        fprintf(stream,"dbg2       numThreads:                    %d\n",numThreads);
        fprintf(stream,"dbg2       ImageListFile:                 %s\n",ImageListFile);
        fprintf(stream,"dbg2       use_camera_mode:               %d\n",use_camera_mode);
        fprintf(stream,"dbg2       outputimage_specified:         %d\n",outputimage_specified);
        fprintf(stream,"dbg2       OutputImageFile:               %s\n",OutputImageFile);
        fprintf(stream,"dbg2       output_format:                 %d\n",output_format);
        fprintf(stream,"dbg2       bounds_specified:              %d\n",bounds_specified);
        fprintf(stream,"dbg2       Bounds: west:                  %f\n",control.OutputBounds[0]);
        fprintf(stream,"dbg2       Bounds: east:                  %f\n",control.OutputBounds[1]);
        fprintf(stream,"dbg2       Bounds: south:                 %f\n",control.OutputBounds[2]);
        fprintf(stream,"dbg2       Bounds: north:                 %f\n",control.OutputBounds[3]);
        fprintf(stream,"dbg2       Bounds buffer:                 %f\n",bounds_buffer);
        fprintf(stream,"dbg2       set_spacing:                   %d\n",set_spacing);
        fprintf(stream,"dbg2       spacing_priority:              %d\n",spacing_priority);
        fprintf(stream,"dbg2       dx_set:                        %f\n",dx_set);
        fprintf(stream,"dbg2       dy_set:                        %f\n",dy_set);
        fprintf(stream,"dbg2       set_dimensions:                %d\n",set_dimensions);
        fprintf(stream,"dbg2       control.OutputDim[0]:          %d\n",control.OutputDim[0]);
        fprintf(stream,"dbg2       control.OutputDim[1]:          %d\n",control.OutputDim[1]);
        fprintf(stream,"dbg2       control.use_projection:        %d\n",control.use_projection);
        if (control.use_projection)
            fprintf(stream,"dbg2       projection_pars:               %s\n",projection_pars);
        fprintf(stream,"dbg2       navigation_specified:          %d\n",navigation_specified);
        fprintf(stream,"dbg2       NavigationFile:                %s\n",NavigationFile);
        fprintf(stream,"dbg2       use_tide:                      %d\n",use_tide);
        fprintf(stream,"dbg2       TideFile:                      %s\n",TideFile);
        fprintf(stream,"dbg2       ImageQualityFile_specified:    %d\n",ImageQualityFile_specified);
        fprintf(stream,"dbg2       ImageQualityFile:              %s\n",ImageQualityFile);
        fprintf(stream,"dbg2       use_imagequality:              %d\n",use_imagequality);
        fprintf(stream,"dbg2       imageQualityThreshold:         %f\n",imageQualityThreshold);
        fprintf(stream,"dbg2       imageQualityFilterLength:      %f\n",imageQualityFilterLength);
        fprintf(stream,"dbg2       control.use_topography:        %d\n",control.use_topography);
        fprintf(stream,"dbg2       TopographyGridFile:            %s\n",TopographyGridFile);
        fprintf(stream,"dbg2       control.calibration_set:       %d\n",control.calibration_set);
        fprintf(stream,"dbg2       StereoCameraCalibrationFile:   %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"dbg2       control.fov_fudgefactor:       %f\n",control.fov_fudgefactor);
        if (control.corr_mode == MBPM_CORRECTION_BRIGHTNESS) {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_BRIGHTNESS\n",control.corr_mode);
        }
        else if (control.corr_mode == MBPM_CORRECTION_CAMERA_SETTINGS) {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_CAMERA_SETTINGS\n",control.corr_mode);
        }
        else if (control.corr_mode == MBPM_CORRECTION_RANGE) {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_RANGE\n",control.corr_mode);
            fprintf(stream,"dbg2       control.corr_range_target:     %f\n",control.corr_range_target);
            fprintf(stream,"dbg2       control.corr_range_coeff:      %f\n",control.corr_range_coeff);
        }
        else if (control.corr_mode == MBPM_CORRECTION_STANDOFF) {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_STANDOFF\n",control.corr_mode);
            fprintf(stream,"dbg2       control.corr_standoff_target:  %f\n",control.corr_standoff_target);
            fprintf(stream,"dbg2       control.corr_standoff_coeff:   %f\n",control.corr_standoff_coeff);
        }
        else if (control.corr_mode == MBPM_CORRECTION_FILE) {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_FILE\n",control.corr_mode);
            fprintf(stream,"dbg2       ImageCorrectionFile:           %s\n",ImageCorrectionFile);
        }
        else {
            fprintf(stream,"dbg2       control.corr_mode:             %d MBPM_CORRECTION_NONE\n",control.corr_mode);
        }
        fprintf(stream,"dbg2       control.reference_gain:        %f\n",control.reference_gain);
        fprintf(stream,"dbg2       control.reference_exposure:    %f\n",control.reference_exposure);
        fprintf(stream,"dbg2       PlatformFile:                  %s\n",PlatformFile);
        fprintf(stream,"dbg2       platform_specified:            %d\n",platform_specified);
        fprintf(stream,"dbg2       camera_sensor:                 %d\n",camera_sensor);
        fprintf(stream,"dbg2       nav_sensor:                    %d\n",nav_sensor);
        fprintf(stream,"dbg2       sensordepth_sensor:            %d\n",sensordepth_sensor);
        fprintf(stream,"dbg2       heading_sensor:                %d\n",heading_sensor);
        fprintf(stream,"dbg2       altitude_sensor:               %d\n",altitude_sensor);
        fprintf(stream,"dbg2       attitude_sensor:               %d\n",attitude_sensor);
        if (control.priority_mode == MBPM_PRIORITY_CENTRALITY_ONLY)
            fprintf(stream,"dbg2       control.priority_mode:         %d (priority by centrality in source image only)\n",control.priority_mode);
        else
            {
            fprintf(stream,"dbg2       control.priority_mode:         %d (priority by centrality in source image and difference from target standoff)\n",control.priority_mode);
            fprintf(stream,"dbg2       control.standoff_target:       %f\n",control.standoff_target);
            fprintf(stream,"dbg2       control.standoff_range:        %f\n",control.standoff_range);
            }
        fprintf(stream,"dbg2       control.range_max:             %f\n",control.range_max);
        fprintf(stream,"dbg2       control.trimPixels:            %u\n",control.trimPixels);
        fprintf(stream,"dbg2       control.sectionPixels:         %u\n",control.sectionPixels);
        }
    else if (verbose == 1)
        {
        fprintf(stream,"\nProgram <%s>\n",program_name);
        fprintf(stream,"Control Parameters:\n");
        fprintf(stream,"  numThreads:                    %d\n",numThreads);
        fprintf(stream,"  ImageListFile:                 %s\n",ImageListFile);
        fprintf(stream,"  use_camera_mode:               %d\n",use_camera_mode);
        fprintf(stream,"  outputimage_specified:         %d\n",outputimage_specified);
        fprintf(stream,"  OutputImageFile:               %s\n",OutputImageFile);
        fprintf(stream,"  output_format:                 %d\n",output_format);
        fprintf(stream,"  bounds_specified:              %d\n",bounds_specified);
        fprintf(stream,"  Bounds: west:                  %f\n",control.OutputBounds[0]);
        fprintf(stream,"  Bounds: east:                  %f\n",control.OutputBounds[1]);
        fprintf(stream,"  Bounds: south:                 %f\n",control.OutputBounds[2]);
        fprintf(stream,"  Bounds: north:                 %f\n",control.OutputBounds[3]);
        fprintf(stream,"  Bounds buffer:                 %f\n",bounds_buffer);
        fprintf(stream,"  set_spacing:                   %d\n",set_spacing);
        fprintf(stream,"  spacing_priority:              %d\n",spacing_priority);
        fprintf(stream,"  dx_set:                        %f\n",dx_set);
        fprintf(stream,"  dy_set:                        %f\n",dy_set);
        fprintf(stream,"  set_dimensions:                %d\n",set_dimensions);
        fprintf(stream,"  control.OutputDim[0]:          %d\n",control.OutputDim[0]);
        fprintf(stream,"  control.OutputDim[1]:          %d\n",control.OutputDim[1]);
        fprintf(stream,"  control.use_projection:        %d\n",control.use_projection);
        if (control.use_projection)
            fprintf(stream,"  projection_pars:               %s\n",projection_pars);
        fprintf(stream,"  navigation_specified:          %d\n",navigation_specified);
        fprintf(stream,"  NavigationFile:                %s\n",NavigationFile);
        fprintf(stream,"  use_tide:                      %d\n",use_tide);
        fprintf(stream,"  TideFile:                      %s\n",TideFile);
        fprintf(stream,"  ImageQualityFile_specified:    %d\n",ImageQualityFile_specified);
        fprintf(stream,"  ImageQualityFile:              %s\n",ImageQualityFile);
        fprintf(stream,"  use_imagequality:              %d\n",use_imagequality);
        fprintf(stream,"  imageQualityThreshold:         %f\n",imageQualityThreshold);
        fprintf(stream,"  imageQualityFilterLength:      %f\n",imageQualityFilterLength);
        fprintf(stream,"  control.use_topography:        %d\n",control.use_topography);
        fprintf(stream,"  TopographyGridFile:            %s\n",TopographyGridFile);
        fprintf(stream,"  control.calibration_set:       %d\n",control.calibration_set);
        fprintf(stream,"  StereoCameraCalibrationFile:   %s\n",StereoCameraCalibrationFile);
        fprintf(stream,"  control.fov_fudgefactor:       %f\n",control.fov_fudgefactor);
        if (control.corr_mode == MBPM_CORRECTION_BRIGHTNESS) {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_BRIGHTNESS\n",control.corr_mode);
        }
        else if (control.corr_mode == MBPM_CORRECTION_CAMERA_SETTINGS) {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_CAMERA_SETTINGS\n",control.corr_mode);
        }
        else if (control.corr_mode == MBPM_CORRECTION_RANGE) {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_RANGE\n",control.corr_mode);
            fprintf(stream,"  control.corr_range_target:     %f\n",control.corr_range_target);
            fprintf(stream,"  control.corr_range_coeff:      %f\n",control.corr_range_coeff);
        }
        else if (control.corr_mode == MBPM_CORRECTION_STANDOFF) {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_STANDOFF\n",control.corr_mode);
            fprintf(stream,"  control.corr_standoff_target:  %f\n",control.corr_standoff_target);
            fprintf(stream,"  control.corr_standoff_coeff:   %f\n",control.corr_standoff_coeff);
        }
        else if (control.corr_mode == MBPM_CORRECTION_FILE) {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_FILE\n",control.corr_mode);
            fprintf(stream,"  ImageCorrectionFile:           %s\n",ImageCorrectionFile);
        }
        else {
            fprintf(stream,"  control.corr_mode:             %d MBPM_CORRECTION_NONE\n",control.corr_mode);
        }
        fprintf(stream,"  control.reference_gain:        %f\n",control.reference_gain);
        fprintf(stream,"  control.reference_exposure:    %f\n",control.reference_exposure);
        fprintf(stream,"  PlatformFile:                  %s\n",PlatformFile);
        fprintf(stream,"  platform_specified:            %d\n",platform_specified);
        fprintf(stream,"  camera_sensor:                 %d\n",camera_sensor);
        fprintf(stream,"  nav_sensor:                    %d\n",nav_sensor);
        fprintf(stream,"  sensordepth_sensor:            %d\n",sensordepth_sensor);
        fprintf(stream,"  heading_sensor:                %d\n",heading_sensor);
        fprintf(stream,"  altitude_sensor:               %d\n",altitude_sensor);
        fprintf(stream,"  attitude_sensor:               %d\n",attitude_sensor);
        if (control.priority_mode == MBPM_PRIORITY_CENTRALITY_ONLY)
            fprintf(stream,"  control.priority_mode:         %d (priority by centrality in source image only)\n",control.priority_mode);
        else
            {
            fprintf(stream,"  control.priority_mode:         %d (priority by centrality in source image and difference from target standoff)\n",control.priority_mode);
            fprintf(stream,"  control.standoff_target:       %f\n",control.standoff_target);
            fprintf(stream,"  control.standoff_range:        %f\n",control.standoff_range);
            }
        fprintf(stream,"  control.range_max:             %f\n",control.range_max);
        fprintf(stream,"  control.trimPixels:            %u\n",control.trimPixels);
        fprintf(stream,"  control.sectionPixels:         %u\n",control.sectionPixels);
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

        /* if photomosaic bounds not specified use grid bounds */
        if (bounds_specified == MB_NO)
            {
            mb_topogrid_bounds(verbose, control.topogrid_ptr, control.OutputBounds, &error);
            }
        }

    /* if bounds not specified then quit */
    if (control.OutputBounds[0] >= control.OutputBounds[1] || control.OutputBounds[2] >= control.OutputBounds[3])
        {
        fprintf(stream,"\nGrid bounds not properly specified:\n\t%f %f %f %f\n",
        control.OutputBounds[0],control.OutputBounds[1],control.OutputBounds[2],control.OutputBounds[3]);
        fprintf(stream,"\nProgram <%s> Terminated\n",
            program_name);
        error = MB_ERROR_BAD_PARAMETER;
        exit(error);
        }

    /* if stereo calibration not specified then quit */
    if (!control.calibration_set)
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
    if (control.calibration_set)
        {
        fstorage.open(StereoCameraCalibrationFile, FileStorage::READ);
        if(fstorage.isOpened() )
            {
            fstorage["M1"] >> control.cameraMatrix[0];
            fstorage["D1"] >> control.distCoeffs[0];
            fstorage["M2"] >> control.cameraMatrix[1];
            fstorage["D2"] >> control.distCoeffs[1];
            fstorage["R"] >> control.R;
            fstorage["T"] >> control.T;
            fstorage["R1"] >> control.R1;
            fstorage["R2"] >> control.R2;
            fstorage["P1"] >> control.P1;
            fstorage["P2"] >> control.P2;
            fstorage["Q"] >> control.Q;
            fstorage.release();
            control.isVerticalStereo = fabs(control.P2.at<double>(1, 3)) > fabs(control.P2.at<double>(0, 3));
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
            cerr << "M1:" << endl << control.cameraMatrix[0] << endl << endl;
            cerr << "D1:" << endl << control.distCoeffs[0] << endl << endl;
            cerr << "M2:" << endl << control.cameraMatrix[1] << endl << endl;
            cerr << "D2:" << endl << control.distCoeffs[1] << endl << endl;
            cerr << "R:" << endl << control.R << endl << endl;
            cerr << "T:" << endl << control.T << endl << endl;
            cerr << "R1:" << endl << control.R1 << endl << endl;
            cerr << "R2:" << endl << control.R2 << endl << endl;
            cerr << "P1:" << endl << control.P1 << endl << endl;
            cerr << "P2:" << endl << control.P2 << endl << endl;
            cerr << "Q:" << endl << control.Q << endl << endl;
            }
        }

    /* image correction table */
    if (control.corr_mode == MBPM_CORRECTION_FILE) {
        /* read in the image correction table - this is expected to be a stereo correction table */
        fstorage.open(ImageCorrectionFile, FileStorage::READ);
        if(fstorage.isOpened() )
            {
            fstorage["ImageCorrectionBounds"] >> control.corr_bounds;
            fstorage["ImageCorrectionTable1"] >> control.corr_table[0];
            fstorage["ImageCorrectionTable2"] >> control.corr_table[1];
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
        control.ncorr_x = control.corr_table[0].size[0];
        control.ncorr_y = control.corr_table[0].size[1];
        control.ncorr_z = control.corr_table[0].size[2];
        control.corr_xmin = control.corr_bounds.at<float>(0, 0);
        control.corr_xmax = control.corr_bounds.at<float>(0, 1);
        control.bin_dx = control.corr_bounds.at<float>(0, 2);
        control.corr_ymin = control.corr_bounds.at<float>(1, 0);
        control.corr_ymax = control.corr_bounds.at<float>(1, 1);
        control.bin_dy = control.corr_bounds.at<float>(1, 2);
        control.corr_zmin = control.corr_bounds.at<float>(2, 0);
        control.corr_zmax = control.corr_bounds.at<float>(2, 1);
        control.bin_dz = control.corr_bounds.at<float>(2, 2);
//fprintf(stderr, "\nImage correction:\n");
//fprintf(stderr, "x: %d %f %f %f\n", control.ncorr_x, control.corr_xmin, control.corr_xmax, control.bin_dx);
//fprintf(stderr, "y: %d %f %f %f\n", control.ncorr_y, control.corr_ymin, control.corr_ymax, control.bin_dy);
//fprintf(stderr, "z: %d %f %f %f\n", control.ncorr_z, control.corr_zmin, control.corr_zmax, control.bin_dz);

        /* Get reference intensity value for each camera.
         * If the target standoff value is specified use the correction table value
         * at the center x and y and the target standoff z. If no target standoff
         * value is specified use the correction table value at the x y center
         * which is located halfway between the first and last nonzero values */
        control.ibin_xcen = control.ncorr_x / 2;
        control.jbin_ycen = control.ncorr_y / 2;
        control.kbin_zcen = control.ncorr_z / 2;
        for (int icamera = 0; icamera < 2; icamera++) {
            if (control.priority_mode == MBPM_PRIORITY_CENTRALITY_PLUS_STANDOFF
                && control.standoff_target > control.corr_zmin && control.standoff_target < control.corr_zmax) {
                control.kbin_zcen = (control.standoff_target - control.corr_zmin) / control.bin_dz;
            } else {
                control.kbin_zcen = control.ncorr_z / 2;
                int k0 = control.ncorr_z;
                int k1 = -1;
                for (int k=0;k<control.ncorr_z;k++) {
                    if (control.corr_table[icamera].at<float>(control.ibin_xcen, control.jbin_ycen, k) > 0.0) {
                        if (k0 > k)
                            k0 = k;
                        k1 = k;
                    }

                    /* reset k0 because there is an isolated nonzero value at k == 0 */
                    else if (k == 1 && k0 == 0) {
                      k0 = control.ncorr_z;
                      k1 = -1;
                    }
                }
                if (k1 >= k0) {
                  control.kbin_zcen = (k0 + k1) / 2;
                }
            }
            control.referenceIntensity[icamera] =  control.corr_table[icamera].at<float>(control.ibin_xcen, control.jbin_ycen, control.kbin_zcen);

//fprintf(stderr, "\nImage correction camera: %d\n", icamera);
//fprintf(stderr, "center: %d %d %d\n", control.ibin_xcen, control.jbin_ycen, control.kbin_zcen);
//fprintf(stderr, "control.referenceIntensity[%d]: %f\n", icamera, control.referenceIntensity[icamera]);
//fprintf(stderr, "\nCorrection Table[%d]:\n", icamera);
//for (int i=0;i<control.ncorr_x;i++) {
//for (int j=0;j<control.ncorr_y;j++) {
//for (int k=0;k<control.ncorr_z;k++) {
//fprintf(stderr,"    %d %d %d   %f\n", i, j, k, control.corr_table[icamera].at<float>(i, j, k));
//}
//}
//}

        }
    }

    /* else the reference intensity is 70.0 */
    else {
        control.referenceIntensity[0] = 1.0;
        control.referenceIntensity[1] = 1.0;
    }

    /* deal with projected gridding */
    double deglontokm, deglattokm;
    if (control.use_projection)
        {
        /* check for UTM with undefined zone */
        if (strcmp(projection_pars, "UTM") == 0
            || strcmp(projection_pars, "U") == 0
            || strcmp(projection_pars, "utm") == 0
            || strcmp(projection_pars, "u") == 0)
            {
            reference_lon = 0.5 * (control.OutputBounds[0] + control.OutputBounds[1]);
            if (reference_lon < 180.0)
                reference_lon += 360.0;
            if (reference_lon >= 180.0)
                reference_lon -= 360.0;
            utm_zone = (int)(((reference_lon + 183.0)
                / 6.0) + 0.5);
            reference_lat = 0.5 * (control.OutputBounds[2] + control.OutputBounds[3]);
            if (reference_lat >= 0.0)
                sprintf(projection_id, "UTM%2.2dN", utm_zone);
            else
                sprintf(projection_id, "UTM%2.2dS", utm_zone);
            }
        else
            strcpy(projection_id, projection_pars);

        /* set projection flag */
        int proj_status = mb_proj_init(verbose, projection_id, &(control.pjptr), &error);

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
        if (control.OutputBounds[0] < -360.0 || control.OutputBounds[0] > 360.0
            || control.OutputBounds[1] < -360.0 || control.OutputBounds[1] > 360.0
            || control.OutputBounds[2] < -90.0 || control.OutputBounds[2] > 90.0
            || control.OutputBounds[3] < -90.0 || control.OutputBounds[3] > 90.0)
            {
            /* first point */
            double xx = control.OutputBounds[0];
            double yy = control.OutputBounds[2];
            double xlon, ylat;
            mb_proj_inverse(verbose, control.pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = xlon;
            pbounds[1] = xlon;
            pbounds[2] = ylat;
            pbounds[3] = ylat;

            /* second point */
            xx = control.OutputBounds[1];
            yy = control.OutputBounds[2];
            mb_proj_inverse(verbose, control.pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = MIN(pbounds[0], xlon);
            pbounds[1] = MAX(pbounds[1], xlon);
            pbounds[2] = MIN(pbounds[2], ylat);
            pbounds[3] = MAX(pbounds[3], ylat);

            /* third point */
            xx = control.OutputBounds[0];
            yy = control.OutputBounds[3];
            mb_proj_inverse(verbose, control.pjptr, xx, yy,
                    &xlon, &ylat, &error);
            mb_apply_lonflip(verbose, lonflip, &xlon);
            pbounds[0] = MIN(pbounds[0], xlon);
            pbounds[1] = MAX(pbounds[1], xlon);
            pbounds[2] = MIN(pbounds[2], ylat);
            pbounds[3] = MAX(pbounds[3], ylat);

            /* fourth point */
            xx = control.OutputBounds[1];
            yy = control.OutputBounds[3];
            mb_proj_inverse(verbose, control.pjptr, xx, yy,
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
            pbounds[0] = control.OutputBounds[0];
            pbounds[1] = control.OutputBounds[1];
            pbounds[2] = control.OutputBounds[2];
            pbounds[3] = control.OutputBounds[3];

            /* first point */
            double xx, yy;
            double xlon = pbounds[0];
            double ylat = pbounds[2];
            mb_proj_forward(verbose, control.pjptr, xlon, ylat,
                    &xx, &yy, &error);
            control.OutputBounds[0] = xx;
            control.OutputBounds[1] = xx;
            control.OutputBounds[2] = yy;
            control.OutputBounds[3] = yy;

            /* second point */
            xlon = pbounds[1];
            ylat = pbounds[2];
            mb_proj_forward(verbose, control.pjptr, xlon, ylat,
                    &xx, &yy, &error);
            control.OutputBounds[0] = MIN(control.OutputBounds[0], xx);
            control.OutputBounds[1] = MAX(control.OutputBounds[1], xx);
            control.OutputBounds[2] = MIN(control.OutputBounds[2], yy);
            control.OutputBounds[3] = MAX(control.OutputBounds[3], yy);

            /* third point */
            xlon = pbounds[0];
            ylat = pbounds[3];
            mb_proj_forward(verbose, control.pjptr, xlon, ylat,
                    &xx, &yy, &error);
            control.OutputBounds[0] = MIN(control.OutputBounds[0], xx);
            control.OutputBounds[1] = MAX(control.OutputBounds[1], xx);
            control.OutputBounds[2] = MIN(control.OutputBounds[2], yy);
            control.OutputBounds[3] = MAX(control.OutputBounds[3], yy);

            /* fourth point */
            xlon = pbounds[1];
            ylat = pbounds[3];
            mb_proj_forward(verbose, control.pjptr, xlon, ylat,
                    &xx, &yy, &error);
            control.OutputBounds[0] = MIN(control.OutputBounds[0], xx);
            control.OutputBounds[1] = MAX(control.OutputBounds[1], xx);
            control.OutputBounds[2] = MIN(control.OutputBounds[2], yy);
            control.OutputBounds[3] = MAX(control.OutputBounds[3], yy);
            }

        /* calculate grid properties */
        if (set_spacing == MB_YES)
            {
            control.OutputDim[0] = (control.OutputBounds[1] - control.OutputBounds[0])/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = dx_set;
            control.OutputDim[1] = (control.OutputBounds[3] - control.OutputBounds[2])/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                control.OutputBounds[1] = control.OutputBounds[0] + dx_set * (control.OutputDim[0] - 1);
                control.OutputBounds[3] = control.OutputBounds[2] + dy_set * (control.OutputDim[1] - 1);
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

        mb_coor_scale(verbose,0.5*(pbounds[2]+pbounds[3]),&control.mtodeglon,&control.mtodeglat);

/* fprintf(stream," Projected coordinates on: proj_status:%d  projection:%s\n",
proj_status, projection_id);
fprintf(stream," Lon Lat Bounds: %f %f %f %f\n",
pbounds[0], pbounds[1], pbounds[2], pbounds[3]);
fprintf(stream," XY Bounds: %f %f %f %f\n",
control.OutputBounds[0], control.OutputBounds[1], control.OutputBounds[2], control.OutputBounds[3]);*/
        }

    /* deal with no projection */
    else
        {
        /* calculate grid properties */
        mb_coor_scale(verbose,0.5*(control.OutputBounds[2]+control.OutputBounds[3]),&control.mtodeglon,&control.mtodeglat);
        deglontokm = 0.001/control.mtodeglon;
        deglattokm = 0.001/control.mtodeglat;
        if (set_spacing == MB_YES
            && (units[0] == 'M' || units[0] == 'm'))
            {
            control.OutputDim[0] = (control.OutputBounds[1] - control.OutputBounds[0])/(control.mtodeglon*dx_set) + 1;
            if (dy_set <= 0.0)
                dy_set = control.mtodeglon * dx_set / control.mtodeglat;
            control.OutputDim[1] = (control.OutputBounds[3] - control.OutputBounds[2])/(control.mtodeglat*dy_set) + 1;
            if (spacing_priority == MB_YES)
                {
                control.OutputBounds[1] = control.OutputBounds[0] + control.mtodeglon * dx_set * (control.OutputDim[0] - 1);
                control.OutputBounds[3] = control.OutputBounds[2] + control.mtodeglat * dy_set * (control.OutputDim[1] - 1);
                }
            strcpy(units, "meters");
            }
        else if (set_spacing == MB_YES
            && (units[0] == 'K' || units[0] == 'k'))
            {
            control.OutputDim[0] = (control.OutputBounds[1] - control.OutputBounds[0])*deglontokm/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = deglattokm * dx_set / deglontokm;
            control.OutputDim[1] = (control.OutputBounds[3] - control.OutputBounds[2])*deglattokm/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                control.OutputBounds[1] = control.OutputBounds[0] + dx_set * (control.OutputDim[0] - 1) / deglontokm;
                control.OutputBounds[3] = control.OutputBounds[2] + dy_set * (control.OutputDim[1] - 1) / deglattokm;
                }
            strcpy(units, "km");
            }
        else if (set_spacing == MB_YES
            && (units[0] == 'F' || units[0] == 'f'))
            {
            control.OutputDim[0] = (control.OutputBounds[1] - control.OutputBounds[0])/(control.mtodeglon*0.3048*dx_set) + 1;
            if (dy_set <= 0.0)
                dy_set = control.mtodeglon * dx_set / control.mtodeglat;
            control.OutputDim[1] = (control.OutputBounds[3] - control.OutputBounds[2])/(control.mtodeglat*0.3048*dy_set) + 1;
            if (spacing_priority == MB_YES)
                {
                control.OutputBounds[1] = control.OutputBounds[0] + control.mtodeglon * 0.3048 * dx_set * (control.OutputDim[0] - 1);
                control.OutputBounds[3] = control.OutputBounds[2] + control.mtodeglat * 0.3048 * dy_set * (control.OutputDim[1] - 1);
                }
            strcpy(units, "feet");
            }
        else if (set_spacing == MB_YES)
            {
            control.OutputDim[0] = (control.OutputBounds[1] - control.OutputBounds[0])/dx_set + 1;
            if (dy_set <= 0.0)
                dy_set = dx_set;
            control.OutputDim[1] = (control.OutputBounds[3] - control.OutputBounds[2])/dy_set + 1;
            if (spacing_priority == MB_YES)
                {
                control.OutputBounds[1] = control.OutputBounds[0] + dx_set * (control.OutputDim[0] - 1);
                control.OutputBounds[3] = control.OutputBounds[2] + dy_set * (control.OutputDim[1] - 1);
                }
            strcpy(units, "degrees");
            }

        /* copy bounds to pbounds */
        pbounds[0] = control.OutputBounds[0];
        pbounds[1] = control.OutputBounds[1];
        pbounds[2] = control.OutputBounds[2];
        pbounds[3] = control.OutputBounds[3];
        }

    /* calculate other grid properties */
    control.OutputDx[0] = (control.OutputBounds[1] - control.OutputBounds[0])/(control.OutputDim[0]-1);
    control.OutputDx[1] = (control.OutputBounds[3] - control.OutputBounds[2])/(control.OutputDim[1]-1);

    /* pbounds is used to check for images of interest using navigation
      - expand it a little to account for image size */
    pbounds[0] -= control.mtodeglon * bounds_buffer;
    pbounds[1] += control.mtodeglon * bounds_buffer;
    pbounds[2] -= control.mtodeglat * bounds_buffer;
    pbounds[3] += control.mtodeglat * bounds_buffer;

    /* output information */
    if (verbose >= 1)
        {
        fprintf(stdout,"\nOutput Image Parameters:\n");
        if (outputimage_specified)
            fprintf(stream,"  OutputImageFile:    %s\n",OutputImageFile);
        else
            fprintf(stream,"  No image output - listing images that would be used for the defined area\n");
        if (control.use_projection)
            fprintf(stream,"  projection:         %s\n",projection_id);
        else
            fprintf(stream,"  projection:         Geographic\n");
        fprintf(stream,"  control.OutputBounds[0]: west:       %.9f\n",control.OutputBounds[0]);
        fprintf(stream,"  control.OutputBounds[1]: east:       %.9f\n",control.OutputBounds[1]);
        fprintf(stream,"  control.OutputBounds[2]: south:      %.9f\n",control.OutputBounds[2]);
        fprintf(stream,"  control.OutputBounds[3]: north:      %.9f\n",control.OutputBounds[3]);
        fprintf(stream,"  control.OutputDx[0]: dx:             %.9f\n",control.OutputDx[0]);
        fprintf(stream,"  control.OutputDx[1]: dy:             %.9f\n",control.OutputDx[1]);
        fprintf(stream,"  control.OutputDim[0]: xdim:          %d\n",control.OutputDim[0]);
        fprintf(stream,"  control.OutputDim[1]: ydim:          %d\n",control.OutputDim[1]);
        }

    /* If output file specified then create an an output image and priority map
        in the processing structure for each processing thread. The images will
        be combined after all processing is complete. */
    if (outputimage_specified) {
        for (int ithread = 0; ithread < numThreads; ithread++) {
            processPars[ithread].OutputImage.create(control.OutputDim[1], control.OutputDim[0], CV_8UC3);
            processPars[ithread].OutputPriority.create(control.OutputDim[1], control.OutputDim[0], CV_32FC1);
#ifdef DEBUG
            processPars[ithread].OutputIntensityCorrection.create(control.OutputDim[1], control.OutputDim[0], CV_32FC1);
            processPars[ithread].OutputStandoff.create(control.OutputDim[1], control.OutputDim[0], CV_32FC1);
#endif
        }
    }

    /* read in navigation if desired */
    if (navigation_specified)
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
    if (use_tide)
        {
        if ((tfp = fopen(TideFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to open tide file <%s> for reading\n",TideFile);
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
            fprintf(stderr,"\nUnable to open tide file <%s> for reading\n",TideFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            bool value_ok = false;

            /* read the tide values */
            int nget = sscanf(buffer,"%lf %lf", &ttime[ntide], &ttide[ntide]);
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

    /* read in image quality if desired */
    if (ImageQualityFile_specified)
        {
        if ((tfp = fopen(ImageQualityFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to open image quality file <%s> for reading\n",ImageQualityFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        nquality = 0;
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            nquality++;
        fclose(tfp);

        /* allocate arrays for quality */
        if (nquality > 1)
            {
            status = mb_mallocd(verbose,__FILE__,__LINE__,nquality*sizeof(double),(void **)&qtime,&error);
            status = mb_mallocd(verbose,__FILE__,__LINE__,nquality*sizeof(double),(void **)&qquality,&error);

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

        /* if no image quality data then quit */
        else
            {
            fclose(tfp);
            error = MB_ERROR_BAD_DATA;
            fprintf(stderr,"\nUnable to read data from image quality file <%s>\n",ImageQualityFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }

        /* read the data points in the image quality file */
        nquality = 0;
        if ((tfp = fopen(ImageQualityFile, "r")) == NULL)
            {
            error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr,"\nUnable to open image quality file <%s> for reading\n",ImageQualityFile);
            fprintf(stderr,"\nProgram <%s> Terminated\n",
                program_name);
            exit(error);
            }
        while ((result = fgets(buffer,MB_PATH_MAXLINE,tfp)) == buffer)
            {
            bool value_ok = false;

            /* read the image quality values */
            int nget = sscanf(buffer,"%lf %lf",
                &qtime[nquality], &qquality[nquality]);
            if (nget >= 2)
                value_ok = true;

            /* output some debug values */
            if (verbose >= 5 && value_ok)
                {
                fprintf(stderr,"\ndbg5  New image quality point read in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       quality[%d]: %f %f\n",
                    nquality,qtime[nquality],qquality[nquality]);
                }
            else if (verbose >= 5)
                {
                fprintf(stderr,"\ndbg5  Error parsing line in image quality file in program <%s>\n",program_name);
                fprintf(stderr,"dbg5       line: %s\n",buffer);
                }

            /* check for reverses or repeats in time */
            if (value_ok)
                {
                if (nquality == 0)
                    nquality++;
                else if (qtime[nquality] > qtime[nquality-1])
                    nquality++;
                else if (nquality > 0 && qtime[nquality] <= qtime[nquality-1]
                    && verbose >= 5)
                    {
                    fprintf(stderr,"\ndbg5  Tide time error in program <%s>\n",program_name);
                    fprintf(stderr,"dbg5       quality[%d]: %f %f\n",
                        nquality-1,qtime[nquality-1],qquality[nquality-1]);
                    fprintf(stderr,"dbg5       quality[%d]: %f %f\n",
                        nquality,qtime[nquality],qquality[nquality]);
                    }
                }
            strncpy(buffer,"\0",sizeof(buffer));
            }
        fclose(tfp);
        if (nquality > 0) {
            use_imagequality = true;
            if (imageQualityFilterLength > 0.0)
                mb_apply_time_filter(verbose, nquality, qtime, qquality, imageQualityFilterLength, &error);
        }

        /* output information */
        if (verbose >= 1)
            {
            fprintf(stdout,"\nImage Quality Parameters:\n");
            fprintf(stream,"  ImageQualityFile:  %s\n",ImageQualityFile);
            fprintf(stream,"  nquality:          %d\n",nquality);
            }
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
            double time_d;
            double camera_navlon;
            double camera_navlat;
            double camera_sensordepth;
            double camera_heading;
            double camera_roll;
            double camera_pitch;
            double image_gain;
            double image_exposure;

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
            if (use_this_image && use_imagequality) {
                if (nquality > 1) {
                    intstat = mb_linear_interp(verbose, qtime-1, qquality-1, nquality,
                                                time_d, &image_quality, &iqtime, &error);
                }
                if (image_quality < imageQualityThreshold) {
                    use_this_image = false;
                }
            }

            /* check navigation for location close to or inside destination image bounds */
            if (use_this_image) {
                if (nnav > 0 && time_d >= ntime[0] && time_d <= ntime[nnav-1]) {
                    /* get navigation for this image */
                    intstat = mb_linear_interp_longitude(verbose,
                            ntime-1, nlon-1,
                            nnav, time_d, &navlon, &intime,
                            &error);
                    intstat = mb_linear_interp_latitude(verbose,
                            ntime-1, nlat-1,
                            nnav, time_d, &navlat, &intime,
                            &error);
                    if (navlon < pbounds[0] || navlon > pbounds[1]
                        || navlat < pbounds[2] || navlat > pbounds[3]) {
                        use_this_image = false;
                    }
                }
                else {
                    use_this_image = false;
                }
            }

            /* calculate camera pose */
            if (use_this_image) {
                intstat = mb_linear_interp_heading(verbose,
                        ntime-1, nheading-1,
                        nnav, time_d, &heading, &intime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nspeed-1,
                        nnav, time_d, &speed, &intime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, ndraft-1,
                        nnav, time_d, &draft, &intime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nroll-1,
                        nnav, time_d, &roll, &intime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, npitch-1,
                        nnav, time_d, &pitch, &intime,
                        &error);
                intstat = mb_linear_interp(verbose,
                        ntime-1, nheave-1,
                        nnav, time_d, &heave, &intime,
                        &error);
                if (heading < 0.0)
                    heading += 360.0;
                else if (heading > 360.0)
                    heading -= 360.0;
                sensordepth = draft + heave;

                /* get tide for this image */
                tide = 0.0;
                if (ntide > 1) {
                    intstat = mb_linear_interp(verbose, ttime-1, ttide-1,
                            ntide, time_d, &tide, &ittime, &error);
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

            /* if no output specified but image would be used, print it out and
                reset use_this_image flag off so it is not read and processed
                - this lets us get a list of the images to be used without taking
                the time to process them */
            if (!outputimage_specified && use_this_image) {
                fprintf(stdout,"%s\n", imageFile);
                use_this_image = false;
            }

            // Start processing thread
            else if (use_this_image) {
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
                            fprintf(stderr,"  Aspect ratio:                 %f\n\n", control.aspectRatio[1]);
                        }
                        imageFirst.release();
                    }
                }

                if (control.sectionPixels > 0)
                    mbphotomosaicThreads[numThreadsSet]
                        = std::thread(process_image_sectioned2, verbose, &processPars[numThreadsSet], &control,
                                        &thread_status[numThreadsSet], &thread_error[numThreadsSet]);
                else
                    mbphotomosaicThreads[numThreadsSet]
                        = std::thread(process_image, verbose, &processPars[numThreadsSet], &control,
                                        &thread_status[numThreadsSet], &thread_error[numThreadsSet]);
                numThreadsSet++;
            }

            /* If full set of threads has been started, wait to join them all and then reset */
            if (numThreadsSet == numThreads) {
                for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
                  /* join the thread (wait until it completes) */
                  mbphotomosaicThreads[ithread].join();
                }
                numThreadsSet = 0;
            }
        }
    }

    /* If any threads has been started but not joined, wait to join them all and then reset */
    if (numThreadsSet > 0) {
        for (unsigned int ithread = 0; ithread < numThreadsSet; ithread++) {
          /* join the thread (wait until it completes) */
          mbphotomosaicThreads[ithread].join();
        }
        numThreadsSet = 0;
    }

    /* close imagelist file */
    status = mb_imagelist_close(verbose, &imagelist_ptr, &error);

    /* if more than one thread was used, combine the images into the image from
        the first thread */
    if (numThreads > 1) {
        fprintf(stderr, "\n");
        for (int ithread = 1; ithread < numThreads; ithread++) {
            fprintf(stderr, "Merging mosaic from thread %d of %d\n", ithread, numThreads);
            for (int i = 0; i < control.OutputDim[0]; i++) {
                for (int j = 0; j < control.OutputDim[1]; j++) {
                    if (processPars[ithread].OutputPriority.at<float>(j,i) > processPars[0].OutputPriority.at<float>(j,i)) {
                        processPars[0].OutputImage.at<Vec3b>(j,i)[0] = processPars[ithread].OutputImage.at<Vec3b>(j,i)[0];
                        processPars[0].OutputImage.at<Vec3b>(j,i)[1] = processPars[ithread].OutputImage.at<Vec3b>(j,i)[1];
                        processPars[0].OutputImage.at<Vec3b>(j,i)[2] = processPars[ithread].OutputImage.at<Vec3b>(j,i)[2];
                        processPars[0].OutputPriority.at<float>(j,i) = processPars[ithread].OutputPriority.at<float>(j,i);
#ifdef DEBUG
                        processPars[0].OutputIntensityCorrection.at<float>(j,i) = processPars[ithread].OutputIntensityCorrection.at<float>(j,i);
                        processPars[0].OutputStandoff.at<float>(j,i) = processPars[ithread].OutputStandoff.at<float>(j,i);
#endif
                    }
                }
            }
            processPars[ithread].OutputImage.release();
            processPars[ithread].OutputPriority.release();
#ifdef DEBUG
            processPars[ithread].OutputIntensityCorrection.release();
            processPars[ithread].OutputStandoff.release();
#endif
        }
    }

    /* Write out the ouput image */
    if (outputimage_specified) {
        /* for tiff format just write out the existing image */
        if (output_format == MBPM_FORMAT_TIFF) {
            status = imwrite(OutputImageFile, processPars[0].OutputImage);
            processPars[0].OutputImage.release();
        }

        /* for png format first add alpha channel and set black pixels to have
            alpha=0 to make those pixels transparent */
        else if (output_format == MBPM_FORMAT_PNG) {
            Mat OutputImageBGRA;
            cvtColor(processPars[0].OutputImage, OutputImageBGRA, COLOR_BGR2BGRA);
            for (int j = 0; j < OutputImageBGRA.rows; ++j) {
                for (int i = 0; i < OutputImageBGRA.cols; ++i)
                {
                    // if pixel is black set alpha to zero
                    if (OutputImageBGRA.at<Vec4b>(j, i)[0] == 0
                        && OutputImageBGRA.at<Vec4b>(j, i)[1] == 0
                        && OutputImageBGRA.at<Vec4b>(j, i)[2] == 0) {
                        OutputImageBGRA.at<Vec4b>(j, i)[3] = 0;
                    }
                }
            }
            status = imwrite(OutputImageFile, OutputImageBGRA);
            processPars[0].OutputImage.release();
            OutputImageBGRA.release();
        }

        /* write the corresponding world file */
        if (status == MB_SUCCESS)
            {
            /* open output world file */
            mb_path OutputWorldFile;
            strcpy(OutputWorldFile, OutputImageFile);
            OutputWorldFile[strlen(OutputImageFile)-4] = '\0';
            if (output_format == MBPM_FORMAT_TIFF) {
                strcat(OutputWorldFile,".tfw");
            }
            else if (output_format == MBPM_FORMAT_PNG) {
                strcat(OutputWorldFile,".pgw");
            }
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
                control.OutputDx[0], -control.OutputDx[1],
                control.OutputBounds[0] - 0.5 * control.OutputDx[0],
                control.OutputBounds[3] + 0.5 * control.OutputDx[1]);

            /* close the world file */
            fclose(tfp);

#ifdef DEBUG
            /* write out grids of priority, range, standoff */
            mb_path OutputGridFile;
            mb_path xlabel;
            mb_path ylabel;
            mb_path zlabel;
            mb_path title;
            if (control.use_projection) {
              sprintf(xlabel, "Easting (%s)", units);
              sprintf(ylabel, "Northing (%s)", units);
            }
            else {
              strcpy(xlabel, "Longitude");
              strcpy(ylabel, "Latitude");
            }
            int xdim = control.OutputDim[0];
            int ydim = control.OutputDim[1];
            double xmin = control.OutputBounds[0];
            double xmax = control.OutputBounds[1];
            double ymin = control.OutputBounds[2];
            double ymax = control.OutputBounds[3];
            double zmin = processPars[0].OutputPriority.at<float>(0,0);
            double zmax = zmin;
            double dx = control.OutputDx[0];
            double dy = control.OutputDx[1];
            float *grid = nullptr;
            int grid_status = mb_mallocd(verbose, __FILE__, __LINE__,
                                xdim * ydim * sizeof(float),
                                (void **)&grid, &error);
            float NaN = std::numeric_limits<float>::quiet_NaN();

            strcpy(OutputGridFile, OutputImageFile);
            OutputWorldFile[strlen(OutputGridFile)-5] = '\0';
            strcat(OutputGridFile,"_priority.grd");
            strcpy(zlabel, "Topography (m)");
            strcpy(title, "Pixel Priority Map");
            for (int i = 0; i < xdim; i++) {
                for (int j = 0; j < ydim; j++) {
                    int k = i * ydim + (ydim - 1 - j);
                    grid[k] = processPars[0].OutputPriority.at<float>(j,i);
                    zmin = MIN(zmin, grid[k]);
                    zmax = MAX(zmax, grid[k]);
                }
            }
            mb_write_gmt_grd(verbose, OutputGridFile, grid,
                                NaN, xdim, ydim,
                                xmin, xmax, ymin, ymax, zmin, zmax, dx, dy,
                                xlabel, ylabel, zlabel, title,
                                projection_id, 2, argv, &error);

            strcpy(OutputGridFile, OutputImageFile);
            OutputWorldFile[strlen(OutputGridFile)-5] = '\0';
            strcat(OutputGridFile,"_intensitychange.grd");
            strcpy(zlabel, "Intensity Change");
            strcpy(title, "Pixel Intensity Change Map");
            for (int i = 0; i < xdim; i++) {
                for (int j = 0; j < ydim; j++) {
                    int k = i * ydim + (ydim - 1 - j);
                    grid[k] = processPars[0].OutputIntensityCorrection.at<float>(j,i);
                    zmin = MIN(zmin, grid[k]);
                    zmax = MAX(zmax, grid[k]);
                }
            }
            mb_write_gmt_grd(verbose, OutputGridFile, grid,
                                NaN, xdim, ydim,
                                xmin, xmax, ymin, ymax, zmin, zmax, dx, dy,
                                xlabel, ylabel, zlabel, title,
                                projection_id, 2, argv, &error);

            strcpy(OutputGridFile, OutputImageFile);
            OutputWorldFile[strlen(OutputGridFile)-5] = '\0';
            strcat(OutputGridFile,"_standoff.grd");
            strcpy(zlabel, "Topography (m)");
            strcpy(title, "Pixel Standoff Map");
            for (int i = 0; i < xdim; i++) {
                for (int j = 0; j < ydim; j++) {
                    int k = i * ydim + (ydim - 1 - j);
                    grid[k] = processPars[0].OutputStandoff.at<float>(j,i);
                    zmin = MIN(zmin, grid[k]);
                    zmax = MAX(zmax, grid[k]);
                }
            }
            mb_write_gmt_grd(verbose, OutputGridFile, grid,
                                NaN, xdim, ydim,
                                xmin, xmax, ymin, ymax, zmin, zmax, dx, dy,
                                xlabel, ylabel, zlabel, title,
                                projection_id, 2, argv, &error);

            mb_freed(verbose, __FILE__, __LINE__, (void **)&grid, &error);
#endif

            /* announce it */
            fprintf(stderr, "\nOutput photomosaic: %s\n",OutputImageFile);

            }
        else
            {
            fprintf(stderr, "Could not save: %s\n",OutputImageFile);
            }

        processPars[0].OutputPriority.release();
#ifdef DEBUG
        processPars[0].OutputIntensityCorrection.release();
        processPars[0].OutputStandoff.release();
#endif
    }

    /* deallocate topography grid array if necessary */
    if (control.use_topography)
        status = mb_topogrid_deall(verbose, &control.topogrid_ptr, &error);

    /* deallocate projection */
    if (control.use_projection)
        int proj_status = mb_proj_free(verbose, &(control.pjptr), &error);

    /* deallocate navigation arrays if necessary */
    if (navigation_specified && nnav > 0)
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
