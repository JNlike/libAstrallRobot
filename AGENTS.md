# Astrall Project Guide

## Architecture Principles

- Python calls into the `astrall` extension module.
- C++ owns lifecycle, runtime object graphs, threading boundaries, and hardware abstractions.
- `Runtime::fromConfig()` is the preferred construction path for robot systems.
- `Backend`, `Controller`, `Planner`, `Navigator`, `StateMachine`, `Camera`, and `Radar` must stay separated by responsibility.
- Real hardware handles should be opened once through `Runtime`, not repeatedly from Python.

## Repository Layout

- `astrall_core/`: modern C++ core library, configs, examples, SDK wrapper, and core tests.
- `astrall_py/`: pybind11 Python module exposing core runtime APIs.
- `astrall_ros2/`: ROS2 adaptation layer, including SDK bridge, static TF, external LiDAR boundary docs, and network diagnostics.

## Naming

- C++ code lives in `namespace astrall`.
- C++ public methods use the existing C++ style, for example `getCurrentPose()` and `startMission()`.
- Python bindings expose snake_case names, for example `get_current_pose()` and `start_mission()`.
- Use strong domain types such as `Twist2D`, `Pose2D`, and `Point2D`; avoid ambiguous tuple-style command APIs.

## Documentation Localization

- Whenever the root `README.md` is changed, update the matching localized documentation in the same change:
  `docs/README.zh-CN.md`, `docs/README.ja.md`, `docs/README.fr.md`, and `docs/README.de.md`.
- Keep the localized README files semantically aligned with the English source, including build commands, hardware boundaries, configuration defaults, and examples.
- Do not leave localized README files stale when adding, removing, or substantially rewording root README content.

## Build Commands

The target deployment platform is Linux. `ASTRALL_ENABLE_SDK` should default to `ON` when `CMAKE_SYSTEM_NAME` is `Linux`, and default to `OFF` otherwise because the bundled Astrall SDK library is a Linux `.so`.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the Python demo from the repository root after adding the build output directory to `PYTHONPATH`.

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

On non-Linux development machines, keep the default `ASTRALL_ENABLE_SDK=OFF` unless a compatible SDK library is available.

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

## Boundaries Not To Break

- `Planner` only plans paths; it must not send velocity commands.
- `Controller` only controls motion; it must not own route sequencing.
- `Navigator` connects planning and control for a goal.
- `StateMachine` runs mission behavior through `Navigator`; it must not command `Backend` directly.
- Binding code belongs in `astrall_py/src`; core code belongs in `astrall_core/include/astrall` and `astrall_core/src`.
- Numpy arrays returned from device bindings must own/copy their data unless a later zero-copy lifetime design is explicit.
- `Radar` is only for simulation, demo, mock, or non-ROS minimal runtime point clouds. Do not make it the production PointCloud2 source for Nav2 or FAST-LIO.
- Astrall SDK integration is for chassis control and robot-state bridging, including `AstrallMove`, IMU, SPORT, joystick, RGB camera, SDK status, and system status. It must not read LiDAR point clouds.
- Production LiDAR must be handled by a vendor SDK, UDP parser, or ROS2 LiDAR driver using the documented front/rear LiDAR IPs and ports.
- Python belongs at the high-level task layer. Do not put local chassis closed-loop control in Python; use ROS2 actions such as `NavigateToPose` or `FollowWaypoints`.
