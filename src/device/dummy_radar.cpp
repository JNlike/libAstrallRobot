#include "astrall/device/dummy_radar.hpp"

#include <algorithm>
#include <cmath>

namespace astrall {

DummyRadar::DummyRadar(int point_count)
    : point_count_(std::max(1, point_count)) {}

PointCloud DummyRadar::getPointCloud() {
    PointCloud cloud;
    cloud.points.reserve(static_cast<std::size_t>(point_count_));

    for (int i = 0; i < point_count_; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(point_count_);
        const float angle = t * 6.28318530718f;
        PointXYZI point;
        point.x = std::cos(angle) * (1.0f + t);
        point.y = std::sin(angle) * (1.0f + t);
        point.z = 0.2f * std::sin(angle * 4.0f);
        point.intensity = t;
        cloud.points.push_back(point);
    }
    return cloud;
}

}  // namespace astrall
