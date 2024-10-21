import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command, PathJoinSubstitution, EnvironmentVariable
from launch.conditions import IfCondition
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare

def generate_launch_description():

#---------------- Paths ----------------
    urdf_path = PathJoinSubstitution(
        [FindPackageShare("description"), "urdf/robot", "robot.urdf.xacro"]
    )

    rviz_config_path = PathJoinSubstitution(
        [FindPackageShare('description'), 'rviz', 'description.rviz']
    )

#---------------- Arguments ----------------
    model_arg = DeclareLaunchArgument(
            name="model",
            default_value=urdf_path,                                      
            description="Absolute path to robot urdf file"
            )
    rviz_arg = DeclareLaunchArgument(
            name='rviz', 
            default_value='true',
            description='Run rviz'
        )
    sim_arg = DeclareLaunchArgument(
            name='use_sim_time', 
            default_value='false',
            description='Use simulation time'
        )
    
#---------------- Node ----------------
    joint_state_publisher = Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            parameters=[
                {'use_sim_time': LaunchConfiguration('use_sim_time')}
            ]
        )
    robot_state_publisher = Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[
                {
                    'use_sim_time': LaunchConfiguration('use_sim_time'),
                    'robot_description': Command(['xacro ', LaunchConfiguration('model')])
                }
            ]
        )
    rviz = Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            arguments=['-d', rviz_config_path],
            condition=IfCondition(LaunchConfiguration("rviz")),
            parameters=[{'use_sim_time': LaunchConfiguration('use_sim_time')}]
        )

    return LaunchDescription([
        model_arg,
        rviz_arg,
        sim_arg,
        joint_state_publisher,
        robot_state_publisher,
        rviz,
    ])