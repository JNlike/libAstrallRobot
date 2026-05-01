#pragma once

#include <mutex>

#include "astrall/backend/backend.hpp"

namespace astrall {

class SimBackend final : public Backend {
public:
    explicit SimBackend(double dt = 0.02);

    void sendVelocity(const Twist2D& cmd) override;
    Pose2D getCurrentPose() const override;
    void stop() override;
    std::optional<BackendImuData> latestImu() const override;
    std::optional<WheelSpeeds> latestWheelSpeeds() const override;
    BackendStatus status() const override;
    bool hasControlAuthority() const override;

    double dt() const;
    Twist2D lastCommand() const;

private:
    double dt_ = 0.02;
    Pose2D pose_;
    Twist2D last_cmd_;
    mutable std::mutex mutex_;
};

}  // namespace astrall
