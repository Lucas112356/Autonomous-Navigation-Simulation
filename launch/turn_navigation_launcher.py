import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import xacro


def generate_launch_description():
    pkg = get_package_share_directory('navigation_sim')
    nav2_bringup = get_package_share_directory('nav2_bringup')

    # Process xacro → URDF string
    xacro_file = os.path.join(pkg, 'urdf', 'robot.urdf.xacro')
    robot_description = xacro.process_file(xacro_file).toxml()

    world_file = os.path.join(pkg, 'worlds', 'turn_navigation.sdf')
    bridge_config = os.path.join(pkg, 'config', 'bridge.yaml')
    nav2_params = os.path.join(pkg, 'config', 'nav2_params.yaml')
    gui_config_file = os.path.join(pkg, 'config', 'gazebo_gui_turn_navigation.config')

    return LaunchDescription([

        # Launch Gazebo Harmonic
        ExecuteProcess(
            cmd=['gz', 'sim',
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
                '-x', '0', '-y', '0', '-z', '0.1',
                '-Y', '3.14'
            ],
            output='screen'
        ),

        # Robot State Publisher (TF tree)
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[{
                'robot_description': robot_description,
                'use_sim_time': True
            }],
            output='screen'
        ),

        # ROS to Gazebo bridge
        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            parameters=[{'config_file': bridge_config}],
            output='screen'
        ),

        # Nav2 bringup 
        TimerAction(
            period=7.0, # delayed to let Gazebo fully load
            actions=[
                IncludeLaunchDescription(
                    PythonLaunchDescriptionSource(
                        os.path.join(nav2_bringup, 'launch', 'navigation_launch.py')
                    ),
                    launch_arguments={
                        'use_sim_time': 'True',
                        'params_file': nav2_params,
                        'autostart': 'True',
                    }.items()
                )
            ]
        ),

        # RViz2 for visualization 
        TimerAction(
            period=10.0, # delayed slightly after Nav2
            actions=[
                Node(
                    package='rviz2',
                    executable='rviz2',
                    name='rviz2',
                    output='screen',
                    arguments=['-d', os.path.join(pkg, 'config', 'turn_nav.rviz')],
                    parameters=[{'use_sim_time': True}]
                )
            ]
        ),

        # Turn navigation
        TimerAction(
            period=21.0, # delayed for full boot up
            actions=[
                Node(
                    package='navigation_sim',
                    executable='turn_navigation',
                    name='turn_navigation',
                    output='screen',
                    parameters=[{'use_sim_time': True}]
                )   
            ]
        ),
    ])