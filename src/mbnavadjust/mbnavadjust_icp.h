/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_icp.c                8/4/2018
 *
 *    Copyright (c) 2018-2024 by
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

#ifndef MBNAVADJUSTTEST_ICP_H
#define MBNAVADJUSTTEST_ICP_H

#include <vector>
#include <proj.h>

/* PCL include files */
#include <pcl/common/common.h>
#include <pcl/common/centroid.h>
#include <pcl/common/geometry.h>

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/point_representation.h>

#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/filters/filter.h>

#include <pcl/features/normal_3d.h>

#include <pcl/registration/icp.h>
#include <pcl/registration/icp_nl.h>
#include <pcl/registration/transforms.h>
#include <pcl/registration/correspondence_rejection_trimmed.h>
#include <pcl/registration/correspondence_rejection_one_to_one.h>

#include <pcl/io/ply_io.h>
//#include <pcl/io/pcd_io.h>

/* MBIO include files */
extern "C" {
    #include "mb_status.h"
    #include "mb_define.h"
    #include "mb_process.h"
    #include "mb_aux.h"
    #include "mbnavadjust_io.h"
}

namespace mbsystem {

typedef pcl::PointXYZRGB PointT;
typedef pcl::PointCloud<PointT> PointCloudT;

typedef pcl::Normal NormalT;
typedef pcl::PointCloud<NormalT> NormalCloudT;

typedef pcl::PointXYZRGBNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointNormalCloudT;

// forward declare new structs and classes
class  bathPoint;
struct icp_results;
struct icp_param;

/**
 * @brief Performs ICP alignment on two mbnavadjust swaths
 * @param verbose [IN] the verbosity level of the output to stderr
 * @param targetSection [IN] pointer to the section struct for the target
 * @param sourceSection [IN] pointer to the section struct for the source
 * @param targetSwath [IN] pointer to the swath struct for the target
 * @param sourceSwath [IN] pointer to the swath struct for the source
 * @param results [OUT] struct to return the results of the alignment in
 * @param icpParam [IN] struct to specify the parameters of the alignment
 * @param savePLY [IN] boolean value, indicates if PLY files of each step should be saved for debug
 * @return either MB_SUCCESS or MB_FAILURE (1 and 0 respectivley).
 *
 * This function performs the ICP alignment based on the icp parameters structure passed into the function
 */
int perform_icp(const int verbose, struct mbna_section* targetSection, struct mbna_section* sourceSection,
                struct swath* targetSwath, struct swath* sourceSwath, icp_results &results,
                const icp_param &icpParam, const int savePLY = 0);

/**
 * @brief load_crossing
 * @param verbose [IN] the verbosity level of the output to stderr
 * @param project [IN] the project file, loaded with mbnavadjust_read_project()
 * @param crossing [IN] pointer to the crossing struct to load from
 * @param section1 [OUT] pointer to a section structure to load the new section into
 * @param section2 [OUT] pointer to a section structure to load the new section into
 * @param swath1 [OUT] pointer to a swath structure to load the new swath into
 * @param swath2 [OUT] pointer to a swath structure to load the new swath into
 * @return either MB_SUCCESS or MB_FAILURE (1 and 0 respectivley).
 *      Memory pointed to by the swath pointers may need to be freed. the section pointers DO NOT
 *      need to be freed, they point to memory in the project structure.
 */
int load_crossing(const int verbose, mbna_project &project, mbna_crossing* crossing,
                  mbna_section* &tgtSection, mbna_section* &srcSection,
                  swath* &tgtSwath, swath* &srcSwath);

/**
 * @brief adds a swath to the provided vector of bathPoints
 * @param verbose [IN] the verbasity level for debug info
 * @param swathIn [IN] the mb-system swath data struct
 * @param ptOut [OUT] the vector of bathPoints to add to
 * @param draftOffset [IN] offset of the sensor draft in meters, applied to all points
 * @return either MB_SUCCESS or MB_FAILURE (1 and 0 respectivley).
 *         If failure, specifics are returned in the error parameter
 *
 * This function ADDS to the vector, and as such can be used to add multiple swaths to the same vector.
 * If the desire is to have one swath per vector be sure to clear the vector before using this
 * function.
 */
int swath_to_pointVector(const int verbose, ::swath* swathIn, std::vector<bathPoint> &ptOut, const double draftOffset = 0.0);

/**
 * @brief transforms all points in a vector to local coordinates based on a provided origin
 * @param verbose [IN] the verbasity level for debug info
 * @param origin_lat [IN] origin point of the transform in degrees latitude
 * @param origin_lon [IN] origin point of the transform in degrees longitude
 * @param ptOut [OUT] vector of bathPoints to transform
 * @param error [OUT] the mb-system error return code, only changed if there is an error.
 * @return either MB_SUCCESS or MB_FAILURE (1 and 0 respectivley).
 *         If failure, specifics are returned in the error parameter
 *
 * This function transforms all points in a vector to local coordinates. The latitude and
 * longitude to be considered the origin are passed in, so must be stored externally if this
 * transform is to be repeated. The transfom uses the WGS84 ellipsoid and the Extended Tranverse Mercator projection
 * for the transform.
 */
int transform_to_local(const int verbose, const double origin_lat, const double origin_lon, std::vector<bathPoint> &ptOut, int& error);

/**
 * @brief Get the lat, long, and draft of the sensor at a representitive point in navigation (center of section)
 * @param verbose [IN] verbosity of output
 * @param swathIn [IN] swath to get data from
 * @param lat [OUT] lattitude of the vehicle in the world reference frame
 * @param lon [OUT] longitude of the vehicle in the world reference frame
 * @param draft [OUT] the depth of th esensor under the surface in meters
 * @return either MB_SUCCESS or MB_FAILURE (1 and 0 respectivley).
 */
int get_section_Central_nav(const int verbose, struct swath *swathIn, double &lat, double &lon, double &draft, double &heading);

/**
 * @brief mbsystem::haxby_colormap
 * @param value [IN] the value to calculate a color for
 * @param min [IN] the lowest value in the dataset, if bathymetery this should be the deepest value (most negative)
 * @param max [IN] The highest value in the dataset, if bathymetery this should be the shallowest value (least4 negative)
 * @param red [OUT] The variable to hold the red value
 * @param blue [OUT] The variable to hold the blue value
 * @param green [OUT] The variable to hod the green value
 * @param IS_BATH [IN] Optional boolean, set false if this is topographic (+z is up)
 */
void haxby_colormap(const double value, const double min, const double max, uint8_t &red, uint8_t &green, uint8_t &blue, const bool IS_BATH = true);

/**
 * @brief make a new point cloud from a vector of bathymetry points
 * @param verbose [IN] the verbasity level for debug info
 * @param swathPoints [IN] The vector of mbsystem points to make into a cloud
 * @param error [OUT] the mb-system error return code, only changed if there is an error.
 * @return shared pointer to the new point cloud, null if error
 *
 * This function takes a vector of points from mbsystem and loads them into a PCL point cloud, returning a shared pointer to that cloud
 */
PointCloudT::Ptr pointVector_to_pointCloud(const int verbose, std::vector<bathPoint> swathPoints);

/**
 * @brief make a new point cloud with normal data from a vector of bathymetry points
 * @param verbose [IN] the verbasity level for debug info
 * @param swathPoints [IN] The vector of mbsystem points to make into a cloud
 * @param radius [IN] radious of points to use for estimation, in meters (10cm defualt)
 * @return shared pointer to a new cloud, with normals for all points
 *
 * This function is equivalent to calling pointVector_to_pointCloud() and then pointCloud_to_pointNormalCloud()
 */
PointNormalCloudT::Ptr pointVector_to_pointNormalCloud(const int verbose, std::vector<bathPoint> swathPoints, const double radius = 0.10);

/**
 * @brief compute surface normals for a cloud, and return a pointer to a new cloud with that data
 * @param verbose [IN] the verbasity level for debug info
 * @param cloud [IN] shared pointer to the cloud of interest
 * @param radius [IN] radious of points to use for estimation, in meters (10cm defualt)
 * @return shared pointer to a new cloud, with normals for all points
 */
PointNormalCloudT::Ptr pointCloud_to_pointNormalCloud(const int verbose, PointCloudT::Ptr pCloud, const double radius = 0.10);

/**
 * @brief apply an x,y,z translation to a point cloud and
 * @param pCloud [IN] The point cloud, will be modified in place
 * @param x [IN] The x translation in meters
 * @param y [IN] The y translation in meters
 * @param z [IN] The z translation in meters
 * @return The translation as a 4x4 matrix (this translation * identity).
 */
template<typename P>
Eigen::Matrix4f apply_Translation(typename pcl::PointCloud<P>::Ptr cloud, const double x, const double y, const double z)
{
    Eigen::Affine3f transform(Eigen::Translation3f(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)));
    pcl::transformPointCloud(*cloud, *cloud, transform.matrix());
    return transform.matrix();
}

