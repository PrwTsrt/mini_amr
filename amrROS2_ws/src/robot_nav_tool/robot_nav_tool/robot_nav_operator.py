#! /usr/bin/env python3

import time
from enum import Enum

from action_msgs.msg import GoalStatus
from geometry_msgs.msg import PoseStamped
from geometry_msgs.msg import PoseWithCovarianceStamped
from lifecycle_msgs.srv import GetState
from nav2_msgs.action import NavigateThroughPoses, NavigateToPose, FollowWaypoints, ComputePathToPose, ComputePathThroughPoses
from nav2_msgs.srv import LoadMap, ClearEntireCostmap, ManageLifecycleNodes, GetCostmap

import rclpy

from rclpy.action import ActionClient
from rclpy.node import Node
from rclpy.qos import QoSDurabilityPolicy, QoSHistoryPolicy, QoSReliabilityPolicy
from rclpy.qos import QoSProfile

import math

from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from std_msgs.msg import String
import numpy as np 


class NavigationResult(Enum):
    UKNOWN = 0
    SUCCEEDED = 1
    CANCELED = 2
    FAILED = 3 


class BasicNavigator(Node):
    def __init__(self):
        super().__init__(node_name='basic_navigator')
        self.initial_pose = PoseStamped()
        self.initial_pose.header.frame_id = 'map'
        self.goal_handle = None
        self.result_future = None
        self.feedback = None
        self.status = None

        amcl_pose_qos = QoSProfile(
          durability=QoSDurabilityPolicy.TRANSIENT_LOCAL,
          reliability=QoSReliabilityPolicy.RELIABLE,
          history=QoSHistoryPolicy.KEEP_LAST,
          depth=1)

        self.initial_pose_received = False
        self.nav_through_poses_client = ActionClient(self,
                                                     NavigateThroughPoses,
                                                     'navigate_through_poses')
        self.nav_to_pose_client = ActionClient(self, NavigateToPose, 'navigate_to_pose')
        self.follow_waypoints_client = ActionClient(self, FollowWaypoints, 'follow_waypoints')
        self.compute_path_to_pose_client = ActionClient(self, ComputePathToPose, 'compute_path_to_pose')
        self.compute_path_through_poses_client = ActionClient(self, ComputePathThroughPoses,
                                                              'compute_path_through_poses')
        self.localization_pose_sub = self.create_subscription(PoseWithCovarianceStamped,
                                                              'amcl_pose',
                                                              self._amclPoseCallback,
                                                              amcl_pose_qos)
        self.initial_pose_pub = self.create_publisher(PoseWithCovarianceStamped,
                                                      'initialpose',
                                                      10)
        self.change_maps_srv = self.create_client(LoadMap, 'map_server/load_map')
        self.clear_costmap_global_srv = self.create_client(
            ClearEntireCostmap, 'global_costmap/clear_entirely_global_costmap')
        self.clear_costmap_local_srv = self.create_client(
            ClearEntireCostmap, 'local_costmap/clear_entirely_local_costmap')
        self.get_costmap_global_srv = self.create_client(GetCostmap, 'global_costmap/get_costmap')
        self.get_costmap_local_srv = self.create_client(GetCostmap, 'local_costmap/get_costmap')

        self.workstatus_pub = self.create_publisher(String, 'robot_status/working',10)

        self.robot_pose = [2.882, 0.920, 1.560]
      
        self.wp = []
   
        self.request_cancel = False
 
    def setInitialPose(self, initial_pose):
        self.initial_pose_received = False
        self.initial_pose = initial_pose
        self._setInitialPose()

    def goThroughPoses(self, poses):
        # Sends a `NavThroughPoses` action request
        self.debug("Waiting for 'NavigateThroughPoses' action server")
        while not self.nav_through_poses_client.wait_for_server(timeout_sec=1.0):
            self.info("'NavigateThroughPoses' action server not available, waiting...")

        goal_msg = NavigateThroughPoses.Goal()
        goal_msg.poses = poses

        self.info('Navigating with ' + str(len(goal_msg.poses)) + ' goals.' + '...')
        send_goal_future = self.nav_through_poses_client.send_goal_async(goal_msg,
                                                                         self._feedbackCallback)
        rclpy.spin_until_future_complete(self, send_goal_future)
        self.goal_handle = send_goal_future.result()

        if not self.goal_handle.accepted:
            self.error('Goal with ' + str(len(poses)) + ' poses was rejected!')
            return False
        self.workstatus_pub = self.create_publisher(String, 'robot_status/working',10)

        self.result_future = self.goal_handle.get_result_async()
        return True

    def goToPose(self, pose):
        # Sends a `NavToPose` action request
        self.debug("Waiting for 'NavigateToPose' action server")
        while not self.nav_to_pose_client.wait_for_server(timeout_sec=1.0):
            self.info("'NavigateToPose' action server not available, waiting...")

        goal_msg = NavigateToPose.Goal()
        goal_msg.pose = pose

        self.info('Navigating to goal: ' + str(pose.pose.position.x) + ' ' +
                      str(pose.pose.position.y) + '...')
        send_goal_future = self.nav_to_pose_client.send_goal_async(goal_msg,
                                                                   self._feedbackCallback)
        rclpy.spin_until_future_complete(self, send_goal_future)
        self.goal_handle = send_goal_future.result()

        if not self.goal_handle.accepted:
            self.error('Goal to ' + str(pose.pose.position.x) + ' ' +
                           str(pose.pose.position.y) + ' was rejected!')
            return False

        self.result_future = self.goal_handle.get_result_async()
        return True

    def followWaypoints(self, poses):
        # Sends a `FollowWaypoints` action request
        self.debug("Waiting for 'FollowWaypoints' action server")
        while not self.follow_waypoints_client.wait_for_server(timeout_sec=1.0):
            self.info("'FollowWaypoints' action server not available, waiting...")

        goal_msg = FollowWaypoints.Goal()
        goal_msg.poses = poses

        self.info('Following ' + str(len(goal_msg.poses)) + ' goals.' + '...')
        send_goal_future = self.follow_waypoints_client.send_goal_async(goal_msg,
                                                                        self._feedbackCallback)
        rclpy.spin_until_future_complete(self, send_goal_future)
        self.goal_handle = send_goal_future.result()

        if not self.goal_handle.accepted:
            self.error('Following ' + str(len(poses)) + ' waypoints request was rejected!')
            return False

        self.result_future = self.goal_handle.get_result_async()
        return True

    def cancelNav(self):
        self.get_logger().info('Canceling current goal.')
        if self.result_future:
            future = self.goal_handle.cancel_goal_sync()        
            rclpy.spin_until_future_complete(self, future)
        return

      
    def isNavComplete(self):
        if not self.result_future:
            # task was cancelled or completed
            return True
        rclpy.spin_until_future_complete(self, self.result_future, timeout_sec=0.10)
        if self.result_future.result():
            self.status = self.result_future.result().status
            if self.status != GoalStatus.STATUS_SUCCEEDED:
                self.debug('Goal with failed with status code: {0}'.format(self.status))
                return True
        else:
            # Timed out, still processing, not complete yet
            return False

        self.debug('Goal succeeded!')
        return True

    def getFeedback(self):
        return self.feedback

    def getResult(self):
        if self.status == GoalStatus.STATUS_SUCCEEDED:
            return NavigationResult.SUCCEEDED
        elif self.status == GoalStatus.STATUS_ABORTED:
            return NavigationResult.FAILED
        elif self.status == GoalStatus.STATUS_CANCELED:
            return NavigationResult.CANCELED
        else:
            return "UNKNOWN"
            #return NavigationResult.UNKNOWN

    def waitUntilNav2Active(self):
        self._waitForNodeToActivate('amcl')
        self._waitForInitialPose()
        self._waitForNodeToActivate('bt_navigator')
        self.info('Nav2 is ready for use!')
    

    def getPath(self, start, goal):
        # Sends a `NavToPose` action request
        self.debug("Waiting for 'ComputePathToPose' action server")
        while not self.compute_path_to_pose_client.wait_for_server(timeout_sec=1.0):
            self.info("'ComputePathToPose' action server not available, waiting...")

        goal_msg = ComputePathToPose.Goal()
        goal_msg.goal = goal
        goal_msg.start = start

        self.info('Getting path...')
        send_goal_future = self.compute_path_to_pose_client.send_goal_async(goal_msg)
        rclpy.spin_until_future_complete(self, send_goal_future)
        self.goal_handle = send_goal_future.result()

        if not self.goal_handle.accepted:
            self.error('Get path was rejected!')
            return None

        self.result_future = self.goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self, self.result_future)
        self.status = self.result_future.result().status
        if self.status != GoalStatus.STATUS_SUCCEEDED:
            self.warn('Getting path failed with status code: {0}'.format(self.status))
            return None

        return self.result_future.result().result.path

    def getPathThroughPoses(self, start, goals):
        # Sends a `NavToPose` action request
        self.debug("Waiting for 'ComputePathThroughPoses' action server")
        while not self.compute_path_through_poses_client.wait_for_server(timeout_sec=1.0):
            self.info("'ComputePathThroughPoses' action server not available, waiting...")

        goal_msg = ComputePathThroughPoses.Goal()
        goal_msg.goals = goals
        goal_msg.start = start

        self.info('Getting path...')
        send_goal_future = self.compute_path_through_poses_client.send_goal_async(goal_msg)
        rclpy.spin_until_future_complete(self, send_goal_future)
        self.goal_handle = send_goal_future.result()

        if not self.goal_handle.accepted:
            self.error('Get path was rejected!')
            return None

        self.result_future = self.goal_handle.get_result_async()
        rclpy.spin_until_future_complete(self, self.result_future)
        self.status = self.result_future.result().status
        if self.status != GoalStatus.STATUS_SUCCEEDED:
            self.warn('Getting path failed with status code: {0}'.format(self.status))
            return None

        return self.result_future.result().result.path

    def changeMap(self, map_filepath):
        while not self.change_maps_srv.wait_for_service(timeout_sec=1.0):
            self.info('change map service not available, waiting...')
        req = LoadMap.Request()
        req.map_url = map_filepath
        future = self.change_maps_srv.call_async(req)
        rclpy.spin_until_future_complete(self, future)
        status = future.result().result
        if status != LoadMap.Response().RESULT_SUCCESS:
            self.error('Change map request failed!')
        else:
            self.info('Change map request was successful!')
   

    def clearAllCostmaps(self):
        self.clearLocalCostmap()
        self.clearGlobalCostmap()
   

    def clearLocalCostmap(self):
        while not self.clear_costmap_local_srv.wait_for_service(timeout_sec=1.0):
            self.info('Clear local costmaps service not available, waiting...')
        req = ClearEntireCostmap.Request()
        future = self.clear_costmap_local_srv.call_async(req)
        rclpy.spin_until_future_complete(self, future)
   

    def clearGlobalCostmap(self):
        while not self.clear_costmap_global_srv.wait_for_service(timeout_sec=1.0):
            self.info('Clear global costmaps service not available, waiting...')
        req = ClearEntireCostmap.Request()
        future = self.clear_costmap_global_srv.call_async(req)
        rclpy.spin_until_future_complete(self, future)


    def getGlobalCostmap(self):
        while not self.get_costmap_global_srv.wait_for_service(timeout_sec=1.0):
            self.info('Get global costmaps service not available, waiting...')
        req = GetCostmap.Request()
        future = self.get_costmap_global_srv.call_async(req)
        rclpy.spin_until_future_complete(self, future)
        return future.result().map

    def getLocalCostmap(self):
        while not self.get_costmap_local_srv.wait_for_service(timeout_sec=1.0):
            self.info('Get local costmaps service not available, waiting...')
        req = GetCostmap.Request()
        future = self.get_costmap_local_srv.call_async(req)
        rclpy.spin_until_future_complete(self, future)
        return future.result().map

    def lifecycleStartup(self):
        self.info('Starting up lifecycle nodes based on lifecycle_manager.')
        srvs = self.get_service_names_and_types()
        for srv in srvs:
            if srv[1][0] == 'nav2_msgs/srv/ManageLifecycleNodes':
                srv_name = srv[0]
                self.info('Starting up ' + srv_name)
                mgr_client = self.create_client(ManageLifecycleNodes, srv_name)
                while not mgr_client.wait_for_service(timeout_sec=1.0):
                    self.info(srv_name + ' service not available, waiting...')
                req = ManageLifecycleNodes.Request()
                req.command = ManageLifecycleNodes.Request().STARTUP
                future = mgr_client.call_async(req)

                # starting up requires a full map->odom->base_link TF tree
                # so if we're not successful, try forwarding the initial pose
                while True:
                    rclpy.spin_until_future_complete(self, future, timeout_sec=0.10)
                    if not future:
                        self._waitForInitialPose()
                    else:
                        break
        self.info('Nav2 is ready for use!')


    def lifecycleShutdown(self):
        self.info('Shutting down lifecycle nodes based on lifecycle_manager.')
        srvs = self.get_service_names_and_types()
        for srv in srvs:
            if srv[1][0] == 'nav2_msgs/srv/ManageLifecycleNodes':
                srv_name = srv[0]
                self.info('Shutting down ' + srv_name)
                mgr_client = self.create_client(ManageLifecycleNodes, srv_name)
                while not mgr_client.wait_for_service(timeout_sec=1.0):
                    self.info(srv_name + ' service not available, waiting...')
                req = ManageLifecycleNodes.Request()
                req.command = ManageLifecycleNodes.Request().SHUTDOWN
                future = mgr_client.call_async(req)
                rclpy.spin_until_future_complete(self, future)
                future.result()
   

    def _waitForNodeToActivate(self, node_name):
        # Waits for the node within the tester namespace to become active
        self.debug('Waiting for ' + node_name + ' to become active..')
        node_service = node_name + '/get_state'
        state_client = self.create_client(GetState, node_service)
        while not state_client.wait_for_service(timeout_sec=1.0):
            self.info(node_service + ' service not available, waiting...')

        req = GetState.Request()
        state = 'unknown'
        while (state != 'active'):
            self.debug('Getting ' + node_name + ' state...')
            future = state_client.call_async(req)
            rclpy.spin_until_future_complete(self, future)
            if future.result() is not None:
                state = future.result().current_state.label
                self.debug('Result of get_state: %s' % state)
            time.sleep(2)
    

    def _waitForInitialPose(self):
        while not self.initial_pose_received:
            self.info('Setting initial pose')
            self._setInitialPose()        

            self.info('Waiting for amcl_pose to be received')
            rclpy.spin_once(self, timeout_sec=1.0)


    def _amclPoseCallback(self, msg):
        self.debug('Received amcl pose')
        self.initial_pose_received = True
        

    def _feedbackCallback(self, msg):
        self.debug('Received action feedback message')
        self.feedback = msg.feedback

    def _setInitialPose(self):
        msg = PoseWithCovarianceStamped()
        msg.pose.pose = self.initial_pose.pose
        msg.header.frame_id = self.initial_pose.header.frame_id
        msg.header.stamp = self.initial_pose.header.stamp
        self.info('Publishing Initial Pose')
        self.initial_pose_pub.publish(msg)

    def info(self, msg):
        self.get_logger().info(msg)

    def warn(self, msg):
        self.get_logger().warn(msg)


    def error(self, msg):
        self.get_logger().error(msg)

    def debug(self, msg):
        self.get_logger().debug(msg)


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
    
    def set_goal(self, x,y,yaw):
        heading = yaw #/ 180 * 3.14159 
        q = self.quaternion_from_euler(0,0,yaw)
        
        goal_pose = PoseStamped()
        goal_pose.header.frame_id = 'map'
        goal_pose.header.stamp = self.get_clock().now().to_msg()
        goal_pose.pose.position.x = x
        goal_pose.pose.position.y = y
        goal_pose.pose.position.z = 0.0
        goal_pose.pose.orientation.x = q[1]
        goal_pose.pose.orientation.y = q[2]
        goal_pose.pose.orientation.z = q[3]
        goal_pose.pose.orientation.w = q[0]
        
        return goal_pose
    
    def euler_from_quaternion(self, quaternion):
        x = quaternion.x
        y = quaternion.y
        z = quaternion.z
        w = quaternion.w

        sinr_cosp = 2 * (w * x + y * z)
        cosr_cosp = 1 - 2 * (x * x + y * y)
        roll = np.arctan2(sinr_cosp, cosr_cosp)

        sinp = 2 * (w * y - z * x)
        pitch = np.arcsin(sinp)

        siny_cosp = 2 * (w * z + x * y)
        cosy_cosp = 1 - 2 * (y * y + z * z)
        yaw = np.arctan2(siny_cosp, cosy_cosp)

        return roll, pitch, yaw


    def getInitialPose(self, x, y ,theta):

        ip = PoseStamped()
        ip.header.frame_id = "map"
        ip.header.stamp = self.get_clock().now().to_msg()
        ip.pose.position.x = x
        ip.pose.position.y = y

        q = self.quaternion_from_euler(0.0,0.0,theta)
        ip.pose.orientation.w = q[0]
        ip.pose.orientation.x = q[1]
        ip.pose.orientation.y = q[2]
        ip.pose.orientation.z = q[3]

        self.setInitialPose(ip)

    def updateWorkStatus(self, stat):
        msg = String()
        msg.data = stat

        self.workstatus_pub.publish(msg)

