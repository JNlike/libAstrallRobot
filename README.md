# Astrall Robot Runtime

Astrall is a minimal modern C++20 robot runtime for factory inspection robots. The core runtime is written in C++, while Python calls it through the `astrall` pybind11 module.

## Architecture

```text
Python
  |
  v
astrall pybind11 module
  |
  v
Runtime::fromConfig(configs/robot.yaml)
  |
  +--> Backend <----------+
  |                       |
  +--> Controller --------+
  |                       |
  +--> Planner --> Navigator --> StateMachine
  |
  +--> Camera
  |
  +--> Radar
```

Design principles:

- Python calls the runtime.
- C++ manages lifecycle and hardware abstractions.
- `Runtime` owns and shares the system objects.
- `config.yaml` selects concrete implementations.

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

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Python Usage

Set `PYTHONPATH` to the directory containing the compiled `astrall` extension.

Linux/macOS single-config example:

```bash
PYTHONPATH=build python examples/python_demo.py
```

Windows multi-config example:

```powershell
$env:PYTHONPATH="build\Release"
python examples\python_demo.py
```

Python API example:

```python
import astrall as al

rt = al.from_config("configs/robot.yaml")

img = rt.camera().get_frame()
cloud = rt.radar().get_pointcloud()

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
./build/astrall_cpp_demo
```

On Windows multi-config generators:

```powershell
.\build\Release\astrall_cpp_demo.exe
```

## Tests

After building the Python extension:

```bash
PYTHONPATH=build pytest tests/smoke_test.py
```

On Windows:

```powershell
$env:PYTHONPATH="build\Release"
pytest tests\smoke_test.py
```

## Notes

- Do not repeatedly construct real hardware devices from Python. Prefer `Runtime.from_config()` so hardware handles are shared and owned in one place.
- Device data is copied into numpy arrays in this first version. A future zero-copy path should define explicit ownership and lifetime rules.
- `RealBackend` is a placeholder that prints commands; it does not connect to physical hardware yet.
