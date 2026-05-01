[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Robot Runtime für Astrall-Roboterhunde

Dieses Repository stellt eine minimale, moderne C++20-Robotik-Laufzeitumgebung für Fabrikinspektionsroboter auf Basis von Astrall-Roboterhund-Hardware bereit. Astrall ist der Name des Roboteranbieters/der Plattform; dieses Projekt ist die Laufzeit- und Integrationsschicht um diese Plattform. Die zentrale Laufzeit ist in C++ geschrieben, während Python sie über das pybind11-Modul `astrall` aufruft.

## Repository-Struktur

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

Designprinzipien:

- Python ruft die Laufzeit auf.
- C++ verwaltet Lebenszyklus und Hardwareabstraktionen.
- `Runtime` besitzt und teilt die Systemobjekte.
- `config.yaml` wählt konkrete Implementierungen aus.
- `Radar` ist nur eine Punktwolkenabstraktion für Simulation, Demo oder Mock. Produktions-LiDAR-Daten für FAST-LIO, Nav2 und RViz müssen aus einem Hersteller-SDK, einem UDP-Parser oder einem bestehenden ROS2-LiDAR-Treiber stammen, der `sensor_msgs/msg/PointCloud2` veröffentlicht.
- Die Astrall-SDK-Schicht dient der Steuerung der Roboterbasis und der Statusüberbrückung: `AstrallMove`, IMU, SPORT, Joystick, RGB-Kamera, SDK-Status und Systemstatus.
- Python sollte auf der Aufgabenebene bleiben. Es sollte ROS2-Actions wie `NavigateToPose`/`FollowWaypoints` aufrufen und nicht direkt die geschlossene Regelung des Chassis ausführen.

## ROS2-Deployment-Form

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

Das vordere LiDAR wird unter `10.18.0.120` mit MSOP `6699`, DIFOP `7788` und IMU `6688` erwartet. Das hintere LiDAR wird unter `10.18.0.121` mit MSOP `6969`, DIFOP `7878` und IMU `6868` erwartet. Diese LiDAR-Streams sind keine Astrall-SDK-Abonnementtopics.

## Abhängigkeiten

- C++20-Compiler
- CMake 3.20+
- Python 3
- pybind11
- yaml-cpp
- numpy für Python-Tests und Demos

Ubuntu:

```bash
sudo apt install cmake g++ pybind11-dev libyaml-cpp-dev python3-numpy
```

Alternative Paketmanager wie vcpkg oder conda können ebenfalls `pybind11` und `yaml-cpp` bereitstellen.

Unter Windows verwenden Sie einen Compiler und ein `yaml-cpp`-Paket, die mit derselben ABI gebaut wurden. Kombinieren Sie zum Beispiel ein mit MSVC gebautes `yaml-cpp` mit MSVC oder ein mit MinGW gebautes `yaml-cpp` mit MinGW.

## Build

`ASTRALL_ENABLE_SDK` ist unter Linux standardmäßig `ON` und auf anderen Plattformen standardmäßig `OFF`. Die mitgelieferte SDK-Bibliothek ist eine Linux-`.so`, daher können lokale Nicht-Linux-Builds den Simulations-/Demo-Core weiterhin ohne SDK-Wrapper kompilieren.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Python-Nutzung

Setzen Sie `PYTHONPATH` auf das Verzeichnis, das die kompilierte `astrall`-Erweiterung enthält.

Linux/macOS-Beispiel mit Single-Config:

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

Windows/Nicht-Linux-Beispiel mit Multi-Config:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

Python-API-Beispiel:

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

## C++-Demo

```bash
cmake --build build --target astrall_cpp_demo --config Release
./build/astrall_core/astrall_cpp_demo
```

Bei Windows-Multi-Config-Generatoren:

```powershell
.\build\astrall_core\Release\astrall_cpp_demo.exe
```

## Tests

Nach dem Build der Python-Erweiterung:

```bash
PYTHONPATH=build/astrall_py pytest astrall_core/tests/smoke_test.py
```

Unter Windows:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
pytest astrall_core\tests\smoke_test.py
```

## Hinweise

- Konstruieren Sie reale Hardwaregeräte nicht wiederholt aus Python. Bevorzugen Sie `Runtime.from_config()`, damit Hardware-Handles an einer Stelle geteilt und besessen werden.
- C++-Geräteschnittstellen verwenden `ImageFrame` und `PointCloud`; Python `get_frame()` und `get_pointcloud()` geben der Einfachheit halber kopierte numpy-Arrays zurück. Ein zukünftiger Zero-Copy-Pfad sollte explizite Regeln für Besitz und Lebensdauer definieren.
- `RealBackend` ist SDK-gestützt, wenn `ASTRALL_ENABLE_SDK=ON` ist. `backend: real` initialisiert das Astrall SDK und fordert standardmäßig die Kontrolle an; verwenden Sie dies nur in einer sicheren Hardwareumgebung.
- `sdk_ip` und `robot_ip` dokumentieren das erwartete Netzwerk und die Diagnose. Die aktuelle Astrall-C-API stellt keine IP-Setter bereit.
- Siehe `docs/astrall_ros2_architecture.md` für ROS2 base-driver, LiDAR-driver, localization, Nav2 und den Testplan.
