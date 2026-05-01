#pragma once

#include <vector>

namespace astrall {

struct Twist2D {
    double vx = 0.0;
    double vy = 0.0;
    // Angular velocity around the z axis, in radians per second.
    double w = 0.0;

    Twist2D() = default;
    Twist2D(double vx_, double vy_, double w_) : vx(vx_), vy(vy_), w(w_) {}
};

struct Point2D {
    double x = 0.0;
    double y = 0.0;

    Point2D() = default;
    Point2D(double x_, double y_) : x(x_), y(y_) {}
};

struct Pose2D {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;

    Pose2D() = default;
    Pose2D(double x_, double y_, double theta_) : x(x_), y(y_), theta(theta_) {}
};

struct Path {
    std::vector<Pose2D> waypoints;
};

}  // namespace astrall
