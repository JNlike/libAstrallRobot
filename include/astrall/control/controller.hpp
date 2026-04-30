#pragma once

#include <memory>

#include "astrall/backend/backend.hpp"

namespace astrall {

class Controller {
public:
    explicit Controller(std::shared_ptr<Backend> backend);
    Controller(std::shared_ptr<Backend> backend,
               double max_v,
               double max_w,
               double position_tolerance,
               double angle_tolerance);

    void setVelocity(const Twist2D& cmd);
    bool goToPose(const Pose2D& target);
    void stop();

private:
    Twist2D computeControl(const Pose2D& current, const Pose2D& target);
    bool isReached(const Pose2D& current, const Pose2D& target) const;

    std::shared_ptr<Backend> backend_;
    double max_v_ = 1.0;
    double max_w_ = 1.0;
    double position_tolerance_ = 0.1;
    double angle_tolerance_ = 0.1;
};

}  // namespace astrall
