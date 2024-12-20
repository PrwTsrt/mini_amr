/*
 * Copyright 2015 Fetch Robotics Inc.
 * Author: Michael Ferguson, Sriramvarun Nidamarthy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <angles/angles.h>
#include <dock_lidar/icp_2d.h>
#include <dock_lidar/perception.h>

#include <chrono>
#include <iostream>
#include <list>
#include <queue>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <sensor_msgs/point_cloud2_iterator.hpp>
#include <string>
#include <vector>
#include <stdio.h>




double getPoseDistance(const geometry_msgs::msg::Pose a,
                       const geometry_msgs::msg::Pose b) {
  double dx = a.position.x - b.position.x;
  double dy = a.position.y - b.position.y;
  return sqrt(dx * dx + dy * dy);
}

DockPerception::DockPerception(std::shared_ptr<rclcpp::Node> node_ptr)
    : listener_(node_ptr->get_clock()),
      running_(false),
      tracking_frame_("odom"),
      found_dock_(false) {
  node_ptr_ = node_ptr;
  debug_ = false;

  // enable debugging for first time test
  // Should we publish the debugging cloud

  //  if (!pnh.getParam("debug", debug_))
  //{
  //  debug_ = false;
  //  }

  // Create coefficient vectors for a second order butterworth filter
  // with a cutoff frequency of 10 Hz assuming the loop is updated at 50 Hz
  // [b, a] = butter(order, cutoff_frequ/half_sampling_freq)
  // [b, a] = butter(2, 10/25)

  float b_arr[] = {0.20657, 0.41314, 0.20657};
  float a_arr[] = {1.00000, -0.36953, 0.19582};
  std::vector<float> b(b_arr, b_arr + sizeof(b_arr) / sizeof(float));
  std::vector<float> a(a_arr, a_arr + sizeof(a_arr) / sizeof(float));
  dock_pose_filter_.reset(new LinearPoseFilter2D(b, a));

  // Limit the average reprojection error of points onto
  // the ideal dock. This prevents the robot docking
  // with something that is very un-dock-like.
  // TODO: parameterize (maybe)
  max_alignment_error_ = 0.012;

  // Create ideal cloud
  // Front face is 300mm long
  /*for (double y = -0.15; y <= 0.15; y += 0.001) {
    geometry_msgs::msg::Point p;
    p.x = p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }
  // Each side is 100mm long, at 45 degree angle
  for (double x = 0.0; x < 0.05; x += 0.001) {
    geometry_msgs::msg::Point p;
    p.x = x;
    p.y = 0.15 + x;
    p.z = 0.0;
    ideal_cloud_.push_back(p);
    p.y = -0.15 - x;
    ideal_cloud_.insert(ideal_cloud_.begin(), p);
  }*/

 //Poo add Slamtec docking dimension
 //Center 118 mm
 //Front face is 118mm long
 
 /*for (double y = -0.059; y <= 0.059; y += 0.001){
    geometry_msgs::msg::Point p;
    p.x = p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }
   
  // Each side is 62mm long with 88 mm-depth
  for (double y = 0.001; y <= 0.062; y += 0.001){
    geometry_msgs::msg::Point p;
    p.x = 0.088;
    p.y = 0.059 + y;
    p.z = 0.0;
    ideal_cloud_.push_back(p);
    p.y = -0.059 - y;
    ideal_cloud_.insert(ideal_cloud_.begin(), p);
  }
 
  // Each side is 48mm long with 0 mm-depth
  for (double y = 0.001; y <= 0.048; y += 0.001){
    geometry_msgs::msg::Point p;
    p.x = 0.0;
    p.y = 0.121 + y;
    p.z = 0.0;
    ideal_cloud_.push_back(p);
    p.y = -0.121 - y;
    ideal_cloud_.insert(ideal_cloud_.begin(), p);
  }
*/


 //Poo add mini docking dimension
 //triangle 118.94 mm (each side) with 86.41-depth mm
 //liner 48 mm (each side)
 
 
 //triangle big
