import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import ExecuteProcess
from launch_ros.actions import Node
import xacro
from launch_ros.actions import Node
from launch.actions import TimerAction
from launch.actions import ExecuteProcess, TimerAction

def generate_launch_description():
    # Fetch package share directory
    pkg = get_package_share_directory('navigation_sim')

    # Process xacro → URDF string
    xacro_file = os.path.join(pkg, 'urdf', 'robot.urdf.xacro')
    robot_description = xacro.process_file(xacro_file).toxml()

    # Paths to configuration files
    world_file = os.path.join(pkg, 'worlds', 'straight_corridor.sdf')
    bridge_config = os.path.join(pkg, 'config', 'bridge.yaml')
    gui_config_file = os.path.join(pkg, 'config', 'gazebo_gui.config')
    
    return LaunchDescription([

        # Launch Gazebo Harmonic 
        ExecuteProcess(
            cmd=[
                'gz', 'sim', 
                '-r', world_file,
                '--gui-config', gui_config_file
            ],
            output='screen'
        ),

        # Spawn the robot into Gazebo
        Node(
            package='ros_gz_sim',
            executable='create',
            arguments=[
                '-name', 'navigation_sim',
                '-string', robot_description,
                '-x', '0.0', 
                '-y', '0.0', 
                '-z', '0.1',
                '-Y', '3.14'
            ],
            output='screen'
        ),

        # Robot State Publisher (Broadcasts the static & dynamic TF tree)
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': robot_description,
                'use_sim_time': True  # Critical for synchronization in simulation
            }],
            output='screen'
        ),

        # ROS, Gazebo bidirectional bridge
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            parameters=[{'config_file': bridge_config}],
            output='screen'
        ),

        # Auto-start navigation after 7 seconds
        TimerAction(
            period=7.0,
            actions=[
                Node(
                    package='navigation_sim',
                    executable='navigation',
                    output='screen'
                )
            ]
        )
    ])