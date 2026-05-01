#pragma once

namespace astrall {

enum class NavStatus {
    Idle,
    Running,
    Reached,
    // Reserved for future obstacle-aware navigation.
    Blocked,
    Failed
};

enum class RobotState {
    Idle,
    Init,
    Navigate,
    Inspect,
    // Reserved for a future return-home mission flow.
    ReturnHome,
    Error,
    EmergencyStop
};

}  // namespace astrall
