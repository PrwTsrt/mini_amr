import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from time import sleep


def main():

    rclpy.init()
    node = rclpy.create_node('cmd_vel_pub')
    publisher = node.create_publisher(Twist, "/cmd_vel", 1)
    msg = Twist()
    # for i in range(1):      

    try:
        print("Running... Press Ctrl+C to stop.")
        # msg.linear.x = 0.2
        # msg.linear.x = -0.2
        msg.angular.z = 0.3
        node.get_logger().info('Publishing: "%f"' % msg.linear.x)
        
        while True:
            publisher.publish(msg)
            sleep(0.1)  
            
    except KeyboardInterrupt:
        print("\nCtrl+C detected. Exiting loop...")

    node.get_logger().info('Set cmd_vel to 0')
    msg.linear.x = 0.0
    publisher.publish(msg)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()