class Mission:
    def __init__(self):
        
        self.navigator = BasicNavigator()

        self.goal_point = ''
    
    def start2Pose(self, wp):
        wp_list = []
        for p in wp:
            point = self.navigator.set_goal(p[0], p[1], 0)
            wp_list.append(point)
        self.navigator.goThroughPoses(wp_list)
        #self.navigator.followWaypoints(wp_list)
    def startGoal(self, goal):
        self.navigator.updateWorkStatus("Got Nav Order") 

        
        point = self.navigator.set_goal(goal[0],goal[1],goal[2])
        self.navigator.goToPose(point)


    def startNav(self, waypoint):

        nav_start = self.navigator.get_clock().now()
        i = 0
        while not self.navigator.isNavComplete():
            i = i + 1
            feedback = self.navigator.getFeedback()
            if feedback and i % 5 == 0:
                print('Executing current waypoint: ' +
                str(feedback.current_waypoint + 1) + '/' + str(len(waypoint)))  
                self.navigator.updateWorkStatus("Moving")      
        
        # Do something depending on the return code
        result = self.navigator.getResult()
        if result == NavigationResult.SUCCEEDED:
            print('Goal succeeded!')
            self.navigator.updateWorkStatus("Goal Success")
            
        elif result == NavigationResult.CANCELED:
            print('Goal was canceled!')
            
        elif result == NavigationResult.FAILED:
            print('Goal failed!')
        else:
            print('Goal has an invalid return status!')
 
    
    def set_initial_pose(self, x,y,theta):
        self.navigator.getInitialPose(x,y,theta)
      
    def set_pose_history(self,x,y,yaw):
        self.pose_history[0] = x
        self.pose_history[1] = y
        self.pose_history[2] = yaw
    
    def cancel_operation(self):
        self.navigator.cancelNav()
    

def main():
    rclpy.init()
    ms = Mission()
    ms.call('A', 'C')
    
 
 
if __name__ == '__main__':
    main()