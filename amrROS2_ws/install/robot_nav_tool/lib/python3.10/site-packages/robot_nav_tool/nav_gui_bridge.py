import rclpy 
from rclpy.node import Node
from sensor_msgs.msg import BatteryState
from rclpy.time import Time
from geometry_msgs.msg import PoseStamped,PoseWithCovarianceStamped, Vector3, Twist
from rclpy.duration import Duration
from std_msgs.msg import String, Int8
from rclpy.qos import qos_profile_sensor_data, qos_profile_system_default
#from .robot_navigator import Mission
from .robot_nav_operator import Mission

import json

import numpy as np

class NavGUIBridge(Node):
    def __init__(self):
        super().__init__('nav_gui_bridge')

        self.command_sub = self.create_subscription(Vector3, 'nav_p2p/setpoint', self.nsetpoint_callback, qos_profile_system_default)
        self.go_sub = self.create_subscription(Int8, 'nav_p2p/go', self.ngo_callback, 10)
        self.named_sub = self.create_subscription(String, 'nav_p2p/named_target', self.nnamed_callback, 10)
        
        self.setpoint_pub = self.create_publisher(Vector3, 'nav_p2p/setpoint',qos_profile_system_default)
        self.twist_sub = self.create_subscription(Twist, 'cmd_vel', self.vel_callback, 10)
        self.workstatus_pub = self.create_publisher(String, 'robot_status/working',10)


        self.operator = Mission()

        self.navp2p_data = [0.0,0.0,0.0]
        self.named_target = "home"

        self.node_config_path = "/home/smr/workspaces/amrROS2_ws/src/robot_nav_tool/json/named_point.json"
        self.node_config = {}
        self.goal_point = Vector3()

        print("The Bridge is Activated")

    def vel_callback(self,msg):
        v = msg.linear.x
        w = msg.angular.z

        status = String()
        if v == 0:
            if w == 0:
                 status.data = "Waiting"
            elif w!=0:
                 status.data = "Turning"
        else:
            status.data = "Moving"

        self.workstatus_pub.publish(status)

    def read_node_json(self):
        f = open(self.node_config_path)
        data = json.load(f)
        self.node_config = data
    
    def nsetpoint_callback(self, msg):
        self.navp2p_data = [msg.x, msg.y, msg.z]

    def nnamed_callback(self, msg):
        self.read_node_json()
        self.named_target = msg.data

        if self.named_target in self.node_config:
            print(self.named_target)
            p = self.node_config[self.named_target]

            point = Vector3()

            point.x = p[0]
            point.y = p[1]
            point.z = p[2]
            self.goal_point = point

            self.setpoint_pub.publish(point)

    def ngo_callback(self, msg):
        if msg.data == 1:
            self.operator.startGoal(self.navp2p_data)

def main():
	rclpy.init()
	nb = NavGUIBridge()
	rclpy.spin(nb)



if __name__ == "__main__":
	main()
