[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Astrall RobotDog Cpp

这是一个基于 Astrall SDK 的 C++17 机器狗控制示例项目。

本项目演示如何初始化 SDK、订阅机器狗遥测数据、读取设备与电源状态、发送心跳消息，并通过键盘控制机器狗的运动、模式和灯光。

## 功能

- 基于 CMake 的 C++ 项目结构。
- 通过 `lib/astrall_sdk` 集成 Astrall SDK。
- 支持键盘控制运动模式、移动、授权和灯光。
- 订阅 IMU 数据和运动数据。
- 周期性输出心跳、设备信息、系统状态、运动状态和电池状态。
- 构建后自动复制 SDK 运行时库到可执行文件目录。

## 项目结构

```text
.
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── keyboard.cpp
│   └── keyboard.h
├── lib/
│   └── astrall_sdk/
│       ├── include/
│       ├── x86-64/
│       └── arm64/
└── docs/
```

## 环境要求

- Linux 环境
- CMake 3.10 或更高版本
- 支持 C++17 的编译器
- POSIX 线程支持
- Astrall 机器狗和兼容的 Astrall SDK 运行时

## 构建

```bash
cmake -S . -B build
cmake --build build
```

生成的可执行文件名为 `sdk_demo`。构建完成后，所需的 Astrall SDK 运行时库会被复制到可执行文件目录。

## 运行

```bash
./build/sdk_demo
```

请在已连接机器狗运行环境的终端中运行程序。该示例会直接从标准输入读取键盘按键。

## 键盘控制

| 按键 | 功能 |
| --- | --- |
| `1` | 切换到阻尼模式 |
| `2` | 切换到固定站立模式 |
| `3` | 切换到固定趴下模式 |
| `4` | 切换到移动模式 |
| `5` | 开始自动充电 |
| `6` | 退出充电 |
| `9` | 请求 SDK 控制权限 |
| `0` | 停止移动 |
| `w` / `s` | 增加 / 减少前进速度 |
| `a` / `d` | 增加 / 减少横向速度 |
| `q` / `e` | 增加 / 减少偏航速度 |
| `n` | 打开灯光 |
| `o` | 关闭灯光 |

移动按键仅在机器狗处于移动模式时生效。

## 注意事项

请在安全、开阔的区域谨慎使用该示例。启用移动前，请确认机器狗已获得 SDK 控制权限，并且周围有足够空间。

## 许可证

请参阅 [LICENSE](../LICENSE)。
