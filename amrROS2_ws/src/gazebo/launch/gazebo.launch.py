import os
from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, GroupAction, ExecuteProcess, SetEnvironmentVariable
from launch.substitutions import Command, LaunchConfiguration, EnvironmentVariable, FindExecutable
from launch.conditions import IfCondition
from launch_ros.actions import Node, PushRosNamespace, SetRemap
from launch_ros.parameter_descriptions import ParameterValue
from nav2_common.launch import RewrittenYaml
from launch.conditions import IfCondition, UnlessCondition
from launch_ros.descriptions import ParameterFile


def generate_launch_description():

    robot_description_dir = get_package_share_directory("description")
    gazebo_dir = get_package_share_directory("gazebo")

    world_path = os.path.join(gazebo_dir, "world/empty.sdf")
    gazebo_model_path = os.path.join(gazebo_dir, "models")

    set_gz_resource_cmd = SetEnvironmentVariable(name="IGN_GAZEBO_RESOURCE_PATH", value = gazebo_model_path)
    set_gz_model_cmd = SetEnvironmentVariable(name="IGN_GAZEBO_MODEL_PATH", value = gazebo_model_path)

    gz_env = {'GZ_SIM_SYSTEM_PLUGIN_PATH':
           ':'.join([os.environ.get('GZ_SIM_SYSTEM_PLUGIN_PATH', default=''),
                     os.environ.get('LD_LIBRARY_PATH', default='')]),
           'IGN_GAZEBO_SYSTEM_PLUGIN_PATH':  # TODO(CH3): To support pre-garden. Deprecated.
                      ':'.join([os.environ.get('IGN_GAZEBO_SYSTEM_PLUGIN_PATH', default=''),
                                os.environ.get('LD_LIBRARY_PATH', default='')])}

    params_file = LaunchConfiguration("params_file")
    namespace = LaunchConfiguration("namespace")
    use_rviz = LaunchConfiguration("rviz")
    use_sim_time = LaunchConfiguration("sim")
    namespace = LaunchConfiguration("namespace")

    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value= [EnvironmentVariable('NAMESPACE')],
        description='prefix for node name')
    
    
    declare_model_cmd = DeclareLaunchArgument(
            name="model", 
            default_value=os.path.join(robot_description_dir, "urdf/robot", "robot.urdf.xacro"),
            description="Absolute path to robot urdf file")
    
    declare_use_rviz_cmd = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    
    declare_use_sim_time_cmd = DeclareLaunchArgument(
            name='sim', 
            default_value='true',
            description='Enable use_sime_time to true'
        )
    declare_use_namespace_cmd = DeclareLaunchArgument(
            name='use_namespace', 
            default_value='true',
            description='Enable use_sime_time to true'
        )
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(gazebo_dir, "config", "gazebo.yaml"),
        description='Full path to the parameters file'
        )
    
    declare_headless_cmd = DeclareLaunchArgument(
            name="run_headless",
            default_value="false",
            description="Start GZ in hedless mode and don't start RViz (overrides use_rviz)"
        )   
    declare_gz_verbose_cmd = DeclareLaunchArgument(
            "gz_verbosity",
            default_value="3",
            description="Verbosity level for Ignition Gazebo (0~4).",
        )
    declare_log_level_cmd = DeclareLaunchArgument(
            name="log_level",
            default_value="warn",
            description="The level of logging that is applied to all ROS 2 nodes launched by this script.",
        )

    configured_params = ParameterFile(
        RewrittenYaml(
            source_file=params_file,
            root_key=namespace,
            param_rewrites={},
            convert_types=True),
        allow_substs=True
    )
    
    robot_description = ParameterValue(Command(["xacro ", LaunchConfiguration("model")]),
                                       value_type=str)
    
    robot_state_publisher_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        parameters=[{"robot_description": robot_description}]
    )

    rviz_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        output="screen",
        arguments=["-d", os.path.join(gazebo_dir, "rviz", "bringup.rviz")],
        condition=IfCondition(use_rviz),
        parameters=[{'use_sim_time': use_sim_time}]
    )

    robot_localization = Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[configured_params],
            remappings=[("odometry/filtered", "odom")]
        ) 
    
    gazebo = [
        ExecuteProcess(
            condition=IfCondition(LaunchConfiguration("run_headless")),
            cmd=[FindExecutable(name="ign"), 'gazebo',  '-r', '-v', LaunchConfiguration("gz_verbosity"), '-s', '--headless-rendering', world_path],
            output='screen',
            additional_env=gz_env, # type: ignore
            shell=False,
        ),
        ExecuteProcess(
            condition=UnlessCondition(LaunchConfiguration("run_headless")),
            cmd=[FindExecutable(name="ign"), 'gazebo',  '-r', '-v', LaunchConfiguration("gz_verbosity"), world_path],
            output='screen',
            additional_env=gz_env, # type: ignore
            shell=False,
        )
    ]

    spawn_entity = Node(
        package="ros_gz_sim",
        executable="create",
        output="screen",
        arguments=[
            "-name",
            "robot",
            "-topic",
            "robot_description",
            "-z",
            "1.0",
            "-x",
            "-2.0",
            "--ros-args",
            "--log-level",
            LaunchConfiguration("log_level"),
        ],
        parameters=[{"use_sim_time":LaunchConfiguration("sim")}],
    )

    ros_gz_bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            "scan@sensor_msgs/msg/LaserScan[ignition.msgs.LaserScan",
            "imu@sensor_msgs/msg/Imu[ignition.msgs.IMU",
            "sky_cam@sensor_msgs/msg/Image@ignition.msgs.Image",
            "robot_cam@sensor_msgs/msg/Image@ignition.msgs.Image",
            "camera_info@sensor_msgs/msg/CameraInfo@ignition.msgs.CameraInfo",
            "clock@rosgraph_msgs/msg/Clock[ignition.msgs.Clock",
        ],
        output="screen",
    )

    joint_state_broadcaster_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            "joint_state_broadcaster",
            "--controller-manager",
            "/controller_manager",
        ],
    )

    controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=["diff_drive_base_controller", 
                   "--controller-manager", 
                   "/controller_manager"
        ],
    )

    relay_odom = Node(
        name="relay_odom",
        package="topic_tools",
        executable="relay",
        parameters=[
            {
                "input_topic": "diff_drive_base_controller/odom",
                "output_topic": "odom_raw",
            }
        ],
        output="screen",
    )
    
    relay_cmd_vel = Node(
        name="relay_cmd_vel",
        package="topic_tools",
        executable="relay",
        parameters=[{
                "input_topic": "cmd_vel",
                "output_topic": "diff_drive_base_controller/cmd_vel_unstamped",
        }],
        output="screen",
    )

    launch_elements = GroupAction(
     actions=[
        PushRosNamespace(namespace),
        SetRemap('/tf','tf'),
        SetRemap('/tf_static','tf_static'),
        robot_state_publisher_node,
        rviz_node, 
        robot_localization,
        spawn_entity,
        ros_gz_bridge,
        joint_state_broadcaster_spawner,
        controller_spawner,
        relay_odom,
        relay_cmd_vel
      ] + gazebo
   )

    return LaunchDescription([
        # Declare the launch options
        set_gz_resource_cmd,
        set_gz_model_cmd,
        declare_headless_cmd,
        declare_gz_verbose_cmd,
        declare_log_level_cmd,
        declare_namespace_cmd,
        declare_params_file_cmd,
        declare_model_cmd,
        declare_use_sim_time_cmd,
        declare_use_rviz_cmd,
        declare_use_namespace_cmd,
        #launch all of the bringup nodes
        launch_elements
    ])
