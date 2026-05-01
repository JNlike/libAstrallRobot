[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# 面向 Astrall 机器狗的机器人运行时

本仓库提供一个面向工厂巡检机器人的极简现代 C++20 机器人运行时，运行目标是 Astrall 机器狗硬件。Astrall 是机器狗提供商/平台名称；本项目是围绕该平台构建的运行时和集成层。核心运行时由 C++ 编写，Python 通过 `astrall` pybind11 模块调用它。

## 仓库结构

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

设计原则：

- Python 调用运行时。
- C++ 管理生命周期和硬件抽象。
- `Runtime` 为演示、仿真、Python 任务入口和最小非 ROS 使用场景拥有并共享系统对象。
- `config.yaml` 选择具体实现。
- `Planner`、`Controller`、`Navigator` 和 `StateMachine` 是 core 中的演示/仿真/最小运行时组件。生产导航使用 FAST-LIO/localization 加 Nav2。
- `Radar` 只是仿真、演示或 mock 点云抽象。用于 FAST-LIO、Nav2 和 RViz 的生产 LiDAR 数据必须来自厂商 SDK、UDP 解析器，或发布 `sensor_msgs/msg/PointCloud2` 的现有 ROS2 LiDAR 驱动。
- Astrall SDK 层用于机器人底盘控制和状态桥接：`AstrallMove`、IMU、SPORT、摇杆、RGB 相机、SDK 状态和系统状态。
- `astrall_ros2/controller_node` 打包为 `astrall_base_driver`。它直接创建并使用 `astrall_core::Backend`；它不是 Nav2 controller，也不应构造完整的 core `Runtime`。
- Python 应保持在任务层。它应调用 `NavigateToPose`/`FollowWaypoints` 等 ROS2 action，而不是直接运行底盘闭环控制。

## ROS2 部署形态

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

astrall_base_driver 基于 Backend 遥测发布 /astrall/imu、/astrall/wheel_speeds、
/astrall/status 和 /diagnostics。
```

前置 LiDAR 预期位于 `10.18.0.120`，MSOP 为 `6699`，DIFOP 为 `7788`，IMU 为 `6688`。后置 LiDAR 预期位于 `10.18.0.121`，MSOP 为 `6969`，DIFOP 为 `7878`，IMU 为 `6868`。这些 LiDAR 流不是 Astrall SDK 订阅主题。

## 实现状态

| 领域 | 状态 |
| --- | --- |
| Core runtime | 已实现仿真/演示/最小运行时：`Backend`、`Controller`、`Planner`、`Navigator`、`StateMachine`、dummy camera 和 dummy radar。 |
| Base driver | 已实现为 `astrall_base_driver`：订阅 `/cmd_vel`，通过 `Backend` 向 SDK 发送速度，发布 IMU、轮速、状态和 diagnostics。 |
| 外部必需组件 | 生产 LiDAR ROS2 驱动或 UDP 解析器、FAST-LIO/localization、Nav2、地图/costmap，以及经过验证的传感器标定。 |
| 未来工作 | 生产相机传输、更完整的 SDK fake 测试、更强的 Runtime factory，以及面向部署的诊断。 |

## 依赖

- C++20 编译器
- CMake 3.20+
- Python 3
- pybind11
- yaml-cpp
- 用于 Python 测试和演示的 numpy

Ubuntu：

```bash
sudo apt install cmake g++ pybind11-dev libyaml-cpp-dev python3-numpy
```

vcpkg 或 conda 等替代包管理器也可以提供 `pybind11` 和 `yaml-cpp`。

在 Windows 上，请使用与 `yaml-cpp` 包 ABI 一致的编译器。例如，将 MSVC 构建的 `yaml-cpp` 与 MSVC 搭配，或将 MinGW 构建的 `yaml-cpp` 与 MinGW 搭配。

## 构建

`ASTRALL_ENABLE_SDK` 在 Linux 上默认 `ON`，在其他平台默认 `OFF`。内置 SDK 库是 Linux `.so`，因此非 Linux 本地构建仍然可以在不启用 SDK wrapper 的情况下编译仿真/演示核心。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Python 使用

将 `PYTHONPATH` 设置为包含已编译 `astrall` 扩展的目录。

Linux/macOS 单配置示例：

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

Windows/非 Linux 多配置示例：

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

Python API 示例：

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

## C++ 演示

```bash
cmake --build build --target astrall_cpp_demo --config Release
./build/astrall_core/astrall_cpp_demo
```

在 Windows 多配置生成器上：

```powershell
.\build\astrall_core\Release\astrall_cpp_demo.exe
```

## 测试

构建 Python 扩展后：

```bash
PYTHONPATH=build/astrall_py pytest astrall_core/tests/smoke_test.py
```

在 Windows 上：

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
pytest astrall_core\tests\smoke_test.py
```

## 注意事项

- 不要从 Python 反复构造真实硬件设备。优先使用 `Runtime.from_config()`，让硬件句柄在一个位置共享并由该位置拥有。
- C++ 设备接口使用 `ImageFrame` 和 `PointCloud`；Python 的 `get_frame()` 和 `get_pointcloud()` 会返回复制后的 numpy 数组以便使用。未来的零拷贝路径应定义明确的所有权和生命周期规则。
- 当 `ASTRALL_ENABLE_SDK=ON` 时，`RealBackend` 由 SDK 支持。`backend: real` 会初始化 Astrall SDK，并默认请求控制权限；只能在安全的硬件环境中使用。它的 `getCurrentPose()` 使用 SDK odom/yaw 遥测，面向演示和最小运行时路径，不代表生产定位。
- `sdk_ip` 和 `robot_ip` 记录预期网络和诊断信息。当前 Astrall C API 不公开 IP setter。
- 请参阅 `docs/astrall_ros2_architecture.md` 了解 ROS2 base-driver、LiDAR-driver、localization、Nav2 和测试计划。
