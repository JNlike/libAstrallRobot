#include <memory>

#include <pybind11/pybind11.h>

#include "astrall/runtime.hpp"

namespace py = pybind11;

void bind_runtime(py::module_& m) {
    py::class_<astrall::Runtime, std::shared_ptr<astrall::Runtime>>(m, "Runtime")
        .def_static("from_config", &astrall::Runtime::fromConfig, py::arg("config_path"))
        .def("backend", &astrall::Runtime::backend)
        .def("controller", &astrall::Runtime::controller)
        .def("planner", &astrall::Runtime::planner)
        .def("navigator", &astrall::Runtime::navigator)
        .def("state_machine", &astrall::Runtime::stateMachine)
        .def("camera", &astrall::Runtime::camera)
        .def("radar", &astrall::Runtime::radar);

    m.def("from_config", &astrall::Runtime::fromConfig, py::arg("config_path"));
}
