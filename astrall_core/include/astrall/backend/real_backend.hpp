#pragma once

#include <string>

#include "astrall/backend/backend.hpp"

namespace astrall {

class RealBackend final : public Backend {
public:
    RealBackend(std::string port = "/dev/ttyUSB0", int baudrate = 115200);

    void sendVelocity(const Twist2D& cmd) override;
    Pose2D getCurrentPose() const override;
    void stop() override;

    const std::string& port() const;
    int baudrate() const;

private:
    std::string port_;
    int baudrate_ = 115200;
    Pose2D pose_;
};

}  // namespace astrall
