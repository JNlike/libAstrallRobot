#include "astrall/backend/real_backend.hpp"

#include <stdexcept>
#include <utility>

#if ASTRALL_ENABLE_SDK
#include "astrall/sdk/astrall_sdk_wrapper.hpp"
#endif

namespace astrall {

namespace {

#if ASTRALL_ENABLE_SDK
BackendImuData toCoreImuData(const AstrallImuData& data, const std::string& quaternion_order) {
    BackendImuData imu;
    imu.timestamp = data.timestamp;
    if (quaternion_order == "wxyz") {
        imu.quaternion_xyzw = {
            data.quaternion[1],
            data.quaternion[2],
            data.quaternion[3],
            data.quaternion[0],
        };
    } else {
        imu.quaternion_xyzw = {
            data.quaternion[0],
            data.quaternion[1],
            data.quaternion[2],
            data.quaternion[3],
        };
    }
    imu.angular_velocity = {data.gyroscope[0], data.gyroscope[1], data.gyroscope[2]};
    imu.linear_acceleration = {data.accelerometer[0], data.accelerometer[1], data.accelerometer[2]};
    imu.roll = data.roll;
    imu.pitch = data.pitch;
    imu.yaw = data.yaw;
    imu.odom_x = data.odomX;
    imu.odom_y = data.odomY;
    return imu;
}

WheelSpeeds toCoreWheelSpeeds(const AstrallSportData& data) {
    WheelSpeeds speeds;
    speeds.timestamp = data.timestamp;
    speeds.values = {data.wheelSpeed[0], data.wheelSpeed[1], data.wheelSpeed[2], data.wheelSpeed[3]};
    return speeds;
}
#endif

}  // namespace

RealBackend::RealBackend(const RealBackendConfig& config)
    : config_(config) {
    initializeSdk();
}

RealBackend::RealBackend(std::string port, int baudrate)
    : port_(std::move(port)), baudrate_(baudrate) {
    initializeSdk();
}

RealBackend::~RealBackend() = default;

void RealBackend::sendVelocity(const Twist2D& cmd) {
#if ASTRALL_ENABLE_SDK
    if (!sdk_ || !sdk_->move(cmd)) {
        throw std::runtime_error("Astrall SDK move failed");
    }
#else
    (void)cmd;
    throw std::runtime_error("RealBackend requires ASTRALL_ENABLE_SDK=ON");
#endif
}

Pose2D RealBackend::getCurrentPose() const {
    return pose_;
}

void RealBackend::stop() {
#if ASTRALL_ENABLE_SDK
    if (!sdk_ || !sdk_->stop()) {
        throw std::runtime_error("Astrall SDK stop failed");
    }
#else
    throw std::runtime_error("RealBackend requires ASTRALL_ENABLE_SDK=ON");
#endif
}

std::optional<BackendImuData> RealBackend::latestImu() const {
#if ASTRALL_ENABLE_SDK
    if (!sdk_) {
        return std::nullopt;
    }
    const auto imu = sdk_->latestImu();
    if (!imu.has_value()) {
        return std::nullopt;
    }
    return toCoreImuData(*imu, config_.sdk_quaternion_order);
#else
    return std::nullopt;
#endif
}

std::optional<WheelSpeeds> RealBackend::latestWheelSpeeds() const {
#if ASTRALL_ENABLE_SDK
    if (!sdk_) {
        return std::nullopt;
    }
    const auto sport = sdk_->latestSport();
    if (!sport.has_value()) {
        return std::nullopt;
    }
    return toCoreWheelSpeeds(*sport);
#else
    return std::nullopt;
#endif
}

BackendStatus RealBackend::status() const {
    BackendStatus backend_status;
    backend_status.initialized = initialized_;
#if ASTRALL_ENABLE_SDK
    if (!sdk_) {
        backend_status.error = true;
        backend_status.message = "sdk_not_initialized";
        return backend_status;
    }

    backend_status.connected = initialized_;
    backend_status.control_authority = config_.request_control;

    const auto sdk_status = sdk_->latestSdkStatus();
    if (sdk_status.has_value()) {
        backend_status.connected = sdk_status->link != 0;
        backend_status.control_authority = sdk_status->ctrlAuthority != 0;
    }

    const auto system_status = sdk_->systemStatus();
    if (system_status.has_value()) {
        backend_status.system_code = static_cast<std::uint32_t>(system_status->sysStatus);
        backend_status.error_code = static_cast<std::uint32_t>(system_status->errorCode);
        backend_status.warning_code = static_cast<std::uint32_t>(system_status->warnCode);
        backend_status.error =
            system_status->sysStatus == ASTRALL_SYSTEM_CODE_ERROR ||
            system_status->errorCode != ASTRALL_ERR_NONE;
    }

    if (backend_status.error) {
        backend_status.message = "robot_system_error";
    } else if (config_.request_control && !backend_status.control_authority) {
        backend_status.message = "no_control_authority";
    } else {
        backend_status.message = "ok";
    }
#else
    backend_status.error = true;
    backend_status.message = "RealBackend requires ASTRALL_ENABLE_SDK=ON";
#endif
    return backend_status;
}

bool RealBackend::hasControlAuthority() const {
#if ASTRALL_ENABLE_SDK
    return sdk_ && sdk_->hasControlAuthority();
#else
    return false;
#endif
}

const RealBackendConfig& RealBackend::config() const {
    return config_;
}

const std::string& RealBackend::port() const {
    return port_;
}

int RealBackend::baudrate() const {
    return baudrate_;
}

void RealBackend::initializeSdk() {
#if ASTRALL_ENABLE_SDK
    AstrallSdkConfig sdk_config;
    sdk_config.sdk_ip = config_.sdk_ip;
    sdk_config.robot_ip = config_.robot_ip;
    sdk_config.init_timeout_ms = config_.init_timeout_ms;
    sdk_config.command_timeout_ms = config_.command_timeout_ms;

    sdk_ = std::make_unique<AstrallSdkWrapper>();
    if (!sdk_->init(sdk_config)) {
        throw std::runtime_error("Astrall SDK init failed in RealBackend");
    }
    if (config_.request_control && !sdk_->requestControl()) {
        throw std::runtime_error("Astrall SDK control request failed in RealBackend");
    }
    if (!sdk_->subscribeImu(config_.imu_frequency_hz)) {
        throw std::runtime_error("Astrall SDK IMU subscription failed in RealBackend");
    }
    if (!sdk_->subscribeSport(config_.sport_frequency_hz)) {
        throw std::runtime_error("Astrall SDK SPORT subscription failed in RealBackend");
    }
    initialized_ = true;
#else
    throw std::runtime_error("RealBackend requires ASTRALL_ENABLE_SDK=ON");
#endif
}

}  // namespace astrall
