#include <memory>

#include <pybind11/pybind11.h>

#include "astrall/backend/backend.hpp"
#include "astrall/backend/real_backend.hpp"
#include "astrall/backend/sim_backend.hpp"

namespace py = pybind11;

void bind_backend(py::module_& m) {
    py::class_<astrall::Backend, std::shared_ptr<astrall::Backend>>(m, "Backend")
        .def("send_velocity", &astrall::Backend::sendVelocity, py::arg("cmd"))
        .def("get_current_pose", &astrall::Backend::getCurrentPose)
        .def("stop", &astrall::Backend::stop);

    py::class_<astrall::SimBackend, astrall::Backend, std::shared_ptr<astrall::SimBackend>>(m, "SimBackend")
        .def(py::init<double>(), py::arg("dt") = 0.02)
        .def("dt", &astrall::SimBackend::dt)
        .def("last_command", &astrall::SimBackend::lastCommand);

    py::class_<astrall::RealBackend, astrall::Backend, std::shared_ptr<astrall::RealBackend>>(m, "RealBackend")
        .def(py::init<std::string, int>(), py::arg("port") = "/dev/ttyUSB0", py::arg("baudrate") = 115200)
        .def("port", &astrall::RealBackend::port)
        .def("baudrate", &astrall::RealBackend::baudrate);
}
