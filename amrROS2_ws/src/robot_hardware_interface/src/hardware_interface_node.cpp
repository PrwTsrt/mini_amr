
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
    declare_parameter("serial_port", "/dev/ttyUSB0");
    serial_port_ = get_parameter("serial_port").as_string();
    hardware_interface = std::make_shared<HardwareInterface>(serial_port_);

    cmd_vel_sub_ = create_subscription<geometry_msgs::msg::Twist>(
        "cmd_vel", 10, std::bind(&HardwareInterfaceNode::twistCallback, this, _1));

    nav_status_sub_ = create_subscription<action_msgs::msg::GoalStatusArray>(
        "navigate_to_pose/_action/status", 10, std::bind(&HardwareInterfaceNode::statusCallback, this, _1));

    imu_pub_    = create_publisher<sensor_msgs::msg::Imu>("imu/data_raw", 10);
    odom_pub_   = create_publisher<nav_msgs::msg::Odometry>("odom_raw", 10);
    batt_pub_   = create_publisher<sensor_msgs::msg::BatteryState>("battery", 10);

    range_left_pub_   = create_publisher<sensor_msgs::msg::Range>("range/left", 10);
    range_center_pub_ = create_publisher<sensor_msgs::msg::Range>("range/center", 10);
    range_right_pub_  = create_publisher<sensor_msgs::msg::Range>("range/right", 10);

  /////////////////////////////////////////////////////////////////////////////////////////
    // timer_imu_  = create_wall_timer(25ms , std::bind(&HardwareInterfaceNode::timerIMUCallback, this));
    // timer_odom_ = create_wall_timer(25ms , std::bind(&HardwareInterfaceNode::timerOdomCallback, this));    
    // timer_range_ = create_wall_timer(25ms, std::bind(&HardwareInterfaceNode::timerRangeCallback, this));
    // timer_batt_ = create_wall_timer(200ms, std::bind(&HardwareInterfaceNode::timerBattCallback, this));
    // timer_ip_          = create_wall_timer(10s  , std::bind(&HardwareInterfaceNode::timerIPCallback, this));
  /////////////////////////////////////////////////////////////////////////////////////////

    timer_ip_          = create_wall_timer(10s  , std::bind(&HardwareInterfaceNode::timerIPCallback, this));
    timer_update_data_ = create_wall_timer(1ms  , std::bind(&HardwareInterfaceNode::timerUpdateCallback, this));

    ut_fov_       = 20.0;
    ut_min_range_ = 0.03;
    ut_max_range_ = 0.50;

    msg_imu_.header.frame_id = "imu_frame";
    msg_imu_.angular_velocity_covariance[0] = 0.1199;
    msg_imu_.angular_velocity_covariance[4] = 0.5753;
    msg_imu_.angular_velocity_covariance[8] = 0.0267;
    
    msg_imu_.linear_acceleration_covariance[0] = 0.0088;
    msg_imu_.linear_acceleration_covariance[4] = 0.0550;
    msg_imu_.linear_acceleration_covariance[8] = 0.0267;

    msg_odom_.header.frame_id = "odom_frame";
    msg_odom_.child_frame_id  = "base_footprint";
    msg_odom_.twist.covariance[0] = 0.0001;
    msg_odom_.twist.covariance[7] = 0.0001;
    msg_odom_.twist.covariance[35] = 0.0001;

    msg_range_left_.header.frame_id   = "left_ranger_link";
    msg_range_center_.header.frame_id = "center_ranger_link";
    msg_range_right_.header.frame_id  = "right_ranger_link";

    msg_range_left_.radiation_type   = 0;
    msg_range_center_.radiation_type = 0;
    msg_range_right_.radiation_type  = 0;

    msg_range_left_.field_of_view   = ut_fov_ * (3.14/180);
    msg_range_center_.field_of_view = ut_fov_ * (3.14/180);
    msg_range_right_.field_of_view  = ut_fov_ * (3.14/180);

    msg_range_left_.min_range   = ut_min_range_;
    msg_range_center_.min_range = ut_min_range_;
    msg_range_right_.min_range  = ut_min_range_;

    msg_range_left_.max_range   = ut_max_range_;
    msg_range_center_.max_range = ut_max_range_;
    msg_range_right_.max_range  = ut_max_range_;

  }

private:

  std::string serial_port_;
  std::shared_ptr<HardwareInterface> hardware_interface;
  
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_sub_;
  rclcpp::Subscription<action_msgs::msg::GoalStatusArray>::SharedPtr nav_status_sub_;

  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  rclcpp::Publisher<sensor_msgs::msg::BatteryState>::SharedPtr batt_pub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr range_left_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr range_center_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Range>::SharedPtr range_right_pub_;
  
/////////////////////////////////////////////////////////////////////////////////////////
  // rclcpp::TimerBase::SharedPtr timer_imu_;
  // rclcpp::TimerBase::SharedPtr timer_odom_;
  // rclcpp::TimerBase::SharedPtr timer_range_;
  // rclcpp::TimerBase::SharedPtr timer_ip_;
  // rclcpp::TimerBase::SharedPtr timer_batt_;
