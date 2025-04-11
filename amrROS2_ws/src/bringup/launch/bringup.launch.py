import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction
from launch.substitutions import Command, LaunchConfiguration, EnvironmentVariable
from launch.conditions import IfCondition
from launch_ros.actions import Node, PushRosNamespace, SetRemap
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():

    robot_description_dir = get_package_share_directory("description")
    robot_bringup_dir = get_package_share_directory("bringup")

    params_file = LaunchConfiguration("params_file")
    use_rviz = LaunchConfiguration("bringup_rviz")
    use_sim_time = LaunchConfiguration("sim")
    
    declare_model_cmd = DeclareLaunchArgument(
            name="model", 
            default_value=os.path.join(robot_description_dir, "urdf/robot", "robot.urdf.xacro"),
            description="Absolute path to robot urdf file")
    
    declare_use_rviz_cmd = DeclareLaunchArgument(
            name='bringup_rviz', 
            default_value='false',
            description='Run rviz'
        )
    declare_use_sim_time_cmd = DeclareLaunchArgument(
            name='sim', 
            default_value='false',
            description='Enable use_sime_time to true'
        )
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(robot_bringup_dir, "config", "bringup.yaml"),
        description='Full path to the parameters file')
    
    robot_description = ParameterValue(Command(["xacro ", LaunchConfiguration("model")]),
                                       value_type=str)

    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}]
    )

    joint_state_publisher_node = Node(
        package="joint_state_publisher",
        executable="joint_state_publisher",
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", os.path.join(robot_description_dir, "rviz", "bringup.rviz")],
        condition=IfCondition(use_rviz),
        parameters=[{'use_sim_time': use_sim_time}]
    )
    
    # scan = IncludeLaunchDescription(os.path.join(
    #     get_package_share_directory("sllidar_ros2"),
    #     "launch",
    #     "sllidar_s2e_launch.py")
    # ) 

    scan = Node(
            package='sllidar_ros2',
            executable='sllidar_node',
            name='sllidar_node',
            parameters=[{'channel_type': 'udp', 
                         'udp_ip': '192.168.11.2',
                         'udp_port': 8089,
                         'frame_id': 'laser',
                         'inverted': False, 
                         'angle_compensate': True, 
                         'scan_mode': 'Sensitivity'}],
            remappings=[("scan", "raw_scan")],
            output='screen')

    laser_filter = Node(
            package='laser_filters',
            executable='scan_to_scan_filter_chain',
            remappings=[('scan', 'raw_scan'),
                ('scan_filtered','scan')],
            parameters=[params_file],
        )


    # camera = Node(
    #         package='usb_cam', 
    #         executable='usb_cam_node_exe',
    #         output='screen',
    #         name="usb_camera",
    #         parameters=[params_file]
    #     )

    robot_localization = Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[params_file],
            remappings=[("odometry/filtered", "odom")]
        ) 

    hardware_node = Node(
        package="robot_hardware_interface",
        executable="robot_hardware",
        name="robot_hardware",
        output="screen",
        parameters=[{'serial_port': "/dev/teensy"}],
    )

    launch_elements = GroupAction(
     actions=[
        SetRemap('/tf','tf'),
        SetRemap('/tf_static','tf_static'),
        robot_state_publisher_node,
        joint_state_publisher_node,   
        scan,
        laser_filter,
        rviz_node, 
        robot_localization,
        hardware_node
      ]
   )

    return LaunchDescription([
        # Declare the launch options
        declare_params_file_cmd,
        declare_model_cmd,
        declare_use_sim_time_cmd,
        declare_use_rviz_cmd,
        #launch all of the bringup nodes
        launch_elements
    ])
