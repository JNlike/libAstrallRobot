# Astrall Robot Runtime

Astrall is a minimal modern C++20 robot runtime for factory inspection robots. The core runtime is written in C++, while Python calls it through the `astrall` pybind11 module.

## Repository Layout

```text
astrall_core/
  include/astrall/
  src/
  configs/
  examples/
  tests/
  lib/astrall_sdk/

astrall_py/
  src/

astrall_ros2/
  radar_node/
  camera_node/
  controller_node/
  odom_node/
  tf_node/
  scripts/
```

Design principles:

- Python calls the runtime.
- C++ manages lifecycle and hardware abstractions.
- `Runtime` owns and shares the system objects.
- `config.yaml` selects concrete implementations.
- `Radar` is a simulation/demo/mock point-cloud abstraction only. Production LiDAR data for FAST-LIO, Nav2, and RViz must come from a vendor SDK, UDP parser, or existing ROS2 LiDAR driver publishing `sensor_msgs/msg/PointCloud2`.
- The Astrall SDK layer is for robot-base control and status bridging: `AstrallMove`, IMU, SPORT, joystick, RGB camera, SDK status, and system status.
- Python should stay at the task layer. It should call ROS2 actions such as `NavigateToPose`/`FollowWaypoints`, not run the chassis closed loop directly.

## ROS2 Deployment Shape

```text
Nav2 /cmd_vel
  |
  v
astrall_ros2/controller_node --> astrall_core Backend --> RealBackend --> Astrall SDK
  |
  +--> /astrall/imu
  +--> /astrall/wheel_speeds
  +--> /astrall/status
  +--> /diagnostics

astrall_ros2/radar_node -> /front/points_raw, /rear/points_raw
FAST-LIO/localization --> odom + TF
Nav2 costmaps ---------> PointCloud2 inputs
```

The front LiDAR is expected at `10.18.0.120` with MSOP `6699`, DIFOP `7788`, and IMU `6688`. The rear LiDAR is expected at `10.18.0.121` with MSOP `6969`, DIFOP `7878`, and IMU `6868`. These LiDAR streams are not Astrall SDK subscription topics.

## Dependencies

- C++20 compiler
- CMake 3.20+
- Python 3
- pybind11
- yaml-cpp
- numpy for Python tests and demos

Ubuntu:

```bash
sudo apt install cmake g++ pybind11-dev libyaml-cpp-dev python3-numpy
```

Alternative package managers such as vcpkg or conda can also provide `pybind11` and `yaml-cpp`.

On Windows, use a compiler and `yaml-cpp` package built with the same ABI. For example, pair MSVC-built `yaml-cpp` with MSVC, or MinGW-built `yaml-cpp` with MinGW.

## Build

`ASTRALL_ENABLE_SDK` defaults to `ON` on Linux and `OFF` on other platforms. The bundled SDK library is a Linux `.so`, so non-Linux local builds can still compile the simulation/demo core without the SDK wrapper.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Python Usage

Set `PYTHONPATH` to the directory containing the compiled `astrall` extension.

Linux/macOS single-config example:

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

Windows/non-Linux multi-config example:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

Python API example:

```python
import astrall as al

rt = al.from_config("astrall_core/configs/robot.yaml")

img = rt.camera().get_frame()
cloud = rt.radar().get_pointcloud()  # demo/mock cloud, not production LiDAR

sm = rt.state_machine()
sm.start_mission([
    al.Point2D(1.0, 0.0),
    al.Point2D(2.0, 1.0),
])

while sm.running():
    sm.update()
```

## C++ Demo

```bash
cmake --build build --target astrall_cpp_demo --config Release
./build/astrall_core/astrall_cpp_demo
```

On Windows multi-config generators:

```powershell
.\build\astrall_core\Release\astrall_cpp_demo.exe
```

## Tests

After building the Python extension:

```bash
PYTHONPATH=build/astrall_py pytest astrall_core/tests/smoke_test.py
```

On Windows:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
pytest astrall_core\tests\smoke_test.py
```

## Notes

- Do not repeatedly construct real hardware devices from Python. Prefer `Runtime.from_config()` so hardware handles are shared and owned in one place.
- C++ device interfaces use `ImageFrame` and `PointCloud`; Python `get_frame()` and `get_pointcloud()` return copied numpy arrays for convenience. A future zero-copy path should define explicit ownership and lifetime rules.
- `RealBackend` is SDK-backed when `ASTRALL_ENABLE_SDK=ON`. `backend: real` initializes the Astrall SDK and requests control by default; use only in a safe hardware environment.
- `sdk_ip` and `robot_ip` document the expected network and diagnostics. The current Astrall C API does not expose IP setters.
- See `docs/astrall_ros2_architecture.md` for the ROS2 base-driver, LiDAR-driver, localization, Nav2, and test plan.
