**问题总结**

当前项目的总体方向是对的：`astrall_core` 负责 C++ Runtime 和 SDK 隔离，`astrall_py` 做 Python 高层入口，`astrall_ros2` 做 ROS2 适配层。但现在更像一个架构雏形/MVP，还不是完整生产级机器人 Runtime。

主要问题如下：

1. **Runtime 还不够“生产级”**
   `Runtime::fromConfig()` 已经能统一创建 Backend、Controller、Planner、Navigator、StateMachine、Camera、Radar，但目前很多实现仍是 demo/sim 级别。真实部署时 ROS2 base driver 又直接创建 `Backend`，没有走完整 Runtime，这需要在文档中明确是有意为之。

2. **闭源 Astrall SDK 的生命周期封装不够安全**
   `RealBackend` 和 `AstrallSdkWrapper` 析构时没有明确 `stop()`、释放控制权、关闭订阅、deinit 的顺序。对机器人底盘来说，这属于高优先级风险。

3. **SDK 调用线程边界不清楚**
   回调数据缓存用了 mutex，但 `move()`、`systemStatus()`、`requestControl()` 等 SDK 调用没有统一串行化。闭源 C API 是否线程安全未知，ROS2 timer、订阅回调和 SDK 回调可能交错。

4. **真实位姿没有接入**
   `RealBackend::getCurrentPose()` 目前只是返回未更新的 `pose_`。这意味着 core 里的 `Navigator / Controller` 不能用于真实机器人导航闭环。

5. **core 内部导航与 Nav2 概念容易混淆**
   `Planner / Controller / Navigator / StateMachine` 在 core 中存在，但真实 ROS2 部署应由 FAST-LIO/localization + Nav2 负责定位和导航闭环。core 这套更适合 demo、仿真、最小任务流，不应和 Nav2 抢职责。

6. **ROS2 不是单纯数据接口**
   ROS2 当前承担数据发布、控制入口、系统集成三件事：发布 IMU/轮速/状态，接收 `/cmd_vel`，对接 Nav2/FAST-LIO/LiDAR 驱动。文档里如果说“ROS2 只是数据接口”会过于简化。

7. **Nav2 不能只描述成“路径规划”**
   Nav2 通常包含 global planner、local controller、costmap、behavior tree、recovery 等。更准确说法是：Nav2 负责导航闭环并输出 `/cmd_vel`。

8. **LiDAR/Radar 边界虽已写清，但仍需防止误用**
   `Radar` 是 demo/mock 点云接口，不应成为生产 PointCloud2 来源。生产 LiDAR 应来自 vendor SDK、UDP parser 或 ROS2 LiDAR driver。

9. **状态查询可能太重**
   `RealBackend::status()` 内部会同步调用 SDK `systemStatus()`。ROS2 safety/status/diagnostics 多处调用 status，可能造成频繁 SDK 查询。

10. **扩展机制仍偏硬编码**
    planner、camera、radar 类型都在 `Runtime::fromConfig()` 中硬编码判断。后续新增真实相机、更多 planner、不同 backend 时，Runtime 会变胖。

11. **测试覆盖不足**
    目前有 mapper、sim backend、navigator 异常等测试，但缺少 fake SDK 下的真实 backend 测试：控制权丢失、SDK 失败码、超时 stop、析构释放、状态缓存等都还没覆盖。

12. **文档可能给人“已完整实现”的错觉**
    `radar_node`、`odom_node`、`camera_node` 目前更多是边界说明，不是完整生产节点。README 应明确哪些是已实现，哪些是预期外部组件或未来工作。

**ToDoList**

1. **明确生产架构文档**
   - 把生产链路固定描述为：
     `LiDAR ROS2 driver -> PointCloud2 -> FAST-LIO/localization -> odom + TF -> Nav2 -> /cmd_vel -> astrall_base_driver -> astrall_core Backend -> RealBackend -> Astrall SDK`
   - 明确 core `Planner / Navigator / StateMachine` 是 demo/sim/minimal runtime，不是生产 Nav2 替代品。
   - 明确 `astrall_ros2/controller_node` 是 base driver，不是 Nav2 controller。

2. **补强 SDK 生命周期**
   - 给 `AstrallSdkWrapper` 增加显式 `shutdown()`。
   - 析构时按顺序执行 stop、release control、关闭订阅、deinit。
   - `RealBackend` 析构时调用安全停机路径。
   - 确保析构不抛异常。

3. **定义 SDK 线程模型**
   - 所有闭源 SDK C API 调用统一串行化。
   - 明确回调中只缓存数据，不直接做复杂操作。
   - 避免 `status()`、`move()`、`stop()` 并发打到底层 SDK。

4. **重构状态读取**
   - 将 `systemStatus()` 结果缓存起来。
   - `Backend::status()` 返回快照，避免高频同步 SDK 查询。
   - ROS2 diagnostics 使用缓存状态。

5. **处理真实位姿边界**
   - 方案 A：`RealBackend::getCurrentPose()` 明确不用于生产导航，只用于 SDK odom 快照或返回 unavailable。
   - 方案 B：用 SDK `odomX/odomY/yaw` 更新 `pose_`，但文档说明精度和用途。
   - 生产导航仍以 FAST-LIO/外部 localization 的 odom/TF 为准。

6. **引入 fake SDK 测试接口**
   - 抽象 `SdkClient` 或 `AstrallSdkApi` 接口。
   - `RealBackend` 依赖接口而不是直接依赖真实 SDK wrapper。
   - 用 fake SDK 测试 init 失败、控制权丢失、move 失败、stop 超时、状态错误、析构释放。

7. **强化 ROS2 base driver**
   - 确认 `/cmd_vel` timeout 一定触发 stop。
   - no control authority 时禁止发送 move。
   - backend error 时 stop 并发布 diagnostics error。
   - monitor mode 下不请求控制权、不发运动命令。

8. **减少 Runtime 硬编码**
   - 拆出 `createPlanner()`、`createCamera()`、`createRadar()`。
   - 后续再考虑注册式 factory。
   - 保持 `Runtime::fromConfig()` 只负责装配，不堆业务分支。

9. **文档标注实现状态**
   - README 中区分：已实现、外部依赖、未来计划。
   - 标注 `radar_node / odom_node / camera_node` 当前主要是边界文档。
   - 中文、日文、法文、德文 README 与英文同步更新。

10. **避免误用 Radar**
    - 保留 `Radar` 为 mock/demo。
    - 不让它发布生产 PointCloud2。
    - ROS2 LiDAR 数据只来自外部 driver/vendor SDK/UDP parser。

11. **补齐安全测试**
    - fake SDK 单元测试。
    - ROS2 `/cmd_vel` timeout 测试。
    - no-control-authority 测试。
    - SDK error -> stop 测试。
    - diagnostics 状态测试。

12. **最后再考虑 Eigen**
    - 当前不需要引入。
    - 等到做 3D transform、滤波、标定、优化、复杂运动学时再作为内部实现依赖引入。
    - 不把 Eigen 类型暴露到 Python 或公共 API。