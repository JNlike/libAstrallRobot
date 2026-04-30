# Astrall Project Guide

## Architecture Principles

- Python calls into the `astrall` extension module.
- C++ owns lifecycle, runtime object graphs, threading boundaries, and hardware abstractions.
- `Runtime::fromConfig()` is the preferred construction path for robot systems.
- `Backend`, `Controller`, `Planner`, `Navigator`, `StateMachine`, `Camera`, and `Radar` must stay separated by responsibility.
- Real hardware handles should be opened once through `Runtime`, not repeatedly from Python.

## Naming

- C++ code lives in `namespace astrall`.
- C++ public methods use the existing C++ style, for example `getCurrentPose()` and `startMission()`.
- Python bindings expose snake_case names, for example `get_current_pose()` and `start_mission()`.
- Use strong domain types such as `Twist2D`, `Pose2D`, and `Point2D`; avoid ambiguous tuple-style command APIs.

## Build Commands

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the Python demo from the repository root after adding the build output directory to `PYTHONPATH`.

```bash
PYTHONPATH=build python examples/python_demo.py
```

On Windows with multi-config generators, the module may be under `build/Release`.

```powershell
$env:PYTHONPATH="build\Release"
python examples\python_demo.py
```

## Boundaries Not To Break

- `Planner` only plans paths; it must not send velocity commands.
- `Controller` only controls motion; it must not own route sequencing.
- `Navigator` connects planning and control for a goal.
- `StateMachine` runs mission behavior through `Navigator`; it must not command `Backend` directly.
- Binding code belongs in `bindings/python`; core code belongs in `include/astrall` and `src`.
- Numpy arrays returned from device bindings must own/copy their data unless a later zero-copy lifetime design is explicit.
