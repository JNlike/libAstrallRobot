#pragma once

namespace astrall {

enum class NavStatus {
    Idle,
    Running,
    Reached,
    Blocked,
    Failed
};

enum class RobotState {
    Idle,
    Init,
    Navigate,
    Inspect,
    ReturnHome,
    Error,
    EmergencyStop
};

}  // namespace astrall