/* for (double y = -0.119; y < 0; y += 0.005) {
    geometry_msgs::msg::Point p;
    p.x = (0.08641/0.11894)*y + 0.08641;
    p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }
  for (double y = 0; y <= 0.119; y += 0.005) {
    geometry_msgs::msg::Point p;
    p.x = (-0.08641/0.11894)*y + 0.08641;
    p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }

  // Each side is 48mm long with 0 mm-depth
  for (double y = 0.001; y <= 0.048; y += 0.005) {
    geometry_msgs::msg::Point p;
    p.x = 0.0;
    p.y = 0.119 + y;
    p.z = 0.0;
    ideal_cloud_.push_back(p);
    p.y = -0.119 - y;
    ideal_cloud_.insert(ideal_cloud_.begin(), p);
  }*/

//triangle small TinyRB
  for (double y = -0.096; y < 0; y += 0.001) {
    geometry_msgs::msg::Point p;
    p.x = (0.05527/0.095725)*y + 0.05527;
    p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }
  for (double y = 0; y <= 0.096; y += 0.001) {
    geometry_msgs::msg::Point p;
    p.x = (-0.05527/0.095725)*y + 0.05527;
    p.z = 0.0;
    p.y = y;
    ideal_cloud_.push_back(p);
    front_cloud_.push_back(p);
  }

  // Each side is 22mm long with 0 mm-depth
  for (double y = 0.001; y <= 0.022; y += 0.001) {
    geometry_msgs::msg::Point p;
    p.x = 0.0;
    p.y = 0.096 + y;
    p.z = 0.0;
    ideal_cloud_.push_back(p);
    p.y = -0.096 - y;
    ideal_cloud_.insert(ideal_cloud_.begin(), p);
  }


/*
  char str_all[10000] = "";
  for (long unsigned int i =0; i <ideal_cloud_.size();i++)
  {
    char str_c[800] = "";
    sprintf(str_c,"(%f,%f),",ideal_cloud_[i].x,ideal_cloud_[i].y);
    strcat(str_all,str_c);
  }
  strcat(str_all,"\n"); 
  RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str_all);*/

  /*
    // Debugging publishers first
    if (debug_) {
      debug_points_ =
          nh.advertise<sensor_msgs::msg::PointCloud2>("dock_points", 10);
    }
  */
  // Init base scan only after publishers are created
  scan_sub_ = node_ptr_->create_subscription<sensor_msgs::msg::LaserScan>(
      "scan", 10, std::bind(&DockPerception::callback, this, _1));
  std::cout << "Dock perception initialized\n";
}

bool DockPerception::start(const geometry_msgs::msg::PoseStamped& pose) {
  running_ = false;
  found_dock_ = false;
  dock_ = pose;
  running_ = true;

  return true;
}

bool DockPerception::stop() {
  running_ = false;
  return true;
}

bool DockPerception::getPose(geometry_msgs::msg::PoseStamped& pose,
                             std::string frame) {
  // All of this requires a lock on the dock_
  std::lock_guard<std::mutex> lock(dock_mutex_);

  if (!found_dock_) {
    return false;
  }

  // TODO: add in timeout function.
  /*
  if (ros::Time::now() > dock_stamp_ + ros::Duration(0.35)) {
    ROS_DEBUG_NAMED("dock_perception", "Dock pose timed out");
    return false;
  }
*/
  // Check for a valid orientation.
  tf2::Quaternion q;
  // change here
  tf2::fromMsg(dock_.pose.orientation, q);
  if (!isValid(q)) {
    std::cout << "Dock orientation invalid.\n";
    return false;
  }

  pose = dock_;

  if (frame != "") {
    // Transform to desired frame
    try {
      listener_.waitTransform(frame, pose.header.frame_id);
      listener_.transformPose(frame, pose, pose);
    } catch (const tf2::TransformException& ex) {
      std::cout << "Couldn't transform dock pose\n";
      return false;
    }
  }

  return found_dock_;
}

