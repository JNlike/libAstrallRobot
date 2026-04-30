#include "astrall/control/controller.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>
#include <utility>

namespace astrall {

namespace {

double normalizeAngle(double angle) {
    while (angle > std::numbers::pi) {
        angle -= 2.0 * std::numbers::pi;
    }
    while (angle < -std::numbers::pi) {
        angle += 2.0 * std::numbers::pi;
    }
    return angle;
}

}  // namespace

Controller::Controller(std::shared_ptr<Backend> backend)
    : backend_(std::move(backend)) {
    if (!backend_) {
        throw std::invalid_argument("Controller requires a backend");
    }
}

Controller::Controller(std::shared_ptr<Backend> backend,
                       double max_v,
                       double max_w,
                       double position_tolerance,
                       double angle_tolerance)
    : Controller(std::move(backend)) {
    max_v_ = max_v;
    max_w_ = max_w;
    position_tolerance_ = position_tolerance;
    angle_tolerance_ = angle_tolerance;
}

void Controller::setVelocity(const Twist2D& cmd) {
    backend_->sendVelocity(cmd);
}

bool Controller::goToPose(const Pose2D& target) {
    const Pose2D current = backend_->getCurrentPose();
    if (isReached(current, target)) {
        stop();
        return true;
    }
    backend_->sendVelocity(computeControl(current, target));
    return false;
}

void Controller::stop() {
    backend_->stop();
}

Twist2D Controller::computeControl(const Pose2D& current, const Pose2D& target) {
    constexpr double kp_pos = 1.0;
    constexpr double kp_theta = 1.0;
    Twist2D cmd;
    cmd.vx = std::clamp(kp_pos * (target.x - current.x), -max_v_, max_v_);
    cmd.vy = std::clamp(kp_pos * (target.y - current.y), -max_v_, max_v_);
    cmd.w = std::clamp(kp_theta * normalizeAngle(target.theta - current.theta), -max_w_, max_w_);
    return cmd;
}

bool Controller::isReached(const Pose2D& current, const Pose2D& target) const {
    const double dx = target.x - current.x;
    const double dy = target.y - current.y;
    const double distance = std::hypot(dx, dy);
    const double angle_error = std::abs(normalizeAngle(target.theta - current.theta));
    return distance <= position_tolerance_ && angle_error <= angle_tolerance_;
}

}  // namespace astrall
