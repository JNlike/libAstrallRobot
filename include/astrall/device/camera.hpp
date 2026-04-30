#pragma once

#include <cstdint>
#include <vector>

namespace astrall {

struct ImageFrame {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<std::uint8_t> data;
};

class Camera {
public:
    virtual ~Camera() = default;
    virtual ImageFrame getFrame() = 0;
};

}  // namespace astrall
