#pragma once

#include <memory>
#include <vector>

#include "astrall/common/geometry.hpp"
#include "astrall/common/status.hpp"
#include "astrall/navigation/navigator.hpp"

namespace astrall {

class StateMachine {
public:
    explicit StateMachine(std::shared_ptr<Navigator> navigator);

    void startMission(const std::vector<Point2D>& route);
    void update();
    bool running() const;
    RobotState state() const;
    void emergencyStop();

private:
    void startCurrentGoal();

    std::shared_ptr<Navigator> navigator_;
    std::vector<Point2D> route_;
    std::size_t route_index_ = 0;
    RobotState state_ = RobotState::Idle;
};

}  // namespace astrall
