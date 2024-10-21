import os
from ament_index_python import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.actions import GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.substitutions import TextSubstitution
from launch_ros.actions import Node
from launch_ros.actions import PushRosNamespace,SetRemap
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import EnvironmentVariable, LaunchConfiguration

def generate_launch_description():
    #robot_ns_launch_arg = DeclareLaunchArgument("robot_ns", default_value=TextSubstitution(text="lapras"))
    
    namespace = LaunchConfiguration("namespace")
    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value= [EnvironmentVariable('NAMESPACE')],
        description='prefix for node name')

    map_pose_provider = Node(
            package='robot_data_tool',
            executable='map_pose_provider',
            output='screen',
            remappings = [('/tf','tf'),('/tf_static','tf_static')],
        )
        
    nav_gui_bridge =  Node(
            package='robot_nav_tool',
            executable='nav_gui_bridge',
            output='screen',
        )
        
    robot_detail = Node(
	    package='robot_data_tool',
	    executable='robot_detail',
	    )

    rosbridge_websocket = Node(
        package='rosbridge_server',
        executable='rosbridge_websocket',
      )         
                
    launch_elements = GroupAction(
    	actions=[
        	PushRosNamespace(namespace),
        	SetRemap('/tf','tf'),
        	SetRemap('/tf_static','tf_static'),
            map_pose_provider,
            nav_gui_bridge,
            robot_detail,
            rosbridge_websocket,
        ]
       )
    
    return LaunchDescription([    	
    	 declare_namespace_cmd,
         launch_elements
 
       
    ])
