[English](../README.md) | [中文](README.zh-CN.md) | [日本語](README.ja.md) | [Français](README.fr.md) | [Deutsch](README.de.md)

# Astrall ロボットランタイム

Astrall は、工場点検ロボット向けの最小構成でモダンな C++20 ロボットランタイムです。コアランタイムは C++ で実装され、Python は `astrall` pybind11 モジュールを通じて呼び出します。

## リポジトリ構成

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

設計原則:

- Python はランタイムを呼び出します。
- C++ がライフサイクルとハードウェア抽象を管理します。
- `Runtime` がシステムオブジェクトを所有し、共有します。
- `config.yaml` が具体的な実装を選択します。
- `Radar` はシミュレーション、デモ、mock 用の点群抽象に限られます。FAST-LIO、Nav2、RViz 向けの本番 LiDAR データは、ベンダー SDK、UDP パーサー、または `sensor_msgs/msg/PointCloud2` を publish する既存の ROS2 LiDAR ドライバーから取得する必要があります。
- Astrall SDK レイヤーは、ロボットベース制御と状態ブリッジングに使用します: `AstrallMove`、IMU、SPORT、ジョイスティック、RGB カメラ、SDK ステータス、システムステータス。
- Python はタスク層に留めるべきです。シャーシの閉ループ制御を直接実行するのではなく、`NavigateToPose`/`FollowWaypoints` などの ROS2 action を呼び出してください。

## ROS2 デプロイ構成

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

フロント LiDAR は `10.18.0.120`、MSOP `6699`、DIFOP `7788`、IMU `6688` を想定しています。リア LiDAR は `10.18.0.121`、MSOP `6969`、DIFOP `7878`、IMU `6868` を想定しています。これらの LiDAR ストリームは Astrall SDK の購読トピックではありません。

## 依存関係

- C++20 コンパイラ
- CMake 3.20+
- Python 3
- pybind11
- yaml-cpp
- Python のテストとデモ用の numpy

Ubuntu:

```bash
sudo apt install cmake g++ pybind11-dev libyaml-cpp-dev python3-numpy
```

vcpkg や conda などの代替パッケージマネージャーでも `pybind11` と `yaml-cpp` を提供できます。

Windows では、`yaml-cpp` パッケージと同じ ABI でビルドされたコンパイラを使用してください。たとえば、MSVC でビルドされた `yaml-cpp` は MSVC と、MinGW でビルドされた `yaml-cpp` は MinGW と組み合わせます。

## ビルド

`ASTRALL_ENABLE_SDK` は Linux では既定で `ON`、その他のプラットフォームでは既定で `OFF` です。同梱 SDK ライブラリは Linux の `.so` であるため、非 Linux のローカルビルドでも SDK wrapper なしでシミュレーション/デモ用コアをコンパイルできます。

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Python の使い方

コンパイル済みの `astrall` 拡張を含むディレクトリを `PYTHONPATH` に設定します。

Linux/macOS のシングル構成例:

```bash
PYTHONPATH=build/astrall_py python astrall_core/examples/python_demo.py
```

Windows/非 Linux のマルチ構成例:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
python astrall_core\examples\python_demo.py
```

Python API 例:

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

## C++ デモ

```bash
cmake --build build --target astrall_cpp_demo --config Release
./build/astrall_core/astrall_cpp_demo
```

Windows のマルチ構成ジェネレーターでは:

```powershell
.\build\astrall_core\Release\astrall_cpp_demo.exe
```

## テスト

Python 拡張をビルドした後:

```bash
PYTHONPATH=build/astrall_py pytest astrall_core/tests/smoke_test.py
```

Windows では:

```powershell
$env:PYTHONPATH="build\astrall_py\Release"
pytest astrall_core\tests\smoke_test.py
```

## 注意

- Python から実ハードウェアデバイスを繰り返し構築しないでください。ハードウェアハンドルを 1 か所で共有および所有できるよう、`Runtime.from_config()` を優先してください。
- C++ のデバイスインターフェースは `ImageFrame` と `PointCloud` を使用します。Python の `get_frame()` と `get_pointcloud()` は利便性のためコピー済みの numpy 配列を返します。将来のゼロコピー経路では、明示的な所有権とライフタイム規則を定義する必要があります。
- `ASTRALL_ENABLE_SDK=ON` の場合、`RealBackend` は SDK によってバックアップされます。`backend: real` は Astrall SDK を初期化し、既定で制御権限を要求します。安全なハードウェア環境でのみ使用してください。
- `sdk_ip` と `robot_ip` は、想定されるネットワークと診断情報を記録します。現在の Astrall C API は IP setter を公開していません。
- ROS2 base-driver、LiDAR-driver、localization、Nav2、テスト計画については `docs/astrall_ros2_architecture.md` を参照してください。
