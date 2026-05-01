#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/string.hpp>

#include "astrall/backend/backend_factory.hpp"
#include "astrall/control/twist_command_mapper.hpp"

namespace {

using namespace std::chrono_literals;

astrall::Twist2D fromRosTwist(const geometry_msgs::msg::Twist& msg) {
    return astrall::Twist2D{msg.linear.x, msg.linear.y, msg.angular.z};
}

}  // namespace

class AstrallBaseNode final : public rclcpp::Node {
public:
    AstrallBaseNode()
        : Node("astrall_base_node") {
        loadParameters();
        configureBackend();
        configureRosInterfaces();
    }

private:
    double declareCompatDouble(const std::string& name,
                               const std::string& deprecated_name,
                               double fallback) {
        const double value = declare_parameter(name, fallback);
        const double deprecated_value = declare_parameter(deprecated_name, value);
        if (value == fallback && deprecated_value != value) {
            RCLCPP_WARN(
                get_logger(),
                "Parameter '%s' is deprecated; use '%s' instead.",
                deprecated_name.c_str(),
                name.c_str());
            return deprecated_value;
        }
        if (deprecated_value != value) {
            RCLCPP_WARN(
                get_logger(),
                "Ignoring deprecated parameter '%s' because '%s' is set.",
                deprecated_name.c_str(),
                name.c_str());
        }
        return value;
    }

    int declareCompatInt(const std::string& name,
                         const std::string& deprecated_name,
                         int fallback) {
        const int value = declare_parameter(name, fallback);
        const int deprecated_value = declare_parameter(deprecated_name, value);
        if (value == fallback && deprecated_value != value) {
            RCLCPP_WARN(
                get_logger(),
                "Parameter '%s' is deprecated; use '%s' instead.",
                deprecated_name.c_str(),
                name.c_str());
            return deprecated_value;
        }
        if (deprecated_value != value) {
            RCLCPP_WARN(
                get_logger(),
                "Ignoring deprecated parameter '%s' because '%s' is set.",
                deprecated_name.c_str(),
                name.c_str());
        }
        return value;
    }

