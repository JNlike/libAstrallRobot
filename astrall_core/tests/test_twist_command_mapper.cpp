#include <cassert>

#include "astrall/control/twist_command_mapper.hpp"

int main() {
    const astrall::VelocityLimits limits{1.0, 0.5, 0.75};
    const astrall::TwistMappingConfig mapping{
        2.0,
        3.0,
        4.0,
        1,
        -1,
        -1,
    };

    const astrall::Twist2D cmd{2.0, 1.0, -2.0};
    const astrall::Twist2D mapped = astrall::mapRosTwistToSdkMove(cmd, limits, mapping);

    assert(mapped.vx == 2.0);
    assert(mapped.vy == -1.5);
    assert(mapped.w == 3.0);

    const astrall::Twist2D negative = astrall::mapRosTwistToSdkMove(
        astrall::Twist2D{-2.0, -1.0, 2.0}, limits, mapping);
    assert(negative.vx == -2.0);
    assert(negative.vy == 1.5);
    assert(negative.w == -3.0);

    return 0;
}