/**
 * @brief downsample both clouds using voxel grid method (ref: http://pointclouds.org/documentation/tutorials/voxel_grid.php)
 * @param cloud [IN] shared pointer to the cloud of interest
 * @param leafSize [IN] the size of the voxel grid cube (x=y=z) in meters (10cm default)
 * @return MB_SUCCESS or MB_FAILURE
 */
template<typename P>
int downsample_Voxel(typename pcl::PointCloud<P>::Ptr cloud, const float leafSize = 0.10f)
{
    pcl::VoxelGrid<P> grid;
    grid.setLeafSize(leafSize, leafSize, leafSize);

    grid.setInputCloud(cloud);
    grid.filter(*cloud);

    return MB_SUCCESS;
}

/**
 * @brief Uses the statistical Outlier Removal method to remove noise from both clouds
 * @param cloud [IN] shared pointer to the cloud of interest
 * @param neighbors [IN] the number of neighbors to use for calculating average distance and standard deviation
 * @param stdDev [IN] the standard deviation multiplier
 * @return MB_SUCCESS or MB_FAILURE
 *
 *         max distance = avg_distance + (stdDevMult * std_deviation)
 */
template<typename P>
int downsample_SOR(typename pcl::PointCloud<P>::Ptr cloud, const int neighbors = 50, const double stdDev = 1.0)
{
    pcl::StatisticalOutlierRemoval<P> sor;
    sor.setMeanK(neighbors);
    sor.setStddevMulThresh(stdDev);

    sor.setInputCloud(cloud);
    sor.filter(*cloud);

    return MB_SUCCESS;
}

