#!/usr/bin/env python3
# Copyright 2019 Open Source Robotics Foundation, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.import time

#try:
    #from python_qt_binding import loadUi
    #from python_qt_binding.QtGui import *
    #from python_qt_binding.QtCore import *
    #from python_qt_binding.QtWidgets import *
#except ImportError:
#        pass

import time
import os



#from rclpy.qos import qos_profile_default


from rclpy.qos import QoSDurabilityPolicy, QoSHistoryPolicy, QoSReliabilityPolicy
from rclpy.qos import QoSProfile

#from sensor_msgs.msg import LaserScan
#from geometry_msgs.msg import Pose
from custom_interface.msg import Initdock

import rclpy
from rclpy.action import ActionServer
from rclpy.node import Node

from custom_interface.action import Autodock

from geometry_msgs.msg import PoseStamped, Pose, Twist # Pose with ref frame and timestamp
from tf2_msgs.msg import TFMessage
from rclpy.duration import Duration
#from robot_navigator import BasicNavigator, NavigationResult # Helper module
from tf_transformations import euler_from_quaternion, quaternion_from_euler, quaternion_multiply
from tf2_ros import TransformException, LookupException, ConnectivityException, ExtrapolationException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
import math
import threading 
# Enables publishers, subscribers, and action servers to be in a single node
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from std_msgs.msg import String, Bool

current_pose = Pose()
current_head_angle = 0.0
dock_pose = Pose()
found_dock = False
event_obj = None

is_charge = False


class Robot_Pose(Node):
         
    def __init__(self):
   
      # Initialize the class using the constructor
      super().__init__('robot_pose_docking')
      self.tf_buffer = Buffer()
      self.tf_listener = TransformListener(self.tf_buffer,self)
      self.thread_update_pose = threading.Thread(target=self.loop_update_pose)
      self.thread_update_pose.start()
      
    def loop_update_pose(self):
        global current_pose
        global current_head_angle
        while (True):
            self.update_pose()
            #self.get_logger().info('current x= ' + '{:.3f}'.format(current_pose.position.x) + ' y='+ '{:.3f}'.format(current_pose.position.y))
            time.sleep(0.02)

    def update_pose(self):
        global current_pose
        global current_head_angle
        try:
            #self.get_logger().info(f'3')
            transf_stamped = self.tf_buffer.lookup_transform('odom', 'base_link', rclpy.time.Time())
            #self.get_logger().info(f'0')
            t = transf_stamped.transform.translation
            r = transf_stamped.transform.rotation
            
            current_pose.position.x = t.x
            current_pose.position.y = t.y
            current_pose.position.z = t.z
            current_pose.orientation = r
            #self.get_logger().info(f'1')
            current_head_angle = self.calculate_heading(current_pose)
            
           # transf_dock_stamped = self.tf_buffer.lookup_transform('odom', 'dock', rclpy.time.Time())
            #self.get_logger().info(f'0')
            #t_dock = transf_dock_stamped.transform.translation
            #r_dock = transf_dock_stamped.transform.rotation
            
            #dock_pose.position.x = t_dock.x
            #dock_pose.position.y = t_dock.y
            #dock_pose.position.z = 0.0
            #dock_pose.orientation = r_dock
            #self.get_logger().info(f'1')
            #current_head_angle = self.calculate_heading(current_pose)
            
            #angle = yaw * 180 / math.pi
            #self.get_logger().info(f'2')
            #self.get_logger().info('dock x= ' + '{:.3f}'.format(dock_pose.position.x) + ' y='+ '{:.3f}'.format(dock_pose.position.y))
            #self.get_logger().info('current head='+ '{:.2f}'.format(self.current_head_angle))
        except TransformException as ex:
            self.get_logger().info(f'Cound not transform base_link to map!')   

    def calculate_heading(self, pose):
        quant = pose.orientation
        orie_list = [quant.x,quant.y,quant.z,quant.w]
        (roll, pitch, yaw) = euler_from_quaternion(orie_list)
        #self.get_logger().info('current head='+ '{:.2f}'.format(yaw))
        return yaw
    
class ReadDock_Pose(Node):
         
    def __init__(self):
   
        # Initialize the class using the constructor
        super().__init__('read_dock_pose')
        self.callback_group = ReentrantCallbackGroup()
        self.create_subscription(
            Initdock,
            '/init_dock',  # Replace with your actual topic name
            self.dock_pose_callback,
            callback_group = ReentrantCallbackGroup(),
            qos_profile=1)
      

    def dock_pose_callback(self, msg):
        # Your custom logic here
        # Access laser scan data using msg.ranges, msg.intensities, etc.
        global dock_pose
        global found_dock
        #if (not found_dock):
        dock_pose.position.x = msg.x
        dock_pose.position.y = msg.y
        dock_pose.position.z = 0.0
        dock_pose.orientation.x = 0.0
        dock_pose.orientation.y = 0.0
        dock_pose.orientation.z = msg.z
        dock_pose.orientation.w = msg.w
        
        found_dock = True
        #self.get_logger().info('dock x= ' + '{:.2f}'.format(dock_pose.position.x) + ' y='+ '{:.2f}'.format(dock_pose.position.y))

