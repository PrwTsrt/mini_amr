from launch import LaunchDescription
from launch_ros.actions import Node, PushRosNamespace,  SetRemap
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument, GroupAction
from launch.substitutions import EnvironmentVariable, LaunchConfiguration

import os

def generate_launch_description():
    
    namespace = LaunchConfiguration("namespace")
    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value= [EnvironmentVariable('NAMESPACE')],
        description='prefix for node name')
        
    dock_lidar = Node(
    	package='dock_lidar',
      	executable='dock_coordinates',
     	name='dock_coordinates',
    	parameters=[os.path.join(get_package_share_directory('dock_lidar'),'config','dock_config.yaml')]
   	)
    	
    auto_dock = Node(
      	package='action_autodock',
     	executable='autodock_action_server',
      	name='autodock',
     	parameters=[os.path.join(get_package_share_directory('action_autodock'),'config','auto_dock_config.yaml')]
        )
    	
    launch_elements = GroupAction(
    	actions=[
        PushRosNamespace(namespace),
        SetRemap('/tf','tf'),
        SetRemap('/tf_static','tf_static'),
        dock_lidar,
        auto_dock,
        ]
    )
    
    return LaunchDescription([
        declare_namespace_cmd,
        launch_elements
    ])
    
    
   