/// struct to hold the parameters for an ICP alignment procedure
struct icp_param {
    unsigned int maxIterations;   // Max iterations to run ICP
    unsigned int overlap;         // estimated overlap of two swaths, set < 0 to ignore
    double       maxDistance;     // maximum distance, in meters, of a valid correspondence
    bool         one2many;        // if true, use enforce correspondence one-to-one ratio
    bool         tgtSOR;          // perform Statistical Outlier Removal on the target
    bool         srcSOR;          // perform Statistical Outlier Removal on the source
    unsigned int SOR_neighbors;   // number of nearest neighbors to use for SOR
    double       SOR_stdDev;      // standard deviation to use for SOR
    double       epsilonT;        // change in transform distance to stop ICP early (~1e-6 is good)
    double       epsilonFit;      // change in Fitness value to stop ICP early (~1e-6 is good)
    double       xEst;            // estimated x transform from rough alignment
    double       yEst;            // estimated y transform from rough alignment
    double       zEst;            // estimated z transform from rough alignment
};

/// struct to hold the results of an ICP alignment
struct icp_results {
    Eigen::Matrix4f transform;                  // final transform as a 4x4 Affine matrix
    int             tgtFile;                    // MB_SYSTEM file ID for section 1
    int             tgtSection;                 // MB_SYSTEM section number for section 1
    int             srcFile;                    // MB_SYSTEM file ID for section 2
    int             srcSection;                 // MB_SYSTEM section number for section 2
    int             overlap;                    // estimated overlap percentage before ICP
    double          fitness_rough;              // fitness value, for considering all points
    double          fitness_fine;               // fitness value, considering only correspondence points
    unsigned long   targetPoints;               // number of points in the source cloud after filtering
    unsigned long   sourcePoints;               // number of points in the source cloud after filtering
    unsigned long   correspondenceCount;        // number of correspondence points used
    long            milliseconds;               // time it took to calculate this result
    double          Tx;                         // Translation in the X direction (meters in local frame)
    double          Ty;                         // Translation in the Y direction (meters in local frame)
    double          Tz;                         // Translation in the Z direction (meters in local frame)
    double          Rx;                         // Rotation about the X Axis (radians in local frame)
    double          Ry;                         // Rotation about the Y Axis (radians in local frame)
    double          Rz;                         // Rotation about the Z Axis (radians in local frame)
};

/// stream overload operator
std::ostream& operator<< (std::ostream &out, const icp_results &in);

