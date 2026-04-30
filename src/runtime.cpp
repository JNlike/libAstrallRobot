#include "astrall/runtime.hpp"

#include <stdexcept>

#include <yaml-cpp/yaml.h>

#include "astrall/backend/real_backend.hpp"
#include "astrall/backend/sim_backend.hpp"
#include "astrall/device/dummy_camera.hpp"
#include "astrall/device/dummy_radar.hpp"
#include "astrall/planning/straight_line_planner.hpp"

namespace astrall {

namespace {

template <typename T>
T getOr(const YAML::Node& node, const char* key, const T& fallback) {
    if (!node || !node[key]) {
        return fallback;
    }
    return node[key].as<T>();
}

}  // namespace

std::shared_ptr<Runtime> Runtime::fromConfig(const std::string& config_path) {
    YAML::Node config = YAML::LoadFile(config_path);
    auto runtime = std::shared_ptr<Runtime>(new Runtime());

    const std::string backend_type = getOr(config["robot"], "backend", std::string("sim"));
    if (backend_type == "sim") {
        const double dt = getOr(config["backend"]["sim"], "dt", 0.02);
        runtime->backend_ = std::make_shared<SimBackend>(dt);
    } else if (backend_type == "real") {
        const std::string port = getOr(config["backend"]["real"], "port", std::string("/dev/ttyUSB0"));
        const int baudrate = getOr(config["backend"]["real"], "baudrate", 115200);
        runtime->backend_ = std::make_shared<RealBackend>(port, baudrate);
    } else {
        throw std::runtime_error("Unsupported backend type: " + backend_type);
    }

    const YAML::Node controller_config = config["controller"];
    runtime->controller_ = std::make_shared<Controller>(
        runtime->backend_,
        getOr(controller_config, "max_v", 1.0),
        getOr(controller_config, "max_w", 1.0),
        getOr(controller_config, "position_tolerance", 0.1),
        getOr(controller_config, "angle_tolerance", 0.1));

    const YAML::Node planner_config = config["planner"];
    const std::string planner_type = getOr(planner_config, "type", std::string("straight_line"));
    if (planner_type != "straight_line") {
        throw std::runtime_error("Unsupported planner type: " + planner_type);
    }
    runtime->planner_ = std::make_shared<StraightLinePlanner>(
        getOr(planner_config, "waypoint_count", 20));

    const YAML::Node camera_config = config["camera"];
    const std::string camera_type = getOr(camera_config, "type", std::string("dummy"));
    if (camera_type != "dummy") {
        throw std::runtime_error("Unsupported camera type: " + camera_type);
    }
    runtime->camera_ = std::make_shared<DummyCamera>(
        getOr(camera_config, "width", 640),
        getOr(camera_config, "height", 480),
        getOr(camera_config, "channels", 3));

    const YAML::Node radar_config = config["radar"];
    const std::string radar_type = getOr(radar_config, "type", std::string("dummy"));
    if (radar_type != "dummy") {
        throw std::runtime_error("Unsupported radar type: " + radar_type);
    }
    runtime->radar_ = std::make_shared<DummyRadar>(
        getOr(radar_config, "point_count", 1024));

    runtime->navigator_ = std::make_shared<Navigator>(
        runtime->planner_, runtime->controller_, runtime->backend_);
    runtime->state_machine_ = std::make_shared<StateMachine>(runtime->navigator_);

    return runtime;
}

std::shared_ptr<Backend> Runtime::backend() {
    return backend_;
}

std::shared_ptr<Controller> Runtime::controller() {
    return controller_;
}

std::shared_ptr<Planner> Runtime::planner() {
    return planner_;
}

std::shared_ptr<Navigator> Runtime::navigator() {
    return navigator_;
}

std::shared_ptr<StateMachine> Runtime::stateMachine() {
    return state_machine_;
}

std::shared_ptr<Camera> Runtime::camera() {
    return camera_;
}

std::shared_ptr<Radar> Runtime::radar() {
    return radar_;
}

}  // namespace astrall
