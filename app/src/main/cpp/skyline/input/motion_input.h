// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "common.h"
#include "common/quaternion.h"
#include "common/vector_math.h"

namespace skyline::input {

class MotionInput {
public:
    explicit MotionInput();

    MotionInput(const MotionInput&) = default;
    MotionInput& operator=(const MotionInput&) = default;

    MotionInput(MotionInput&&) = default;
    MotionInput& operator=(MotionInput&&) = default;

    void SetPID(float new_kp, float new_ki, float new_kd);
    void SetAcceleration(const Common::Vec3f& acceleration);
    void SetGyroscope(const Common::Vec3f& gyroscope);
    void SetQuaternion(const Common::Quaternion<float>& quaternion);
    void SetGyroBias(const Common::Vec3f& bias);
    void SetGyroThreshold(float threshold);

    void EnableReset(bool reset);
    void ResetRotations();

    void UpdateRotation(u64 elapsed_time);
    void UpdateOrientation(u64 elapsed_time);

    [[nodiscard]] std::array<Common::Vec3f, 3> GetOrientation() const;
    [[nodiscard]] Common::Vec3f GetAcceleration() const;
    [[nodiscard]] Common::Vec3f GetGyroscope() const;
    [[nodiscard]] Common::Vec3f GetGyroBias() const;
    [[nodiscard]] Common::Vec3f GetRotations() const;
    [[nodiscard]] Common::Quaternion<float> GetQuaternion() const;

    [[nodiscard]] bool IsMoving(float sensitivity) const;
    [[nodiscard]] bool IsCalibrated(float sensitivity) const;

private:
    void ResetOrientation();
    void SetOrientationFromAccelerometer();

    // PID constants
    float kp;
    float ki;
    float kd;

    // PID errors
    Common::Vec3f real_error;
    Common::Vec3f integral_error;
    Common::Vec3f derivative_error;

    // Quaternion containing the device orientation
    Common::Quaternion<float> quat{{0.0f, 0.0f, -1.0f}, 0.0f};

    // Number of full rotations in each axis
    Common::Vec3f rotations;

    // Acceleration vector measurement in G force
    Common::Vec3f accel;

    // Gyroscope vector measurement in radians/s.
    Common::Vec3f gyro;

    // Vector to be substracted from gyro measurements
    Common::Vec3f gyro_bias;

    // Minimum gyro amplitude to detect if the device is moving
    float gyro_threshold = 0.0f;

    // Number of invalid sequential data
    u32 reset_counter = 0;

    // If the provided data is invalid the device will be autocalibrated
    bool reset_enabled = true;

    // Use accelerometer values to calculate position
    bool only_accelerometer = true;
};

} // namespace Core::HID
