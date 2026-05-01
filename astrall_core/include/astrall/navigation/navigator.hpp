#pragma once

#include <cstddef>
#include <memory>

#include "astrall/backend/backend.hpp"
#include "astrall/common/status.hpp"
#include "astrall/control/controller.hpp"
#include "astrall/planning/planner.hpp"

namespace astrall {

// Connects the core Planner and Controller for demo/sim/minimal runtime goals.
// It is not a Nav2 replacement; production route execution should stay in ROS2
// actions and Nav2, which then publish /cmd_vel to the base driver.
class Navigator {
public:
    Navigator(std::shared_ptr<Planner> planner,
              std::shared_ptr<Controller> controller,
              std::shared_ptr<Backend> backend);

    void setGoal(const Point2D& goal);
    NavStatus update();
    void cancel();
    NavStatus status() const;

private:
    std::shared_ptr<Planner> planner_;
    std::shared_ptr<Controller> controller_;
    std::shared_ptr<Backend> backend_;
    Path path_;
    std::size_t current_index_ = 0;
    NavStatus status_ = NavStatus::Idle;
};

}  // namespace astrall
