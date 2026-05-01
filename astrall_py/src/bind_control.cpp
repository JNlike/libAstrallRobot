#include <memory>

#include <pybind11/pybind11.h>

#include "astrall/control/controller.hpp"

namespace py = pybind11;

void bind_control(py::module_& m) {
    py::class_<astrall::Controller, std::shared_ptr<astrall::Controller>>(m, "Controller")
        .def(py::init<std::shared_ptr<astrall::Backend>>(), py::arg("backend"))
        .def(py::init<std::shared_ptr<astrall::Backend>, double, double, double, double>(),
             py::arg("backend"),
             py::arg("max_v"),
             py::arg("max_w"),
             py::arg("position_tolerance"),
             py::arg("angle_tolerance"))
        .def("set_velocity", &astrall::Controller::setVelocity, py::arg("cmd"))
        .def("go_to_pose", &astrall::Controller::goToPose, py::arg("target"))
        .def("stop", &astrall::Controller::stop);
}
