#include "astrall/sdk/astrall_sdk_wrapper.hpp"

#include <chrono>

namespace astrall {

namespace {

milliseconds timeoutFrom(std::uint32_t timeout_ms) {
    return milliseconds{timeout_ms};
}

}  // namespace

AstrallSdkWrapper::AstrallSdkWrapper() = default;
AstrallSdkWrapper::~AstrallSdkWrapper() = default;

bool AstrallSdkWrapper::init(const AstrallSdkConfig& config) {
    config_ = config;

    Callbacks callbacks;
    callbacks.sdk_status = [this](const AstrallSdkStatus& status) {
        std::lock_guard<std::mutex> lock(mutex_);
        latest_sdk_status_ = status;
    };

    auto client = std::make_unique<Client>();
    const Result result = client->init(callbacks, timeoutFrom(config_.init_timeout_ms));
    {
        std::lock_guard<std::mutex> lock(mutex_);
        last_result_ = result;
    }

    if (!succeeded(result)) {
        return false;
    }

    client_ = std::move(client);
    return true;
}

bool AstrallSdkWrapper::requestControl() {
    if (!client_) {
        return false;
    }
    return succeededAndRemember(client_->set_auth(Auth::sdk, timeoutFrom(config_.command_timeout_ms)));
}

bool AstrallSdkWrapper::releaseControl() {
    if (!client_) {
        return false;
    }
    return succeededAndRemember(client_->set_auth(Auth::joystick, timeoutFrom(config_.command_timeout_ms)));
}

bool AstrallSdkWrapper::move(double vx, double vy, double wz) {
    return move(Twist2D{vx, vy, wz});
}

bool AstrallSdkWrapper::move(const Twist2D& cmd) {
    if (!client_) {
        return false;
    }

    const auto sdk_status = latestSdkStatus();
    if (sdk_status.has_value() && sdk_status->ctrlAuthority == 0) {
        stop();
        return false;
    }

    return succeededAndRemember(client_->move(
        Velocity{
            static_cast<float>(cmd.vx),
            static_cast<float>(cmd.vy),
            static_cast<float>(cmd.w),
        },
        timeoutFrom(config_.command_timeout_ms)));
}

bool AstrallSdkWrapper::stop() {
    if (!client_) {
        return false;
    }

    return succeededAndRemember(client_->move(Velocity{}, timeoutFrom(config_.command_timeout_ms)));
}

std::optional<AstrallImuData> AstrallSdkWrapper::latestImu() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_imu_;
}

std::optional<AstrallSportData> AstrallSdkWrapper::latestSport() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_sport_;
}

std::optional<AstrallJoystickData> AstrallSdkWrapper::latestJoystick() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_joystick_;
}

std::optional<AstrallSdkStatus> AstrallSdkWrapper::latestSdkStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_sdk_status_;
}

std::optional<AstrallSystemStatus> AstrallSdkWrapper::systemStatus() const {
    if (!client_) {
        return std::nullopt;
    }

    AstrallSystemStatus status;
    const Result result = client_->system_status(status);
    const_cast<AstrallSdkWrapper*>(this)->succeededAndRemember(result);
    if (!succeeded(result)) {
        return std::nullopt;
    }
    return status;
}

bool AstrallSdkWrapper::subscribeImu(int freq_hz) {
    if (!client_) {
        return false;
    }

    return succeededAndRemember(client_->subscribe_imu(
        [this](const AstrallImuData& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            latest_imu_ = data;
        },
        frequencyFromHz(freq_hz),
        timeoutFrom(config_.command_timeout_ms)));
}

bool AstrallSdkWrapper::subscribeSport(int freq_hz) {
    if (!client_) {
        return false;
    }

    return succeededAndRemember(client_->subscribe_sport(
        [this](const AstrallSportData& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            latest_sport_ = data;
        },
        frequencyFromHz(freq_hz),
        timeoutFrom(config_.command_timeout_ms)));
}

bool AstrallSdkWrapper::subscribeJoystick(int freq_hz) {
    if (!client_) {
        return false;
    }

    return succeededAndRemember(client_->subscribe_joystick(
        [this](const AstrallJoystickData& data) {
            std::lock_guard<std::mutex> lock(mutex_);
            latest_joystick_ = data;
        },
        frequencyFromHz(freq_hz),
        timeoutFrom(config_.command_timeout_ms)));
}

Result AstrallSdkWrapper::lastResult() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_result_;
}

bool AstrallSdkWrapper::hasControlAuthority() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return latest_sdk_status_.has_value() && latest_sdk_status_->ctrlAuthority != 0;
}

Frequency AstrallSdkWrapper::frequencyFromHz(int freq_hz) {
    if (freq_hz >= 250) {
        return Frequency::hz_250;
    }
    if (freq_hz >= 125) {
        return Frequency::hz_125;
    }
    if (freq_hz >= 50) {
        return Frequency::hz_50;
    }
    if (freq_hz >= 25) {
        return Frequency::hz_25;
    }
    if (freq_hz >= 1) {
        return Frequency::hz_1;
    }
    return Frequency::close;
}

bool AstrallSdkWrapper::succeededAndRemember(Result result) {
    std::lock_guard<std::mutex> lock(mutex_);
    last_result_ = result;
    return succeeded(result);
}

}  // namespace astrall
