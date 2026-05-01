#include <cassert>
#include <stdexcept>

#include "astrall/backend/backend_factory.hpp"

int main() {
    astrall::BackendFactoryConfig sim_config;
    sim_config.kind = astrall::BackendKind::Sim;
    const auto backend = astrall::createBackend(sim_config);

    const astrall::BackendStatus status = backend->status();
    assert(status.initialized);
    assert(status.connected);
    assert(status.control_authority);
    assert(!status.error);
    assert(status.message == "sim");
    assert(!backend->latestImu().has_value());
    assert(!backend->latestWheelSpeeds().has_value());
    assert(backend->hasControlAuthority());

#if !ASTRALL_ENABLE_SDK
    astrall::BackendFactoryConfig real_config;
    real_config.kind = astrall::BackendKind::Real;
    bool threw = false;
    try {
        (void)astrall::createBackend(real_config);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    assert(threw);
#endif

    return 0;
}
