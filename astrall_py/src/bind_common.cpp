#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "astrall/common/geometry.hpp"
#include "astrall/common/status.hpp"

namespace py = pybind11;

void bind_common(py::module_& m) {
    py::class_<astrall::Twist2D>(m, "Twist2D")
        .def(py::init<>())
        .def(py::init<double, double, double>(), py::arg("vx"), py::arg("vy"), py::arg("w"))
        .def_readwrite("vx", &astrall::Twist2D::vx)
        .def_readwrite("vy", &astrall::Twist2D::vy)
        .def_readwrite("w", &astrall::Twist2D::w)
        .def("__repr__", [](const astrall::Twist2D& t) {
            return "Twist2D(vx=" + std::to_string(t.vx) +
                   ", vy=" + std::to_string(t.vy) +
                   ", w=" + std::to_string(t.w) + ")";
        });

    py::class_<astrall::Point2D>(m, "Point2D")
        .def(py::init<>())
        .def(py::init<double, double>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &astrall::Point2D::x)
        .def_readwrite("y", &astrall::Point2D::y)
        .def("__repr__", [](const astrall::Point2D& p) {
            return "Point2D(x=" + std::to_string(p.x) +
                   ", y=" + std::to_string(p.y) + ")";
        });

    py::class_<astrall::Pose2D>(m, "Pose2D")
        .def(py::init<>())
        .def(py::init<double, double, double>(), py::arg("x"), py::arg("y"), py::arg("theta"))
        .def_readwrite("x", &astrall::Pose2D::x)
        .def_readwrite("y", &astrall::Pose2D::y)
        .def_readwrite("theta", &astrall::Pose2D::theta)
        .def("__repr__", [](const astrall::Pose2D& p) {
            return "Pose2D(x=" + std::to_string(p.x) +
                   ", y=" + std::to_string(p.y) +
                   ", theta=" + std::to_string(p.theta) + ")";
        });

    py::class_<astrall::Path>(m, "Path")
        .def(py::init<>())
        .def_readwrite("waypoints", &astrall::Path::waypoints);

    py::enum_<astrall::NavStatus>(m, "NavStatus")
        .value("Idle", astrall::NavStatus::Idle)
        .value("Running", astrall::NavStatus::Running)
        .value("Reached", astrall::NavStatus::Reached)
        .value("Blocked", astrall::NavStatus::Blocked)
        .value("Failed", astrall::NavStatus::Failed);

    py::enum_<astrall::RobotState>(m, "RobotState")
        .value("Idle", astrall::RobotState::Idle)
        .value("Init", astrall::RobotState::Init)
        .value("Navigate", astrall::RobotState::Navigate)
        .value("Inspect", astrall::RobotState::Inspect)
        .value("ReturnHome", astrall::RobotState::ReturnHome)
        .value("Error", astrall::RobotState::Error)
        .value("EmergencyStop", astrall::RobotState::EmergencyStop);
}
