#include <iostream>

#include "astrall/runtime.hpp"

int main() {
    auto rt = astrall::Runtime::fromConfig("configs/robot.yaml");

    const auto frame = rt->camera()->getFrame();
    const auto cloud = rt->radar()->getPointCloud();

    std::cout << "image shape: (" << frame.height << ", "
              << frame.width << ", " << frame.channels << ")\n";
    std::cout << "cloud shape: (" << cloud.points.size() << ", 4)\n";

    auto sm = rt->stateMachine();
    sm->startMission({
        astrall::Point2D{1.0, 0.0},
        astrall::Point2D{2.0, 1.0},
    });

    for (int i = 0; i < 500 && sm->running(); ++i) {
        sm->update();
    }

    const auto pose = rt->backend()->getCurrentPose();
    std::cout << "final state: " << static_cast<int>(sm->state()) << '\n';
    std::cout << "final pose: x=" << pose.x
              << " y=" << pose.y
              << " theta=" << pose.theta << '\n';

    return 0;
}