/// Class to provide thread-atomic logging output. will write to stream on destruction or flush()
class Log {
   std::ostringstream st;   // stringstream for logging the output
   std::ostream &stream;    // stream to output to when flushed or destructed, stderr by default
public:
   Log(std::ostream &s=std::cerr) :stream(s) {}
   Log(const int verbose, const char* file, const int line, std::ostream &s=std::cerr):stream(s) {
       st << "L" << verbose << " file:" << file << " line:" << line << " - ";
   }
   ~Log() { stream << st.str(); }

   template <typename T>
   Log& operator<<(T const& t) {
      st << t;
      return *this;
   }

   Log& operator<<( std::ostream&(*f)(std::ostream&) ) {
       st << f;
       return *this;
   }

   void flush() { stream << st.str(); st.clear(); }
};

/// Class to represent a single point of bathymitry. one point from a ping
class bathPoint {
public:
    bathPoint(struct ping p, const int idx, const double draft = 0.0);
    bathPoint(const double lat, const double lon, const double altitude);

    /// gets the mbnavadjust flag of this point
    char     getFlag()  { return flag; }
    /// Gets the PCL version of the point
    PointT getPointPCL();

    /// @returns true if the point is NOT flagged as invalid
    static bool notFlagged(bathPoint p);
    /// @returns true if the point IS flagged as invalid
    static bool isFlagged(bathPoint p);
    /// @returns overload of the stream operator for printing a point
    friend std::ostream& operator<<(std::ostream &out, const bathPoint &p);

    /** PUBLIC DATA **/
    PJ_COORD pos;   // the point in lat/long format, directly from MB-system import

private:
    char flag;      // the mb-system flag for this point
};

std::ostream& operator<<(std::ostream &out, bathPoint &p);

/**
 * @brief The MBSystem_ICP class, ICP algorithm inherited from PCL
 */
template<typename P>
class MBSystem_ICP : public pcl::IterativeClosestPoint<P, P> {
public:

    /**
     * @brief add a correspondence rejector based on mbnavadjust estimated overlap
     * @param swathOverlap [IN] the estimated overlap from mbnavadjust, between 0 and 100.
     * @return MB_SUCCESS or MB_FAILURE
     */
    int correspondence_OverlapEstimation(const float swathOverlap)
    {
        int status = MB_FAILURE;
        if(swathOverlap <= 1 && swathOverlap >= 0) {
            // create a correspondance rejector object based on the expected overlap
            pcl::registration::CorrespondenceRejectorTrimmed::Ptr estOverlap(new pcl::registration::CorrespondenceRejectorTrimmed);
            estOverlap->setOverlapRatio(swathOverlap);
            this->addCorrespondenceRejector(estOverlap);
            status = MB_SUCCESS;
        }

        return status;
    }

    /**
     * @brief Sets a correspondence rejector that ensures all correspondences are one-to-one
     * @return MB_SUCCESS or MB_FAILURE
     */
    int correspondence_OneToOne()
    {
        pcl::registration::CorrespondenceRejectorOneToOne::Ptr one2one(new pcl::registration::CorrespondenceRejectorOneToOne);
        this->addCorrespondenceRejector(one2one);
        return MB_SUCCESS;
    }

    /**
     * @brief sets the epsilon, the difference between two iterations when icp is considered "done"
     * @param epsilon [IN] the epsilon, used for both transformation epsilon and fitness epsilon
     * @return MB_SUCCESS or MB_FAILURE
     */
    int setEpsilon(const double epsilon = 1e-6)
    {
        this->setTransformationEpsilon(epsilon);
        this->setEuclideanFitnessEpsilon(epsilon);
        return MB_SUCCESS;
    }

    /**
     * @brief gets the fitness score taking into consideration only correspondence points
     * @param transform [IN] the transform to apply to the source cloud for comparison
     * @return the fitness score, sum of the squared distances
     */
    double getFitnessScore_transform_correspondence(Eigen::Matrix4f transform)
    {
        pcl::PointCloud<P>  input_transformed;  // the input cloud with transform applied
        double              score;              // accumulator for squared distances
        int                 nr;                 // counter for number of distances found

        // apply transformation to the source cloud
        pcl::transformPointCloud(*(this->input_), input_transformed, transform);

        // initialize the counters and iterate through all correspondence points
        nr    = 0;
        score = 0.0;
        for(size_t i = 0; i < this->correspondences_->size(); i++) {
            // find nearest neighbor in the target, get the squared distance back in nn_dists
            int srcPt = (*this->correspondences_)[i].index_query;
            int tgtPt = (*this->correspondences_)[i].index_match;

            // will be -1 if no correspondence found
            if(tgtPt >= 0) {
                score += pcl::geometry::squaredDistance(input_transformed.at(srcPt), this->target_->at(tgtPt));
                nr++;
            }
        }

        // return the average of squared distances, or max value of a double if no points found
        score = (nr > 0 ? (score / nr) : std::numeric_limits<double>::max() );
        return score;
    }

