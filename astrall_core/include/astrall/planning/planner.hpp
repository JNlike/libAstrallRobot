#pragma once

#include "astrall/common/geometry.hpp"

namespace astrall {

class Planner {
public:
    virtual ~Planner() = default;
    virtual Path plan(const Pose2D& start, const Point2D& goal) = 0;
};

}  // namespace astrall
