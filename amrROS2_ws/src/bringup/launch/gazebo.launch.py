import os
from ament_index_python.packages import get_package_share_directory
from launch.actions import SetEnvironmentVariable, DeclareLaunchArgument, ExecuteProcess
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command, LaunchConfiguration, FindExecutable
from launch_ros.actions import Node
from launch.conditions import IfCondition
import launch
from launch import LaunchDescription



def generate_launch_description():

    description_dir_path = get_package_share_directory("description")
    bringup_dir_path = get_package_share_directory("bringup")

    world_path = os.path.join(bringup_dir_path, "world/empty.sdf")
    gazebo_model_path = os.path.join(bringup_dir_path, "models")

    pose = {'x': LaunchConfiguration('x_pose', default='-2.00'),
            'y': LaunchConfiguration('y_pose', default='0.00'),
            'z': LaunchConfiguration('z_pose', default='0.01'),
            'R': LaunchConfiguration('roll', default='0.00'),
            'P': LaunchConfiguration('pitch', default='0.00'),
            'Y': LaunchConfiguration('yaw', default='0.00')}

    gz_resource = SetEnvironmentVariable(name="GZ_SIM_RESOURCE_PATH", value = gazebo_model_path)
    gz_model = SetEnvironmentVariable(name="GAZEBO_MODEL_PATH", value = gazebo_model_path)

    gz_env = {'GZ_SIM_SYSTEM_PLUGIN_PATH':
           ':'.join([os.environ.get('GZ_SIM_SYSTEM_PLUGIN_PATH', default=''),
                     os.environ.get('LD_LIBRARY_PATH', default='')]),
                     'IGN_GAZEBO_SYSTEM_PLUGIN_PATH':  # TODO(CH3): To support pre-garden. Deprecated.
                      ':'.join([os.environ.get('IGN_GAZEBO_SYSTEM_PLUGIN_PATH', default=''),
                                os.environ.get('LD_LIBRARY_PATH', default='')])}
    

    model_arg = DeclareLaunchArgument(
        name="model",
        default_value=os.path.join(
        description_dir_path, "urdf/robot", "robot.urdf.xacro"),
        description="Path to robot urdf file"
    )
    rviz_arg = DeclareLaunchArgument(
            name='rviz', 
            default_value='false',
            description='Run rviz'
        )
    use_sim_time_arg = DeclareLaunchArgument(
            name='sim', 
            default_value='true',
            description='Enable use_sime_time to true'
        )
    run_headless_arg = DeclareLaunchArgument(
                name="run_headless",
                default_value="false",
                description="Start GZ in hedless mode and don't start RViz (overrides use_rviz)",
            )   
    gz_verbose_arg = DeclareLaunchArgument(
                "gz_verbosity",
                default_value="3",
                description="Verbosity level for Ignition Gazebo (0~4).",
            )
    log_level_arg = DeclareLaunchArgument(
                name="log_level",
                default_value="warn",
                description="The level of logging that is applied to all ROS 2 nodes launched by this script.",
            )   
    
    robot_description = ParameterValue(Command(["xacro ", LaunchConfiguration("model")]), value_type=str)

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
        arguments=["-d", os.path.join(description_dir_path, "rviz", "gazebo.rviz")],
        condition=IfCondition(LaunchConfiguration("rviz")),
        parameters=[{'use_sim_time': LaunchConfiguration("sim")}]
    )
    robot_localization = Node(
            package='robot_localization',
            executable='ekf_node',
            name='ekf_filter_node',
            output='screen',
            parameters=[
                os.path.join(bringup_dir_path, "config", "bringup.yaml"),
                {'use_sim_time': LaunchConfiguration("sim")}
                 ],
            remappings=[("odometry/filtered", "odom")]
    )
    gazebo = [
        ExecuteProcess(
            condition=launch.conditions.IfCondition(LaunchConfiguration("run_headless")),
            cmd=[FindExecutable(name="gz"), 'sim',  '-r', '-v', LaunchConfiguration("gz_verbosity"), '-s', '--headless-rendering', world_path],
            output='screen',
            additional_env=gz_env, 
            shell=False,
        ),
        ExecuteProcess(
            condition=launch.conditions.UnlessCondition(LaunchConfiguration("run_headless")),
            cmd=[FindExecutable(name="gz"), 'sim',  '-r', '-v', LaunchConfiguration("gz_verbosity"), world_path],
            output='screen',
            additional_env=gz_env, 
            shell=False,
        )
    ]
    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        arguments=[
            '-name', "robot",
            '-topic',"robot_description",
            '-x', pose['x'], '-y', pose['y'], '-z', pose['z'],
            '-R', pose['R'], '-P', pose['P'], '-Y', pose['Y']],
        parameters=[{"use_sim_time":LaunchConfiguration("sim")}],
    )

    bridge = Node(
        package="ros_gz_bridge",
        executable="parameter_bridge",
        arguments=[
            # "tf@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V", # Collid with tf from ekf node
            "/joint_states@sensor_msgs/msg/JointState[gz.msgs.Model",
            "/cmd_vel@geometry_msgs/msg/Twist]gz.msgs.Twist",
            "/odom_raw@nav_msgs/msg/Odometry[gz.msgs.Odometry",
            "/scan@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan",
            "/imu@sensor_msgs/msg/Imu[gz.msgs.IMU",
            "/robot_cam@sensor_msgs/msg/Image@gz.msgs.Image",
            "/camera_info@sensor_msgs/msg/CameraInfo@gz.msgs.CameraInfo",
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
        ],
        output="screen",
    )


    return LaunchDescription([
        gz_resource,
        gz_model,
        model_arg,
        rviz_arg,
        use_sim_time_arg,
        run_headless_arg,
        gz_verbose_arg,
        log_level_arg,
        robot_state_publisher_node,
        rviz_node,
        robot_localization,
        spawn_entity,
        bridge,
    ]
        + gazebo
    )




        
