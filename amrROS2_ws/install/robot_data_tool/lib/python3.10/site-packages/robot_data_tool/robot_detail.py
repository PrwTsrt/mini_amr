import rclpy
from rclpy.node import Node

from std_msgs.msg import String, Float32

import tf2_ros

import numpy as np

class RobotDetail(Node):
    def __init__(self):
        super().__init__('robot_detail_node')
    
        self.name_pub = self.create_publisher(String,'robot_detail',10)
        self.battery_sub = self.create_subscription(Float32, 'battery_level', self.battery_callback, 10)
        self.battery_percent_pub = self.create_publisher(String, 'battery_percent', 10)

        self.timer1 = self.create_timer(5.0, self.timer1_callback) 
        self.robot_name = "sut_robot1"

        self.max_voltage = 12.4
        self.min_voltage = 11.3

        self.standby_stat_pub = self.create_publisher(String, 'ready_status', 10)

    
    def battery_callback(self, msg):
        bv = msg.data
        batt_range = self.max_voltage - self.min_voltage
        percent = (bv - self.min_voltage)/batt_range

        msg = String()
        msg.data = percent

        self.battery_percent_pub.publish(msg)

    def timer1_callback(self):
        name_msg = String()
        name_msg.data = self.robot_name

        self.name_pub.publish(name_msg)

        ready_msg = String()
        ready_msg.data = "ROBOT IS OK"
        self.standby_stat_pub.publish(ready_msg)
		
def main():
	rclpy.init()
	
	rd = RobotDetail()
	rclpy.spin(rd)
	
	rclpy.shutdown()
	
if __name__ == "__main__":
	main()