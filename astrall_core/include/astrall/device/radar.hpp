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

// Simulation/demo point-cloud abstraction only.
//
// Production LiDAR data for Nav2, FAST-LIO, and ROS2 costmaps must come from
// an external LiDAR driver, vendor SDK, or UDP parser that publishes
// sensor_msgs/msg/PointCloud2. The Astrall SDK subscription API does not expose
// LiDAR/Radar/PointCloud topics, so this interface must not be treated as the
// production point-cloud source.
class Radar {
public:
    virtual ~Radar() = default;
    virtual PointCloud getPointCloud() = 0;
};

}  // namespace astrall