class Charge_Status(Node):
    
    def __init__(self):
   
        # Initialize the class using the constructor
        super().__init__('charge_status')
        self.callback_group = ReentrantCallbackGroup()
        self.create_subscription(
            Bool,
            '/charge_state',  # Replace with your actual topic name
            self.update_status,
            callback_group = ReentrantCallbackGroup(),
            qos_profile=1) 
    
    def update_status(self, msg):
        # Your custom logic here
        # Access laser scan data using msg.ranges, msg.intensities, etc.
        global is_charge
        is_charge = msg.data
        


class AutodockActionServer(Node):
    
    def __init__(self):
        super().__init__('autodock_action_server')
        global current_pose
        global current_head_angle
        global dock_pose
        global found_dock
        global event_obj
        
        self.declare_parameters(
            namespace='',
            parameters=[
                ('pre_dock_dist', 0.4),
                ('dock_angular_speed_search', 0.2),
                ('dock_wait_after_search', 3.0),
                ('dock_angular_speed', 0.2),
                ('dock_angular_speed_final', 0.1),
                ('dock_linear_speed', -0.1),
                ('dock_wait_at_pre_dock', 3.0),
                ('dock_linear_speed_final', -0.05),
                ('dock_time_final', 5.0), #seconds
                ('undock_time_step1', 5.0), #seconds
                ('undock_speed_step1', 0.05), #m/sec
                ('undock_dist_step2', 0.4), #meters from dock position
                ('undock_speed_step2', 0.1), #m/sec
                ('undock_angular_speed_step2', 0.1) #m/sec
            ])
        
        self.pre_dock_dist = self.get_parameter('pre_dock_dist').get_parameter_value().double_value
        self.dock_angular_speed_search = self.get_parameter('dock_angular_speed_search').get_parameter_value().double_value
        self.dock_wait_after_search = self.get_parameter('dock_wait_after_search').get_parameter_value().double_value
        self.dock_angular_speed = self.get_parameter('dock_angular_speed').get_parameter_value().double_value
        self.dock_angular_speed_final = self.get_parameter('dock_angular_speed_final').get_parameter_value().double_value
        self.dock_linear_speed = self.get_parameter('dock_linear_speed').get_parameter_value().double_value
        self.dock_wait_at_pre_dock = self.get_parameter('dock_wait_at_pre_dock').get_parameter_value().double_value
        self.dock_linear_speed_final = self.get_parameter('dock_linear_speed_final').get_parameter_value().double_value
        self.dock_time_final = self.get_parameter('dock_time_final').get_parameter_value().double_value
        self.undock_time_step1 = self.get_parameter('undock_time_step1').get_parameter_value().double_value
        self.undock_speed_step1 = self.get_parameter('undock_speed_step1').get_parameter_value().double_value
        self.undock_dist_step2 = self.get_parameter('undock_dist_step2').get_parameter_value().double_value
        self.undock_speed_step2 = self.get_parameter('undock_speed_step2').get_parameter_value().double_value
        self.undock_angular_speed_step2 = self.get_parameter('undock_angular_speed_step2').get_parameter_value().double_value

        current_pose = Pose()
        current_head_angle = 0.0 #radian
        
        #found_dock = False
        #dock_pose = Pose()
        self.charge_pose = Pose()
        self.pre_charge_pose = Pose()
        self.undock_pose = Pose()
        self.is_docking = False
        #self.charge_dist = 0.35 # charging distance relative to dock (m)
        self.pre_charge_dist = 0.6 # pre-charge point distance from dock (m)
        event_obj = threading.Event()

        self.publisher_ = self.create_publisher(String, 'command_dock', 10)

        #tf_listener = TransformListener(tf_buffer,self)
        
        #self.event = threading.Event()
        
        #process_a = multiprocessing.Process(target=self.loop_update_pose)
        #process_a.start()
        #process_a.join()

        
        

        #thread_control_pose = threading.Thread(target=self.move_back_robot())
       
        #timer_period = 0.2  # seconds
        #self.timer = self.create_timer(timer_period, self.update_pose)
        
        self._action_server = ActionServer(
            self,
            Autodock,
            'autodock',
            self.execute_callback,
            callback_group = ReentrantCallbackGroup())
        
        #self.create_subscription(
        #    TFMessage,
        #    '/tf',  # Replace with your actual topic name
        #    self.tf_pose_callback,
        #    1)
        
        
        self.pub = self.create_publisher(Twist, 'cmd_vel', 	10)
        

    def execute_callback(self, goal_handle):
        self.get_logger().info('Executing goal...')

        
        
        global event_obj
        event_obj.set()

        if (goal_handle.request.is_dock): #dock
            #self.pre_charge_dist = goal_handle.request.offset_inter_point
            self.pre_charge_dist = self.pre_dock_dist
            #self.cal_intermediate_point()
            self.dock_robot(goal_handle)
        else: #undock
            #self.pre_charge_dist = goal_handle.request.offset_inter_point
            self.pre_charge_dist = self.undock_dist_step2
            #self.cal_intermediate_point()
            self.undock_robot(goal_handle)
        
        #while (event_obj.is_set):
        #    time.sleep(0.1)

        # navigator = BasicNavigator()
        # # Wait for navigation to fully activate. Use this line if autostart is set to true.
        # navigator.waitUntilNav2Active()
        
        # goal_pose = PoseStamped()
        # goal_pose.header.frame_id = 'map'
        # goal_pose.header.stamp = navigator.get_clock().now().to_msg()
        # goal_pose.pose.position.x = self.pre_charge_pose.position.x
        # goal_pose.pose.position.y = self.pre_charge_pose.position.y
        # goal_pose.pose.position.z = 0.0
        # goal_pose.pose.orientation.x = 0.0
        # goal_pose.pose.orientation.y = 0.0
        # goal_pose.pose.orientation.z = self.pre_charge_pose.orientation.z
        # goal_pose.pose.orientation.w = self.pre_charge_pose.orientation.w
        
        # self.get_logger().info('goal = x '+ '{:.2f}'.format(goal_pose.pose.position.x)
        #             + ' ,y '+ '{:.2f}'.format(goal_pose.pose.position.y)
        #             + ' ,yaw '+ '{:.2f}'.format(goal_pose.pose.orientation.z)
        #             + ' ,w '+ '{:.2f}'.format(goal_pose.pose.orientation.w))
        
        # #dockRobot(goal_pose, dock_type ='')

        # navigator.goToPose(goal_pose)
        # i = 0
        # while not navigator.isNavComplete():
        # ################################################
        # #
        # # Implement some code here for your application!
        # #
        # ################################################
    
        #     # Do something with the feedback
        #     i = i + 1
        #     feedback = navigator.getFeedback()
        #     if feedback and i % 5 == 0:
        #         self.get_logger().info('Distance remaining: ' + '{:.2f}'.format(
        #             feedback.distance_remaining) + ' meters.')
        
        #     # Some navigation timeout to demo cancellation
        #   #  if Duration.from_msg(feedback.navigation_time) > Duration(seconds=600.0):
        #   #      navigator.cancelNav()
        
        #     # Some navigation request change to demo preemption
        #   #  if Duration.from_msg(feedback.navigation_time) > Duration(seconds=120.0):
        #   #      goal_pose.pose.position.x = -3.0
        #   #      navigator.goToPose(goal_pose)

        # #for i in range(1, goal_handle.request.order):
        # #    feedback_msg.partial_sequence.append(
        # #        feedback_msg.partial_sequence[i] + feedback_msg.partial_sequence[i-1])
        # #    self.get_logger().info('Feedback: {0}'.format(feedback_msg.partial_sequence))
        # #    goal_handle.publish_feedback(feedback_msg)
        # #    time.sleep(1)
        # #yaw_dist = goal_pose.pose.orientation.z  
        # #navigator.spin(spin_dist=1.57, time_allowance=10)
        
        #result = navigator.getResult()
        
        goal_handle.succeed()

        result = Autodock.Result()
        #result.sequence = feedback_msg.partial_sequence
        return result
    
    
        #self.get_logger().info('dock x= ' + '{:.2f}'.format(self.dock_pose.position.x) + ' y='+ '{:.2f}'.format(self.dock_pose.position.y))
    
    #def tf_pose_callback(self, msg):
        # Your custom logic here
        # Access laser scan data using msg.ranges, msg.intensities, etc.
        #self.update_pose()
        #self.get_logger().info('tf x= ' + '{:.2f}'.format(msg.transforms[0].transform.translation.x) + ' y='+ '{:.2f}'.format(msg.transforms[0].transform.translation.y))

    """  def robot_pose_callback(self, msg):
        # Your custom logic here
        # Access laser scan data using msg.ranges, msg.intensities, etc.
        self.get_logger().info('Receive')
        current_pose = msg
        self.get_logger().info('Receive: current x= ' + '{:.2f}'.format(msg.position.x) + ' y='+ '{:.2f}'.format(msg.position.y)) """
       
    def send_feedback(self,goal_handle, step, text):
        feedback_msg = Autodock.Feedback()
        feedback_msg.step = step
        feedback_msg.text.data = text
        goal_handle.publish_feedback(feedback_msg)
            

    def cal_intermediate_point(self,dock_pose_local):    
        global found_dock
        #global dock_pose

        #self.get_logger().info('dock x= ' + '{:.2f}'.format(dock_pose.position.x) + ' y='+ '{:.2f}'.format(dock_pose.position.y))
        q_ori = [dock_pose_local.orientation.x , dock_pose_local.orientation.y, dock_pose_local.orientation.z, dock_pose_local.orientation.w]
        # Rotate the previous pose by 180* about Z
        q_rot = quaternion_from_euler(0, 0, 3.14159)
        q_new = quaternion_multiply(q_rot, q_ori)
        
        #quant = self.dock_pose.orientation
        
        #finding angle of docking
        #angle = yaw * 180 / math.pi
        #orie_list = [q_new.x,q_new.y,q_new.z,q_new.w]
        (roll, pitch, yaw) = euler_from_quaternion(q_new)

        
        self.pre_charge_pose.position.x = dock_pose_local.position.x + math.cos(yaw)*self.pre_charge_dist
        self.pre_charge_pose.position.y = dock_pose_local.position.y + math.sin(yaw)*self.pre_charge_dist
        self.pre_charge_pose.orientation.z = q_new[2]
        self.pre_charge_pose.orientation.w = q_new[3]

        
        #self.pre_charge_pose.orientation.w = -self.pre_charge_pose.orientation.w
        #self.get_logger().info('pre_charge x= ' + '{:.2f}'.format(self.pre_charge_pose.position.x) + ' y='+ '{:.2f}'.format(self.pre_charge_pose.position.y)+ ' head='+ '{:.4f}'.format(yaw))

       # self.charge_pose.position.x = self.dock_pose.position.x + math.cos(yaw)*self.charge_dist
       # self.charge_pose.position.y = self.dock_pose.position.y + math.sin(yaw)*self.charge_dist
       # self.charge_pose.orientation.z = q_new[2]
       # self.charge_pose.orientation.w = q_new[3]
        
       # self.get_logger().info('charge x= ' + '{:.2f}'.format(self.charge_pose.position.x) + ' y='+ '{:.2f}'.format(self.charge_pose.position.y)+ ' head='+ '{:.4f}'.format(yaw))
        
        
        #self.get_logger().info("dock x='%f' y='%f'" %(dock_x ,dock_y))
        #RCLCPP_INFO(self.get_logger(), "Received request to cancel goal");
        #print("test")
        #self.get_logger().info("test")
        #time.sleep(100)

    def cal_undock_point(self,dist):    
        #self.get_logger().info('dock x= ' + '{:.2f}'.format(self.dock_pose.position.x) + ' y='+ '{:.2f}'.format(self.dock_pose.position.y))
        q_ori = [current_pose.orientation.x , current_pose.orientation.y, current_pose.orientation.z, current_pose.orientation.w]
        # Rotate the previous pose by 180* about Z
        q_rot = quaternion_from_euler(0, 0, 0)
        q_new = quaternion_multiply(q_rot, q_ori)
        
        #quant = self.dock_pose.orientation
        
        #finding angle of docking
        #angle = yaw * 180 / math.pi
        #orie_list = [q_new.x,q_new.y,q_new.z,q_new.w]
        (roll, pitch, yaw) = euler_from_quaternion(q_new)

        
        self.undock_pose.position.x = current_pose.position.x + math.cos(yaw)*dist
        self.undock_pose.position.y = current_pose.position.y + math.sin(yaw)*dist
        self.undock_pose.orientation.z = q_new[2]
        self.undock_pose.orientation.w = q_new[3]

        
        #self.pre_charge_pose.orientation.w = -self.pre_charge_pose.orientation.w
        #self.get_logger().info('undock_point x= ' + '{:.2f}'.format(self.undock_pose.position.x) + ' y='+ '{:.2f}'.format(self.undock_pose.position.y)+ ' head='+ '{:.4f}'.format(yaw))

       # self.charge_pose.position.x = self.dock_pose.position.x + math.cos(yaw)*self.charge_dist
       # self.charge_pose.position.y = self.dock_pose.position.y + math.sin(yaw)*self.charge_dist
       # self.charge_pose.orientation.z = q_new[2]
       # self.charge_pose.orientation.w = q_new[3]
        
       # self.get_logger().info('charge x= ' + '{:.2f}'.format(self.charge_pose.position.x) + ' y='+ '{:.2f}'.format(self.charge_pose.position.y)+ ' head='+ '{:.4f}'.format(yaw))
        
        
        #self.get_logger().info("dock x='%f' y='%f'" %(dock_x ,dock_y))
        #RCLCPP_INFO(self.get_logger(), "Received request to cancel goal");
        #print("test")
        #self.get_logger().info("test")
        #time.sleep(100)

    
    
    def calculate_heading(self, pose):
        quant = pose.orientation
        orie_list = [quant.x,quant.y,quant.z,quant.w]
        (roll, pitch, yaw) = euler_from_quaternion(orie_list)
        #self.get_logger().info('current head='+ '{:.2f}'.format(yaw))
        return yaw
    
    # def read_pose(self):
    #     try:
    #         #self.get_logger().info(f'3')
    #         transf_stamped = self.tf_buffer.lookup_transform('map', 'base_link', rclpy.time.Time())
    #         #self.get_logger().info(f'0')
    #         t = transf_stamped.transform.translation
    #         r = transf_stamped.transform.rotation
    #         x = t.x
    #         y = t.y
    #         z = t.z
    #         #self.get_logger().info(f'1')
    #         quant = r
    #         orie_list = [quant.x,quant.y,quant.z,quant.w]
    #         (roll, pitch, yaw) = euler_from_quaternion(orie_list)
    #         theta = yaw 
    #         #angle = yaw * 180 / math.pi
    #         #self.get_logger().info(f'2')
    #         self.get_logger().info('read x= ' + '{:.2f}'.format(x) + ' y='+ '{:.2f}'.format(y)+ ' angle='+ '{:.2f}'.format(theta))
    #         return (x,y,theta)
    #     except TransformException as ex:
    #         self.get_logger().info(f'cannot read position')
    #         return (0,0,0)
    
    def calculate_heading(self, pose):
        quant = pose.orientation
        orie_list = [quant.x,quant.y,quant.z,quant.w]
        (roll, pitch, yaw) = euler_from_quaternion(orie_list)
        #self.get_logger().info('current head='+ '{:.2f}'.format(self.current_head_angle))
        return yaw
    
    def opposite_angle(self, angle_radians):
        # หามุมตรงข้ามกัน
        opposite_radians = (angle_radians + math.pi) % (2 * math.pi)
        return opposite_radians
    
    def calculate_angle_distance(self, current_angle, target_angle):
        # คำนวณระยะห่างระหว่างมุมสองมุม
        delta_angle = target_angle - current_angle
        delta_angle = (delta_angle + math.pi) % (2 * math.pi) - math.pi
        self.get_logger().info('delta_angle ='+'{:.3f}'.format(delta_angle))
        return delta_angle
    
    def calculate_dist(self, target_pose):
        # คำนวณระยะห่างระหว่างจุด
        global current_pose
        dist = math.sqrt(math.pow(target_pose.position.x-current_pose.position.x,2)+math.pow(target_pose.position.y-current_pose.position.y,2))
        return dist
    
    def calculate_direction_to_target(self, target_pose,is_backward):
        global current_pose
        quant = current_pose.orientation
        orie_list = [quant.x,quant.y,quant.z,quant.w]
        (roll, pitch, yaw) = euler_from_quaternion(orie_list)
        if (is_backward)    :
            robot_yaw = self.opposite_angle(yaw)
        else:
            robot_yaw = yaw
        
        target_vector = math.atan2(target_pose.position.y-current_pose.position.y,target_pose.position.x-current_pose.position.x)
        robot_yaw = robot_yaw % (2 * math.pi)
        target_vector = target_vector % (2 * math.pi)
        
        self.get_logger().info('robot_yaw ='+'{:.3f}'.format(robot_yaw)+' target_vector ='+'{:.3f}'.format(target_vector))
                                       
        if (self.calculate_angle_distance(robot_yaw,target_vector) > math.pi/2):
            self.get_logger().info('return -1')
            return -1
            
        else:
            self.get_logger().info('return 1')
            return 1
        
    

    def rotate_openloop(self,rotate_speed):
        twist = Twist()
        twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = rotate_speed
        self.pub.publish(twist)
    
    def rotate(self,target_angle,rotate_speed):
        global current_pose
        global current_head_angle
        current_head_angle = self.calculate_heading(current_pose)
        angle_speed = rotate_speed
        angle_dist = self.calculate_angle_distance(current_head_angle,target_angle)
        #self.get_logger().info('differnet angle rotate = ' + '{:.2f}'.format(angle_dist))
        if (abs(angle_dist) < 0.01):
            #self.command_timer.stop()
            return
        else:    
            if (angle_dist > 0):
                angular_speed = angle_speed
            else:
                angular_speed = -angle_speed
            
            if (abs(angle_dist) > 0.5):
                angular_speed = angular_speed*5.0
            
            twist = Twist()
            twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = angular_speed
            self.pub.publish(twist)





    
    def command_rotate_robot(self, target_angle,rotate_speed):
        global current_pose
        global current_head_angle
        current_head_angle = self.calculate_heading(current_pose)
        angle_dist = self.calculate_angle_distance(current_head_angle,target_angle)
        #self.get_logger().info('differnet angle = ' + '{:.2f}'.format(angle_dist))
        if (abs(angle_dist) > 0.01):
            #timer_period = 0.2  # control-loop in seconds
            #self.command_timer = self.create_timer(timer_period, self.rotate(target_angle))
            command_complete = False
            #current_angl = self.angle_dist
            while(not command_complete):
                self.rotate(target_angle,rotate_speed)
                time.sleep(0.02)
                current_head_angle = self.calculate_heading(current_pose)
                angle_dist = self.calculate_angle_distance(current_head_angle,target_angle)
                #self.get_logger().info('differnet angle command = ' + '{:.2f}'.format(angle_dist))
                if (abs(angle_dist) < 0.01):
                    self.command_complete = True
                    break

    def move_open_loop(self,speed,duration):
        timeout = time.time()+duration #seconds
        while(time.time() < timeout):
            twist = Twist()
            twist.linear.x = speed; twist.linear.y = 0.0; twist.linear.z = 0.0
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0
            self.pub.publish(twist)
            time.sleep(0.2)
    
    def move_open_loop_check_charge(self,speed,duration):
        timeout = time.time()+duration #seconds
        while((time.time() < timeout)and(not is_charge)):
            twist = Twist()
            twist.linear.x = speed; twist.linear.y = 0.0; twist.linear.z = 0.0
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0
            self.pub.publish(twist)
            time.sleep(0.2)



    def move_linear_robot(self,target_pose,speed):
        global current_pose
        #self.current_head_angle = self.calculate_heading(current_pose)
        #angle_speed = 0.05
        dist = self.calculate_dist(target_pose)
        if (dist > 0.15): # > 15 cm
            speed = speed*self.calculate_direction_to_target(target_pose,(speed < 0))
        else: # < 15 cm use 25% speed
            speed = 0.25*speed*self.calculate_direction_to_target(target_pose,(speed < 0))
        #self.get_logger().info('differnet angle rotate = ' + '{:.2f}'.format(angle_dist))
        if (dist < 0.02): # 5 mm
            #self.command_timer.stop()
            return
        else:    
            twist = Twist()
            twist.linear.x = speed; twist.linear.y = 0.0; twist.linear.z = 0.0
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0
            self.pub.publish(twist)
        
       
    
    def command_move_robot(self, target_pose,speed):
        global current_pose
        dist = self.calculate_dist(target_pose)
        #self.get_logger().info('distance pose = ' + '{:.3f}'.format(dist))
        if (dist > 0.02):
            #timer_period = 0.2  # control-loop in seconds
            #self.command_timer = self.create_timer(timer_period, self.rotate(target_angle))
            command_complete = False
            #current_angl = self.angle_dist
            
            while(not command_complete):
                self.move_linear_robot(target_pose,speed)
                time.sleep(0.02)
                dist = self.calculate_dist(target_pose)
                #self.get_logger().info('x ='+'{:.3f}'.format(current_pose.position.x)+' y ='+'{:.3f}'.format(current_pose.position.y)+\
                #                       ' tx ='+'{:.3f}'.format(target_pose.position.x)+' ty ='+'{:.3f}'.format(target_pose.position.y)+\
                #                      ' distance linear = ' + '{:.3f}'.format(dist))
                self.get_logger().info('distance linear = ' + '{:.3f}'.format(dist))
                if (dist < 0.02):
                    self.command_complete = True
                    break
    
    def move_robot(self,target_pose,speed,rotate_speed):
        global current_pose
        global current_head_angle
        dist = self.calculate_dist(target_pose)
        current_head_angle = self.calculate_heading(current_pose)
        angle = math.atan2(target_pose.position.y-current_pose.position.y,target_pose.position.x-current_pose.position.x)
        if (speed < 0.0 ): #backward
            target_angle = self.opposite_angle(angle)
        else: #forward
            target_angle = angle

        angle_dist = self.calculate_angle_distance(current_head_angle,target_angle)
        if (abs(angle_dist) > 0.02):
            if (abs(angle_dist) > 0.2): 
            	rotate_speed = rotate_speed
   
            if (angle_dist > 0):
                angular_speed = rotate_speed
            else:
                angular_speed = -rotate_speed
            twist = Twist()
            twist.linear.x = 0.0; twist.linear.y = 0.0; twist.linear.z = 0.0
            twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = angular_speed
            self.pub.publish(twist)
        elif (dist > 0.02) :
            direction = self.calculate_direction_to_target(target_pose,(speed < 0))
            if (dist > 0.2): # > 15 cm
                sspeed = speed*direction
            else: # < 15 cm use 25% speed
                sspeed = 0.25*speed*direction    

            #self.get_logger().info('sspeed ='+'{:.3f}'.format(sspeed))
            
            if (dist <= 0.02): # 5 mm
                #self.command_timer.stop()
                return
            else:    
                twist = Twist()
                twist.linear.x = sspeed; twist.linear.y = 0.0; twist.linear.z = 0.0
                twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0
                self.pub.publish(twist)
        


    def move_to_pre_charge(self, speed,rotate_speed):
        global current_pose
        global dock_pose
        
        dist = self.calculate_dist(dock_pose)
        #self.get_logger().info('distance pose = ' + '{:.3f}'.format(dist))
        if (dist > 0.02):
            #timer_period = 0.2  # control-loop in seconds
            #self.command_timer = self.create_timer(timer_period, self.rotate(target_angle))
            command_complete = False
            #current_angl = self.angle_dist
           

            while(not command_complete):
                self.cal_intermediate_point(dock_pose)
                self.move_robot(self.pre_charge_pose,speed,rotate_speed)
                
                time.sleep(0.03)
                dist = self.calculate_dist(self.pre_charge_pose)
                
                #self.get_logger().info('x ='+'{:.3f}'.format(current_pose.position.x)+' y ='+'{:.3f}'.format(current_pose.position.y)+\
                #                       ' tx ='+'{:.3f}'.format(self.pre_charge_pose.position.x)+' ty ='+'{:.3f}'.format(self.pre_charge_pose.position.y)+\
                #                       ' distance = ' + '{:.3f}'.format(dist))
                self.get_logger().info('distance = ' + '{:.3f}'.format(dist))
                if (dist < 0.02):
                    command_complete = True
                    break

    def command_move_rotate_robot(self, target_pose,speed,rotate_speed):
        global current_pose
        
        dist = self.calculate_dist(target_pose)
        #self.get_logger().info('distance pose = ' + '{:.3f}'.format(dist))
        if (dist > 0.02):
            #timer_period = 0.2  # control-loop in seconds
            #self.command_timer = self.create_timer(timer_period, self.rotate(target_angle))
            command_complete = False
            #current_angl = self.angle_dist
           

            while(not command_complete):
                global current_pose
                #self.update_pose()
                #self.cal_intermediate_point(dock_pose)

                self.move_robot(target_pose,speed,rotate_speed)
                time.sleep(0.03)
                
                dist = self.calculate_dist(target_pose)
                #self.get_logger().info('x ='+'{:.3f}'.format(current_pose.position.x)+' y ='+'{:.3f}'.format(current_pose.position.y)+\
                #                       ' tx ='+'{:.3f}'.format(target_pose.position.x)+' ty ='+'{:.3f}'.format(target_pose.position.y)+\
                #                       ' distance = ' + '{:.3f}'.format(dist))
                self.get_logger().info('distance = ' + '{:.3f}'.format(dist))
                if (dist < 0.02):
                    command_complete = True
                    break



    def dock_robot(self,goal_handle):
        global event_obj
        global found_dock
        global dock_pose
        global current_head_angle
        global current_pose

       # big robot
       # search_angular_speed = 0.15
       # angular_speed = 0.1
       # angular_speed_final = 0.1
       # linear_speed_final = -0.075
       # linear_speed = -0.4 #backward 
       
       # TinyRB robot
       # search_angular_speed = 0.20
       # angular_speed = 0.04
       # angular_speed_final = 0.1
       # linear_speed_final = -0.05
       # linear_speed = -0.1 #backward 
        
        self.get_logger().info('docking...............................................')
        self.send_feedback(goal_handle,1,'searching dock')
        
        #step1 -find dock
        found_dock = False
        msg = String()
        msg.data = 'start'
        self.publisher_.publish(msg) # command to start finding dock coordinate with topic /command_dock

        start_angle = self.calculate_heading(current_pose)
        ts = time.time()
        while (not found_dock):
            self.rotate_openloop(self.dock_angular_speed_search)
            time.sleep(0.1)
            angle = self.calculate_heading(current_pose)
            delay = time.time()-ts
            if (delay > 4.0 and (abs(self.calculate_angle_distance(angle,start_angle)) < 0.02)):
                break

        dock = Pose()
	
        if (not found_dock):
            self.get_logger().info('dock not found')
            self.send_feedback(goal_handle,2,'dock not found') 
            msg.data = 'shutdown'
            self.publisher_.publish(msg)
            event_obj.clear()
            return
        else:
            #delay for stable docking position 
            time.sleep(self.dock_wait_after_search)
            dock = dock_pose
            self.cal_intermediate_point(dock)
            ss = 'pre dock x= ' + '{:.2f}'.format(self.pre_charge_pose.position.x) + ' y='+ '{:.2f}'.format(self.pre_charge_pose.position.y) + os.linesep \
                + 'dock x= ' + '{:.2f}'.format(dock.position.x) + ' y='+ '{:.2f}'.format(dock.position.y)
            self.send_feedback(goal_handle,2,'move to pre charge point' + os.linesep + ss)  

        #step2 - move to precharge pose
        self.move_to_pre_charge(self.dock_linear_speed,self.dock_angular_speed)
        time.sleep(self.dock_wait_at_pre_dock)
        dock = dock_pose #check dock position again
        angle = math.atan2(dock.position.y-current_pose.position.y,dock.position.x-current_pose.position.x)
        #angle = self.calculate_heading(dock)
        target_angle = self.opposite_angle(angle)
        self.command_rotate_robot(target_angle,self.dock_angular_speed_final)
        ss = 'current position x= ' + '{:.2f}'.format(current_pose.position.x) + ' y='+ '{:.2f}'.format(current_pose.position.y) + os.linesep \
                + 'dock x= ' + '{:.2f}'.format(dock.position.x) + ' y='+ '{:.2f}'.format(dock.position.y)
        self.send_feedback(goal_handle,3,'move to dock' + os.linesep + ss)  

        #step3 - move to dock
        self.move_open_loop_check_charge(self.dock_linear_speed_final,self.dock_time_final)

        #step4 - finish docking
        self.get_logger().info('finish docking')
        msg.data = 'shutdown'
        self.publisher_.publish(msg)
        #time.sleep(3.0)
        found_dock = False
        event_obj.clear()
        
        """ self.current_head_angle = self.calculate_heading(current_pose)
        angle = math.atan2(self.pre_charge_pose.position.y-current_pose.position.y,self.pre_charge_pose.position.x-current_pose.position.x)
        target_angle = self.opposite_angle(angle)
        self.get_logger().info('step 1 ')
        self.command_rotate_robot(target_angle,current_pose,angular_speed)
        self.get_logger().info('step 2 ')
        self.command_move_robot(self.pre_charge_pose,current_pose,linear_speed)
        
        self.current_head_angle = self.calculate_heading(current_pose)
        angle = math.atan2(self.charge_pose.position.y-current_pose.position.y,self.charge_pose.position.x-current_pose.position.x)
        target_angle = self.opposite_angle(angle)
        self.get_logger().info('step 3 ')
        self.command_rotate_robot(target_angle,current_pose,angular_speed)
        self.get_logger().info('step 4 ')
        self.command_move_robot(self.charge_pose,current_pose,linear_speed) """


    


    def undock_robot(self,goal_handle): 
        global event_obj
        global current_pose
        #angular_speed = 0.1
        #linear_speed = 0.1 #forward
        self.cal_undock_point(self.undock_dist_step2)
        
        self.send_feedback(goal_handle,1,'start undocking.......')
        self.move_open_loop(self.undock_speed_step1,self.undock_time_step1)
        
        ss = 'undock point x= ' + '{:.2f}'.format(self.undock_pose.position.x) + ' y='+ '{:.2f}'.format(self.undock_pose.position.y)
        self.get_logger().info(ss)

        self.send_feedback(goal_handle,2,'move to '+ss)
        self.command_move_rotate_robot(self.undock_pose,self.undock_speed_step2,self.undock_angular_speed_step2)
        #angle = math.atan2(self.dock_pose.position.y-current_pose.position.y,self.dock_pose.position.x-current_pose.position.x)
        #target_angle = self.opposite_angle(angle)
        #self.command_rotate_robot(target_angle,current_pose,angular_speed)
        event_obj.clear()
        self.get_logger().info('finish undocking')



    
#SPIN_QUEUE = []
#PERIOD = 0.01

def main(args=None):
    rclpy.init(args=args)
    
    #SPIN_QUEUE.append(AutodockActionServer())
    #SPIN_QUEUE.append(RobotPose())
    robot_pose = Robot_Pose()
    readdock_pose = ReadDock_Pose()
    autodock_action_server = AutodockActionServer()
    charge_status = Charge_Status()
    
    
    #robot_pose = RobotPose()

    try:
        #rclpy.spin(robot_pose)
        #rclpy.spin(autodock_action_server)
        # Set up mulithreading
        executor = MultiThreadedExecutor(num_threads=6)
        executor.add_node(autodock_action_server)
        executor.add_node(robot_pose)
        executor.add_node(readdock_pose)
        executor.add_node(charge_status)
   
        try:
            # Spin the nodes to execute the callbacks
            executor.spin()
        finally:
            # Shutdown the nodes
            executor.shutdown()
            autodock_action_server.destroy_node()
            robot_pose.destroy_node()
            readdock_pose.destroy_node()
            charge_status.destroy_node()
 
    finally:
    # Shutdown
        rclpy.shutdown()   
        
        
    #except KeyboardInterrupt:
        #pass


if __name__ == '__main__':
    main()