    /**
     * @brief gets the global fitness score. Identical to PCL function except the ability to apply an arbitrary transform
     * @param transform [IN] the transform to apply to the source cloud for comparison
     * @return the fitness score, sum of the squared distances
     */
    double getFitnessScore_transform(Eigen::Matrix4f transform, double max_range = std::numeric_limits<double>::max())
    {
        pcl::PointCloud<P>  input_transformed;  // the input cloud with transform applied
        std::vector<int>    nn_indices(1);      // matching indicies from nearest neighbor search
        std::vector<float>  nn_dists(1);        // distances of each nearest neighbor (parallel vector to nn_indices)
        double              score;              // accumulator for squared distances
        int                 nr;                 // counter for number of distances found

        // apply transformation to the source cloud
        pcl::transformPointCloud(*(this->input_), input_transformed, transform);

        // initialize the counters and iterate through all points in the source cloud
        nr    = 0;
        score = 0.0;
        for(size_t i = 0; i < input_transformed.points.size(); i++) {
            // find nearest neighbor in the target, get the squared distance back in nn_dists
            this->tree_->nearestKSearch(input_transformed.points[i], 1, nn_indices, nn_dists);

            //deal with occulsions and incomplete targets
            if(nn_dists[0] <= max_range) {
                score += nn_dists[0];
                nr++;
            }
        }

        // return the average of squared distances, or max value of a double if no points found
        score = (nr > 0 ? (score / nr) : std::numeric_limits<double>::max() );
        return score;
    }

    /**
     * @brief get the list of correspondence points and print some stats
     * @return pointer to a list of correspondence points
     */
    pcl::CorrespondencesPtr correspondence_getList() { return this->correspondences_; }

};

/**
 * @brief The MBSystem_ICP class, ICP algorithm inherited from PCL
 */
template<typename P, typename N>
class MBSystem_ICPNormal : public pcl::IterativeClosestPointWithNormals<P, N> {
public:

    /**
     * @brief add a correspondence rejector based on mbnavadjust estimated overlap
     * @param swathOverlap [IN] the estimated overlap from mbnavadjust, between 0 and 100.
     * @return MB_SUCCESS or MB_FAILURE
     */
    int setOverlapEstimation(const float swathOverlap)
    {
        int status = MB_FAILURE;
        if(swathOverlap <= 1 && swathOverlap >= 0) {
            // create a correspondance rejector object based on the expected overlap
            pcl::registration::CorrespondenceRejectorTrimmed::Ptr estOverlap(new pcl::registration::CorrespondenceRejectorTrimmed);
            estOverlap->setOverlapRatio(swathOverlap);
            this->addCorrespondenceRejector(estOverlap);
            status = MB_SUCCESS;
        }

        return status;
    }

    /**
     * @brief sets the epsilon, the difference between two iterations when icp is considered "done"
     * @param epsilon [IN] the epsilon, used for both transformation epsilon and fitness epsilon
     * @return MB_SUCCESS or MB_FAILURE
     */
    int setEpsilon(const double epsilon = 1e-6)
    {
        this->setTransformationEpsilon(epsilon);
        this->setEuclideanFitnessEpsilon(epsilon);
        return MB_SUCCESS;
    }

    pcl::CorrespondencesPtr getCorrespondencesPtr() {
        std::cerr << "Total Correspondences: " << this->correspondences_->size() << std::endl;
        for (uint32_t i = 0; i < this->correspondences_->size() && i < 50; i++) {
            std::cerr << (*this->correspondences_)[i];
            pcl::Correspondence currentCorrespondence = (*this->correspondences_)[i];
            std::cerr << "Index of the source point: " << currentCorrespondence.index_query << std::endl;
            std::cerr << "Index of the matching target point: " << currentCorrespondence.index_match << std::endl;
            std::cerr << "Distance between the corresponding points: " << currentCorrespondence.distance << std::endl;
            std::cerr << "Weight of the confidence in the correspondence: " << currentCorrespondence.weight << std::endl;
        }
        return this->correspondences_;
    }

};

}   /* ned of mbsystem namespace */


#endif  /* mbnavadjusttest_icp_H */
