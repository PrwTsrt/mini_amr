import rclpy
from rclpy.node import Node
from std_msgs.msg import String, UInt64
from action_msgs.msg import GoalStatusArray
import socket


class IpPublisher(Node):

    def __init__(self):
        super().__init__("ip_publisher")
        # self.pub_ = self.create_publisher(String, "ip_address", 10)
        self.pub_ = self.create_publisher(UInt64, "ip_address", 10)
        self.sub_ = self.create_subscription(GoalStatusArray, "navigate_to_pose/_action/status", self.statusCallback, 10)
        self.period_ = 1
        self.get_logger().info("Publishing ip address with %f sec period" % self.period_)

        self.status_ = 0
        self.timer_ = self.create_timer(self.period_, self.timerCallback)
        self.sub_

    def getIp(self):

        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))  # Google's public DNS server
        ip = s.getsockname()[0]
        s.close()
        segments = ip.split('.')
        filled_segments = [segment.zfill(3) for segment in segments]
        int_ip = int(''.join(filled_segments) + str(self.status_))
        # self.get_logger().info("Ip address is %d " % int_ip)
        return int_ip

    def timerCallback(self):
        # msg  = String()
        msg = UInt64()
        msg.data = self.getIp()
        self.pub_.publish(msg)

    def statusCallback(self, msg):
        if msg.status_list:  # Check if the list is not empty
            latest_status = msg.status_list[-1]  # Access the last element in the list
            self.status_ = latest_status.status
            # self.get_logger().info("Current status is: %d" % self.status_)

def main():
    rclpy.init()

    ip_publisher = IpPublisher()
    rclpy.spin(ip_publisher)
    
    ip_publisher.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
