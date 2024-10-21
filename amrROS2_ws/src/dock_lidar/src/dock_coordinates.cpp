
#include <dock_lidar/perception.h>
#include <tf2_ros/transform_broadcaster.h>

#include <chrono>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <iostream>
#include <rclcpp/rclcpp.hpp>

#include "custom_interface/msg/initdock.hpp"
#include "dock_lidar/tf2listener.h"
#include "sensor_msgs/msg/joy.hpp"
#include <string>
#include "std_msgs/msg/string.hpp"
//#include "rclcpp_lifecycle/lifecycle_node.hpp"

using namespace std::chrono_literals;

class DockCoordinates : public rclcpp::Node
{
public:
  DockCoordinates() : Node("dock_coordinates"), tf2_listen(this->get_clock())
  {
    RCLCPP_INFO(this->get_logger(), "begin dock_coordinates");
    this->declare_parameter("point_cloud_split_dist", 0.1);
    this->declare_parameter("minimum_point_cloud", 5);
    this->declare_parameter("max_alignment_error", 0.010);
    this->declare_parameter("max_dock_width", 0.4);
    this->declare_parameter("min_dock_width", 0.3);
    point_cloud_split_dist = this->get_parameter("point_cloud_split_dist").as_double();
    minimum_point_cloud = this->get_parameter("minimum_point_cloud").as_int();
    max_alignment_error = this->get_parameter("max_alignment_error").as_double();
    max_dock_width = this->get_parameter("max_dock_width").as_double();
    min_dock_width = this->get_parameter("min_dock_width").as_double();
    RCLCPP_INFO(rclcpp::get_logger("dock coordinates"), "max_alignment_error: %f", max_alignment_error);
    RCLCPP_INFO(rclcpp::get_logger("dock coordinates"), "minimum_point_cloud: %d", minimum_point_cloud);
    RCLCPP_INFO(rclcpp::get_logger("dock coordinates"), "point_cloud_split_dist: %f", point_cloud_split_dist);
    RCLCPP_INFO(rclcpp::get_logger("dock coordinates"), "max_dock_width: %f", max_dock_width);
    RCLCPP_INFO(rclcpp::get_logger("dock coordinates"), "min_dock_width: %f", min_dock_width);
    
    tbr = std::make_shared<tf2_ros::TransformBroadcaster>(this);
    publisher_ = this->create_publisher<custom_interface::msg::Initdock>(
        "init_dock", 10);
    //this->declare_parameter<int>("reset_goal_button", 3);
    //this->get_parameter("reset_goal_button", reset_goal_button);
    control_dock_sub_ = this->create_subscription<std_msgs::msg::String>(
        "command_dock", 10, [this](const std_msgs::msg::String::SharedPtr msg)
        {
          //std::vector<int> pressed_buttons = msg->buttons;
          RCLCPP_INFO(this->get_logger(), "I heard: '%s'", msg->data.c_str());
          
          if (strcmp(msg->data.c_str(),"shutdown") == 0) {
            RCLCPP_INFO(this->get_logger(), "shutdown dock estimate");
            this->found_dockk = false;
            this->perception_ptr->stop();
            is_execute = false;
          } 
          else if (strcmp(msg->data.c_str(),"start") == 0)//start
          {
            RCLCPP_INFO(this->get_logger(), "start dock estimate");
            update_init_dock(init_dock_pose);
            perception_ptr->start(init_dock_pose);
            is_execute = true;
          }
        
          
          });
  }

  // the init_objects function would return a shared_ptr to this class. It will
  // be stored in new_ptr before being passed to
  // the init_node function of Sepclass to initialise its memeber functions.
  void init_objects()
  {
    std::shared_ptr<rclcpp::Node> new_ptr = shared_ptr_from_this();
    perception_ptr = std::make_shared<DockPerception>(new_ptr);
    this->perception_ptr->point_cloud_split_dist = point_cloud_split_dist;
    this->perception_ptr->minimum_point_cloud = minimum_point_cloud;
    this->perception_ptr->max_alignment_error_ = max_alignment_error;
    this->perception_ptr->max_dock_width = max_dock_width;
    this->perception_ptr->min_dock_width = min_dock_width;
    
  }

