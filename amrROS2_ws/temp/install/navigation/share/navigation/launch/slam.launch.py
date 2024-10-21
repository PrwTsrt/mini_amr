import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.actions import Node


def generate_launch_description():

    navigation_dir = get_package_share_directory("navigation")
    slam_toolbox_dir = get_package_share_directory("slam_toolbox")
    nav2_dir = get_package_share_directory("nav2_bringup")


    use_sim_time_arg = DeclareLaunchArgument(
            name='sim', 
            default_value='false',
            description='Enable use_sime_time to true'
        )
    rviz_arg = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    
    rviz = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        condition=IfCondition(LaunchConfiguration("rviz")),
        arguments=["-d", os.path.join(navigation_dir, "rviz", "slam.rviz")],
    )
    slam = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
            [slam_toolbox_dir, "launch", "online_async_launch.py"]
        )),
        launch_arguments={
            'use_sim_time': LaunchConfiguration("sim"),
            'slam_params_file': PathJoinSubstitution(
                [navigation_dir, "config", "mapper_params_online_async.yaml"]
            )
        }.items()
    )
    nav = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [nav2_dir, 'launch', 'navigation_launch.py']
            )),
            launch_arguments={
                'use_sim_time': LaunchConfiguration("sim"),
                'params_file':  PathJoinSubstitution(
                    [navigation_dir, "config", "navigation.yaml"]
                )
            }.items()
        )


    return LaunchDescription([
        use_sim_time_arg,
        rviz_arg,
        slam,
        nav,
        rviz,
    ])