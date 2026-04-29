#ifndef ASTRALL_SDK_HPP
#define ASTRALL_SDK_HPP

#include "interface.h"

#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace astrall {

using milliseconds = std::chrono::milliseconds;

enum class Result : std::uint16_t {
    failed = ASTRALL_RES_FAILED,
    timeout = ASTRALL_RES_TIMEOUT,
    running = ASTRALL_RES_RUNNING,
    succeeded = ASTRALL_RES_SUCCESSED,
    invalid_param = ASTRALL_RES_INVALID_PARAM,
    rc_no_release = ASTRALL_RES_RC_NORELEASE,
    been_obtained = ASTRALL_RES_BEEN_OBTAINED,
    without_auth = ASTRALL_RES_WITHOUT_AUTH,
};

enum class SystemCode : std::uint8_t {
    init = ASTRALL_SYSTEM_CODE_INIT,
    standby = ASTRALL_SYSTEM_CODE_STANDBY,
    running = ASTRALL_SYSTEM_CODE_RUNNING,
    warning = ASTRALL_SYSTEM_CODE_WARNING,
    error = ASTRALL_SYSTEM_CODE_ERROR,
    shutdown = ASTRALL_SYSTEM_CODE_SHUTDOWN,
};

enum class SportMode : std::uint16_t {
    damping = ASTRALL_SPORT_MODE_CMD_DAMPING,
    fixed_stand = ASTRALL_SPORT_MODE_CMD_FIXEDSTAND,
    fixed_down = ASTRALL_SPORT_MODE_CMD_FIXEDDOWN,
    move = ASTRALL_SPORT_MODE_CMD_MOVE,
    auto_charge = ASTRALL_SPORT_MODE_CMD_AUTOCHARGE,
    exit_charge = ASTRALL_SPORT_MODE_CMD_EXITCHARGE,
    get_right = ASTRALL_SPORT_MODE_CMD_GET_RIGHT,
};

enum class SportStatus : std::uint16_t {
    unknown = ASTRALL_SPORT_STATUS_UNKNOWN,
    init = ASTRALL_SPORT_STATUS_INIT,
    damping = ASTRALL_SPORT_STATUS_DAMPING,
    fixed_stand = ASTRALL_SPORT_STATUS_FIXEDSTAND,
    fixed_down = ASTRALL_SPORT_STATUS_FIXEDDOWN,
    move = ASTRALL_SPORT_STATUS_MOVE,
    upstairs = ASTRALL_SPORT_STATUS_UPSTAIRS,
    car = ASTRALL_SPORT_STATUS_CAR,
    auto_charge = ASTRALL_SPORT_STATUS_AUTOCHAEGE,
    search_charging = ASTRALL_SPORT_STATUS_SEARCHCHARGING,
    plugging_charging = ASTRALL_SPORT_STATUS_PLUGGINGCHARGING,
    at_charging = ASTRALL_SPORT_STATUS_ATCHARGING,
    exit_charging = ASTRALL_SPORT_STATUS_EXITCHARGING,
    charging = ASTRALL_SPORT_STATUS_CHARGING,
    fixed_standing = ASTRALL_SPORT_STATUS_FIXEDSTANDING,
    fixed_downing = ASTRALL_SPORT_STATUS_FIXEDDOWNING,
    get_right = ASTRALL_SPORT_STATUS_GET_RIGHT,
};

enum class Auth : std::uint16_t {
    sdk = ASTRALL_AUTH_SDK,
    joystick = ASTRALL_AUTH_JOYSTICK,
};

enum class Light : std::uint16_t {
    open = ASTRALL_CMD_LIGHT_OPEN,
    close = ASTRALL_CMD_LIGHT_CLOSE,
};

enum class Topic : std::uint16_t {
    imu = ASTRALL_SUB_TOPIC_ID_IMU,
    sport = ASTRALL_SUB_TOPIC_ID_SPORT,
    camera_rgb = ASTRALL_SUB_TOPIC_ID_CAMERA_RGB,
    joystick = ASTRALL_SUB_TOPIC_ID_JOYSTICK,
};

enum class Frequency : std::uint16_t {
    close = ASTRALL_SUB_FREQ_CLOSE,
    hz_1 = ASTRALL_SUB_FREQ_1HZ,
    hz_25 = ASTRALL_SUB_FREQ_25HZ,
    hz_50 = ASTRALL_SUB_FREQ_50HZ,
    hz_125 = ASTRALL_SUB_FREQ_125HZ,
    hz_250 = ASTRALL_SUB_FREQ_250HZ,
};

using SdkStatus = AstrallSdkStatus;
using DeviceInfo = AstrallDeviceInfo;
using SystemStatus = AstrallSystemStatus;
using PowerStatus = AstrallPowerStatus;
using ImuData = AstrallImuData;
using SportData = AstrallSportData;
using JoystickData = AstrallJoystickData;

struct Velocity {
    float x = 0.0f;
    float y = 0.0f;
    float yaw = 0.0f;
};

struct Callbacks {
    std::function<void()> heartbeat;
    std::function<void(const SdkStatus&)> sdk_status;
};

[[nodiscard]] constexpr bool succeeded(Result result) noexcept
{
    return result == Result::succeeded;
}

[[nodiscard]] constexpr std::uint16_t to_underlying(Result result) noexcept
{
    return static_cast<std::uint16_t>(result);
}

namespace detail {

template <typename Enum>
[[nodiscard]] constexpr auto native_cast(Enum value) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(value);
}

[[nodiscard]] constexpr Result result_from(std::uint16_t value) noexcept
{
    return static_cast<Result>(value);
}

