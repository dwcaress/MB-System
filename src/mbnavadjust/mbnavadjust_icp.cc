/*--------------------------------------------------------------------
 *    The MB-system:    mbnavadjust_icp.c          8/4/2018
 *
 *    Copyright (c) 2018-2023 by
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
 *    Code to utilize an ICP algorithm in Point Cloud Library (PCL) 
 *    to match point clouds in MBnavadjust, including this source file,
 *    was developed by MBARI summer intern Ethan Slattery, working
 *    with David Caress, during June-August 2018.
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
#include "mbnavadjust_icp.h"

/* C++ include files */
#include <iostream>
#include <iomanip>
#include <sstream>

/** PCL includes **/
//#include <pcl/visualization/pcl_visualizer.h>

using std::endl;
using std::vector;
using mbsystem::bathPoint;
using mbsystem::MBSystem_ICP;

static inline float map(const float value, const float inLow, const float inHigh, const float outLow, const float outHigh) {
    return (((value - inLow)*(outHigh - outLow)) / (inHigh-inLow) ) + outLow;
}

int mbsystem::swath_to_pointVector(const int verbose, ::swath* swathIn, std::vector<bathPoint> &ptOut, const double draftOffset)
{
    ::ping tmpPing; // temporary ping struct to hold data and load into vector

    // iterate through all the pings in the swath, each ping has multiple points
    for(int i = 0; i < swathIn->npings; i++) {
        tmpPing = swathIn->pings[i];

        if(verbose >= 2) {
            std::cerr << "ping #" << tmpPing.pingnumber
                      << " time:" << tmpPing.time_d << " lat:"  << tmpPing.navlat
                      << " lon:"  << tmpPing.navlon << " heading:"   << tmpPing.heading
                      << " beamsBath:" << tmpPing.beams_bath
                      << " beamsBathAloc:" << tmpPing.beams_bath_alloc << endl;
        }

        // push all the points from the current ping onto the vector
        for(int j = 0; j < tmpPing.beams_bath; j++) {
            ptOut.push_back(bathPoint(tmpPing, j, draftOffset));
        }

    }
    return MB_SUCCESS;
}

int mbsystem::transform_to_local(const int verbose, const double origin_lat, const double origin_lon, std::vector<bathPoint> &ptOut, int& error)
{
    std::ostringstream projArgs;        // easiest way to create a string of arguments for PROJ in c++
    PJ_CONTEXT* ctx;                    // PROJ4 multthreading context for this transformation
    PJ* proj;                           // PROJ4 project file
    int status = MB_FAILURE;            // return status, guilty until proven innocent

    /*** Create the proj 4 settings using the origin lat and long from parameters ***/
    /*** Use of WGS84 ellipsoid and extended tranverse mercator projection are hard-coded ***/
    projArgs << std::setprecision(12) << "+proj=etmerc"
             << " +lat_0=" << origin_lat << " +lon_0=" << origin_lon
             << " +ellps=WGS84 +x_0=0 +y_0=0 +units=m +vunits=m +no_defs";

    if(verbose) {
        std::cerr << "Converting to Local Coordinates using PROJ: " << projArgs.str() << endl;
    }

    // create the threading context
    ctx = proj_context_create();

    // create the PROJ4 project with the specified paramters
    proj = proj_create(ctx, projArgs.str().c_str());
    if(proj) {
        // transform each point in the vector to the local frame
        for(auto &point : ptOut) {

            if(verbose >= 2) {
                std::cerr << "LAT/LON/DEPTH: " << point << " --> ";
            }

            // get coordinates and transform to radians, depth stays in meters
            PJ_COORD rads = point.pos;
            rads.xyz.x = proj_torad(rads.xyz.x);
            rads.xyz.y = proj_torad(rads.xyz.y);

            // execute the transform and save results back into the point
            point.pos = proj_trans(proj, PJ_FWD, rads);

            if(verbose >= 2) {
                std::cerr << "X/Y/DEPTH: " << point << endl;
            }
        }

        proj_destroy(proj);
        status = MB_SUCCESS;
    }
    else {
        error  = MB_ERROR_BAD_PARAMETER;
        std::cerr << "ERROR: FATAL PROJ4 error: " << proj_errno_string(proj_context_errno(ctx)) << " can not create project with settings "
                  << projArgs.str().c_str() << endl << std::flush;
    }

    proj_context_destroy(ctx);
    return status;
}

