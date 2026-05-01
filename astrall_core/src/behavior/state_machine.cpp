#include "astrall/behavior/state_machine.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace astrall {

StateMachine::StateMachine(std::shared_ptr<Navigator> navigator)
    : navigator_(std::move(navigator)) {
    if (!navigator_) {
        throw std::invalid_argument("StateMachine requires a navigator");
    }
}

void StateMachine::startMission(const std::vector<Point2D>& route) {
    route_ = route;
    route_index_ = 0;
    state_ = route_.empty() ? RobotState::Idle : RobotState::Init;
    if (state_ == RobotState::Init) {
        startCurrentGoal();
    }
}

void StateMachine::update() {
    if (state_ == RobotState::Init) {
        state_ = RobotState::Navigate;
    }

    if (state_ == RobotState::Navigate) {
        const NavStatus nav_status = navigator_->update();
        if (nav_status == NavStatus::Reached) {
            state_ = RobotState::Inspect;
        } else if (nav_status == NavStatus::Blocked || nav_status == NavStatus::Failed) {
            // Blocked is reserved for future obstacle-aware navigation.
            state_ = RobotState::Error;
        }
    }

    if (state_ == RobotState::Inspect) {
        std::cout << "[StateMachine] inspect point " << route_index_ << '\n';
        ++route_index_;
        if (route_index_ >= route_.size()) {
            state_ = RobotState::Idle;
        } else {
            startCurrentGoal();
        }
    }
}

bool StateMachine::running() const {
    return state_ == RobotState::Init ||
           state_ == RobotState::Navigate ||
           state_ == RobotState::Inspect ||
           // ReturnHome is reserved for a future mission flow.
           state_ == RobotState::ReturnHome;
}

RobotState StateMachine::state() const {
    return state_;
}

void StateMachine::emergencyStop() {
    navigator_->cancel();
    state_ = RobotState::EmergencyStop;
}

void StateMachine::startCurrentGoal() {
    navigator_->setGoal(route_.at(route_index_));
    state_ = RobotState::Navigate;
}

}  // namespace astrall
