#include "astrall/control/twist_command_mapper.hpp"

#include <algorithm>
#include <cmath>

namespace astrall {

namespace {

double clampSymmetric(double value, double limit) {
    const double magnitude = std::max(0.0, std::abs(limit));
    return std::clamp(value, -magnitude, magnitude);
}

double normalizedSign(int sign) {
    return sign < 0 ? -1.0 : 1.0;
}

}  // namespace

Twist2D clampTwist(const Twist2D& cmd, const VelocityLimits& limits) {
    return Twist2D{
        clampSymmetric(cmd.vx, limits.max_vx),
        clampSymmetric(cmd.vy, limits.max_vy),
        clampSymmetric(cmd.w, limits.max_wz),
    };
}

Twist2D mapRosTwistToSdkMove(const Twist2D& ros_cmd,
                             const VelocityLimits& limits,
                             const TwistMappingConfig& mapping) {
    const Twist2D clamped = clampTwist(ros_cmd, limits);
    return Twist2D{
        mapping.scale_vx * normalizedSign(mapping.sign_vx) * clamped.vx,
        mapping.scale_vy * normalizedSign(mapping.sign_vy) * clamped.vy,
        mapping.scale_wz * normalizedSign(mapping.sign_wz) * clamped.w,
    };
}

}  // namespace astrall
