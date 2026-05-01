#include <cassert>
#include <memory>
#include <stdexcept>

#include "astrall/backend/sim_backend.hpp"
#include "astrall/control/controller.hpp"
#include "astrall/navigation/navigator.hpp"
#include "astrall/planning/planner.hpp"

namespace {

class ThrowingPlanner final : public astrall::Planner {
public:
    astrall::Path plan(const astrall::Pose2D&, const astrall::Point2D&) override {
        throw std::runtime_error("planning failed");
    }
};

}  // namespace

int main() {
    auto backend = std::make_shared<astrall::SimBackend>();
    auto controller = std::make_shared<astrall::Controller>(backend);
    auto planner = std::make_shared<ThrowingPlanner>();

    astrall::Navigator navigator(planner, controller, backend);
    navigator.setGoal(astrall::Point2D{1.0, 0.0});

    assert(navigator.status() == astrall::NavStatus::Failed);
    assert(navigator.update() == astrall::NavStatus::Failed);

    return 0;
}