[[nodiscard]] inline std::uint32_t timeout_ms(milliseconds timeout)
{
    if (timeout.count() < 0 ||
        timeout.count() > static_cast<long long>(std::numeric_limits<std::uint32_t>::max())) {
        throw std::out_of_range("Astrall SDK timeout is outside uint32 millisecond range");
    }

    return static_cast<std::uint32_t>(timeout.count());
}

} // namespace detail

class Client {
public:
    Client() = default;

    explicit Client(const Callbacks& callbacks, milliseconds timeout = milliseconds{60000})
    {
        const auto result = init(callbacks, timeout);
        if (!succeeded(result)) {
            throw std::runtime_error("AstrallSdkInit failed");
        }
    }

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    ~Client()
    {
        deinit();
    }

    [[nodiscard]] Result init(const Callbacks& callbacks = {},
                              milliseconds timeout = milliseconds{60000})
    {
        deinit();

        callbacks_ = callbacks;
        native_config_.heartbeatCb = [this](void*, std::uint16_t) {
            if (callbacks_.heartbeat) {
                callbacks_.heartbeat();
            }
        };
        native_config_.sdkStatusCb = [this](void* data, std::uint16_t len) {
            if (!callbacks_.sdk_status || data == nullptr || len != sizeof(SdkStatus)) {
                return;
            }

            callbacks_.sdk_status(*static_cast<const SdkStatus*>(data));
        };

        const auto result = detail::result_from(
            AstrallSdkInit(native_config_, detail::timeout_ms(timeout)));
        initialized_ = succeeded(result);
        return result;
    }

    void deinit() noexcept
    {
        if (initialized_) {
            AstrallSdkDeinit();
            initialized_ = false;
        }
    }

    [[nodiscard]] bool initialized() const noexcept
    {
        return initialized_;
    }

    [[nodiscard]] Result heartbeat(milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(AstrallHeartbeat(detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result system_status(SystemStatus& status) const
    {
        return detail::result_from(AstrallGetSystemStatus(status));
    }

    [[nodiscard]] Result power_status(PowerStatus& power) const
    {
        return detail::result_from(AstrallGetPowerStatus(power));
    }

    [[nodiscard]] SportStatus sport_status() const
    {
        return static_cast<SportStatus>(AstrallGetSportStatus());
    }

    [[nodiscard]] Result device_info(DeviceInfo& info,
                                     milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(
            AstrallGetDeviceInfo(info, detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result imu_data(ImuData& data) const
    {
        return detail::result_from(AstrallGetImuData(data));
    }

    [[nodiscard]] Result set_sport_mode(SportMode mode,
                                        milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(AstrallSportModeControl(
            static_cast<AstrallSportModeCmd>(detail::native_cast(mode)),
            detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result set_auth(Auth auth,
                                  milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(AstrallAuthControl(
            static_cast<AstrallAuth>(detail::native_cast(auth)),
            detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result move(Velocity velocity,
                              milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(
            AstrallMove(velocity.x, velocity.y, velocity.yaw, detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result set_light(Light light,
                                   milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(AstrallLightControl(
            static_cast<AstrallLightCmd>(detail::native_cast(light)),
            detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result send_message(std::string_view message) const
    {
        if (message.size() > std::numeric_limits<std::uint16_t>::max()) {
            return Result::invalid_param;
        }

        std::string buffer{message};
        return detail::result_from(AstrallSendMessage(
            buffer.data(), static_cast<std::uint16_t>(buffer.size())));
    }

    [[nodiscard]] Result subscribe(Topic topic,
                                   Frequency frequency,
                                   std::function<void(void*, std::uint16_t)> callback,
                                   milliseconds timeout = milliseconds{20}) const
    {
        return detail::result_from(AstrallSubscriptionData(
            static_cast<AstrallSubscribeTopicId>(detail::native_cast(topic)),
            static_cast<AstrallSubscribeFreq>(detail::native_cast(frequency)),
            std::move(callback),
            detail::timeout_ms(timeout)));
    }

    [[nodiscard]] Result subscribe_imu(std::function<void(const ImuData&)> callback,
                                       Frequency frequency = Frequency::hz_250,
                                       milliseconds timeout = milliseconds{20}) const
    {
        return subscribe_typed<ImuData>(Topic::imu, frequency, std::move(callback), timeout);
    }

    [[nodiscard]] Result subscribe_sport(std::function<void(const SportData&)> callback,
                                         Frequency frequency = Frequency::hz_250,
                                         milliseconds timeout = milliseconds{20}) const
    {
        return subscribe_typed<SportData>(Topic::sport, frequency, std::move(callback), timeout);
    }

    [[nodiscard]] Result subscribe_joystick(std::function<void(const JoystickData&)> callback,
                                            Frequency frequency = Frequency::hz_50,
                                            milliseconds timeout = milliseconds{20}) const
    {
        return subscribe_typed<JoystickData>(
            Topic::joystick, frequency, std::move(callback), timeout);
    }

private:
    template <typename Data>
    [[nodiscard]] Result subscribe_typed(Topic topic,
                                         Frequency frequency,
                                         std::function<void(const Data&)> callback,
                                         milliseconds timeout) const
    {
        return subscribe(
            topic,
            frequency,
            [callback = std::move(callback)](void* data, std::uint16_t len) {
                if (!callback || data == nullptr || len != sizeof(Data)) {
                    return;
                }

                callback(*static_cast<const Data*>(data));
            },
            timeout);
    }

    bool initialized_ = false;
    Callbacks callbacks_;
    AstrallConfig native_config_;
};

} // namespace astrall

#endif // ASTRALL_SDK_HPP
