import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, EnvironmentVariable
from launch.conditions import IfCondition
from launch_ros.actions import Node, PushRosNamespace, SetRemap

from launch_ros.descriptions import ParameterFile
from nav2_common.launch import RewrittenYaml


def generate_launch_description():

    robot_navigation_dir = get_package_share_directory("navigation")
    slam_toolbox_dir = get_package_share_directory("slam_toolbox")
    nav2_bringup_dir = get_package_share_directory("nav2_bringup")

    namespace = LaunchConfiguration('namespace')
    params_file = LaunchConfiguration('params_file')
    use_rviz = LaunchConfiguration("rviz")
    use_sim_time = LaunchConfiguration('sim')
    namespace = LaunchConfiguration("namespace")

    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value= [EnvironmentVariable('NAMESPACE')],
        description='prefix for node name')
    
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=PathJoinSubstitution([robot_navigation_dir, "config", "mapper_params_online_async.yaml"]),
        description='Full path to the ROS2 parameters file to use for all launched nodes')
    
    declare_use_sim_time_cmd = DeclareLaunchArgument(
            name='sim', 
            default_value='true',
            description='Enable use_sime_time to true')
    
    declare_use_rviz_cmd = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz')
    
    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=params_file,
            root_key=namespace,
            param_rewrites='',
            convert_types=True),
        allow_substs=True)

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        condition=IfCondition(use_rviz),
        arguments=["-d", os.path.join(robot_navigation_dir, "rviz", "slam.rviz")],
    )

    slam_toolbox_node = Node(
        parameters=[
          configured_params,
          {'use_sim_time': use_sim_time}
        ],
        package='slam_toolbox',
        executable='async_slam_toolbox_node',
        name='slam_toolbox',
        output='screen')
    
    navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [robot_navigation_dir, 'launch', 'navigation_launch.py']
            )),
            launch_arguments={
                'use_sim_time': use_sim_time,
                'params_file':  PathJoinSubstitution(
                    [robot_navigation_dir, "config", "navigation.yaml"]
                )
            }.items()
        )
    
    launch_elements = GroupAction(
     actions=[
        PushRosNamespace(namespace),
        SetRemap('/tf','tf'),
        SetRemap('/tf_static','tf_static'),
        SetRemap('/map','map'),
        SetRemap('/map_metadata','map_metadata'),
        slam_toolbox_node,
        navigation,
        rviz_node,
      ]
   )


    return LaunchDescription([
        # Declare the launch options
        declare_namespace_cmd,
        declare_params_file_cmd,
        declare_use_sim_time_cmd,
        declare_use_rviz_cmd,
        #Launch slam and navigation nodes
        launch_elements
    ])