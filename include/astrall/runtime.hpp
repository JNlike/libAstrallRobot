#pragma once

#include <memory>
#include <string>

#include "astrall/backend/backend.hpp"
#include "astrall/behavior/state_machine.hpp"
#include "astrall/control/controller.hpp"
#include "astrall/device/camera.hpp"
#include "astrall/device/radar.hpp"
#include "astrall/navigation/navigator.hpp"
#include "astrall/planning/planner.hpp"

namespace astrall {

class Runtime {
public:
    static std::shared_ptr<Runtime> fromConfig(const std::string& config_path);

    std::shared_ptr<Backend> backend();
    std::shared_ptr<Controller> controller();
    std::shared_ptr<Planner> planner();
    std::shared_ptr<Navigator> navigator();
    std::shared_ptr<StateMachine> stateMachine();
    std::shared_ptr<Camera> camera();
    std::shared_ptr<Radar> radar();

private:
    std::shared_ptr<Backend> backend_;
    std::shared_ptr<Controller> controller_;
    std::shared_ptr<Planner> planner_;
    std::shared_ptr<Navigator> navigator_;
    std::shared_ptr<StateMachine> state_machine_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Radar> radar_;
};

}  // namespace astrall
