#pragma once

#include "astrall/device/radar.hpp"

namespace astrall {

class DummyRadar final : public Radar {
public:
    explicit DummyRadar(int point_count = 1024);

    PointCloud getPointCloud() override;

private:
    int point_count_ = 1024;
};

}  // namespace astrall
