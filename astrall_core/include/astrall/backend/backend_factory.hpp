#pragma once

#include <memory>
#include <string>

#include "astrall/backend/backend.hpp"
#include "astrall/backend/real_backend.hpp"

namespace astrall {

enum class BackendKind {
    Sim,
    Real
};

struct BackendFactoryConfig {
    BackendKind kind = BackendKind::Sim;
    double sim_dt = 0.02;
    RealBackendConfig real;
};

std::shared_ptr<Backend> createBackend(const BackendFactoryConfig& config);
BackendKind backendKindFromString(const std::string& value);

}  // namespace astrall
