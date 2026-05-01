#include "astrall/backend/backend_factory.hpp"

#include <stdexcept>

#include "astrall/backend/sim_backend.hpp"

namespace astrall {

std::shared_ptr<Backend> createBackend(const BackendFactoryConfig& config) {
    switch (config.kind) {
    case BackendKind::Sim:
        return std::make_shared<SimBackend>(config.sim_dt);
    case BackendKind::Real:
        return std::make_shared<RealBackend>(config.real);
    }
    throw std::runtime_error("Unsupported backend kind");
}

BackendKind backendKindFromString(const std::string& value) {
    if (value == "sim") {
        return BackendKind::Sim;
    }
    if (value == "real") {
        return BackendKind::Real;
    }
    throw std::runtime_error("Unsupported backend type: " + value);
}

}  // namespace astrall
