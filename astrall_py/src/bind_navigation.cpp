#include <memory>

#include <pybind11/pybind11.h>

#include "astrall/navigation/navigator.hpp"

namespace py = pybind11;

void bind_navigation(py::module_& m) {
    py::class_<astrall::Navigator, std::shared_ptr<astrall::Navigator>>(m, "Navigator")
        .def(py::init<std::shared_ptr<astrall::Planner>,
                      std::shared_ptr<astrall::Controller>,
                      std::shared_ptr<astrall::Backend>>(),
             py::arg("planner"), py::arg("controller"), py::arg("backend"))
        .def("set_goal", &astrall::Navigator::setGoal, py::arg("goal"))
        .def("update", &astrall::Navigator::update)
        .def("cancel", &astrall::Navigator::cancel)
        .def("status", &astrall::Navigator::status);
}
