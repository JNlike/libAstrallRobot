[English](README.md) | [中文](docs/README.zh-CN.md) | [日本語](docs/README.ja.md) | [Français](docs/README.fr.md) | [Deutsch](docs/README.de.md)

# Astrall RobotDog Cpp

A C++17 demo project for controlling an Astrall robot dog with the Astrall SDK.

This project shows how to initialize the SDK, subscribe to robot telemetry, read device and power status, send heartbeat messages, and control robot movement from the keyboard.

## Features

- CMake-based C++ project structure.
- Astrall SDK integration through `lib/astrall_sdk`.
- Keyboard command handling for sport modes, movement, authorization, and lights.
- IMU and sport data subscriptions.
- Periodic heartbeat, device information, system status, sport status, and battery status output.
- Runtime SDK library copying after build.

## Project Structure

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

## Requirements

- Linux environment
- CMake 3.10 or newer
- C++17-compatible compiler
- POSIX thread support
- Astrall robot dog and compatible Astrall SDK runtime

## Build

```bash
cmake -S . -B build
cmake --build build
```

The executable is generated as `sdk_demo`. The required Astrall SDK runtime libraries are copied to the executable directory after the build.

## Run

```bash
./build/sdk_demo
```

Run the program in a terminal connected to the robot environment. The demo reads keyboard input directly from standard input.

## Keyboard Controls

| Key | Action |
| --- | --- |
| `1` | Switch to damping mode |
| `2` | Switch to fixed stand mode |
| `3` | Switch to fixed down mode |
| `4` | Switch to move mode |
| `5` | Start auto charge |
| `6` | Exit charge |
| `9` | Request SDK control authority |
| `0` | Stop movement |
| `w` / `s` | Increase / decrease forward velocity |
| `a` / `d` | Increase / decrease lateral velocity |
| `q` / `e` | Increase / decrease yaw velocity |
| `n` | Turn light on |
| `o` | Turn light off |

Movement keys only take effect when the robot is in move mode.

## Notes

Use this demo carefully in a safe, open area. Confirm that the robot has SDK control authority and enough free space before enabling movement.

## License

See [LICENSE](LICENSE).