int mbsystem::get_section_Central_nav(const int verbose, struct swath *swathIn, double &lat, double &lon, double &draft, double &heading)
{
    int          status    = MB_SUCCESS;
    unsigned int centralPt = static_cast<unsigned int>(swathIn->npings / 2);
    (void)verbose;

    // TODO: draft is hardcoded and set outside this function untill draft is added to the ping struct
    if(swathIn->npings > 0 && centralPt < swathIn->npings) {
        lat     = swathIn->pings[centralPt].navlat;
        lon     = swathIn->pings[centralPt].navlon;
        heading = swathIn->pings[centralPt].heading;


        // GROSS! this is gone once we get real draft
        double sum   = 0.0;
        int    count = 0;
        for(int i = 0; i < swathIn->npings; i++) {
            ::ping tmpPing = swathIn->pings[i];

            // push all the points from the current ping onto the vector
            for(int j = 0; j < tmpPing.beams_bath; j++) {
                if(!mb_beam_check_flag(tmpPing.beamflag[j])) {
                    sum += tmpPing.bath[j];
                    count++;
                }
            }
        }
        draft = (sum / count) - 3;
    }

    return status;
}

void mbsystem::haxby_colormap(const double value, const double min, const double max, uint8_t &red, uint8_t &green, uint8_t &blue, const bool IS_BATH)
{
    static const unsigned int NUM_COLORS         = 64;
    static const uint8_t haxby_red[NUM_COLORS]   = {0x25, 0x25, 0x26, 0x26, 0x27, 0x27, 0x28, 0x29, 0x2B, 0x2C, 0x2E, 0x2F, 0x31, 0x36, 0x3E, 0x47, 0x50, 0x59, 0x62, 0x6B, 0x70, 0x75, 0x7A, 0x7F, 0x84, 0x89, 0x93, 0x9D, 0xA8, 0xB2, 0xBD, 0xC8, 0xD0, 0xD5, 0xDB, 0xE0, 0xE6, 0xEC, 0xF0, 0xF3, 0xF5, 0xF8, 0xFA, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    static const uint8_t haxby_green[NUM_COLORS] = {0x39, 0x44, 0x4F, 0x5A, 0x65, 0x71, 0x7C, 0x86, 0x90, 0x9A, 0xA4, 0xAE, 0xB8, 0xC1, 0xC8, 0xCF, 0xD6, 0xDD, 0xE5, 0xEB, 0xEB, 0xEB, 0xEB, 0xEC, 0xEC, 0xEC, 0xEE, 0xF1, 0xF4, 0xF7, 0xFA, 0xFD, 0xFD, 0xFA, 0xF7, 0xF4, 0xF1, 0xEE, 0xEB, 0xE3, 0xDC, 0xD4, 0xCD, 0xC5, 0xBE, 0xB9, 0xB5, 0xB0, 0xAC, 0xA7, 0xA3, 0xA3, 0xA7, 0xAB, 0xAF, 0xB3, 0xB7, 0xBD, 0xC8, 0xD3, 0xDE, 0xE9, 0xF4, 0xFF};
    static const uint8_t haxby_blue[NUM_COLORS]  = {0xAF, 0xBB, 0xC7, 0xD3, 0xDF, 0xEB, 0xF7, 0xFB, 0xFC, 0xFD, 0xFD, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF1, 0xE4, 0xD7, 0xCA, 0xBD, 0xB1, 0xAC, 0xAB, 0xA9, 0xA7, 0xA5, 0xA3, 0x9F, 0x98, 0x92, 0x8B, 0x85, 0x7E, 0x78, 0x73, 0x6D, 0x68, 0x62, 0x5D, 0x58, 0x54, 0x51, 0x4E, 0x4B, 0x48, 0x45, 0x4A, 0x55, 0x5F, 0x69, 0x73, 0x7E, 0x8B, 0x9E, 0xB2, 0xC5, 0xD8, 0xEC, 0xFF};


    double       factor;    // factor for calcuating lookup values
    unsigned int i;         // index in the color lookup table

    // calculate the factor to see if it is out of range
    if(!IS_BATH) {
        factor = (max - value) / (max - min);
    }
    else {
        factor = (value - min) / (max - min);
    }

    // calculate the color based on the factor, taking into account out of range values
    if (factor <= 0.0) {
        // out of range low
        red   = haxby_red[0];
        green = haxby_green[0];
        blue  = haxby_blue[0];
    }
    else if (factor >= 1.0) {
        // out of range high
        red   = haxby_red[NUM_COLORS-1];
        green = haxby_green[NUM_COLORS-1];
        blue  = haxby_blue[NUM_COLORS-1];
    }
    else {
        // in range values
        i = static_cast<unsigned int>(factor * (NUM_COLORS - 1));
        factor *= (NUM_COLORS - 1) - i;

        // assign color from the array
        red   = haxby_red[i];
        green = haxby_green[i];
        blue  = haxby_blue[i];
    }
}

mbsystem::PointCloudT::Ptr mbsystem::pointVector_to_pointCloud(const int verbose, std::vector<bathPoint> swathPoints)
{
    PointCloudT::Ptr pCloud(new PointCloudT);
    (void)verbose;

    for(auto &pt : swathPoints) {
        pCloud->push_back(pt.getPointPCL());
    }

    return pCloud;
}

mbsystem::PointNormalCloudT::Ptr mbsystem::pointVector_to_pointNormalCloud(const int verbose, std::vector<bathPoint> swathPoints, const double radius)
{

    PointCloudT::Ptr       pCloud  = pointVector_to_pointCloud(verbose, swathPoints);
    PointNormalCloudT::Ptr pnCloud = pointCloud_to_pointNormalCloud(verbose, pCloud, radius);
    return pnCloud;
}

mbsystem::PointNormalCloudT::Ptr mbsystem::pointCloud_to_pointNormalCloud(const int verbose, mbsystem::PointCloudT::Ptr pCloud, const double radius)
{
    NormalCloudT::Ptr      nCloud(new NormalCloudT);
    PointNormalCloudT::Ptr pnCloud(new PointNormalCloudT);
    PointT                 center;
    (void)verbose;

    // create normal estimation algorithm object
    pcl::NormalEstimation<PointT, NormalT> normEst;
    pcl::search::KdTree<PointT>::Ptr tree(new pcl::search::KdTree<PointT>());
    normEst.setSearchMethod(tree);
    normEst.setRadiusSearch(radius);
    //normEst.setKSearch(10);

    pcl::computeCentroid(*pCloud, center);
    normEst.setViewPoint(center.x, center.y, (center.z + 50));

    normEst.setInputCloud(pCloud);
    normEst.compute(*nCloud);

    pcl::concatenateFields(*pCloud, *nCloud, *pnCloud);

    return pnCloud;
}


std::ostream& mbsystem::operator<< (std::ostream &out, const mbsystem::icp_results &in){
    const static Eigen::IOFormat CSVFormat(Eigen::StreamPrecision, Eigen::DontAlignCols, ",", ",");
    out  << in.tgtFile << ":" << in.tgtSection << "/" << in.srcFile << ":" << in.srcSection << ","
         << in.overlap << ","
         << in.targetPoints << ","
         << in.sourcePoints << ","
         << in.milliseconds << ","
         << in.fitness_rough << ","
         << in.fitness_fine << ","
         << in.correspondenceCount << ","
         << in.Tx << ","
         << in.Ty << ","
         << in.Tz << ","
         << in.Rx << ","
         << in.Ry << ","
         << in.Rz << ","
         << in.transform.format(CSVFormat) << "\n";
    return out;
}

int mbsystem::load_crossing(const int verbose, mbna_project &project, mbna_crossing* crossing,
                            mbna_section* &tgtSection, mbna_section* &srcSection,
                            swath* &tgtSwath, swath* &srcSwath)
{
    int               error     = MB_SUCCESS;
    int               status    = MB_FAILURE;
    struct swathraw*  swathraw1 = nullptr;
    struct swathraw*  swathraw2 = nullptr;
    struct mbna_file* file1     = static_cast<struct mbna_file*>(&project.files[crossing->file_id_1]);
    struct mbna_file* file2     = static_cast<struct mbna_file*>(&project.files[crossing->file_id_2]);

    // load the section pointers
    // TODO: keep these local?
    tgtSection = static_cast<struct mbna_section*>(&file1->sections[crossing->section_1]);
    srcSection = static_cast<struct mbna_section*>(&file2->sections[crossing->section_2]);

    /* load sections */
    if(verbose) { fprintf(stderr, "Loading section 1 of crossing %d:%d/%d:%d...\n", crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2); }
    status = mbnavadjust_section_load(verbose, &project, crossing->file_id_1, crossing->section_1,
                                      (void **)&swathraw1, (void **)&tgtSwath, tgtSection->num_pings, &error);

    if(verbose) { fprintf(stderr, "Loading section 2 of crossing %d:%d/%d:%d...\n", crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2); }
    status = mbnavadjust_section_load(verbose, &project, crossing->file_id_2, crossing->section_2,
                                      (void **)&swathraw2, (void **)&srcSwath, srcSection->num_pings, &error);

    /* get lon lat positions for soundings */
    if(verbose) { fprintf(stderr, "Transforming section 1 of crossing %d:%d/%d:%d...\n", crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2); }
    status = mbnavadjust_section_translate(verbose, &project, crossing->file_id_1, swathraw1, tgtSwath, 0.0, &error);

    if(verbose) { fprintf(stderr, "Transforming section 2 of crossing %d:%d/%d:%d...\n", crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2); }
    status = mbnavadjust_section_translate(verbose, &project, crossing->file_id_2, swathraw2, srcSwath, 0.0, &error);

    // TODO: memory leak?
    // int mb_freed(int verbose, const char *sourcefile, int sourceline, void **ptr, int *error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&swathraw1, &error);
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&swathraw2, &error);


    return status;
}

