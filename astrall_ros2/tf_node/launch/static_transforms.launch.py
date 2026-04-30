import os

import yaml
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def _unit_scale(unit):
    if unit == "m":
        return 1.0
    if unit == "mm":
        return 0.001
    raise RuntimeError(f"Unsupported extrinsic_unit '{unit}'. Use 'mm' or 'm'.")


def generate_launch_description():
    share = get_package_share_directory("astrall_description")
    path = os.path.join(share, "config", "sensor_extrinsics.yaml")

    with open(path, "r", encoding="utf-8") as f:
        config = yaml.safe_load(f)

    base_frame = config["base_frame"]
    scale = _unit_scale(config["extrinsic_unit"])
    nodes = []

    for child_frame, sensor in config["sensors"].items():
        x, y, z = [str(v * scale) for v in sensor["xyz"]]
        roll, pitch, yaw = [str(v) for v in sensor.get("rpy", [0.0, 0.0, 0.0])]
        nodes.append(
            Node(
                package="tf2_ros",
                executable="static_transform_publisher",
                name=f"{child_frame}_static_tf",
                arguments=[
                    "--x", x,
                    "--y", y,
                    "--z", z,
                    "--roll", roll,
                    "--pitch", pitch,
                    "--yaw", yaw,
                    "--frame-id", base_frame,
                    "--child-frame-id", child_frame,
                ],
            )
        )

    return LaunchDescription(nodes)
