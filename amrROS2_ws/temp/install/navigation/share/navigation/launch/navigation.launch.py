import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression
from launch_ros.substitutions import FindPackageShare
from launch.conditions import IfCondition, UnlessCondition

from launch_ros.actions import Node

def generate_launch_description():

    navigation_dir = get_package_share_directory("navigation")
    bringup_dir = get_package_share_directory("bringup")
    nav2_dir = get_package_share_directory("nav2_bringup")
    slam_toolbox_dir = get_package_share_directory("slam_toolbox")

    default_map_path = os.path.join(navigation_dir, "maps/Turtlebot_Arena_map.yaml")

    slam_config_path = os.path.join(navigation_dir, "config/slam_localization.yaml")

    map_arg = DeclareLaunchArgument(
            name='map', 
            default_value=default_map_path,
            description='Navigation map path'
        )
    sim_arg = DeclareLaunchArgument(
            name='sim', 
            default_value='false',
            description='Enable use_sime_time to true'
        )
    rviz_arg = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    slam_tb_arg = DeclareLaunchArgument(
            name='slam_tb', 
            default_value='true',
            description='Using slam toolbox localization'
        )

    rviz = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        condition=IfCondition(LaunchConfiguration("rviz")),
        arguments=["-d", os.path.join(navigation_dir, "rviz", "slam.rviz")],
        )
    nav = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [navigation_dir, 'launch', 'navigation_launch.py'])),
            launch_arguments={
                'use_sim_time': LaunchConfiguration("sim"),
                'params_file':  PathJoinSubstitution(
                    [navigation_dir, "config", "navigation.yaml"]),
                'map_subscribe_transient_local' : 'true'
            }.items()
        )
    amcl = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [nav2_dir, 'launch', 'localization_launch.py'])),
            condition=UnlessCondition(LaunchConfiguration("slam_tb")),
            launch_arguments={
                'map' : LaunchConfiguration("map"),
                'use_sim_time': LaunchConfiguration("sim"),
            }.items()
        )
    slam_tb_localization = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
            [slam_toolbox_dir, 'launch', 'localization_launch.py'])),
        condition=IfCondition(LaunchConfiguration("slam_tb")),
        launch_arguments={                             
                            'use_sim_time': LaunchConfiguration("sim"),
                            'slam_params_file': slam_config_path,
                            }.items()
        )   


    return LaunchDescription([
        map_arg,
        sim_arg,
        rviz_arg,
        slam_tb_arg,
        # amcl,
        nav,
        rviz,
        slam_tb_localization,
    ])