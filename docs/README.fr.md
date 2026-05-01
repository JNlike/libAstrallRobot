[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Runtime robot pour les robots chiens Astrall

Ce dépôt fournit un runtime robotique C++20 moderne et minimal pour les robots d'inspection en usine basés sur du matériel de robot chien Astrall. Astrall est le nom du fournisseur/de la plateforme robotique ; ce projet est le runtime et la couche d'intégration autour de cette plateforme. Le runtime principal est écrit en C++, tandis que Python l'appelle via le module pybind11 `astrall`.

## Structure du dépôt

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

Principes de conception :

- Python appelle le runtime.
- C++ gère le cycle de vie et les abstractions matérielles.
- `Runtime` possède et partage les objets du système.
- `config.yaml` sélectionne les implémentations concrètes.
- `Radar` est uniquement une abstraction de nuage de points pour la simulation, la démonstration ou les mocks. Les données LiDAR de production pour FAST-LIO, Nav2 et RViz doivent provenir d'un SDK fournisseur, d'un parseur UDP ou d'un pilote LiDAR ROS2 existant publiant `sensor_msgs/msg/PointCloud2`.
- La couche Astrall SDK sert au contrôle de la base robot et au pont d'état : `AstrallMove`, IMU, SPORT, joystick, caméra RGB, état SDK et état système.
- Python doit rester au niveau des tâches. Il doit appeler des actions ROS2 telles que `NavigateToPose`/`FollowWaypoints`, et non exécuter directement la boucle fermée du châssis.

## Forme de déploiement ROS2

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

Le LiDAR avant est attendu à l'adresse `10.18.0.120` avec MSOP `6699`, DIFOP `7788` et IMU `6688`. Le LiDAR arrière est attendu à l'adresse `10.18.0.121` avec MSOP `6969`, DIFOP `7878` et IMU `6868`. Ces flux LiDAR ne sont pas des topics d'abonnement Astrall SDK.

## Dépendances

- Compilateur C++20
- CMake 3.20+
- Python 3
- pybind11
- yaml-cpp
- numpy pour les tests et démonstrations Python

Ubuntu :

```bash
sudo apt install cmake g++ pybind11-dev libyaml-cpp-dev python3-numpy
```

Des gestionnaires de paquets alternatifs comme vcpkg ou conda peuvent également fournir `pybind11` et `yaml-cpp`.

Sous Windows, utilisez un compilateur et un paquet `yaml-cpp` construits avec le même ABI. Par exemple, associez un `yaml-cpp` construit avec MSVC à MSVC, ou un `yaml-cpp` construit avec MinGW à MinGW.

## Compilation

`ASTRALL_ENABLE_SDK` vaut `ON` par défaut sous Linux et `OFF` sur les autres plateformes. La bibliothèque SDK incluse est un `.so` Linux ; les builds locaux non Linux peuvent donc toujours compiler le coeur de simulation/démonstration sans le wrapper SDK.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Utilisation Python

Définissez `PYTHONPATH` vers le répertoire contenant l'extension `astrall` compilée.

Exemple Linux/macOS en configuration unique :

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

Exemple Windows/non Linux en configuration multiple :

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

Exemple d'API Python :

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

## Démonstration C++

```bash
cmake --build build --target astrall_cpp_demo --config Release
./build/astrall_core/astrall_cpp_demo
```

Avec les générateurs Windows multi-configurations :

```powershell
.\build\astrall_core\Release\astrall_cpp_demo.exe
```

## Tests

Après avoir compilé l'extension Python :

```bash
PYTHONPATH=build/astrall_py pytest astrall_core/tests/smoke_test.py
```

Sous Windows :

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
pytest astrall_core\tests\smoke_test.py
```

## Notes

- Ne construisez pas à répétition de vrais périphériques matériels depuis Python. Préférez `Runtime.from_config()` afin que les handles matériels soient partagés et possédés à un seul endroit.
- Les interfaces C++ des périphériques utilisent `ImageFrame` et `PointCloud` ; en Python, `get_frame()` et `get_pointcloud()` retournent des tableaux numpy copiés par commodité. Un futur chemin zéro copie devra définir des règles explicites de propriété et de durée de vie.
- `RealBackend` est adossé au SDK lorsque `ASTRALL_ENABLE_SDK=ON`. `backend: real` initialise l'Astrall SDK et demande le contrôle par défaut ; utilisez-le uniquement dans un environnement matériel sûr.
- `sdk_ip` et `robot_ip` documentent le réseau attendu et les diagnostics. L'API C Astrall actuelle n'expose pas de setters IP.
- Consultez `docs/astrall_ros2_architecture.md` pour le base-driver ROS2, le LiDAR-driver, la localization, Nav2 et le plan de test.
