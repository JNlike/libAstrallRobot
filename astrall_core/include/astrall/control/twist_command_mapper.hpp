#pragma once

#include "astrall/common/geometry.hpp"

namespace astrall {

struct VelocityLimits {
    double max_vx = 1.0;
    double max_vy = 1.0;
    double max_wz = 1.0;
};

struct TwistMappingConfig {
    double scale_vx = 1.0;
    double scale_vy = 1.0;
    double scale_wz = 1.0;
    int sign_vx = 1;
    int sign_vy = 1;
    int sign_wz = 1;
};

Twist2D clampTwist(const Twist2D& cmd, const VelocityLimits& limits);
Twist2D mapRosTwistToSdkMove(const Twist2D& ros_cmd,
                             const VelocityLimits& limits,
                             const TwistMappingConfig& mapping);

}  // namespace astrall
