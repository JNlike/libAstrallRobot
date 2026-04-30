#include "astrall/backend/real_backend.hpp"

#include <iostream>
#include <utility>

namespace astrall {

RealBackend::RealBackend(std::string port, int baudrate)
    : port_(std::move(port)), baudrate_(baudrate) {}

void RealBackend::sendVelocity(const Twist2D& cmd) {
    std::cout << "[RealBackend] send velocity vx=" << cmd.vx
              << " vy=" << cmd.vy
              << " w=" << cmd.w
              << " via " << port_ << "@" << baudrate_ << '\n';
}

Pose2D RealBackend::getCurrentPose() const {
    return pose_;
}

void RealBackend::stop() {
    std::cout << "[RealBackend] stop\n";
}

const std::string& RealBackend::port() const {
    return port_;
}

int RealBackend::baudrate() const {
    return baudrate_;
}

}  // namespace astrall