void DockPerception::callback(
    const sensor_msgs::msg::LaserScan::SharedPtr scan) {
  // Be lazy about search
  if (!running_) {
    return;
  }
  // lock dock_ to prevent other functions from modifying dock_
  std::lock_guard<std::mutex> lock(dock_mutex_);
  // Make sure goal is valid (orientation != 0 0 0 0)
  if (dock_.header.frame_id == "" ||
      (dock_.pose.orientation.z == 0.0 && dock_.pose.orientation.w == 0.0)) {
    // dock_ is of type geometry_msgs::msg::PoseStamped
    // If goal is invalid, set to a point directly ahead of robot
    dock_.header = scan->header;
    for (size_t i = scan->ranges.size() / 2; i < scan->ranges.size(); i++) {
      if (std::isfinite(scan->ranges[i])) {
        double angle = scan->angle_min + i * scan->angle_increment;
        dock_.pose.position.x = cos(angle) * scan->ranges[i];
        dock_.pose.position.y = sin(angle) * scan->ranges[i];
        dock_.pose.orientation.x = 1.0;
        dock_.pose.orientation.y = 0.0;
        dock_.pose.orientation.z = 0.0;
        dock_.pose.orientation.w = 0.0;
        break;
      }
    }
  }

  // Make sure goal is in the tracking frame
  if (dock_.header.frame_id != tracking_frame_) {
    try {
      // wait for the transform between the frame the dock is currently
      // referenced to and the main reference frame
      listener_.waitTransform(tracking_frame_, dock_.header.frame_id);

      listener_.transformPose(tracking_frame_, dock_, dock_);
    } catch (const tf2::TransformException& ex) {
      std::cout << "Couldn't transform dock pose to tracking frame";
      return;
    }
    // TODO: Add in ROS2 debug info
    // ROS_DEBUG_NAMED("dock_perception",
    //              "Transformed initial pose to (%f, %f, %f)",
    //            dock_.pose.position.x, dock_.pose.position.y,
    //          icp_2d::thetaFromQuaternion(dock_.pose.orientation));
  }

  // Cluster the laser scan
  laser_processor::ScanMask mask;
  laser_processor::ScanProcessor processor(*scan, mask);
  processor.splitConnected(point_cloud_split_dist);// Default 0.04 TODO(enhancement) parameterize
  processor.removeLessThan(minimum_point_cloud);
 /* char str2[80] = "";
  sprintf(str2, "cluster no = %d",processor.getClusters().size());
  RCLCPP_INFO(rclcpp::get_logger("rclcpp"),str2);
  */

  // Sort clusters based on distance to last dock
  std::priority_queue<DockCandidatePtr, std::vector<DockCandidatePtr>,
                      CompareCandidates> 
    candidates;
  for (std::list<laser_processor::SampleSet*>::iterator i =
           processor.getClusters().begin();
       i != processor.getClusters().end(); i++) {
    DockCandidatePtr c = extract(*i);
    //if (c && c->valid(found_dock_)) 
    {
      candidates.push(c);
    }
  }

 /* char str1[80] = "";
  sprintf(str1, "candidate number = %d",candidates.size());
   RCLCPP_INFO(rclcpp::get_logger("rclcpp"),str1);
*/
  // Extract ICP pose/fit on best clusters
  DockCandidatePtr best;
  geometry_msgs::msg::Pose best_pose;
  int index = 0;
  int no_candidate = candidates.size();
  double best_score = 999.0;
  while (!candidates.empty()) {
    
     /*char str_all[10000] = "";
     	for (size_t i =0; i < candidates.top()->points.size();i++)
     	{
     		char str_c[800] = "";
    		sprintf(str_c,"(%f,%f),",candidates.top()->points[i].x,candidates.top()->points[i].y);
    		strcat(str_all,str_c);
    	} 
    	strcat(str_all,"\n"); 
  	RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str_all);*/
    
    
    
    geometry_msgs::msg::Pose pose = dock_.pose;
    double score = fit(candidates.top(), pose);
    index++;
    char str[80] = "";
    /*sprintf(str, "candidate %d-%d fit = %f\n",index,no_candidate,score);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str);*/
    /*if (score >= 0) {
      best = candidates.top();
      best_pose = pose;
      break;
    }?*/
    

    if ((score < best_score)&&(score > 0))
    {
      best = candidates.top();
      best_pose = pose;
      best_score = score;
      sprintf(str, "best candidate %d-%d fit = %f, x =%f y=%f",index,no_candidate,score,best_pose.position.x,best_pose.position.y);
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str);
      
      
    
    }
    /*  TODO: Add in pointcloud visualisation feature in ros2
         else  // Let's see what's wrong with this point cloud.
        {
          if (debug_) {
            DockCandidatePtr not_best = candidates.top();

            // Create point cloud
            sensor_msgs::msg::PointCloud2 cloud;
            cloud.header.stamp = scan->header.stamp;
            cloud.header.frame_id = tracking_frame_;
            cloud.width = cloud.height = 0;

            // Allocate space for points
            sensor_msgs::PointCloud2Modifier cloud_mod(cloud);
            cloud_mod.setPointCloud2FieldsByString(1, "xyz");
            cloud_mod.resize(not_best->points.size());

            // Fill in points
            sensor_msgs::PointCloud2Iterator<float> cloud_iter(cloud, "x");
            for (size_t i = 0; i < not_best->points.size(); i++) {
              cloud_iter[0] = not_best->points[i].x;
              cloud_iter[1] = not_best->points[i].y;
              cloud_iter[2] = not_best->points[i].z;
              ++cloud_iter;
            }
            debug_points_.publish(cloud);
          }
        }
       */
    candidates.pop();
  }
  // Did we find dock?
  if ((best.use_count() == 0)||(best_score < 0)||(best_score >= 999.0)) {
    std::cout << "DID NOT FIND THE DOCK!\n";
    return;
  }
  
    
  
  /*
   // Update
   if (debug_) {
     // Create point cloud
     sensor_msgs::msg::PointCloud2 cloud;
     cloud.header.stamp = scan->header.stamp;
     cloud.header.frame_id = tracking_frame_;
     cloud.width = cloud.height = 0;

     // Allocate space for points
     sensor_msgs::PointCloud2Modifier cloud_mod(cloud);
     cloud_mod.setPointCloud2FieldsByString(1, "xyz");
     cloud_mod.resize(best->points.size());

     // Fill in points
     sensor_msgs::PointCloud2Iterator<float> cloud_iter(cloud, "x");
     for (size_t i = 0; i < best->points.size(); i++) {
       cloud_iter[0] = best->points[i].x;
       cloud_iter[1] = best->points[i].y;
       cloud_iter[2] = best->points[i].z;
       ++cloud_iter;
     }
     debug_points_.publish(cloud);
   }
*/

  // Update stamp
  dock_.header.stamp = scan->header.stamp;
  dock_.header.frame_id = tracking_frame_;

  // If this is the first time we've found dock, take whole pose
  if (!found_dock_) {
    dock_.pose = best_pose;
    // Reset the dock pose filter.
    dock_pose_filter_->reset();
    // Set the filter state to the current pose estimate.
    dock_pose_filter_->setFilterState(dock_.pose, dock_.pose);
  } else {
    // Check that pose is not too far from current pose
    double d = getPoseDistance(dock_.pose, best_pose);
    if (d > 0.05) {
      std::cout << "Dock pose jumped: " << d << "\n";
      //return;
    }
  }

  // Filter the pose esitmate.
  //dock_.pose = dock_pose_filter_->filter(best_pose);
  dock_.pose = best_pose;
  dock_stamp_ = scan->header.stamp;
  found_dock_ = true;
  std::cout << "found dock x= " << dock_.pose.position.x <<" y= "<< dock_.pose.position.y <<"\n";
}

