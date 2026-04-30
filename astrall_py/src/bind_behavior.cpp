#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "astrall/behavior/state_machine.hpp"

namespace py = pybind11;

void bind_behavior(py::module_& m) {
    py::class_<astrall::StateMachine, std::shared_ptr<astrall::StateMachine>>(m, "StateMachine")
        .def(py::init<std::shared_ptr<astrall::Navigator>>(), py::arg("navigator"))
        .def("start_mission", &astrall::StateMachine::startMission, py::arg("route"))
        .def("update", &astrall::StateMachine::update)
        .def("running", &astrall::StateMachine::running)
        .def("state", &astrall::StateMachine::state)
        .def("emergency_stop", &astrall::StateMachine::emergencyStop);
}
