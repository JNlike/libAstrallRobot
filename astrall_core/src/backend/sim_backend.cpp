#include "astrall/backend/sim_backend.hpp"

namespace astrall {

SimBackend::SimBackend(double dt) : dt_(dt) {}

void SimBackend::sendVelocity(const Twist2D& cmd) {
    std::lock_guard<std::mutex> lock(mutex_);
    pose_.x += cmd.vx * dt_;
    pose_.y += cmd.vy * dt_;
    pose_.theta += cmd.w * dt_;
    last_cmd_ = cmd;
}

Pose2D SimBackend::getCurrentPose() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pose_;
}

void SimBackend::stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    last_cmd_ = Twist2D{};
}

std::optional<BackendImuData> SimBackend::latestImu() const {
    return std::nullopt;
}

std::optional<WheelSpeeds> SimBackend::latestWheelSpeeds() const {
    return std::nullopt;
}

BackendStatus SimBackend::status() const {
    BackendStatus status;
    status.initialized = true;
    status.connected = true;
    status.control_authority = true;
    status.error = false;
    status.message = "sim";
    return status;
}

bool SimBackend::hasControlAuthority() const {
    return true;
}

double SimBackend::dt() const {
    return dt_;
}

Twist2D SimBackend::lastCommand() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_cmd_;
}

}  // namespace astrall
