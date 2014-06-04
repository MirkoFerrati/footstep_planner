#include "footstep_planner.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/filter.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/segmentation/conditional_euclidean_clustering.h>
#include <pcl/range_image/range_image.h>
#include <pcl/features/range_image_border_extractor.h>
#include <pcl/features/boundary.h>
#include <iostream>

using namespace planner;

void footstepPlanner::setParams(double feasible_area_)
{
    this->feasible_area_=feasible_area_;
}

bool footstepPlanner::step_is_stable(KDL::Frame centroid)
{
  return true;
}

bool footstepPlanner::centroid_is_reachable(KDL::Frame centroid)
{
    KDL::JntArray jnt_pos_in;
    jnt_pos_in.resize(kinematics.q_min.rows());
    SetToZero(jnt_pos_in);
    KDL::JntArray jnt_pos_out;
    jnt_pos_out.resize(kinematics.q_min.rows());
    int ik_valid = kinematics.iksolver->CartToJnt(jnt_pos_in, centroid, jnt_pos_out);
    if (ik_valid>0)
    {
        return true;
    }
      
  return false;
}

std::map<int,Eigen::Matrix<double,4,1>> footstepPlanner::getFeasibleCentroids(std::vector< std::shared_ptr< pcl::PointCloud<pcl::PointXYZ>> > polygons,bool left)
{
    
    Eigen::Matrix<double,4,1> centroid;
    std::map<int,Eigen::Matrix<double,4,1>> centroids;
    for(unsigned int j=0;j<polygons.size();j++)
    {
        std::cout<<"> Polygon number of points: "<<polygons.at(j)->size()<<std::endl;
        
        if(!polygon_in_feasibile_area(polygons.at(j)))
        {
            std::cout<<"!! Polygon "<<j<<" outside the feasible area"<<std::endl;
            continue;
        }
        
        std::vector< pcl::PointCloud<pcl::PointXYZ> > polygon_grid =  compute_polygon_grid(polygons.at(j)); //divide the polygon in smaller ones
       
        for(unsigned int i=0;i<polygon_grid.size();i++)
        {
            //for now we place the foot in the centroid of the polygon (safe if convex)
            if(pcl::compute3DCentroid(polygon_grid.at(i), centroid ))
            {                
                for (int angle=30;angle<360;angle=angle+1000) //TODO: fix this
                {                
                    KDL::Frame temp;
                    temp.p[0]=centroid[0];
                    temp.p[1]=centroid[1];
                    temp.p[2]=centroid[2];
                    temp.M=KDL::Rotation::RPY(0,0,angle);
                   
                    if(centroid_is_reachable(temp)) //check if the centroid id inside the reachable area
                    {
                        if(step_is_stable(temp))//check if the centroid can be used to perform a stable step
                        {
                            
                            // for now we just alternate a left step and a right one, we should use a proper way to do it
                            centroids[i*100+j]=centroid;
                            std::cout<<"> Foot Placed in the centroid of cell "<<i<<" of the polygon "<<j<<std::endl;
                            
                            
                        } 
                        else std::cout<<"!! Step not stable"<<std::endl;
                    } 
                    else std::cout<<"!! Step not reachable"<<std::endl;
                }
            } 
            else std::cout<<"!! ERR: Error in computing the centroid !!"<<std::endl;
        }
    }
    return centroids;
}


std::vector< pcl::PointCloud<pcl::PointXYZ> > footstepPlanner::compute_polygon_grid( std::shared_ptr<pcl::PointCloud<pcl::PointXYZ>> polygon)
{
  std::vector< pcl::PointCloud<pcl::PointXYZ> > polygon_grid;
  
  polygon_grid.push_back(*polygon);
  
  return polygon_grid;
}

double footstepPlanner::dist_from_robot(pcl::PointXYZ point)
{
   //read robot pose, for now 0 0 0
  
  double x_diff = point.x - 0.0;
  double y_diff = point.y - 0.0;
  double z_diff = point.z - 0.0;
  
  return sqrt(x_diff*x_diff + y_diff*y_diff + z_diff*z_diff);
}

bool footstepPlanner::polygon_in_feasibile_area(std::shared_ptr<pcl::PointCloud<pcl::PointXYZ>> polygon)
{
  for (unsigned int i=0;i<polygon->size();i++)
  {
      if(dist_from_robot(polygon->at(i)) < feasible_area_) return true;
  }
  
  return false;
}