  // shared_ptr_from_this would return a shared pointer of the current class
  std::shared_ptr<rclcpp::Node> shared_ptr_from_this()
  {
    return shared_from_this();
  }
  // debugging function to print out the initial dock pose
  void print_idp(geometry_msgs::msg::PoseStamped idp)
  {
    std::cout << "init x: " << idp.pose.position.x << "\n";
    std::cout << "init y: " << idp.pose.position.y << "\n";
    std::cout << "init z: " << idp.pose.orientation.z << "\n";
    std::cout << "init w: " << idp.pose.orientation.w << "\n";
  }

  // this function would update the initial dock pose to be 1m from robot.
  // wrt map frame.
  void update_init_dock(geometry_msgs::msg::PoseStamped &idp)
  {
    tf2_listen.waitTransform("odom", "base_link");
    geometry_msgs::msg::PoseStamped fake_dock;
    // take it that the fake dock is 1m in front of the robot.
    fake_dock.header.frame_id = "base_link";
    fake_dock.pose.position.x = 1;
    // we will transform fake_dock wrt map
    tf2_listen.transformPose("odom", fake_dock, idp);
  }

  void main_test()
  {
    
    //update_init_dock(init_dock_pose);
    //perception_ptr->start(init_dock_pose);
    
    timer_ = this->create_wall_timer(20ms, [this]()
                                     { 
      if (is_execute == false)
          return;
      // if no dock is found yet, call start function with init_dock_pose to let
      // perception assume the dock is 1m ahead of the bot.
      if (this->perception_ptr->getPose(this->dock_pose, "odom") == false &&
          this->found_dockk == false) {
        std::cout << "still finding dock!\n";
        this->update_init_dock(this->init_dock_pose);
        this->perception_ptr->start(this->init_dock_pose);

      } else {
        this->found_dockk = true;

        // publish the transformations of the dock
        auto dock_pose_msg = custom_interface::msg::Initdock();
        dock_pose_msg.x = this->dock_pose.pose.position.x;
        dock_pose_msg.y = this->dock_pose.pose.position.y;
        dock_pose_msg.z = this->dock_pose.pose.orientation.z;
        dock_pose_msg.w = this->dock_pose.pose.orientation.w;
        publisher_->publish(dock_pose_msg);
        // Also, send the transform for visualisation
        this->time_now = rclcpp::Clock().now();

        this->transformStamped.header.stamp = this->time_now;
        this->transformStamped.header.frame_id = "odom";
        this->transformStamped.child_frame_id = "dock";
        this->transformStamped.transform.translation.x =
            this->dock_pose.pose.position.x;
        this->transformStamped.transform.translation.y =
            this->dock_pose.pose.position.y;
        this->transformStamped.transform.translation.z = 0.0;
        this->transformStamped.transform.rotation.x = 0;
        this->transformStamped.transform.rotation.y = 0;
        this->transformStamped.transform.rotation.z =
            this->dock_pose.pose.orientation.z;
        this->transformStamped.transform.rotation.w =
            this->dock_pose.pose.orientation.w;

        this->tbr->sendTransform(this->transformStamped);
      } });
  }

private:
  int reset_goal_button;
  std::shared_ptr<DockPerception> perception_ptr;
  std::shared_ptr<tf2_ros::TransformBroadcaster> tbr;
  rclcpp::Publisher<custom_interface::msg::Initdock>::SharedPtr publisher_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr control_dock_sub_;
  tf2_listener tf2_listen;

  rclcpp::Time time_now;
  geometry_msgs::msg::TransformStamped transformStamped;
  geometry_msgs::msg::PoseStamped dock_pose;
  geometry_msgs::msg::PoseStamped init_dock_pose;
  bool found_dockk = false;
  rclcpp::TimerBase::SharedPtr timer_;

  bool is_execute = false;

  double point_cloud_split_dist;
  int minimum_point_cloud;
  double max_alignment_error;
  double max_dock_width;
  double min_dock_width;

};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  std::shared_ptr<DockCoordinates> min_ptr =
      std::make_shared<DockCoordinates>();

  min_ptr->init_objects();
  min_ptr->main_test();
  rclcpp::spin(min_ptr);
  rclcpp::shutdown();
  return 0;
}