DockCandidatePtr DockPerception::extract(laser_processor::SampleSet* cluster) {
  DockCandidatePtr candidate = std::make_shared<DockCandidate>();

  tf2::Vector3 tf_point;
  geometry_msgs::msg::TransformStamped t_frame;
  tf2::Stamped<tf2::Transform> tf2_t_frame;
  try {
    listener_.waitTransform(tracking_frame_, cluster->header.frame_id);

    t_frame = listener_.getTransform(tracking_frame_, cluster->header.frame_id);
  } catch (const tf2::TransformException& ex) {
    std::cout << "ERROR. COULD NOT TRANSFORM POINT\n";
    return candidate;
  }
  // Transform each point into tracking frame
  size_t i = 0;
  for (laser_processor::SampleSet::iterator p = cluster->begin();
       p != cluster->end(); p++, i++) {
    geometry_msgs::msg::PointStamped pt;
    pt.header = cluster->header;

    tf2::fromMsg(t_frame, tf2_t_frame);
    tf2::Vector3 tf2_point((*p)->x, (*p)->y, 0);
    // apply transform operation
    tf2_point = tf2_t_frame * tf2_point;
    pt.point.x = tf2_point.getX();
    pt.point.y = tf2_point.getY();
    pt.point.z = 0;
    candidate->points.push_back(pt.point);
  }

  // Get distance from cloud center to previous pose
  geometry_msgs::msg::Point centroid = icp_2d::getCentroid(candidate->points);
  double dx = centroid.x - dock_.pose.position.x;
  double dy = centroid.y - dock_.pose.position.y;
  candidate->dist = (dx * dx + dy * dy);

  return candidate;
}

