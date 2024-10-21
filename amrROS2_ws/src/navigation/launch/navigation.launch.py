import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, TimerAction, GroupAction
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression, EnvironmentVariable, TextSubstitution
from launch_ros.substitutions import FindPackageShare
from launch.conditions import IfCondition, UnlessCondition

from launch_ros.actions import Node, PushRosNamespace, SetRemap
from nav2_common.launch import RewrittenYaml, ReplaceString
from launch_ros.descriptions import ParameterFile

def generate_launch_description():

    robot_navigation_dir = get_package_share_directory("navigation")
    robot_bringup_dir = get_package_share_directory("bringup")
    nav2_bringup_dir = get_package_share_directory("nav2_bringup")
    slam_toolbox_dir = get_package_share_directory("slam_toolbox")

    namespace = LaunchConfiguration('namespace')
    use_rviz = LaunchConfiguration("rviz")
    use_sim_time = LaunchConfiguration('sim')
    use_slam_tb = LaunchConfiguration('slam_tb')
    params_file = LaunchConfiguration('params_file')
    map = LaunchConfiguration("map")

    default_map_path = os.path.join(robot_navigation_dir, "maps/Turtlebot_Arena_map.yaml")
    slam_config_path = os.path.join(robot_navigation_dir, "config/slam_localization.yaml")

    params_file_ = ReplaceString(
            source_file=PathJoinSubstitution([robot_navigation_dir, "config", "navigation.yaml"]),
            replacements={'<robot_namespace>': ("/",LaunchConfiguration("namespace"))},)    
    
    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value= [EnvironmentVariable('NAMESPACE')],
        description='prefix for node name')

    declare_map_cmd = DeclareLaunchArgument(
            name='map', 
            default_value=default_map_path,
            description='Navigation map path'
        )
    declare_use_sim_time_cmd = DeclareLaunchArgument(
            name='sim', 
            default_value='false',
            description='Enable use_sime_time to true'
        )
    declare_use_rviz_cmd = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    declare_use_slam_tb_cmd = DeclareLaunchArgument(
            name='slam_tb', 
            default_value='true',
            description='Using slam toolbox localization'
        )
    
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value= params_file_,
        description='Full path to the ROS2 parameters file to use for all launched nodes')
    
    slam_configured_params = ParameterFile(
        RewrittenYaml(
            source_file=slam_config_path,
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
        arguments=["-d", os.path.join(robot_navigation_dir, "rviz", "navigation.rviz")],
        )
 
    navigation = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [robot_navigation_dir, 'launch', 'navigation_launch.py'])),
            launch_arguments={
                'use_sim_time': use_sim_time,
                'params_file':  params_file,
                'map_subscribe_transient_local': 'true'
            }.items()
        )
    
    amcl = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(PathJoinSubstitution(
                [nav2_bringup_dir, 'launch', 'localization_launch.py'])),
            condition=UnlessCondition(use_slam_tb),
            launch_arguments={
                'map' : map,
                'use_sim_time': use_sim_time,
                'params_file':  params_file,
            }.items()
        )

    slam_tb_localization = Node(
        condition=IfCondition(use_slam_tb),
        parameters=[
          slam_configured_params,
          {'use_sim_time': use_sim_time}
        ],
        package='slam_toolbox',
        executable='localization_slam_toolbox_node',
        name='slam_toolbox',
        output='screen',
        remappings=[
                ("/map", "map"),
                ("/map_metadata", "map_metadata")]    
        )   

    launch_elements = GroupAction(
     actions=[
        PushRosNamespace(namespace),
        SetRemap('/tf','tf'),
        SetRemap('/tf_static','tf_static'),
        amcl,
        slam_tb_localization,
        navigation,
        rviz_node,
      ]
   )

    return LaunchDescription([
        # Declare the launch options
        declare_namespace_cmd,
        declare_params_file_cmd,
        declare_map_cmd,
        declare_use_sim_time_cmd,
        declare_use_rviz_cmd,
        declare_use_slam_tb_cmd,
        #Launch all navigation nodes
        launch_elements
    ])