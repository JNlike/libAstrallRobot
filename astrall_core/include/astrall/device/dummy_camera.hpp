#pragma once

#include "astrall/device/camera.hpp"

namespace astrall {

class DummyCamera final : public Camera {
public:
    DummyCamera(int width = 640, int height = 480, int channels = 3);

    ImageFrame getFrame() override;

private:
    int width_ = 640;
    int height_ = 480;
    int channels_ = 3;
};

}  // namespace astrall
