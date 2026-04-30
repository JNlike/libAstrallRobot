#pragma once

#include "astrall/planning/planner.hpp"

namespace astrall {

class StraightLinePlanner final : public Planner {
public:
    explicit StraightLinePlanner(int waypoint_count = 20);

    Path plan(const Pose2D& start, const Point2D& goal) override;
    int waypointCount() const;

private:
    int waypoint_count_ = 20;
};

}  // namespace astrall
