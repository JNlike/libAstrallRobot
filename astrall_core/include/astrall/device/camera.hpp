#pragma once

#include <cstdint>
#include <vector>

namespace astrall {

struct ImageFrame {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<std::uint8_t> data;//width * height * channels bytes of pixel data in row-major order
    // pixel(x,y) = data[(y * width + x) * channels + c] where c is the channel index (0 for R, 1 for G, 2 for B, etc.)
    // [255,255,255,0,0,255,...] a image with 2 pixels: white and blue, in RGB format would be represented as [255,255,255,0,0,255] where the first 3 bytes are the white pixel (R=255,G=255,B=255) and the next 3 bytes are the blue pixel (R=0,G=0,B=255)
};

class Camera {
public:
    virtual ~Camera() = default;
    virtual ImageFrame getFrame() = 0;
};

}  // namespace astrall
