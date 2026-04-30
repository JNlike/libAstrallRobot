#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <memory>
#include <string>

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <std_msgs/msg/float32_multi_array.hpp>
#include <std_msgs/msg/string.hpp>

#include "astrall/control/twist_command_mapper.hpp"
#include "astrall/sdk/astrall_sdk_wrapper.hpp"

namespace {

using namespace std::chrono_literals;

astrall::Twist2D fromRosTwist(const geometry_msgs::msg::Twist& msg) {
    return astrall::Twist2D{msg.linear.x, msg.linear.y, msg.angular.z};
}

std::string resultToString(astrall::Result result) {
    return std::to_string(astrall::to_underlying(result));
}

}  // namespace

class AstrallBaseNode final : public rclcpp::Node {
public:
    AstrallBaseNode()
        : Node("astrall_base_node") {
        loadParameters();
        configureSdk();
        configureRosInterfaces();
    }

private:
    void loadParameters() {
        limits_.max_vx = declare_parameter("max_vx", 1.0);
        limits_.max_vy = declare_parameter("max_vy", 0.5);
        limits_.max_wz = declare_parameter("max_wz", 1.0);

        mapping_.scale_vx = declare_parameter("scale_vx", 1.0);
        mapping_.scale_vy = declare_parameter("scale_vy", 1.0);
        mapping_.scale_wz = declare_parameter("scale_wz", 1.0);
        mapping_.sign_vx = declare_parameter("sign_vx", 1);
        mapping_.sign_vy = declare_parameter("sign_vy", 1);
        mapping_.sign_wz = declare_parameter("sign_wz", 1);

        cmd_vel_timeout_ms_ = declare_parameter("cmd_vel_timeout_ms", 500);
        sdk_config_.sdk_ip = declare_parameter("sdk_ip", std::string("10.18.0.200"));
        sdk_config_.robot_ip = declare_parameter("robot_ip", std::string("10.18.0.100"));
        control_mode_ = declare_parameter("control_mode", std::string("sdk"));
        publish_tf_ = declare_parameter("publish_tf", false);
        base_frame_id_ = declare_parameter("base_frame_id", std::string("base_link"));
        imu_frame_id_ = declare_parameter("imu_frame_id", std::string("imu_link"));
        imu_frequency_hz_ = declare_parameter("imu_frequency_hz", 250);
        sport_frequency_hz_ = declare_parameter("sport_frequency_hz", 250);
        publish_period_ms_ = declare_parameter("publish_period_ms", 20);
        sdk_quaternion_order_ = declare_parameter("sdk_quaternion_order", std::string("xyzw"));

        if (publish_tf_) {
            RCLCPP_WARN(
                get_logger(),
                "publish_tf is set, but this minimal node does not publish localization TF. "
                "Use FAST-LIO/Nav2 localization output for odom->base_link and static TF for sensors.");
        }
    }

    void configureSdk() {
        if (!sdk_.init(sdk_config_)) {
            error_state_ = true;
            status_text_ = "sdk_init_failed:" + resultToString(sdk_.lastResult());
            RCLCPP_ERROR(get_logger(), "Astrall SDK init failed: %s", status_text_.c_str());
            return;
        }

        if (control_mode_ == "sdk" && !sdk_.requestControl()) {
            error_state_ = true;
            status_text_ = "request_control_failed:" + resultToString(sdk_.lastResult());
            RCLCPP_ERROR(get_logger(), "Astrall SDK control request failed: %s", status_text_.c_str());
            sdk_.stop();
            return;
        }

        if (!sdk_.subscribeImu(imu_frequency_hz_)) {
            RCLCPP_WARN(get_logger(), "IMU subscription failed: %s", resultToString(sdk_.lastResult()).c_str());
        }
        if (!sdk_.subscribeSport(sport_frequency_hz_)) {
            RCLCPP_WARN(get_logger(), "SPORT subscription failed: %s", resultToString(sdk_.lastResult()).c_str());
        }

        status_text_ = "ok";
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

        if (error_state_) {
            sdk_.stop();
            return;
        }

        const astrall::Twist2D sdk_cmd =
            astrall::mapRosTwistToSdkMove(fromRosTwist(msg), limits_, mapping_);
        if (!sdk_.move(sdk_cmd)) {
            handleSdkCommandFailure("move_failed");
        }
    }

