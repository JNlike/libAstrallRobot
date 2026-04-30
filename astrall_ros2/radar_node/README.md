# External LiDAR Driver Boundary

Astrall SDK subscriptions expose IMU, SPORT, RGB camera, and joystick data only. They do not expose LiDAR, Radar, PointCloud, or PointCloud2 topics.

Production point clouds for FAST-LIO, Nav2 costmaps, and RViz must come from a vendor SDK, an existing ROS2 driver, or a UDP parser for the LiDAR protocol.

## Network Parameters

| Sensor | IP | MSOP | DIFOP | LiDAR IMU |
| --- | --- | ---: | ---: | ---: |
| Front LiDAR | `10.18.0.120` | `6699` | `7788` | `6688` |
| Rear LiDAR | `10.18.0.121` | `6969` | `7878` | `6868` |

Expected ROS2 outputs:

- `/front/points_raw`: `sensor_msgs/msg/PointCloud2`
- `/rear/points_raw`: `sensor_msgs/msg/PointCloud2`
- `/front/imu`: `sensor_msgs/msg/Imu`
- `/rear/imu`: `sensor_msgs/msg/Imu`

Use `astrall_ros2/scripts/check_lidar_udp.sh <iface>` to verify basic network reachability. UDP packets only prove link activity; they do not prove that point-cloud decoding is correct.
