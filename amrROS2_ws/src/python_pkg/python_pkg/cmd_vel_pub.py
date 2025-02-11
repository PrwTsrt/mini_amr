import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from time import sleep


def main():

    rclpy.init()
    node = rclpy.create_node('cmd_vel_pub')
    publisher = node.create_publisher(Twist, "/cmd_vel", 1)
    msg = Twist()
    for i in range(1):
        # msg.linear.x = 0.2
        msg.linear.x = -0.2
        node.get_logger().info('Publishing: "%f"' % msg.linear.x)
        publisher.publish(msg)
        # sleep(0.05)  

    node.get_logger().info('------------------------------------------')
    msg.linear.x = 0.0
    publisher.publish(msg)
    node.get_logger().info('000000000000000000000000000000000000000000')
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()