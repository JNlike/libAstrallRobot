#include <cassert>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>

#include "astrall/backend/real_backend.hpp"
#include "astrall_sdk.hpp"

namespace {

bool g_sdk_status_control_authority = true;
bool g_move_succeeds = true;
int g_system_status_calls = 0;
float g_odom_x = 1.5f;
float g_odom_y = -2.25f;
float g_yaw = 0.625f;

void resetFakeSdk() {
    g_sdk_status_control_authority = true;
    g_move_succeeds = true;
    g_system_status_calls = 0;
    g_odom_x = 1.5f;
    g_odom_y = -2.25f;
    g_yaw = 0.625f;
}

bool throwsRuntimeErrorForMove(astrall::RealBackend& backend) {
    try {
        backend.sendVelocity(astrall::Twist2D{0.2, 0.0, 0.0});
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

}  // namespace

uint16_t AstrallSdkInit(AstrallConfig& cfg, uint32_t) {
    AstrallSdkStatus status;
    status.link = 1;
    status.ctrlAuthority = g_sdk_status_control_authority ? 1 : 0;
    if (cfg.sdkStatusCb) {
        cfg.sdkStatusCb(&status, sizeof(status));
    }
    return ASTRALL_RES_SUCCESSED;
}

void AstrallSdkDeinit() {}

uint16_t AstrallGetSystemStatus(AstrallSystemStatus& status) {
    ++g_system_status_calls;
    status.sysStatus = ASTRALL_SYSTEM_CODE_RUNNING;
    status.errorCode = ASTRALL_ERR_NONE;
    status.warnCode = ASTRALL_WARN_NONE;
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallSubscriptionData(AstrallSubscribeTopicId topic,
                                 AstrallSubscribeFreq freq,
                                 std::function<void(void*, uint16_t)> cb,
                                 uint32_t) {
    if (freq == ASTRALL_SUB_FREQ_CLOSE || !cb) {
        return ASTRALL_RES_SUCCESSED;
    }

    if (topic == ASTRALL_SUB_TOPIC_ID_IMU) {
        AstrallImuData imu;
        imu.odomX = g_odom_x;
        imu.odomY = g_odom_y;
        imu.yaw = g_yaw;
        cb(&imu, sizeof(imu));
    } else if (topic == ASTRALL_SUB_TOPIC_ID_SPORT) {
        AstrallSportData sport;
        cb(&sport, sizeof(sport));
    }
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallAuthControl(AstrallAuth auth, uint32_t) {
    g_sdk_status_control_authority = auth == ASTRALL_AUTH_SDK;
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallMove(float vx, float vy, float vyaw, uint32_t) {
    const bool zero = vx == 0.0f && vy == 0.0f && vyaw == 0.0f;
    return (zero || g_move_succeeds) ? ASTRALL_RES_SUCCESSED : ASTRALL_RES_FAILED;
}

uint16_t AstrallHeartbeat(uint32_t) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallGetPowerStatus(AstrallPowerStatus&) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallGetSportStatus() {
    return ASTRALL_SPORT_STATUS_MOVE;
}

uint16_t AstrallGetDeviceInfo(AstrallDeviceInfo&, uint32_t) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallGetImuData(AstrallImuData&) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallSportModeControl(AstrallSportModeCmd, uint32_t) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallLightControl(AstrallLightCmd, uint32_t) {
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallSendMessage(char*, uint16_t) {
    return ASTRALL_RES_SUCCESSED;
}

int main() {
    resetFakeSdk();
    {
        astrall::RealBackendConfig config;
        astrall::RealBackend backend(config);
        const astrall::Pose2D pose = backend.getCurrentPose();
        assert(pose.x == static_cast<double>(g_odom_x));
        assert(pose.y == static_cast<double>(g_odom_y));
        assert(pose.theta == static_cast<double>(g_yaw));

        const auto first_status = backend.status();
        const auto second_status = backend.status();
        assert(!first_status.error);
        assert(!second_status.error);
        assert(g_system_status_calls == 1);

        std::this_thread::sleep_for(std::chrono::milliseconds(220));
        (void)backend.status();
        assert(g_system_status_calls == 2);
    }

    resetFakeSdk();
    {
        g_sdk_status_control_authority = false;
        astrall::RealBackendConfig config;
        config.request_control = false;
        astrall::RealBackend backend(config);
        assert(throwsRuntimeErrorForMove(backend));
    }

    resetFakeSdk();
    {
        g_move_succeeds = false;
        astrall::RealBackendConfig config;
        astrall::RealBackend backend(config);
        assert(throwsRuntimeErrorForMove(backend));
    }

    return 0;
}
