#pragma once

#include "astrall/common/geometry.hpp"

namespace astrall {

class Backend {
public:
    virtual ~Backend() = default;

    virtual void sendVelocity(const Twist2D& cmd) = 0;
    virtual Pose2D getCurrentPose() const = 0;
    virtual void stop() = 0;
};

}  // namespace astrall
