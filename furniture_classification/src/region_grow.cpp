/*
 * region_grow.cpp
 *
 *  Created on: May 30, 2012
 *      Author: vsu
 */

#include <pcl/console/parse.h>
#include <pcl/io/pcd_io.h>
#include <pcl/search/kdtree.h>
#include <pcl/surface/mls.h>
#include <pcl/segmentation/region_growing_rgb.h>
#include <pcl/features/normal_3d.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/filters/voxel_grid.h>

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    PCL_INFO ("Usage %s -input_file cloud.pcd -distance_thresh 0.01 -angle_thresh 30 \n", argv[0]);

    return -1;
  }

  std::string filename;
  float distance_thresh = 0.01;
  float angle_thresh = 0.01;

  pcl::console::parse_argument(argc, argv, "-input_file", filename);
  pcl::console::parse_argument(argc, argv, "-distance_thresh", distance_thresh);
  pcl::console::parse_argument(argc, argv, "-angle_thresh", distance_thresh);

  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud_orig(new pcl::PointCloud<pcl::PointXYZRGBA>);
  pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
  pcl::PointCloud<pcl::Normal>::Ptr cloud_normals(new pcl::PointCloud<pcl::Normal>());
  pcl::io::loadPCDFile(filename, *cloud);

  std::cerr << "Loaded file" << std::endl;

  //pcl::VoxelGrid<pcl::PointXYZRGBA> grid;
  //grid.setInputCloud(cloud_orig);
  //grid.setLeafSize(0.01f, 0.01f, 0.01f);
  //grid.filter(*cloud);

  // Create a KD-Tree
  pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGBA>);

  std::cerr << "Created tree" << std::endl;

  pcl::NormalEstimation<pcl::PointXYZRGBA, pcl::Normal> ne;
  ne.setInputCloud(cloud);
  ne.setSearchMethod(tree);
  ne.setRadiusSearch(0.01);
  ne.compute(*cloud_normals);

  std::cerr << "Computed normals" << std::endl;

  pcl::RegionGrowingRGB<pcl::PointXYZRGBA> region_growing;
  region_growing.setCloud(cloud);
  region_growing.setNormals(cloud_normals);
  region_growing.setNeighbourSearchMethod(tree);

  region_growing.setResidualTest(true);
  region_growing.setResidualThreshold(distance_thresh);

  region_growing.setCurvatureTest(true);
  region_growing.setCurvatureThreshold(0.04);


  region_growing.setSmoothMode(false);
  region_growing.setSmoothnessThreshold(angle_thresh * M_PI/180);

  region_growing.setPointColorThreshold(100.0);
  region_growing.setRegionColorThreshold(200.0);

  region_growing.segmentPoints();

  pcl::PointCloud<pcl::PointXYZRGB>::Ptr segments;
  segments = region_growing.getColoredCloud();

  pcl::io::savePCDFile("result.pcd", *segments);

  pcl::visualization::PCLVisualizer viz;
  viz.initCameraParameters();

  pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> rgb(segments);
  viz.addPointCloud<pcl::PointXYZRGB> (segments, rgb);
  viz.spin();

  return 0;
}
