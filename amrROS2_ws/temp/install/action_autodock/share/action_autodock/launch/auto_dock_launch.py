from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='dock_lidar',
            executable='dock_coordinates',
            name='dock_coordinates',
            parameters=[os.path.join(get_package_share_directory('dock_lidar'),'config','dock_config.yaml')]
        ),
        Node(
            package='action_autodock',
            executable='autodock_action_server',
            name='autodock',
            parameters=[os.path.join(get_package_share_directory('action_autodock'),'config','auto_dock_config.yaml')]
        ),
        
    ])