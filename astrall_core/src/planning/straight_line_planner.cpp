#include "astrall/planning/straight_line_planner.hpp"

#include <algorithm>
#include <cmath>

namespace astrall {

StraightLinePlanner::StraightLinePlanner(int waypoint_count)
    : waypoint_count_(std::max(1, waypoint_count)) {}

Path StraightLinePlanner::plan(const Pose2D& start, const Point2D& goal) {
    Path path;
    path.waypoints.reserve(static_cast<std::size_t>(waypoint_count_));

    const double theta = std::atan2(goal.y - start.y, goal.x - start.x);
    for (int i = 1; i <= waypoint_count_; ++i) {
        const double t = static_cast<double>(i) / static_cast<double>(waypoint_count_);
        path.waypoints.emplace_back(
            start.x + (goal.x - start.x) * t,
            start.y + (goal.y - start.y) * t,
            theta);
    }
    return path;
}

int StraightLinePlanner::waypointCount() const {
    return waypoint_count_;
}

}  // namespace astrall
