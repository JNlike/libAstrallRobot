#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "astrall/planning/planner.hpp"
#include "astrall/planning/straight_line_planner.hpp"

namespace py = pybind11;

namespace {

class PyPlanner : public astrall::Planner {
public:
    using astrall::Planner::Planner;

    astrall::Path plan(const astrall::Pose2D& start, const astrall::Point2D& goal) override {
        PYBIND11_OVERRIDE_PURE(
            astrall::Path,
            astrall::Planner,
            plan,
            start,
            goal);
    }
};

}  // namespace

void bind_planning(py::module_& m) {
    py::class_<astrall::Planner, PyPlanner, std::shared_ptr<astrall::Planner>>(m, "Planner")
        .def(py::init<>())
        .def("plan", &astrall::Planner::plan, py::arg("start"), py::arg("goal"));

    py::class_<astrall::StraightLinePlanner, astrall::Planner, std::shared_ptr<astrall::StraightLinePlanner>>(m, "StraightLinePlanner")
        .def(py::init<int>(), py::arg("waypoint_count") = 20)
        .def("plan", &astrall::StraightLinePlanner::plan, py::arg("start"), py::arg("goal"))
        .def("waypoint_count", &astrall::StraightLinePlanner::waypointCount);
}
