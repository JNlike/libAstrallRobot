#pragma once

#include <vector>

namespace astrall {

struct PointXYZI {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float intensity = 0.0f;
};

struct PointCloud {
    std::vector<PointXYZI> points;
};

class Radar {
public:
    virtual ~Radar() = default;
    virtual PointCloud getPointCloud() = 0;
};

}  // namespace astrall
