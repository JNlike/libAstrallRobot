#include "astrall/navigation/navigator.hpp"

#include <stdexcept>
#include <utility>

namespace astrall {

Navigator::Navigator(std::shared_ptr<Planner> planner,
                     std::shared_ptr<Controller> controller,
                     std::shared_ptr<Backend> backend)
    : planner_(std::move(planner)),
      controller_(std::move(controller)),
      backend_(std::move(backend)) {
    if (!planner_ || !controller_ || !backend_) {
        throw std::invalid_argument("Navigator requires planner, controller, and backend");
    }
}

void Navigator::setGoal(const Point2D& goal) {
    path_ = planner_->plan(backend_->getCurrentPose(), goal);
    current_index_ = 0;
    status_ = path_.waypoints.empty() ? NavStatus::Reached : NavStatus::Running;
}

NavStatus Navigator::update() {
    if (status_ != NavStatus::Running) {
        return status_;
    }

    if (current_index_ >= path_.waypoints.size()) {
        controller_->stop();
        status_ = NavStatus::Reached;
        return status_;
    }

    const bool reached = controller_->goToPose(path_.waypoints[current_index_]);
    if (reached) {
        ++current_index_;
        if (current_index_ >= path_.waypoints.size()) {
            controller_->stop();
            status_ = NavStatus::Reached;
        }
    }
    return status_;
}

void Navigator::cancel() {
    controller_->stop();
    path_.waypoints.clear();
    current_index_ = 0;
    status_ = NavStatus::Idle;
}

NavStatus Navigator::status() const {
    return status_;
}

}  // namespace astrall
