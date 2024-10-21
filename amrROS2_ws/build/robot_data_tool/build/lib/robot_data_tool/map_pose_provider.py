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


import tf2_ros

import numpy as np

class MapPose(Node):
    def __init__(self):
        super().__init__('robot_map_pose_node')
        self.tf_broadcaster = tf2_ros.TransformBroadcaster(self)
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)
        self.target_frame = 'map'
        self.timer_tf = self.create_timer(0.1, self.timer_tf_callback)
        self.map_pose_pub = self.create_publisher(Vector3,'robot_map_pose/euler',10)
        self.map_odom_pub = self.create_publisher(Odometry,'robot_map_pose/odom',10)
        self.map_copose_pub = self.create_publisher(PoseWithCovarianceStamped,'robot_map_pose/pose',10)
    
    def timer_tf_callback(self):
        try:
            t = self.tf_buffer.lookup_transform(
                self.target_frame,
                'base_link',
                rclpy.time.Time())
        except TransformException as ex:
            self.get_logger().info(
                f'Could not transform {self.target_frame} to base_link: {ex}')
            self.get_logger().info('Please open NAV STACK')
            return

        pose = Vector3()
        pose.x = t.transform.translation.x
        pose.y = t.transform.translation.y

        q = [t.transform.rotation.w, t.transform.rotation.x, t.transform.rotation.y, t.transform.rotation.z]


        roll,pitch, yaw = self.euler_from_quaternion(q)
        pose.z = yaw
        self.map_pose_pub.publish(pose)

        op = Odometry()
        op.pose.pose.position.x = t.transform.translation.x 
        op.pose.pose.position.y = t.transform.translation.y
        op.pose.pose.position.z = t.transform.translation.z
        op.pose.pose.orientation.w = t.transform.rotation.w
        op.pose.pose.orientation.x = t.transform.rotation.x
        op.pose.pose.orientation.y = t.transform.rotation.y
        op.pose.pose.orientation.z = t.transform.rotation.z

        self.map_odom_pub.publish(op)


        #map pose with covariance
        mp  = PoseWithCovarianceStamped()
        mp.pose.pose.position.x = t.transform.translation.x 
        mp.pose.pose.position.y = t.transform.translation.y
        mp.pose.pose.position.z = t.transform.translation.z
        mp.pose.pose.orientation.w = t.transform.rotation.w
        mp.pose.pose.orientation.x = t.transform.rotation.x
        mp.pose.pose.orientation.y = t.transform.rotation.y
        mp.pose.pose.orientation.z = t.transform.rotation.z

        self.map_copose_pub.publish(mp)

    
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
	
	mp = MapPose()
	rclpy.spin(mp)
	
	rclpy.shutdown()
	
if __name__ == "__main__":
	main()