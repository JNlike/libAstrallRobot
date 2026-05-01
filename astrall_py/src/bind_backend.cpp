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

    py::class_<astrall::RealBackendConfig>(m, "RealBackendConfig")
        .def(py::init<>())
        .def_readwrite("sdk_ip", &astrall::RealBackendConfig::sdk_ip)
        .def_readwrite("robot_ip", &astrall::RealBackendConfig::robot_ip)
        .def_readwrite("init_timeout_ms", &astrall::RealBackendConfig::init_timeout_ms)
        .def_readwrite("command_timeout_ms", &astrall::RealBackendConfig::command_timeout_ms)
        .def_readwrite("imu_frequency_hz", &astrall::RealBackendConfig::imu_frequency_hz)
        .def_readwrite("sport_frequency_hz", &astrall::RealBackendConfig::sport_frequency_hz)
        .def_readwrite("sdk_quaternion_order", &astrall::RealBackendConfig::sdk_quaternion_order)
        .def_readwrite("request_control", &astrall::RealBackendConfig::request_control);

    py::class_<astrall::RealBackend, astrall::Backend, std::shared_ptr<astrall::RealBackend>>(
        m,
        "RealBackend",
        "SDK-backed hardware backend. Prefer Runtime.from_config() so hardware handles are opened once.")
        .def(py::init<const astrall::RealBackendConfig&>(), py::arg("config"))
        .def(py::init<std::string, int>(),
             py::arg("port") = "/dev/ttyUSB0",
             py::arg("baudrate") = 115200,
             "Deprecated compatibility constructor; RealBackend is SDK-backed, not serial-backed.")
        .def("config", &astrall::RealBackend::config)
        .def("port", &astrall::RealBackend::port)
        .def("baudrate", &astrall::RealBackend::baudrate);
}
