#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_common(py::module_& m);
void bind_backend(py::module_& m);
void bind_control(py::module_& m);
void bind_planning(py::module_& m);
void bind_navigation(py::module_& m);
void bind_behavior(py::module_& m);
void bind_device(py::module_& m);
void bind_runtime(py::module_& m);

PYBIND11_MODULE(astrall, m) {
    m.doc() = "Astrall C++20 robot runtime";

    bind_common(m);
    bind_backend(m);
    bind_control(m);
    bind_planning(m);
    bind_navigation(m);
    bind_behavior(m);
    bind_device(m);
    bind_runtime(m);
}
