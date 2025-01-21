
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/range.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "action_msgs/msg/goal_status_array.hpp"
#include <sensor_msgs/msg/battery_state.hpp>

#include <chrono>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <math.h>

#include "hardware_interface/robot_hardware_interface.h"

using std::placeholders::_1;
using namespace std::literals::chrono_literals;
using namespace std;

class HardwareInterfaceNode : public rclcpp::Node
{
public:
  HardwareInterfaceNode() : Node("hardware_interface")
  {
    declare_parameter("serial_port", "/dev/teensy");
    serial_port_ = get_parameter("serial_port").as_string();
    hardware_interface = std::make_shared<HardwareInterface>(serial_port_);

    cmd_vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 10, std::bind(&HardwareInterfaceNode::twistCallback, this, _1));

    nav_status_sub_ = create_subscription<action_msgs::msg::GoalStatusArray>(
        "navigate_to_pose/_action/status", 10, std::bind(&HardwareInterfaceNode::statusCallback, this, _1));

    odom_pub_   = create_publisher<nav_msgs::msg::Odometry>("odom_raw", 10);
    timer_update_data_ = create_wall_timer(1ms  , std::bind(&HardwareInterfaceNode::timerUpdateCallback, this));

    msg_odom_.header.frame_id = "odom_frame";
    msg_odom_.child_frame_id  = "base_footprint";
    msg_odom_.twist.covariance[0] = 0.0001;
    msg_odom_.twist.covariance[7] = 0.0001;
    msg_odom_.twist.covariance[35] = 0.0001;

  }

private:

  std::string serial_port_;
  std::shared_ptr<HardwareInterface> hardware_interface;
  
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Subscription<action_msgs::msg::GoalStatusArray>::SharedPtr nav_status_sub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;

  rclcpp::TimerBase::SharedPtr timer_update_data_;
  nav_msgs::msg::Odometry msg_odom_;

  float ut_fov_;
  float ut_min_range_;
  float ut_max_range_;

  double pos_x_;
  double pos_y_;
  double heading_;

  double yaw_;

  uint64_t prev_update_;
  uint64_t imu_prev_update_;

  void odom_euler_to_quat(float roll, float pitch, float yaw, float *q)
  {
    float cy = cos(yaw * 0.5);
    float sy = sin(yaw * 0.5);
    float cp = cos(pitch * 0.5); //1
    float sp = sin(pitch * 0.5); //0
    float cr = cos(roll * 0.5);  //1
    float sr = sin(roll * 0.5);  //0

    q[0] = cy * cp * cr + sy * sp * sr;
    q[1] = cy * cp * sr - sy * sp * cr; //x    cy * 1 * 0 - sy * 0 * 
    q[2] = sy * cp * sr + cy * sp * cr; //y
    q[3] = sy * cp * cr - cy * sp * sr; //z
  }

  void twistCallback(const geometry_msgs::msg::Twist & msg)
  {
    hardware_interface->SetMotion(msg.linear.x, 0.0, msg.angular.z);
  }

  void statusCallback(const action_msgs::msg::GoalStatusArray & msg)
  {
    if (!msg.status_list.empty()) {
      auto latest_status = msg.status_list.back();
      int status = latest_status.status;
      RCLCPP_INFO(this->get_logger(), "Updated status: %d", status);
      
      hardware_interface->UpdateStatus(status);
    }
  }

  void timerUpdateCallback()
  {
    auto current_time = get_clock()->now();
    if (hardware_interface->update_odom_)
    {
      msg_odom_.header.stamp = current_time;

      uint64_t dt = current_time.nanoseconds() - prev_update_;
      double dt_seconds = static_cast<double>(dt) / 1000000000.0f;

      double delta_heading = static_cast<double>(hardware_interface->odom_velocity.z) * dt_seconds; // radians
      double cos_h = cos(heading_);
      double sin_h = sin(heading_);
      double delta_x = (static_cast<double>(hardware_interface->odom_velocity.x) * cos_h - static_cast<double>(hardware_interface->odom_velocity.y) * sin_h) * dt_seconds; // m
      double delta_y = (static_cast<double>(hardware_interface->odom_velocity.x) * sin_h + static_cast<double>(hardware_interface->odom_velocity.y) * cos_h) * dt_seconds; // m

      pos_x_ += delta_x;
      pos_y_ += delta_y;
      heading_ += delta_heading;

      float q[4];
      odom_euler_to_quat(0.0, 0.0, static_cast<float>(heading_), q);

      msg_odom_.pose.pose.position.x = pos_x_;
      msg_odom_.pose.pose.position.y = pos_y_;
      msg_odom_.pose.pose.position.z = 0.0;

      msg_odom_.pose.pose.orientation.x = (double)q[1];
      msg_odom_.pose.pose.orientation.y = (double)q[2];
      msg_odom_.pose.pose.orientation.z = (double)q[3];
      msg_odom_.pose.pose.orientation.w = (double)q[0];

      msg_odom_.twist.twist.linear.x = hardware_interface->odom_velocity.x;
      msg_odom_.twist.twist.linear.y = hardware_interface->odom_velocity.y;
      msg_odom_.twist.twist.angular.z = hardware_interface->odom_velocity.z;

      hardware_interface->update_odom_ = false;
      odom_pub_->publish(msg_odom_);

      prev_update_ = current_time.nanoseconds();
    }
  }
};


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<HardwareInterfaceNode>());
  rclcpp::shutdown();
  return 0;
}
