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

import numpy as np

import sys
sys.path.append(os.path.join(os.path.dirname(__file__)))
from move_to_pose_backward import PathFinderController, move_to_pose, cal_intermediate_point

#from .MPC_path_genrator import MPC_control_robot

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
            self.get_logger().info(f'Could not transform base_link to odom!')
            #self.get_logger().info(ex)     

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
        
class MPC_control_robot:
    def __init__(self, time_interval, prediction_horizon,no_particle, iterations):
       self.time_interval = time_interval
       self.prediction_horizon = prediction_horizon
       self.iterations = iterations
       self.no_particle = no_particle

    def solve_coefficients_2d(self,T, xs, vs, as_, xe, ve, ae):
        # เมทริกซ์ของค่าสัมประสิทธิ์ (สำหรับการเคลื่อนที่ในแต่ละแกน)
        A = np.array([[T**3, T**4, T**5],
                    [3*T**2, 4*T**3, 5*T**4],
                    [6*T, 12*T**2, 20*T**3]])

        # คำนวณในแกน x
        Bx = np.array([xe[0] - xs[0] - vs[0]*T - 0.5*as_[0]*T**2,
                    ve[0] - vs[0] - as_[0]*T,
                    ae[0] - as_[0]])

        # คำนวณในแกน y
        By = np.array([xe[1] - xs[1] - vs[1]*T - 0.5*as_[1]*T**2,
                    ve[1] - vs[1] - as_[1]*T,
                    ae[1] - as_[1]])

        # แก้ระบบสมการเชิงเส้นหา a3, a4, a5 ในแกน x และ y
        a3_a4_a5_x = np.linalg.solve(A, Bx)
        a3_a4_a5_y = np.linalg.solve(A, By)

        a0 = xs
        a1 = vs
        a2 = as_/2.0
        a3 = np.array([a3_a4_a5_x[0],a3_a4_a5_y[0]])
        a4 = np.array([a3_a4_a5_x[1],a3_a4_a5_y[1]])
        a5 = np.array([a3_a4_a5_x[2],a3_a4_a5_y[2]])
        co_eff = np.array([[a0[0],a1[0],a2[0],a3[0],a4[0],a5[0]],[a0[1],a1[1],a2[1],a3[1],a4[1],a5[1]]],dtype=float)
        return co_eff
    
    def cal_target_point(self, co, t):
        x = co[0,0]+co[0,1]*t+co[0,2]*pow(t,2)+co[0,3]*pow(t,3)+co[0,4]*pow(t,4)+co[0,5]*pow(t,5)
        y = co[1,0]+co[1,1]*t+co[1,2]*pow(t,2)+co[1,3]*pow(t,3)+co[1,4]*pow(t,4)+co[1,5]*pow(t,5)
        return x, y
    
    def traj_plan_frm_zero(self,st_x,st_y,en_x,en_y,vel):
        xs = np.array([st_x,st_y])  # ตำแหน่งเริ่มต้น (x, y)
        vs = np.array([abs(vel), 0])  # ความเร็วเริ่มต้น (x, y)
        as_ = np.array([0.0, 0.0])  # ความเร่งเริ่มต้น (x, y)
        xe = np.array([en_x, en_y])  # ตำแหน่งสุดท้าย (x, y)
        ve = np.array([0, 0])  # ความเร็วสุดท้าย (x, y)
        ae = np.array([0.0, 0.0])  # ความเร่งสุดท้าย (x, y)
        dist = math.sqrt(math.pow(st_x-en_x,2)+math.pow(st_y-en_y,2))
        T = abs(dist/vel)
        
        co_eff = self.solve_coefficients_2d(T, xs, vs, as_, xe, ve, ae)
        return co_eff
        

    def calculate_heading(self,pose):
            quant = pose.orientation
            orie_list = [quant.x,quant.y,quant.z,quant.w]
            (roll, pitch, yaw) = euler_from_quaternion(orie_list)
            #self.get_logger().info('current head='+ '{:.2f}'.format(yaw))
            return yaw
    def calculate_angle_distance(self,current_angle, target_angle):
            # คำนวณระยะห่างระหว่างมุมสองมุม
            delta_angle = target_angle - current_angle
            delta_angle = (delta_angle + math.pi) % (2 * math.pi) - math.pi
            #self.get_logger().info('delta_angle ='+'{:.3f}'.format(delta_angle))
            return delta_angle
        
    def calculate_dist_value(self,x,y,x_tar,y_tar):
            # คำนวณระยะห่างระหว่างจุด
            dist = math.sqrt(math.pow(x-x_tar,2)+math.pow(y-y_tar,2))
            return dist

    def calculate_dist(self, current_pose,target_pose):
            # คำนวณระยะห่างระหว่างจุด
            dist = math.sqrt(math.pow(target_pose.position.x-current_pose.position.x,2)+math.pow(target_pose.position.y-current_pose.position.y,2))
            return dist

    def calculate_new_position_with_velocity(self,x_old, y_old, theta_old, linear_velocity, angular_velocity, delta_time):
            """
            คำนวณตำแหน่งใหม่ของหุ่นยนต์ AMR โดยใช้ความเร็วเชิงเส้นและความเร็วเชิงมุม
            พร้อมกับเพิ่มการรองรับความเร็วเดิมก่อนการเคลื่อนที่
            
            :param x_old: ตำแหน่งแกน X ปัจจุบัน
            :param y_old: ตำแหน่งแกน Y ปัจจุบัน
            :param theta_old: มุมการหมุนปัจจุบัน (radians)
            :param linear_velocity: ความเร็วเชิงเส้นปัจจุบัน (m/s)
            :param angular_velocity: ความเร็วเชิงมุมปัจจุบัน (rad/s)
            :param previous_linear_velocity: ความเร็วเชิงเส้นก่อนหน้า (m/s)
            :param previous_angular_velocity: ความเร็วเชิงมุมก่อนหน้า (rad/s)
            :param delta_time: ช่วงระยะเวลา (s)
            :return: ตำแหน่งใหม่ (x_new, y_new, theta_new)
            """
            
            # คำนวณความเร็วเชิงเส้นเฉลี่ยและความเร็วเชิงมุมเฉลี่ย
            #avg_linear_velocity = (linear_velocity + previous_linear_velocity) / 2
            #avg_angular_velocity = (angular_velocity + previous_angular_velocity) / 2
            avg_linear_velocity = linear_velocity
            avg_angular_velocity = angular_velocity
            # กรณีที่ avg_angular_velocity != 0 จะเป็นการเคลื่อนที่แบบโค้ง
            if avg_angular_velocity != 0:
                # คำนวณรัศมีของเส้นทางโค้ง
                R = avg_linear_velocity / avg_angular_velocity
                theta_new = theta_old + avg_angular_velocity * delta_time
                
                # คำนวณตำแหน่งใหม่
                x_new = x_old + R * (math.sin(theta_new) - math.sin(theta_old))
                y_new = y_old - R * (math.cos(theta_new) - math.cos(theta_old))
                
            else:  # กรณีที่ avg_angular_velocity = 0 จะเป็นการเคลื่อนที่ตรง
                x_new = x_old + avg_linear_velocity * delta_time * math.cos(theta_old)
                y_new = y_old + avg_linear_velocity * delta_time * math.sin(theta_old)
                theta_new = theta_old
            
            # ปรับมุม theta ให้อยู่ในช่วง -π ถึง π
            theta_new = math.atan2(math.sin(theta_new), math.cos(theta_new))
            
            return x_new, y_new, theta_new


    def cal_command(self,robot_pose, target_pose,min_vel,max_vel,min_omega,max_omega):
        self.robot_pose = robot_pose
        self.target_pose = target_pose
        self.min_vel = min_vel
        self.max_vel = max_vel
        self.min_omega = min_omega
        self.max_omega = max_omega

        self.heading_robot = self.calculate_heading(robot_pose)
        self.heading_target =  self.calculate_heading(target_pose)
        self.norm_distance = self.calculate_dist(self.robot_pose,self.target_pose)
        self.norm_orientation = self.calculate_angle_distance(self.heading_robot,self.heading_target)

        best_cmds, best_val = self.particle_swarm_optimization(self.objective_function, self.no_particle, self.iterations)
        vel_cmds = best_cmds[:len(best_cmds)//2]
        omega_cmds = best_cmds[len(best_cmds)//2:]

        return vel_cmds[0],omega_cmds[0]



    def objective_function(self,particle):
         #distance to target point and orientation 
        ratio_distance = 1.0
        ratio_orientation = 1.0 - ratio_distance
        
        vel_cmds = particle[:len(particle)//2]
        omega_cmds = particle[len(particle)//2:]

        x = self.robot_pose.position.x
        y = self.robot_pose.position.y
        head =  self.heading_robot

        for i in range(self.prediction_horizon):
            x,y,head = self.calculate_new_position_with_velocity(x, y, head, vel_cmds[i], omega_cmds[i], self.time_interval)
            
        
        dist = self.calculate_dist_value(x,y,self.target_pose.position.x,self.target_pose.position.y)
        head_dist = self.calculate_angle_distance(head, self.heading_target)

        norm_dist = dist/self.norm_distance
        norm_ang_dist = head_dist/self.norm_orientation

        return (ratio_distance*norm_dist)+(ratio_orientation*norm_ang_dist) 
    

    def particle_swarm_optimization(self, objective_func, num_particles, num_iterations):
        # Initialize particles
        dimensions = self.prediction_horizon*2  # Number of dimensions (variables) include vel and omega in each horizon
        particles_vel = np.random.uniform(low=self.min_vel, high= self.max_vel, size=(num_particles, self.prediction_horizon))
        particles_omega = np.random.uniform(low=self.min_omega, high=self.max_omega, size=(num_particles, self.prediction_horizon))
        particles = np.concatenate((particles_vel, particles_omega), axis=1) #combine particle
        print(particles)

        velocities = np.zeros((num_particles, dimensions),dtype=float)
        best_positions = particles.copy()
        best_values = np.array([objective_func(p) for p in particles])
        global_best_position = best_positions[np.argmin(best_values)]
        global_best_value = np.min(best_values)

        # PSO parameters
        inertia_weight = 0.7
        cognitive_weight = 1.5
        social_weight = 1.5

        for _ in range(num_iterations):
            for i in range(num_particles):
                # Update velocity
                velocities[i] = (
                    inertia_weight * velocities[i]
                    + cognitive_weight * np.random.rand() * (best_positions[i] - particles[i])
                    + social_weight * np.random.rand() * (global_best_position - particles[i])
                )
                # Update position
                particles[i] += velocities[i]
                # Clip position to bounds
                particle = particles[i]
                vel_cmds = particle[:len(particle)//2]
                omega_cmds = particle[len(particle)//2:]
                vel_cmds = np.clip(vel_cmds, self.min_vel, self.max_vel)
                omega_cmds = np.clip(omega_cmds, self.min_omega, self.max_omega)
                particles[i] = np.concatenate((vel_cmds, omega_cmds), axis=0) #combine particle
                
                # Update best positions and values
                value = objective_func(particles[i])
                if value < best_values[i]:
                    best_values[i] = value
                    best_positions[i] = particles[i]
                    if value < global_best_value:
                        global_best_value = value
                        global_best_position = particles[i]

        return global_best_position, global_best_value


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
                ('dock_smooth_profile', True), #smooth move trajectory
                ('dock_angular_speed_search', 0.2),
                ('dock_wait_after_search', 3.0),
                ('dock_angular_speed', 0.2),
                ('dock_angular_speed_final', 0.1),
                ('dock_linear_speed', -0.1),
                ('dock_wait_at_pre_dock', 3.0),
                ('dock_linear_speed_final', -0.05),
                ('dock_time_final', 5.0), #seconds
                ('dock_check_charge_status', False), #check charge status or not
                ('dock_smooth_K_rho',0.5),
                ('dock_smooth_K_alpha', 1.0),
                ('dock_smooth_K_beta',0.1),
                ('dock_smooth_max_vel',-0.3),
                ('dock_smooth_max_omega', 1.0),
                ('dock_smooth_sampling_time',0.001),#seconds
                ('undock_time_step1', 5.0), #seconds
                ('undock_speed_step1', 0.05), #m/sec
                ('undock_dist_step2', 0.4), #meters from dock position
                ('undock_speed_step2', 0.1), #m/sec
                ('undock_angular_speed_step2', 0.1) #m/sec
            ])
        
        self.pre_dock_dist = self.get_parameter('pre_dock_dist').get_parameter_value().double_value
        self.dock_smooth_profile =  self.get_parameter('dock_smooth_profile').get_parameter_value().bool_value
        self.dock_angular_speed_search = self.get_parameter('dock_angular_speed_search').get_parameter_value().double_value
        self.dock_wait_after_search = self.get_parameter('dock_wait_after_search').get_parameter_value().double_value
        self.dock_angular_speed = self.get_parameter('dock_angular_speed').get_parameter_value().double_value
        self.dock_angular_speed_final = self.get_parameter('dock_angular_speed_final').get_parameter_value().double_value
        self.dock_linear_speed = self.get_parameter('dock_linear_speed').get_parameter_value().double_value
        self.dock_wait_at_pre_dock = self.get_parameter('dock_wait_at_pre_dock').get_parameter_value().double_value
        self.dock_linear_speed_final = self.get_parameter('dock_linear_speed_final').get_parameter_value().double_value
        self.dock_time_final = self.get_parameter('dock_time_final').get_parameter_value().double_value
        self.dock_check_charge_status = self.get_parameter('dock_check_charge_status').get_parameter_value().bool_value
        self.dock_smooth_K_rho = self.get_parameter('dock_smooth_K_rho').get_parameter_value().double_value
        self.dock_smooth_K_alpha = self.get_parameter('dock_smooth_K_alpha').get_parameter_value().double_value
        self.dock_smooth_K_beta = self.get_parameter('dock_smooth_K_beta').get_parameter_value().double_value
        self.dock_smooth_max_vel = self.get_parameter('dock_smooth_max_vel').get_parameter_value().double_value
        self.dock_smooth_max_omega = self.get_parameter('dock_smooth_max_omega').get_parameter_value().double_value
        self.dock_smooth_sampling_time = self.get_parameter('dock_smooth_sampling_time').get_parameter_value().double_value
        
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

            #self.get_logger().info('sspeed ='+'{:.3f5oo}'.format(sspeed))
            
            if (dist <= 0.02): # 5 mm
                #self.command_timer.stop()
                return
            else:    
                twist = Twist()
                twist.linear.x = sspeed; twist.linear.y = 0.0; twist.linear.z = 0.0
                twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = 0.0
                self.pub.publish(twist)

    def sign(self, num):
        return -1.0 if num < 0 else 1.0

   

    def move_robot_profile(self,target_pose,speed,rotate_speed,time_sampling):
        global current_pose
        global current_head_angle
        dist = self.calculate_dist(target_pose)
        current_head_angle = self.calculate_heading(current_pose)
        angle = math.atan2(target_pose.position.y-current_pose.position.y,target_pose.position.x-current_pose.position.x)
        if (speed < 0.0 ): #backward
            target_angle = self.opposite_angle(angle)
        else: #forward
            target_angle = angle

        #angle_dist = self.calculate_angle_distance(current_head_angle,target_angle)
       
        move_to_pose(current_pose.position.x, current_pose.position.y, theta_start, x_goal, y_goal, theta_goal,0.5)
        
        self.get_logger().info(ss)  
        self.get_logger().info('generate trajectory suceeded\n')
        cmd_vel, cmd_omega = MPC.cal_command(current_pose,target_pose,speed,-speed,-rotate_speed,rotate_speed)
    
        twist = Twist()
        twist.linear.x = cmd_vel; twist.linear.y = 0.0; twist.linear.z = 0.0
        twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = cmd_omega
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


    def move_to_pre_charge_smooth(self, speed,rotate_speed):
        global current_pose
        global dock_pose
        
        dist = self.calculate_dist(dock_pose)
        #self.get_logger().info('distance pose = ' + '{:.3f}'.format(dist))
        if (dist > 0.02):
            #timer_period = 0.2  # control-loop in seconds
            #self.command_timer = self.create_timer(timer_period, self.rotate(target_angle))
            command_complete = False
            #current_angl = self.angle_dist
            
            controller = PathFinderController(0.5, 1, 0.1)
            dt = 0.01
            inter_dist = 0.2

            # Robot specifications
            #speed = -0.05
            #rotate_speed = 0.03
            MAX_LINEAR_SPEED = abs(speed)
            MAX_ANGULAR_SPEED = rotate_speed

            step = 0
            #rho = np.hypot(x_diff, y_diff)
            while(not command_complete):
                x = current_pose.position.x
                y = current_pose.position.y
                theta = self.calculate_heading(current_pose)
                self.cal_intermediate_point(dock_pose)
                x_goal = self.pre_charge_pose.position.x
                y_goal = self.pre_charge_pose.position.y
                theta_goal = self.calculate_heading(self.pre_charge_pose)
                x_inter,y_inter,theta_inter = cal_intermediate_point(x_goal,y_goal,theta_goal,inter_dist)
                x_diff = x_goal - x
                y_diff = y_goal - y
                
                #goto intermidate_point 
                if (step == 0):
                    x_diff = x_inter - x
                    y_diff = y_inter - y
                    rho, v, w = controller.calc_control_command(
                        x_diff, y_diff, theta, theta_inter)
                    if (rho < 0.1):
                        step = 1
                #goto goal_point 
                else:
                    x_diff = x_goal - x
                    y_diff = y_goal - y
                    rho, v, w = controller.calc_control_command(
                        x_diff, y_diff, theta, theta_goal)

                if abs(speed) > MAX_LINEAR_SPEED:
                    v = np.sign(speed) * MAX_LINEAR_SPEED

                if abs(rotate_speed) > MAX_ANGULAR_SPEED:
                    w = np.sign(rotate_speed) * MAX_ANGULAR_SPEED
                
                time.sleep(self.dock_smooth_sampling_time)
                dist = self.calculate_dist(self.pre_charge_pose)
                
                #self.get_logger().info('x ='+'{:.3f}'.format(current_pose.position.x)+' y ='+'{:.3f}'.format(current_pose.position.y)+\
                #                       ' tx ='+'{:.3f}'.format(self.pre_charge_pose.position.x)+' ty ='+'{:.3f}'.format(self.pre_charge_pose.position.y)+\
                #                       ' distance = ' + '{:.3f}'.format(dist))
                self.get_logger().info('distance = ' + '{:.3f}'.format(dist))
                if (dist < 0.02):
                    command_complete = True
                    break
                
                twist = Twist()
                twist.linear.x = v; twist.linear.y = 0.0; twist.linear.z = 0.0
                twist.angular.x = 0.0; twist.angular.y = 0.0; twist.angular.z = w
                self.pub.publish(twist)
    

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
        dock_angle = math.atan2(dock.position.y-current_pose.position.y,dock.position.x-current_pose.position.x)
        target_angle = self.opposite_angle(dock_angle)
        self.command_rotate_robot(target_angle,self.dock_angular_speed)
        
        if (self.dock_smooth_profile):
            self.move_to_pre_charge_smooth(self.dock_smooth_max_vel,self.dock_smooth_max_omega)
        else:
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
        if self.dock_check_charge_status:
            self.move_open_loop_check_charge(self.dock_linear_speed_final,self.dock_time_final)
        else:
            self.move_open_loop(self.dock_linear_speed_final,self.dock_time_final)

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
        executor.add_node(robot_pose)
        executor.add_node(autodock_action_server)
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
