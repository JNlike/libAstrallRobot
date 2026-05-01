# Astrall ROS2 Layer

This directory is the ROS2 adaptation layer. It should not absorb the core C++ runtime or the pybind11 module.

## Conceptual Nodes

```text
RadarNode
  astrall_ros2/radar_node.
  External LiDAR driver, vendor SDK, or UDP parser.
  Publishes /front/points_raw, /rear/points_raw, /front/imu, /rear/imu.
  Does not use AstrallSubscriptionData for point clouds.

CameraNode
  astrall_ros2/camera_node.
  ROS2 camera adapter when production camera transport is added.

ControllerNode
  astrall_ros2/controller_node, packaged as astrall_base_driver for now.
  Subscribes /cmd_vel, clamps/maps Twist, sends commands through astrall_core Backend,
  and publishes Backend IMU, wheel speeds, robot status, and diagnostics.

OdomNode
  astrall_ros2/odom_node.
  Localization output owner, typically FAST-LIO or another estimator.
  Consumes PointCloud2 + IMU and publishes odom.

TfNode
  astrall_ros2/tf_node, packaged as astrall_description for now.
  Static sensor transforms plus localization TF.
```

## LiDAR Network

```text
Front LiDAR 10.18.0.120
  MSOP 6699   -> point cloud UDP
  DIFOP 7788  -> device/config UDP
  IMU  6688   -> LiDAR internal IMU

Rear LiDAR 10.18.0.121
  MSOP 6969   -> point cloud UDP
  DIFOP 7878  -> device/config UDP
  IMU  6868   -> LiDAR internal IMU
```

Expected ROS2 topics:

- `/front/points_raw`: `sensor_msgs/msg/PointCloud2`
- `/rear/points_raw`: `sensor_msgs/msg/PointCloud2`
- `/front/imu`: `sensor_msgs/msg/Imu`
- `/rear/imu`: `sensor_msgs/msg/Imu`

FAST-LIO consumes PointCloud2 plus IMU and outputs odom/TF. Nav2 consumes PointCloud2 through obstacle or voxel layers and outputs `/cmd_vel`. The Astrall SDK bridge receives `/cmd_vel`, maps it to `Twist2D{vx, vy, w}`, and forwards it to the SDK adapter.
