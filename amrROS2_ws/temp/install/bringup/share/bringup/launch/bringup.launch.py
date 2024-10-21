import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.substitutions import Command, LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.conditions import IfCondition

from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    robot_description_dir = get_package_share_directory("description")
    robot_bringup_dir = get_package_share_directory("bringup")
    
    model_arg = DeclareLaunchArgument(name="model", default_value=os.path.join(
                                    robot_description_dir, "urdf/robot", "robot.urdf.xacro"),
                                    description="Absolute path to robot urdf file")
    
    rviz_arg = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    
    sim_arg = DeclareLaunchArgument(
            name='sim', 
            default_value='false',
            description='Enable use_sime_time to true'
        )
    
    robot_description = ParameterValue(Command(["xacro ", LaunchConfiguration("model")]),
                                       value_type=str)

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}]
    )

    joint_state_publisher_node = Node(
        package="joint_state_publisher",
        executable="joint_state_publisher"
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", os.path.join(robot_description_dir, "rviz", "bringup.rviz")],
        condition=IfCondition(LaunchConfiguration("rviz")),
        parameters=[{'use_sim_time': LaunchConfiguration("sim")}]
    )
    
    scan = IncludeLaunchDescription(os.path.join(
        get_package_share_directory("sllidar_ros2"),
        "launch",
        "sllidar_c1_launch.py")
    )

    camera = IncludeLaunchDescription(os.path.join(
        robot_bringup_dir,
        "launch",
        "camera.launch.py")
    )

    robot_localization = Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[os.path.join(robot_bringup_dir, "config", "ekf.yaml")],
            remappings=[("odometry/filtered", "odom")]
        ) 
    
    micro_ros_esp32 = Node(
            package='micro_ros_agent',
            executable='micro_ros_agent',
            name='micro_ros_esp32',
            output='screen',
            arguments=['serial', '--dev', '/dev/ttyUSB1', '-b', '921600', '-v4']
    )

    micro_ros_raspico = Node(
            package='micro_ros_agent',
            executable='micro_ros_agent',
            name='micro_ros_raspico',
            output='screen',
            arguments=['serial', '--dev', '/dev/ttyACM0', '-b', '921600', '-v4']
    )

    ip_pub_node = Node(
        package="python_pkg",
        executable="ip_publisher",
        name="ip_publisher",
        output="screen",
    )
   

    return LaunchDescription([
        model_arg,
        sim_arg,
        rviz_arg,
        micro_ros_esp32,
        micro_ros_raspico,  
        camera, 
        robot_state_publisher_node,
        joint_state_publisher_node,   
        scan,
        rviz_node, 
        robot_localization,
        ip_pub_node,
    ])
