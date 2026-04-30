from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare
from launch.substitutions import PathJoinSubstitution


def generate_launch_description():
    config = PathJoinSubstitution([
        FindPackageShare("astrall_base_driver"),
        "config",
        "astrall_base.yaml",
    ])

    return LaunchDescription([
        Node(
            package="astrall_base_driver",
            executable="astrall_base_node",
            name="astrall_base_node",
            output="screen",
            parameters=[config],
        ),
    ])
