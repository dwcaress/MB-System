//#include <sstream>
//#include <iostream>
#include <fstream>
//#include <ctime> //This doesn't seem to be needed for running this code

#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/features/normal_3d.h>
#include <pcl/surface/gp3.h>
#include <pcl/io/vtk_io.h>
#include <pcl/io/vtk_lib_io.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/visualization/histogram_visualizer.h>
#include <pcl/octree/octree.h>
#include <pcl/octree/octree_pointcloud_voxelcentroid.h>

using namespace std;



void octreeTest (void);