    void loadParameters() {
        backend_config_.kind = astrall::BackendKind::Real;

        limits_.max_vx = declare_parameter("max_vx", 1.0);
        limits_.max_vy = declare_parameter("max_vy", 0.5);
        limits_.max_w = declareCompatDouble("max_w", "max_wz", 1.0);

        mapping_.scale_vx = declare_parameter("scale_vx", 1.0);
        mapping_.scale_vy = declare_parameter("scale_vy", 1.0);
        mapping_.scale_w = declareCompatDouble("scale_w", "scale_wz", 1.0);
        mapping_.sign_vx = declare_parameter("sign_vx", 1);
        mapping_.sign_vy = declare_parameter("sign_vy", 1);
        mapping_.sign_w = declareCompatInt("sign_w", "sign_wz", 1);

        cmd_vel_timeout_ms_ = declare_parameter("cmd_vel_timeout_ms", 500);
        backend_config_.real.sdk_ip = declare_parameter("sdk_ip", std::string("10.18.0.200"));
        backend_config_.real.robot_ip = declare_parameter("robot_ip", std::string("10.18.0.100"));
        robot_ip_ = backend_config_.real.robot_ip;
        control_mode_ = declare_parameter("control_mode", std::string("sdk"));
        publish_tf_ = declare_parameter("publish_tf", false);
        base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));
        imu_frame_id_ = declare_parameter("imu_frame_id", std::string("imu_link"));
        backend_config_.real.imu_frequency_hz = declare_parameter("imu_frequency_hz", 250);
        backend_config_.real.sport_frequency_hz = declare_parameter("sport_frequency_hz", 250);
        publish_period_ms_ = declare_parameter("publish_period_ms", 20);
        backend_config_.real.sdk_quaternion_order =
            declare_parameter("sdk_quaternion_order", std::string("xyzw"));
        backend_config_.real.request_control = sdkControlEnabled();

        RCLCPP_WARN(
            get_logger(),
            "sdk_ip and robot_ip document the expected network and diagnostics only; "
            "the current Astrall C API does not expose IP setters.");

        if (control_mode_ != "sdk" && control_mode_ != "monitor") {
            error_state_ = true;
            status_text_ = "invalid_control_mode:" + control_mode_;
            RCLCPP_ERROR(
                get_logger(),
                "Unsupported control_mode '%s'. Use 'sdk' or 'monitor'.",
                control_mode_.c_str());
        }

        if (publish_tf_) {
            RCLCPP_WARN(
                get_logger(),
                "publish_tf is set, but this minimal node does not publish localization TF. "
                "Use FAST-LIO/Nav2 localization output for odom->base_link and static TF for sensors.");
        }
    }

    void configureBackend() {
        if (error_state_) {
            return;
        }

        try {
            backend_ = astrall::createBackend(backend_config_);
        } catch (const std::exception& ex) {
            error_state_ = true;
            status_text_ = std::string("base_driver_backend_init_failed:") + ex.what();
            RCLCPP_ERROR(get_logger(), "Astrall base driver backend init failed: %s", ex.what());
            return;
        }

        if (!sdkControlEnabled()) {
            RCLCPP_WARN(
                get_logger(),
                "control_mode=monitor: /cmd_vel will be ignored and no backend motion commands will be sent.");
        }
        status_text_ = sdkControlEnabled() ? "ok" : "ok_monitor";
    }

    void configureRosInterfaces() {
        cmd_sub_ = create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel",
            rclcpp::QoS(10),
            [this](const geometry_msgs::msg::Twist::SharedPtr msg) {
                onCmdVel(*msg);
            });

        imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("/astrall/imu", rclcpp::SensorDataQoS());
        wheel_pub_ = create_publisher<std_msgs::msg::Float32MultiArray>("/astrall/wheel_speeds", 10);
        status_pub_ = create_publisher<std_msgs::msg::String>("/astrall/status", 10);
        diagnostics_pub_ = create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/diagnostics", 10);

        last_cmd_time_ = now();
        safety_timer_ = create_wall_timer(20ms, [this]() { safetyTick(); });
        publish_timer_ = create_wall_timer(
            std::chrono::milliseconds(std::max(1, publish_period_ms_)),
            [this]() { publishTick(); });
    }

    void onCmdVel(const geometry_msgs::msg::Twist& msg) {
        last_cmd_time_ = now();
        timed_out_stop_sent_ = false;

        if (error_state_ || !backend_) {
            return;
        }

        if (!sdkControlEnabled()) {
            status_text_ = "monitor_mode_cmd_vel_ignored";
            return;
        }

        const auto backend_status = backendStatusSnapshot();
        if (backend_status.has_value()) {
            if (backend_status->error) {
                handleBackendCommandFailure(
                    backend_status->message.empty() ? "backend_error" : backend_status->message,
                    backend_status);
                return;
            }
            if (!backend_status->control_authority) {
                status_text_ = "no_control_authority";
                return;
            }
        }

        const astrall::Twist2D cmd =
            astrall::mapRosTwistToSdkMove(fromRosTwist(msg), limits_, mapping_);
        try {
            backend_->sendVelocity(cmd);
        } catch (const std::exception& ex) {
            handleBackendCommandFailure(std::string("move_failed:") + ex.what(), backend_status);
        }
    }

    void safetyTick() {
        if (!backend_) {
            return;
        }

        const auto backend_status = backendStatusSnapshot();
        if (!backend_status.has_value()) {
            return;
        }

        if (sdkControlEnabled() && !backend_status->control_authority) {
            status_text_ = "no_control_authority";
            return;
        }

        if (backend_status->error) {
            error_state_ = true;
            status_text_ = backend_status->message.empty() ? "backend_error" : backend_status->message;
            stopBackendIfControlling(backend_status);
            return;
        }

        const auto elapsed_ms = (now() - last_cmd_time_).nanoseconds() / 1000000;
        if (sdkControlEnabled() && elapsed_ms > cmd_vel_timeout_ms_ && !timed_out_stop_sent_) {
            timed_out_stop_sent_ = true;
            status_text_ = "cmd_vel_timeout_stop";
            stopBackendIfControlling(backend_status);
        }
    }

    void publishTick() {
        publishImu();
        publishWheelSpeeds();
        const auto backend_status = backendStatusSnapshot();
        publishStatus(backend_status);
        publishDiagnostics(backend_status);
    }

    void publishImu() {
        if (!backend_) {
            return;
        }

        const auto imu = backend_->latestImu();
        if (!imu.has_value()) {
            return;
        }

        sensor_msgs::msg::Imu msg;
        msg.header.stamp = now();
        msg.header.frame_id = imu_frame_id_;
        msg.orientation.x = imu->quaternion_xyzw[0];
        msg.orientation.y = imu->quaternion_xyzw[1];
        msg.orientation.z = imu->quaternion_xyzw[2];
        msg.orientation.w = imu->quaternion_xyzw[3];
        msg.angular_velocity.x = imu->angular_velocity[0];
        msg.angular_velocity.y = imu->angular_velocity[1];
        msg.angular_velocity.z = imu->angular_velocity[2];
        msg.linear_acceleration.x = imu->linear_acceleration[0];
        msg.linear_acceleration.y = imu->linear_acceleration[1];
        msg.linear_acceleration.z = imu->linear_acceleration[2];
        imu_pub_->publish(msg);
    }

    void publishWheelSpeeds() {
        if (!backend_) {
            return;
        }

        const auto speeds = backend_->latestWheelSpeeds();
        if (!speeds.has_value()) {
            return;
        }

        std_msgs::msg::Float32MultiArray msg;
        msg.data.assign(speeds->values.begin(), speeds->values.end());
        wheel_pub_->publish(msg);
    }

    void publishStatus(const std::optional<astrall::BackendStatus>& backend_status) {
        std_msgs::msg::String msg;
        msg.data = status_text_;
        if (backend_status.has_value()) {
            if (!backend_status->message.empty()) {
                msg.data = backend_status->message;
                status_text_ = backend_status->message;
            }
        }
        status_pub_->publish(msg);
    }

    void publishDiagnostics(const std::optional<astrall::BackendStatus>& backend_status) {
        diagnostic_msgs::msg::DiagnosticArray array;
        array.header.stamp = now();

        diagnostic_msgs::msg::DiagnosticStatus status;
        status.name = "astrall_base_driver";
        status.hardware_id = robot_ip_;
        status.level = error_state_ ? diagnostic_msgs::msg::DiagnosticStatus::ERROR
                                    : diagnostic_msgs::msg::DiagnosticStatus::OK;
        status.message = status_text_;
        if (backend_status.has_value()) {
            status.level = (error_state_ || backend_status->error)
                               ? diagnostic_msgs::msg::DiagnosticStatus::ERROR
                               : diagnostic_msgs::msg::DiagnosticStatus::OK;
            status.message = backend_status->message.empty() ? status_text_ : backend_status->message;
        }
        array.status.push_back(status);
        diagnostics_pub_->publish(array);
    }

    std::optional<astrall::BackendStatus> backendStatusSnapshot() {
        if (!backend_) {
            return std::nullopt;
        }

        return backend_->status();
    }

    void handleBackendCommandFailure(
        const std::string& message,
        const std::optional<astrall::BackendStatus>& backend_status) {
        error_state_ = true;
        status_text_ = message;
        stopBackendIfControlling(backend_status);
        RCLCPP_ERROR(get_logger(), "Astrall base driver command failed: %s", status_text_.c_str());
    }

    void stopBackendIfControlling(const std::optional<astrall::BackendStatus>& backend_status) {
        if (!backend_ || !sdkControlEnabled()) {
            return;
        }
        if (!backend_status.has_value() || !backend_status->control_authority) {
            return;
        }

        try {
            backend_->stop();
        } catch (const std::exception& ex) {
            status_text_ = std::string("stop_failed:") + ex.what();
            RCLCPP_ERROR(get_logger(), "Astrall base driver stop failed: %s", ex.what());
        }
    }

    bool sdkControlEnabled() const {
        return control_mode_ == "sdk";
    }

    std::shared_ptr<astrall::Backend> backend_;
    astrall::BackendFactoryConfig backend_config_;
    astrall::VelocityLimits limits_;
    astrall::TwistMappingConfig mapping_;

    int cmd_vel_timeout_ms_ = 500;
    int publish_period_ms_ = 20;
    bool publish_tf_ = false;
    bool error_state_ = false;
    bool timed_out_stop_sent_ = false;
    std::string control_mode_ = "sdk";
    std::string base_frame_id_ = "base_link";
    std::string imu_frame_id_ = "imu_link";
    std::string robot_ip_ = "10.18.0.100";
    std::string status_text_ = "starting";
    rclcpp::Time last_cmd_time_;

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_sub_;
    rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr wheel_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diagnostics_pub_;
    rclcpp::TimerBase::SharedPtr safety_timer_;
    rclcpp::TimerBase::SharedPtr publish_timer_;
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<AstrallBaseNode>());
    rclcpp::shutdown();
    return 0;
}