// https://github.com/PointCloudLibrary/pcl/blob/master/apps/in_hand_scanner/src/icp.cpp
int mbsystem::perform_icp(const int verbose, struct mbna_section* targetSection, struct mbna_section* sourceSection,
                          struct swath* targetSwath, struct swath* sourceSwath, mbsystem::icp_results &results,
                          const mbsystem::icp_param &icpParam, const int savePLY)
{
    typedef mbsystem::PointT      myPoint;
    typedef mbsystem::PointCloudT myCloud;

    Log                     dbgLog;       // atomic debug logger
    vector<bathPoint>       tgtPoints;    // vector of points from swath1
    vector<bathPoint>       srcPoints;    // vector of points from swath2
    MBSystem_ICP<myPoint>   icp;          // swath registration class
    myCloud::Ptr            target;       // cloud created from swath1
    myCloud::Ptr            source;       // cloud created from swath2
    myCloud::Ptr            output;       // cloud to hold the transformed source
    Eigen::Matrix4f         transform;    // The rough alignment offset from cross-correlation
    myPoint                 cInit, cRough, cFine;   // centroids of various source cloud stages
    double                  refLat;       // reference longitude for setting as the origin of local frame
    double                  refLon;       // reference longitude for setting as the origin of local frame
    double                  refDraft;     // draft of the sensor in the reference nav point
    double                  refHeading;   // heading of the sensor in the reference nav point
    int                     status;       // the status variable, for return
    int                     error = 0;    // error return value, for mbsystem functions

    // initialize the transform matrix to the identity matrix
    transform = Eigen::Matrix4f::Identity();

    // set values such that the local coordinate frame uses the source cloud sensor as the origin (without rotation).
    // TODO: probably want a function that will get a 4x4 matrix transform of the two nav points (tgt and src)
    mbsystem::get_section_Central_nav(verbose, sourceSwath, refLat, refLon, refDraft, refHeading);

    // create and transform the first swath
    status = mbsystem::swath_to_pointVector(verbose, targetSwath, tgtPoints, refDraft);
    status = mbsystem::transform_to_local(verbose, refLat, refLon, tgtPoints, error);

    // create and transform the first swath
    status = mbsystem::swath_to_pointVector(verbose, sourceSwath, srcPoints, refDraft);
    status = mbsystem::transform_to_local(verbose, refLat, refLon, srcPoints, error);

    // remove flagged points
    if(verbose) {
        dbgLog << "*************** START ICP ON " << results.tgtFile << ":" << results.tgtSection << "/" << results.srcFile << ":" << results.srcSection << " ***************\n";
        dbgLog << results.tgtFile << ":" << results.tgtSection << " contains " << tgtPoints.size() << "points, "
                  << std::count_if(tgtPoints.begin(), tgtPoints.end(), bathPoint::notFlagged) << " are NOT flagged and will be used\n";
        dbgLog << results.srcFile << ":" << results.srcSection << " contains " << srcPoints.size() << "points, "
                  << std::count_if(srcPoints.begin(), srcPoints.end(), bathPoint::notFlagged) << " are NOT flagged and will be used\n";
    }

    // remove all points that are not "flagged" as useful
    tgtPoints.erase(std::remove_if(tgtPoints.begin(), tgtPoints.end(), bathPoint::isFlagged), tgtPoints.end());
    srcPoints.erase(std::remove_if(srcPoints.begin(), srcPoints.end(), bathPoint::isFlagged), srcPoints.end());

    // load the vectors into a point cloud for use with PCL libraries
    target = mbsystem::pointVector_to_pointCloud(verbose, tgtPoints);
    source = mbsystem::pointVector_to_pointCloud(verbose, srcPoints);

    // Add depth coloring if we are using a type with RGB data
    if(savePLY && pcl::traits::has_color<myPoint>()) {
        float deep    = -1 * (MAX(static_cast<float>(targetSection->depthmax), static_cast<float>(sourceSection->depthmax)) - refDraft);
        float shallow = -1 * (MIN(static_cast<float>(targetSection->depthmin), static_cast<float>(sourceSection->depthmin)) - refDraft);

        for (auto& pt : target->points) {
            haxby_colormap(pt.z, deep, shallow, pt.r, pt.g, pt.b);
        }

        for (auto& pt : source->points) {
            haxby_colormap(pt.z, deep, shallow, pt.r, pt.g, pt.b);
        }
    }

    // save the two origional clouds to PLY files if verbosity is high enough
    if(savePLY) {
        pcl::io::savePLYFile("src.ply", *source);
        pcl::io::savePLYFile("tgt.ply", *target);
    }

    // compute 3-d centroid of source before any tranformations
    pcl::computeCentroid(*source, cInit);

    // Apply the rough alignment from translational cross-correlation as ICP starting point
    transform = mbsystem::apply_Translation<mbsystem::PointT>(source, icpParam.xEst, icpParam.yEst, icpParam.zEst);

    // compute 3-d centroid of source after rough alignment
    pcl::computeCentroid(*source, cRough);

    // create new empty cloud in the output pointer, set up ICP
    output.reset(new myCloud);
    icp.setInputSource(source);
    icp.setInputTarget(target);

    /************************************************
     * VARIOUS FILTERS AND CORRESPONDENCE REJECTORS *
     ************************************************/
    if(icpParam.tgtSOR) {
        mbsystem::downsample_SOR<mbsystem::PointT>(target, icpParam.SOR_neighbors, icpParam.SOR_stdDev);
    }

    if(icpParam.srcSOR) {
        mbsystem::downsample_SOR<mbsystem::PointT>(source, icpParam.SOR_neighbors, icpParam.SOR_stdDev);
    }

    if(icpParam.maxDistance > 0) {
        icp.setMaxCorrespondenceDistance(icpParam.maxDistance);
    }

    if(icpParam.overlap > 0) {
        icp.correspondence_OverlapEstimation(icpParam.overlap / 100.0f);
    }

    if(!icpParam.one2many) {
        icp.correspondence_OneToOne();
    }

    icp.setMaximumIterations(icpParam.maxIterations);
    icp.setTransformationEpsilon(icpParam.epsilonT);
    icp.setEuclideanFitnessEpsilon(icpParam.epsilonFit);

    // save filtered clouds to PLY files if verbosity is high enough
    if(savePLY) {
        pcl::io::savePLYFile("src_filter.ply", *source);
        pcl::io::savePLYFile("tgt_filter.ply", *target);
    }
    /** END FILTERING *******************************/

    // Checks for NAN points. If they exists ICP will segfault, so we remove them
    // but this means something went VERY bad earlier on in the program.
    source->is_dense = false;
    target->is_dense = false;
    std::vector<int> tgt_idx, src_idx;
    pcl::removeNaNFromPointCloud(*source, *source, src_idx);
    pcl::removeNaNFromPointCloud(*target, *target, tgt_idx);

    // if there were NAN points in the cloud then something went very wrong, return an error
    if(tgt_idx.size() < target->size()) {
        dbgLog << "FATAL ERROR: Cloud contains non-finite points in " << results.tgtFile << ":" << results.tgtSection << endl;
        status = MB_FAILURE;
    }
    if(src_idx.size() < source->size()) {
        dbgLog << "FATAL ERROR: Cloud contains non-finite points in " << results.srcFile << ":" << results.srcSection << endl;
        status = MB_FAILURE;
    }

    if(verbose) {
        Eigen::Affine3f roughT(transform);
        float tx, ty, tz, rx, ry, rz;
        pcl::getTranslationAndEulerAngles(roughT, tx, ty, tz, rx, ry, rz);
        dbgLog << "\tEstimated Overlap            : " << icpParam.overlap << endl;
        dbgLog << "\tRough Centroid Move          : " << pcl::geometry::distance(cInit, cRough) << "m (" << (cRough.x - cInit.x)  << ", " << (cRough.y - cInit.y) << ", " << (cRough.z - cInit.z) << ")\n";
        dbgLog << "\tRough Translation (x, y, z)  : " << sqrt((tx*tx) + (ty*ty) + (tz*tz)) << "m (" << tx << ", " << ty << ", " << tz << ")\n";
        dbgLog << "\tRough Rotation (r,p,y)       : " << rx << ", " << ry << ", " << rz << endl;
    }

    // Perform alignment and get final transformation
    icp.align(*output);
    transform *= icp.getFinalTransformation();

    // get rough fitness before ICP
    results.fitness_rough = icp.getFitnessScore_transform_correspondence(Eigen::Matrix4f::Identity());

    // Load the results with the translation and rotation required to perform alignment
    Eigen::Affine3f fineT(transform);
    float tx, ty, tz, rx, ry, rz;
    pcl::getTranslationAndEulerAngles(fineT, tx, ty, tz, rx, ry, rz);
    results.Tx = static_cast<double>(tx);
    results.Ty = static_cast<double>(ty);
    results.Tz = static_cast<double>(tz);
    results.Rx = static_cast<double>(rx);
    results.Ry = static_cast<double>(ry);
    results.Rz = static_cast<double>(rz);

    /*** Translation of cloud centroids - NOT USED ***/
//    results.Tx = static_cast<double>(cRough.x - cInit.x);
//    results.Ty = static_cast<double>(cRough.y - cInit.y);
//    results.Tz = static_cast<double>(cRough.z - cInit.z);

    // load the results of ICP into the result structure
    results.overlap                 = icpParam.overlap;
    results.transform               = transform;
    results.targetPoints            = target->size();
    results.sourcePoints            = source->size();
    results.correspondenceCount     = icp.correspondence_getList()->size();
    results.fitness_fine            = icp.getFitnessScore_transform_correspondence(icp.getFinalTransformation());

    if(verbose) {
        // done after ICP with identity matrix since ICP has to run to calculate correspondences
        dbgLog << "\tRough Correspondence Fitness : " << results.fitness_rough << endl;

        pcl::computeCentroid(*output, cFine);
        dbgLog << "\tFinal Centroid Move          : " << pcl::geometry::distance(cInit, cFine)  << "m (" << (cFine.x - cInit.x)  << ", " << (cFine.y - cInit.y) << ", " << (cFine.z - cInit.z) << ")\n";
        dbgLog << "\tFinal Translation (x, y, z)  : " << sqrt((results.Tx*results.Tx) + (results.Ty*results.Ty) + (results.Tz*results.Tz)) << "m (" << results.Tx << ", " << results.Ty << ", " << results.Tz << ")\n";
        dbgLog << "\tFinal Rotation (r, p, y)     : " << results.Rx << ", " << results.Ry << ", " << results.Rz << endl;
        dbgLog << "\tFinal Correspondence Fitness : " << results.fitness_fine << endl;
        dbgLog << "\tFinal Transformation         : " << endl << results.transform << endl;
        dbgLog << "*************** END ICP ON " << results.tgtFile << ":" << results.tgtSection << "/" << results.srcFile << ":" << results.srcSection << " ***************\n";
    }

    if(savePLY) {
        // mark correspondence points red if RGB data is in the point type
        if(pcl::traits::has_color<myPoint>()) {
            pcl::CorrespondencesPtr cList = icp.correspondence_getList();
            float maxDist = 0;

            for(auto c : *cList) {
                maxDist = MAX(maxDist, c.distance);
            }

            for(auto c : *cList) {
                uint8_t pos = static_cast<uint8_t>(map(c.distance, 0, maxDist, 0, 255));
                uint8_t inv = static_cast<uint8_t>(map(c.distance, 0, maxDist, 255, 0));

                target->at(static_cast<size_t>(c.index_match)).r = inv;
                target->at(static_cast<size_t>(c.index_match)).g = pos;
                target->at(static_cast<size_t>(c.index_match)).b = 0;

                output->at(static_cast<size_t>(c.index_query)).r = inv;
                output->at(static_cast<size_t>(c.index_query)).g = pos;
                output->at(static_cast<size_t>(c.index_query)).b = 0;
            }
        }

        pcl::io::savePLYFile("src_reg.ply", *output);
        pcl::io::savePLYFile("tgt_reg.ply", *target);
    }

    return status;
}

