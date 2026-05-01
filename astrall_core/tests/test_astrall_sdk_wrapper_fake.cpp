#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "astrall/sdk/astrall_sdk_wrapper.hpp"

namespace {

std::mutex g_mutex;
std::vector<std::string> g_ops;
int g_active_calls = 0;
int g_max_active_calls = 0;
bool g_control_authority = true;

void resetFakeSdk() {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ops.clear();
    g_active_calls = 0;
    g_max_active_calls = 0;
    g_control_authority = true;
}

void record(const std::string& op) {
    std::lock_guard<std::mutex> lock(g_mutex);
    g_ops.push_back(op);
}

class ApiCallGuard {
public:
    explicit ApiCallGuard(std::string op)
        : op_(std::move(op)) {
        std::lock_guard<std::mutex> lock(g_mutex);
        ++g_active_calls;
        g_max_active_calls = std::max(g_max_active_calls, g_active_calls);
        g_ops.push_back(op_);
    }

    ~ApiCallGuard() {
        std::lock_guard<std::mutex> lock(g_mutex);
        --g_active_calls;
    }

private:
    std::string op_;
};

std::size_t findOp(const std::string& op) {
    const auto iter = std::find(g_ops.begin(), g_ops.end(), op);
    assert(iter != g_ops.end());
    return static_cast<std::size_t>(std::distance(g_ops.begin(), iter));
}

}  // namespace

uint16_t AstrallSdkInit(AstrallConfig& cfg, uint32_t) {
    ApiCallGuard guard("init");
    AstrallSdkStatus status;
    status.link = 1;
    status.ctrlAuthority = g_control_authority ? 1 : 0;
    if (cfg.sdkStatusCb) {
        cfg.sdkStatusCb(&status, sizeof(status));
    }
    return ASTRALL_RES_SUCCESSED;
}

void AstrallSdkDeinit() {
    record("deinit");
}

uint16_t AstrallGetSystemStatus(AstrallSystemStatus& status) {
    ApiCallGuard guard("system_status");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    status.sysStatus = ASTRALL_SYSTEM_CODE_RUNNING;
    status.errorCode = ASTRALL_ERR_NONE;
    status.warnCode = ASTRALL_WARN_NONE;
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallSubscriptionData(AstrallSubscribeTopicId topic,
                                 AstrallSubscribeFreq freq,
                                 std::function<void(void*, uint16_t)> cb,
                                 uint32_t) {
    const std::string prefix = topic == ASTRALL_SUB_TOPIC_ID_IMU          ? "sub_imu"
                               : topic == ASTRALL_SUB_TOPIC_ID_SPORT    ? "sub_sport"
                               : topic == ASTRALL_SUB_TOPIC_ID_JOYSTICK ? "sub_joystick"
                                                                          : "sub_other";
    ApiCallGuard guard(prefix + (freq == ASTRALL_SUB_FREQ_CLOSE ? "_close" : "_open"));
    if (freq == ASTRALL_SUB_FREQ_CLOSE || !cb) {
        return ASTRALL_RES_SUCCESSED;
    }

    if (topic == ASTRALL_SUB_TOPIC_ID_IMU) {
        AstrallImuData imu;
        imu.odomX = 1.25f;
        imu.odomY = -2.5f;
        imu.yaw = 0.75f;
        cb(&imu, sizeof(imu));
    } else if (topic == ASTRALL_SUB_TOPIC_ID_SPORT) {
        AstrallSportData sport;
        sport.wheelSpeed[0] = 3.0f;
        cb(&sport, sizeof(sport));
    } else if (topic == ASTRALL_SUB_TOPIC_ID_JOYSTICK) {
        AstrallJoystickData joystick{};
        cb(&joystick, sizeof(joystick));
    }
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallAuthControl(AstrallAuth auth, uint32_t) {
    ApiCallGuard guard(auth == ASTRALL_AUTH_SDK ? "auth_sdk" : "auth_joystick");
    g_control_authority = auth == ASTRALL_AUTH_SDK;
    return ASTRALL_RES_SUCCESSED;
}

uint16_t AstrallMove(float vx, float vy, float vyaw, uint32_t) {
    const bool zero = vx == 0.0f && vy == 0.0f && vyaw == 0.0f;
    ApiCallGuard guard(zero ? "move_zero" : "move");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    return ASTRALL_RES_SUCCESSED;
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
        astrall::AstrallSdkWrapper sdk;
        assert(sdk.init(astrall::AstrallSdkConfig{}));
        assert(sdk.requestControl());
        assert(sdk.subscribeImu(250));
        assert(sdk.subscribeSport(250));
        assert(sdk.subscribeJoystick(50));
        assert(sdk.latestImu().has_value());
        assert(sdk.latestSport().has_value());
        assert(sdk.latestJoystick().has_value());
        sdk.shutdown();
        sdk.shutdown();
    }

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        assert(std::count(g_ops.begin(), g_ops.end(), "deinit") == 1);
        assert(findOp("move_zero") < findOp("sub_imu_close"));
        assert(findOp("sub_imu_close") < findOp("sub_sport_close"));
        assert(findOp("sub_sport_close") < findOp("sub_joystick_close"));
        assert(findOp("sub_joystick_close") < findOp("auth_joystick"));
        assert(findOp("auth_joystick") < findOp("deinit"));
    }

    resetFakeSdk();
    astrall::AstrallSdkWrapper sdk;
    assert(sdk.init(astrall::AstrallSdkConfig{}));
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&sdk]() {
            assert(sdk.move(astrall::Twist2D{0.1, 0.0, 0.0}));
            assert(sdk.systemStatus().has_value());
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }
    sdk.shutdown();

    {
        std::lock_guard<std::mutex> lock(g_mutex);
        assert(g_max_active_calls == 1);
    }

    return 0;
}
