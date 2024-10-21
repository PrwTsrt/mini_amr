import rclpy
from rclpy.node import Node

from geometry_msgs.msg import Twist
from std_msgs.msg import Float32, Int32

from geometry_msgs.msg import Twist,Pose2D,TransformStamped
from rclpy.time import Time

import math
from nav_msgs.msg import Odometry
from geometry_msgs.msg import Pose,Point,Quaternion,Vector3,PoseWithCovarianceStamped

from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

from sensor_msgs.msg import LaserScan
import tf2_ros

import numpy as np

class AbsoluteScan(Node):
    def __init__(self):
        super().__init__('absolute_scan_node')
        self.tf_broadcaster = tf2_ros.TransformBroadcaster(self)
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.target_frame = 'map'
        self.laser_frame = 'laser_frame'
        self.base_frame = 'base_link'
        self.timer_tf = self.create_timer(0.1, self.timer_tf_callback)
        self.scan_sub = self.create_subscription(LaserScan,'scan/absolute_origin',self.laser_callback,10)
        self.scan_pub = self.create_publisher(LaserScan,'scan/absolute_origin',10)
        
    
    def timer_tf_callback(self):
        try:
            t = self.tf_buffer.lookup_transform(
                self.base_frame,
                self.laser_frame,
                rclpy.time.Time())
        except TransformException as ex:
            self.get_logger().info(
                f'Could not transform {self.base_frame} to {self.laser_frame}: {ex}')
            return

        pose = Vector3()
        pose.x = t.transform.translation.x
        pose.y = t.transform.translation.y

        q = [t.transform.rotation.w, t.transform.rotation.x, t.transform.rotation.y, t.transform.rotation.z]


        roll,pitch, yaw = self.euler_from_quaternion(q)
        pose.z = yaw
        

    def laser_callback(self, msg):
        print(msg.ranges)
    def quaternion_from_euler(self,roll,pitch,yaw):
        cy = math.cos(yaw*0.5)
        sy = math.sin(yaw*0.5)
        cp = math.cos(pitch*0.5)
        sp = math.sin(pitch*0.5)
        cr = math.cos(roll*0.5)
        sr = math.sin(roll*0.5)
        q = [0]*4
        q[0] = cy * cp * cr + sy * sp * sr
        q[1] = cy * cp * sr - sy * sp * cr
        q[2] = sy * cp * sr + cy * sp * cr
        q[3] = sy * cp * cr - cy * sp * sr

        return q

    def euler_from_quaternion(self, quaternion):
        x = quaternion[1]
        y = quaternion[2]
        z = quaternion[3]
        w = quaternion[0]

        sinr_cosp = 2 * (w * x + y * z)
        cosr_cosp = 1 - 2 * (x * x + y * y)
        roll = np.arctan2(sinr_cosp, cosr_cosp)

        sinp = 2 * (w * y - z * x)
        pitch = np.arcsin(sinp)

        siny_cosp = 2 * (w * z + x * y)
        cosy_cosp = 1 - 2 * (y * y + z * z)
        yaw = np.arctan2(siny_cosp, cosy_cosp)

        return roll, pitch, yaw
		
		
def main():
	rclpy.init()
	
	mp = AbsoluteScan()
	rclpy.spin(mp)
	
	rclpy.shutdown()
	
if __name__ == "__main__":
	main()