bathPoint::bathPoint(::ping p, const int idx, const double draft)
{
    flag = p.beamflag[idx];
    pos  = proj_coord(p.bathlon[idx], p.bathlat[idx], -(p.bath[idx]-draft), 0);
    if((!pcl_isfinite(pos.xyz.x) || !pcl_isfinite(pos.xyz.y) || !pcl_isfinite(pos.xyz.z)) && !mb_beam_check_flag(flag) ) {
        Log(2, __FILE__, __LINE__) << "ERROR - bathPoint CTR: unflagged NAN point in ping!\n";
    }
}

bathPoint::bathPoint(const double lat, const double lon, const double altitude)
{
    flag = MB_FLAG_FLAG;
    pos  = proj_coord(lon, lat, altitude, 0);
}

mbsystem::PointT bathPoint::getPointPCL()
{
    PointT p;
    p.x = static_cast<float>(pos.xyz.x);
    p.y = static_cast<float>(pos.xyz.y);
    p.z = static_cast<float>(pos.xyz.z);

    if(pcl::traits::has_color<PointT>()) {
        p.r = 0;
        p.g = 0;
        p.b = 255;
    }

    return p;
}

std::ostream& mbsystem::operator<<(std::ostream &out, mbsystem::bathPoint &p)
{
    out << std::setprecision(12)
        << p.pos.enu.e << ", "
        << p.pos.enu.n << ", "
        << p.pos.enu.u;
    return out;
}

/** static class methods **/
bool bathPoint::notFlagged(bathPoint p) { return !mb_beam_check_flag(p.flag); }
bool bathPoint::isFlagged(bathPoint p)  { return mb_beam_check_flag(p.flag); }