/////////////////////////////////////////////////////////////////////////////////////////

  rclcpp::TimerBase::SharedPtr timer_ip_;
  rclcpp::TimerBase::SharedPtr timer_update_data_;

  sensor_msgs::msg::Imu msg_imu_;
  nav_msgs::msg::Odometry msg_odom_;
  sensor_msgs::msg::Range msg_range_left_, msg_range_center_, msg_range_right_;
  sensor_msgs::msg::BatteryState msg_batt_;

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
    // RCLCPP_INFO(this->get_logger(), "Received linear x: '%f', angular z: '%f'", msg.linear.x, msg.angular.z);
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

    if (hardware_interface->update_imu_)
    {
      msg_imu_.header.stamp = current_time;

      uint64_t dt = current_time.nanoseconds() - imu_prev_update_;
      double dt_seconds = static_cast<double>(dt) / 1000000000.0;

      double imu_delta_z = static_cast<double>(hardware_interface->angular_velocity.z) * dt_seconds;
      yaw_ += imu_delta_z;

      float imu_q[4];
      odom_euler_to_quat(0.0, 0.0, static_cast<float>(yaw_), imu_q);

      msg_imu_.orientation.x = (double)imu_q[1];
      msg_imu_.orientation.y = (double)imu_q[2];
      msg_imu_.orientation.z = (double)imu_q[3];
      msg_imu_.orientation.w = (double)imu_q[0];

      msg_imu_.angular_velocity.x = hardware_interface->angular_velocity.y;
      msg_imu_.angular_velocity.y = hardware_interface->angular_velocity.x * (-1);
      msg_imu_.angular_velocity.z = hardware_interface->angular_velocity.z;

      msg_imu_.linear_acceleration.x = hardware_interface->linear_acceleration.y;
      msg_imu_.linear_acceleration.y = hardware_interface->linear_acceleration.x * (-1);
      msg_imu_.linear_acceleration.z = hardware_interface->linear_acceleration.z;

      hardware_interface->update_imu_ = false;
      imu_pub_->publish(msg_imu_);

      imu_prev_update_ = current_time.nanoseconds();
    }

    else if (hardware_interface->update_odom_)
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

    else if (hardware_interface->update_range_)
    {
      msg_range_left_.header.stamp   = current_time;
      msg_range_center_.header.stamp = current_time;
      msg_range_right_.header.stamp  = current_time;

      msg_range_left_.range = hardware_interface->range_left;
      msg_range_center_.range = hardware_interface->range_center;
      msg_range_right_.range = hardware_interface->range_right;

      hardware_interface->update_range_ = false;

      range_left_pub_->publish(msg_range_left_);
      range_center_pub_->publish(msg_range_center_);
      range_right_pub_->publish(msg_range_right_);
    }

    else if (hardware_interface->update_batt_)
    {
      msg_batt_.voltage = hardware_interface->voltage_;
      msg_batt_.current = hardware_interface->current_;

      hardware_interface->update_batt_ = false;

      batt_pub_->publish(msg_batt_);
    }
  }

  // void timerIMUCallback()
  // {
  //   msg_imu_.header.stamp = get_clock()->now();

  //   msg_imu_.angular_velocity.x = hardware_interface->angular_velocity.x;
  //   msg_imu_.angular_velocity.y = hardware_interface->angular_velocity.y;
  //   msg_imu_.angular_velocity.z = hardware_interface->angular_velocity.z;

  //   msg_imu_.linear_acceleration.x = hardware_interface->linear_acceleration.x;
  //   msg_imu_.linear_acceleration.y = hardware_interface->linear_acceleration.y;
  //   msg_imu_.linear_acceleration.z = hardware_interface->linear_acceleration.z;

  //   imu_pub_->publish(msg_imu_);
  // }

  // void timerOdomCallback()
  // {
  //   msg_odom_.header.stamp = get_clock()->now();
  //   msg_odom_.twist.twist.linear.x = hardware_interface->odom_velocity.x;
  //   msg_odom_.twist.twist.linear.y = hardware_interface->odom_velocity.y;
  //   msg_odom_.twist.twist.angular.z = hardware_interface->odom_velocity.z;
  //   odom_pub_->publish(msg_odom_);
  // }

  // void timerRangeCallback()
  // {
  //   msg_range_left_.header.stamp = get_clock()->now();
  //   msg_range_center_.header.stamp = get_clock()->now();
  //   msg_range_right_.header.stamp = get_clock()->now();

  //   msg_range_left_.range = hardware_interface->range_left;
  //   msg_range_center_.range = hardware_interface->range_center;
  //   msg_range_right_.range = hardware_interface->range_right;

  //   range_left_pub_->publish(msg_range_left_);
  //   range_center_pub_->publish(msg_range_center_);
  //   range_right_pub_->publish(msg_range_right_);
  // }

  // void timerBattCallback()
  // {
  //   auto msg_batt = sensor_msgs::msg::BatteryState();

  //   msg_batt.voltage = hardware_interface->voltage_;
  //   msg_batt.current = hardware_interface->current_;

  //   batt_pub_->publish(msg_batt);
  // }

  void timerIPCallback()
  {
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;

    if (getifaddrs(&interfaces) == -1) {
        cerr << "Error: Unable to get network interfaces." << endl;
        return ;
    }
    for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) {
            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            if (strcmp(ifa->ifa_name, "lo") != 0 && strstr(addressBuffer, "172.") == nullptr) {
                hardware_interface->UpdateIP(addressBuffer);
            }
        }
    }
    if (interfaces != nullptr) {
        freeifaddrs(interfaces);
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