double DockPerception::fit(const DockCandidatePtr& candidate,
                           geometry_msgs::msg::Pose& pose) {
  // Setup initial pose
  geometry_msgs::msg::Transform transform;
  transform.translation.x = pose.position.x;
  transform.translation.y = pose.position.y;
  transform.rotation = pose.orientation;

  tf2::Quaternion yaw_converter;

  // Initial yaw. Presumably the initial goal orientation estimate.
  tf2::Quaternion init_pose, cand_pose;
  tf2::fromMsg(pose.orientation, init_pose);
  if (!isValid(init_pose)) {
    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                 "Initial dock orientation estimate is invalid.");
    return -1.0;
  }


  // ICP the dock
  double fitness = icp_2d::alignICP(ideal_cloud_, candidate->points, transform);
  
  
  
  /*char str[80] = "";
  sprintf(str, "fit1-Fitness = %f", fitness);
  RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str);*/
  
  tf2::fromMsg(transform.rotation, cand_pose);

  if (!isValid(cand_pose)) {
    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                 "Dock candidate orientation estimate is invalid.");
  }

  // If the dock orientation seems flipped, flip it.
  // Check it by finding the relative roation between the two quaternions.
  
  /*if (fabs(angles::normalize_angle(tf2::getYaw(
          cand_pose.inverse() * init_pose))) > 3.1415 * (2.0 / 3.0)) {
    double yaw_angle = 3.1415 + tf2::getYaw(transform.rotation);
    yaw_converter.setRPY(0, 0, yaw_angle);

    transform.rotation.x = yaw_converter.x();
    transform.rotation.y = yaw_converter.y();
    transform.rotation.z = yaw_converter.z();
    transform.rotation.w = yaw_converter.w();
  }*/

  if (fitness >= 0.0) {
    // Initialize the number of times we retry if the fitness is bad.
    double retry = 1;
    // If the fitness is hosed or the angle is borked, try again.
    tf2::fromMsg(transform.rotation, cand_pose);
    while (retry-- && (fitness > max_alignment_error_ ||
                       fabs(angles::normalize_angle(tf2::getYaw(
                           cand_pose.inverse() * init_pose))) > 3.1415 / 4.0)) {
      // Try one more time.

      // Perturb the pose to try to get it out of the local minima.
      transform.translation.x +=
          retry * (0.75 / 100.0) * static_cast<double>((rand() % 200) - 100);
      transform.translation.y +=
          retry * (0.75 / 100.0) * static_cast<double>((rand() % 200) - 100);
      yaw_converter.setRPY(
          0, 0,
          retry * (0.28 / 100.0) * double((rand() % 200) - 100) +
              tf2::getYaw(transform.rotation));

      transform.rotation.x = yaw_converter.x();
      transform.rotation.y = yaw_converter.y();
      transform.rotation.z = yaw_converter.z();
      transform.rotation.w = yaw_converter.w();

      // Rerun ICP.
      fitness = icp_2d::alignICP(ideal_cloud_, candidate->points, transform);
      //char str[80] = "";
      /*sprintf(str, "fit2-Fitness = %f", fitness);
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"%s",str);*/

      // If the dock orientation seems flipped, flip it.
    /*  tf2::fromMsg(transform.rotation, cand_pose);
      if (fabs(angles::normalize_angle(tf2::getYaw(
              cand_pose.inverse() * init_pose))) > 3.1415 * (2.0 / 3.0)) {
        yaw_converter.setRPY(0, 0, (3.1415 + tf2::getYaw(transform.rotation)));
        transform.rotation.x = yaw_converter.x();
        transform.rotation.y = yaw_converter.y();
        transform.rotation.z = yaw_converter.z();
        transform.rotation.w = yaw_converter.w();
      } */
    }
    // If the dock orientation is still really borked, fail.
    tf2::fromMsg(transform.rotation, cand_pose);
    if (!isValid(cand_pose)) {
      RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                   "Dock candidate orientation estimate is invalid.");
      return -1.0;
    }
    if (fabs(angles::normalize_angle(
            tf2::getYaw(cand_pose.inverse() * init_pose))) > 3.1415 / 2.0) {
      //RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"err_normalize_angle");
      fitness = -1.0;
    }

    // Check that fitness is good enough
    //if (!found_dock_ && fabs(fitness) > max_alignment_error_) {
    if (fabs(fitness) > max_alignment_error_) {
      // If not, signal no fit
      //RCLCPP_INFO(rclcpp::get_logger("rclcpp"),"over max_alignment_error_");
      fitness = -1.0;
    }

    // If width of candidate is smaller than the width of dock
    // then the whole dock is not visible...
    //if (candidate->width() < 0.375) {
   
    /*RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
                    "width = %f",candidate->width());*/
    if ((candidate->width() < min_dock_width)||(candidate->width() > max_dock_width)) {  //Slamtec Dock
    //if ((candidate->width() < 0.20)||(candidate->width() >= 0.24)) {  //TinyRB Dock
      // ... and heading is unreliable when close to dock
    //  RCLCPP_ERROR(rclcpp::get_logger("rclcpp"),
    //               "Dock candidate width is unreliable");
      transform.rotation = pose.orientation;
      //fitness = 0.001234;
      fitness = -1.0;
      // Probably can use a different algorithm here, if necessary, which it
      // might not be.
    }

    // Transform ideal cloud, and store for visualization
    candidate->points = icp_2d::transform(
        ideal_cloud_, transform.translation.x, transform.translation.y,
        icp_2d::thetaFromQuaternion(transform.rotation));

    // Get pose
    pose.position.x = transform.translation.x;
    pose.position.y = transform.translation.y;
    pose.position.z = transform.translation.z;
    pose.orientation = transform.rotation;
    //std::cout << "Fitness = " << fitness << "\n";
    
    return fitness;
  }

  //RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Did not converge");
  return -1.0;
}

bool DockPerception::isValid(const tf2::Quaternion& q) {
  return 1e-3 >= fabs(1.0 - q.length());
}
