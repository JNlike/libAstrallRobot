# Astrall ROS2 Architecture Notes

## Responsibility Corrections

- `Radar` in the C++ runtime is a simulation/demo/mock point-cloud interface. It must not become the production source for Nav2 or FAST-LIO point clouds.
- `AstrallSdkWrapper` wraps the Astrall SDK C API inside `astrall_core` only. ROS2 code must talk to `Backend`, not the SDK wrapper.
- `astrall_ros2/controller_node` is the ROS2 bridge for the robot base, packaged as `astrall_base_driver`. It subscribes to `/cmd_vel`, clamps and maps velocities into `Twist2D{vx, vy, w}`, sends that command through `astrall_core::Backend`, publishes `/astrall/imu`, `/astrall/wheel_speeds`, `/astrall/status`, and `/diagnostics` from Backend telemetry/status, and stops on command timeout or backend errors. It is not a Nav2 controller and should not construct the full core `Runtime`.
- `astrall_ros2/radar_node` is responsible for LiDAR UDP/vendor SDK integration and publishes `sensor_msgs/msg/PointCloud2` plus LiDAR IMU topics.
- `localization` should run FAST-LIO or an equivalent stack consuming PointCloud2 plus the chosen IMU source, then publish odom and TF.
- `navigation` should run Nav2. Nav2 consumes PointCloud2 through costmap/voxel layers and outputs `/cmd_vel`.
- `python_task_layer` should call ROS2 actions such as `NavigateToPose` and `FollowWaypoints`, and keep only high-level commands such as mission start/cancel, emergency stop, and robot status.

## Production Chain

```text
LiDAR ROS2 driver
  -> PointCloud2
  -> FAST-LIO/localization
  -> odom + TF
  -> Nav2
  -> /cmd_vel
  -> astrall_base_driver
  -> astrall_core Backend
  -> RealBackend
  -> Astrall SDK
```

`RealBackend::getCurrentPose()` reports SDK odom/yaw telemetry for demos and minimal runtime paths. Production navigation must use localization odom/TF, not this chassis telemetry snapshot.

## Implementation Status

| Area | Status |
| --- | --- |
| Core runtime | Implemented for sim/demo/minimal runtime: `Backend`, `Controller`, `Planner`, `Navigator`, `StateMachine`, dummy camera, and dummy radar. |
| Base driver | Implemented as `astrall_base_driver`: `/cmd_vel` input, Backend velocity output, IMU/wheel/status/diagnostics publishers, monitor mode, timeout stop, and no-control-authority guard. |
| External required components | Production LiDAR ROS2 driver or UDP parser, FAST-LIO/localization, Nav2, maps/costmaps, and validated sensor calibration. |
| Future work | Production camera transport, SDK fake coverage for lifecycle/failure cases, stronger Runtime factories, and deployment-specific diagnostics. |

## Current Repository Findings

- `Radar`, `PointCloud`, and `getPointCloud()` exist only under `astrall_core/include/astrall/device`, `astrall_core/src/device/dummy_radar.cpp`, examples, and Python bindings. There is no production LiDAR driver here.
- `RealBackend` is SDK-backed when `ASTRALL_ENABLE_SDK=ON`. `backend: real` initializes the Astrall SDK, requests control by default, serializes SDK C API calls, and shuts down by stopping, closing subscriptions, releasing control, and deinitializing.
- The base driver publishes telemetry and diagnostics, but it does not publish localization TF. Static sensor TF lives in `astrall_description`; odom/TF comes from FAST-LIO or another estimator.
- `Planner`, `Controller`, `Navigator`, and `StateMachine` follow the intended split: planning, motion control, route execution, and mission behavior remain separate. For production ROS2 navigation, Nav2 should replace this local navigation loop.
- The SDK header currently committed in `astrall_core/lib/astrall_sdk/include/interface.h` should be checked against the target robot SDK before deployment. The provided hardware notes list RGB as `0x0031` and joystick as `0x0100`; the committed header uses different RGB/joystick values. Neither source contains any LiDAR/Radar/PointCloud topic.

## Minimal Run Path

1. Configure Linux host networking on `10.18.0.x`, normally using `10.18.0.200` for the SDK computer.
2. Build astrall_core and ROS2 packages on Linux:

   ```bash
   colcon build --packages-select astrall_base_driver astrall_description
   ```

3. Verify LiDAR reachability:

   ```bash
   astrall_ros2/scripts/check_lidar_udp.sh <iface>
   ```

4. Launch static sensor transforms:

   ```bash
   ros2 launch astrall_description static_transforms.launch.py
   ```

5. Launch the base driver:

   ```bash
   ros2 launch astrall_base_driver astrall_base.launch.py
   ```

6. Launch the external LiDAR driver, FAST-LIO, and Nav2.
7. Send a `NavigateToPose` action. Nav2 should publish `/cmd_vel`; `astrall_ros2/controller_node` should call `AstrallMove`.

## Test Plan

- Unit: Twist-to-SDK mapping, velocity clamp, and `sign_vx/sign_vy/sign_w`; `astrall_core/tests/test_twist_command_mapper.cpp` covers the pure mapping function.
- Unit with fake SDK: SDK wrapper shutdown order, serialized SDK C API calls, cached callbacks, no-control-authority inhibits motion commands, SDK odom/yaw updates `getCurrentPose()`, and system status caching avoids repeated synchronous queries.
- ROS2 integration: publish `/cmd_vel` and confirm the base node calls `move`; stop publishing and confirm timeout stop; set `control_mode=monitor` and confirm `/cmd_vel` is ignored; simulate no-control-authority and confirm diagnostics; feed fake IMU/SPORT callbacks and confirm `/astrall/imu` plus `/astrall/wheel_speeds`.
- LiDAR link: ping `10.18.0.120` and `10.18.0.121`; capture UDP on `6699`, `6688`, `6969`, and `6868`; confirm the chosen ROS2 LiDAR driver publishes PointCloud2.
- Nav2 smoke: LiDAR driver, FAST-LIO, Nav2, `NavigateToPose`, `/cmd_vel`, AstrallMove, emergency stop.