    void safetyTick() {
        const auto sdk_status = sdk_.latestSdkStatus();
        if (sdk_status.has_value() && sdk_status->ctrlAuthority == 0) {
            status_text_ = "no_control_authority";
            sdk_.stop();
            return;
        }

        const auto system_status = sdk_.systemStatus();
        if (system_status.has_value() &&
            (system_status->sysStatus == ASTRALL_SYSTEM_CODE_ERROR ||
             system_status->errorCode != ASTRALL_ERR_NONE)) {
            error_state_ = true;
            status_text_ = "robot_system_error";
            sdk_.stop();
            return;
        }

        const auto elapsed_ms = (now() - last_cmd_time_).nanoseconds() / 1000000;
        if (elapsed_ms > cmd_vel_timeout_ms_ && !timed_out_stop_sent_) {
            timed_out_stop_sent_ = true;
            status_text_ = "cmd_vel_timeout_stop";
            if (!sdk_.stop()) {
                handleSdkCommandFailure("timeout_stop_failed");
            }
        }
    }

    void publishTick() {
        publishImu();
        publishWheelSpeeds();
        publishStatus();
        publishDiagnostics();
    }

    void publishImu() {
        const auto imu = sdk_.latestImu();
        if (!imu.has_value()) {
            return;
        }

        sensor_msgs::msg::Imu msg;
        msg.header.stamp = now();
        msg.header.frame_id = imu_frame_id_;
        if (sdk_quaternion_order_ == "wxyz") {
            msg.orientation.w = imu->quaternion[0];
            msg.orientation.x = imu->quaternion[1];
            msg.orientation.y = imu->quaternion[2];
            msg.orientation.z = imu->quaternion[3];
        } else {
            msg.orientation.x = imu->quaternion[0];
            msg.orientation.y = imu->quaternion[1];
            msg.orientation.z = imu->quaternion[2];
            msg.orientation.w = imu->quaternion[3];
        }
        msg.angular_velocity.x = imu->gyroscope[0];
        msg.angular_velocity.y = imu->gyroscope[1];
        msg.angular_velocity.z = imu->gyroscope[2];
        msg.linear_acceleration.x = imu->accelerometer[0];
        msg.linear_acceleration.y = imu->accelerometer[1];
        msg.linear_acceleration.z = imu->accelerometer[2];
        imu_pub_->publish(msg);
    }

    void publishWheelSpeeds() {
        const auto sport = sdk_.latestSport();
        if (!sport.has_value()) {
            return;
        }

        std_msgs::msg::Float32MultiArray msg;
        msg.data.assign(std::begin(sport->wheelSpeed), std::end(sport->wheelSpeed));
        wheel_pub_->publish(msg);
    }

    void publishStatus() {
        std_msgs::msg::String msg;
        msg.data = status_text_;
        status_pub_->publish(msg);
    }

    void publishDiagnostics() {
        diagnostic_msgs::msg::DiagnosticArray array;
        array.header.stamp = now();

        diagnostic_msgs::msg::DiagnosticStatus status;
        status.name = "astrall_base_driver";
        status.hardware_id = sdk_config_.robot_ip;
        status.level = error_state_ ? diagnostic_msgs::msg::DiagnosticStatus::ERROR
                                    : diagnostic_msgs::msg::DiagnosticStatus::OK;
        status.message = status_text_;
        array.status.push_back(status);
        diagnostics_pub_->publish(array);
    }

    void handleSdkCommandFailure(const std::string& prefix) {
        const auto latest_status = sdk_.latestSdkStatus();
        if (latest_status.has_value() && latest_status->ctrlAuthority == 0) {
            status_text_ = "no_control_authority";
            sdk_.stop();
            return;
        }

        error_state_ = true;
        status_text_ = prefix + ":" + resultToString(sdk_.lastResult());
        sdk_.stop();
        RCLCPP_ERROR(get_logger(), "Astrall SDK command failed: %s", status_text_.c_str());
    }

    astrall::AstrallSdkWrapper sdk_;
    astrall::AstrallSdkConfig sdk_config_;
    astrall::VelocityLimits limits_;
    astrall::TwistMappingConfig mapping_;

    int cmd_vel_timeout_ms_ = 500;
    int imu_frequency_hz_ = 250;
    int sport_frequency_hz_ = 250;
    int publish_period_ms_ = 20;
    bool publish_tf_ = false;
    bool error_state_ = false;
    bool timed_out_stop_sent_ = false;
    std::string control_mode_ = "sdk";
    std::string base_frame_id_ = "base_link";
    std::string imu_frame_id_ = "imu_link";
    std::string sdk_quaternion_order_ = "xyzw";
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
