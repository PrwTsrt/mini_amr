#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

class Publish_Robot_Pose(Node):

    def __init__(self):
        super().__init__('publish_robot_pose')
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        self.publisher = self.create_publisher(PoseStamped, 'robot_pose', 10)
        timer_period = 0.1 #second
        self.timer = self.create_timer(timer_period, self.timer_callback)

    def timer_callback(self):
        try:
            transf_stamped = self.tf_buffer.lookup_transform('map', 'base_link', rclpy.time.Time())
            self.publisher.publish( transf_stamped )
        except TransformException as ex:
            self.get_logger().info(f'Cound not transform base_link to map!')


def main(args=None):
    rclpy.init(args=args)
    publish_robot_pose = Publish_Robot_Pose()

    rclpy.spin(publish_robot_pose)

    publish_robot_pose.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()

