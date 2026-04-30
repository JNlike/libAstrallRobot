#include "astrall/device/dummy_camera.hpp"

#include <algorithm>
#include <cstddef>

namespace astrall {

DummyCamera::DummyCamera(int width, int height, int channels)
    : width_(std::max(1, width)),
      height_(std::max(1, height)),
      channels_(std::max(1, channels)) {}

ImageFrame DummyCamera::getFrame() {
    ImageFrame frame;
    frame.width = width_;
    frame.height = height_;
    frame.channels = channels_;
    frame.data.resize(static_cast<std::size_t>(width_) * height_ * channels_);

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            for (int c = 0; c < channels_; ++c) {
                const auto index = static_cast<std::size_t>((y * width_ + x) * channels_ + c);
                frame.data[index] = static_cast<std::uint8_t>((x + y + c * 64) % 256);
            }
        }
    }
    return frame;
}

}  // namespace astrall
