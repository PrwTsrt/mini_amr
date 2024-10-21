
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

    timer_imu_  = create_wall_timer(25ms , std::bind(&HardwareInterfaceNode::timerIMUCallback, this));
    timer_odom_ = create_wall_timer(25ms , std::bind(&HardwareInterfaceNode::timerOdomCallback, this));    
    timer_ip_   = create_wall_timer(10s  , std::bind(&HardwareInterfaceNode::timerIPCallback, this));
    timer_range_ = create_wall_timer(25ms, std::bind(&HardwareInterfaceNode::timerRangeCallback, this));
    timer_batt_ = create_wall_timer(200ms, std::bind(&HardwareInterfaceNode::timerBattCallback, this));

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

  rclcpp::TimerBase::SharedPtr timer_imu_;
  rclcpp::TimerBase::SharedPtr timer_odom_;
  rclcpp::TimerBase::SharedPtr timer_range_;
  rclcpp::TimerBase::SharedPtr timer_ip_;
  rclcpp::TimerBase::SharedPtr timer_batt_;

  sensor_msgs::msg::Imu msg_imu_;
  nav_msgs::msg::Odometry msg_odom_;
  sensor_msgs::msg::Range msg_range_left_, msg_range_center_, msg_range_right_;

  float ut_fov_;
  float ut_min_range_;
  float ut_max_range_;

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

  void timerIMUCallback()
  {
    msg_imu_.header.stamp = get_clock()->now();

    msg_imu_.angular_velocity.x = hardware_interface->angular_velocity.x;
    msg_imu_.angular_velocity.y = hardware_interface->angular_velocity.y;
    msg_imu_.angular_velocity.z = hardware_interface->angular_velocity.z;

    msg_imu_.linear_acceleration.x = hardware_interface->linear_acceleration.x;
    msg_imu_.linear_acceleration.y = hardware_interface->linear_acceleration.y;
    msg_imu_.linear_acceleration.z = hardware_interface->linear_acceleration.z;

    imu_pub_->publish(msg_imu_);
  }

  void timerOdomCallback()
  {
    msg_odom_.header.stamp = get_clock()->now();
    msg_odom_.twist.twist.linear.x = hardware_interface->odom_velocity.x;
    msg_odom_.twist.twist.linear.y = hardware_interface->odom_velocity.y;
    msg_odom_.twist.twist.angular.z = hardware_interface->odom_velocity.z;
    odom_pub_->publish(msg_odom_);
  }

  void timerRangeCallback()
  {
    msg_range_left_.header.stamp = get_clock()->now();
    msg_range_center_.header.stamp = get_clock()->now();
    msg_range_right_.header.stamp = get_clock()->now();

    msg_range_left_.range = hardware_interface->range_left;
    msg_range_center_.range = hardware_interface->range_center;
    msg_range_right_.range = hardware_interface->range_right;

    range_left_pub_->publish(msg_range_left_);
    range_center_pub_->publish(msg_range_center_);
    range_right_pub_->publish(msg_range_right_);
  }

  void timerBattCallback()
  {
    auto msg_batt = sensor_msgs::msg::BatteryState();

    msg_batt.voltage = hardware_interface->voltage_;
    msg_batt.current = hardware_interface->current_;

    batt_pub_->publish(msg_batt);
  }

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
