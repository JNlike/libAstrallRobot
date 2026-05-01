#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

#include "astrall/common/geometry.hpp"

namespace astrall {

struct BackendImuData {
    long long timestamp = 0;
    std::array<float, 4> quaternion_xyzw{};
    std::array<float, 3> angular_velocity{};
    std::array<float, 3> linear_acceleration{};
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float odom_x = 0.0f;
    float odom_y = 0.0f;
};

struct WheelSpeeds {
    long long timestamp = 0;
    std::array<float, 4> values{};
};

struct BackendStatus {
    bool initialized = false;
    bool connected = false;
    bool control_authority = false;
    bool error = false;
    std::string message;
    std::uint32_t system_code = 0;
    std::uint32_t error_code = 0;
    std::uint32_t warning_code = 0;
};

class Backend {
public:
    virtual ~Backend() = default;

    virtual void sendVelocity(const Twist2D& cmd) = 0;
    virtual Pose2D getCurrentPose() const = 0;
    virtual void stop() = 0;
    virtual std::optional<BackendImuData> latestImu() const = 0;
    virtual std::optional<WheelSpeeds> latestWheelSpeeds() const = 0;
    virtual BackendStatus status() const = 0;
    virtual bool hasControlAuthority() const = 0;
};

}  // namespace astrall
