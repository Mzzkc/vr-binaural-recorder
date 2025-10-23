// vr_types.h - Common VR type definitions
#pragma once

#include <cmath>

namespace vrb {

/**
 * @brief 3D vector for spatial coordinates
 */
struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    // Vector operations
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }

    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }

    float length() const {
        return std::sqrt(x*x + y*y + z*z);
    }

    Vec3 normalized() const {
        float len = length();
        if (len > 0.0001f) {
            return Vec3(x/len, y/len, z/len);
        }
        return Vec3(0, 0, 0);
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return Vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

/**
 * @brief Quaternion for rotations
 */
struct Quat {
    float w, x, y, z;

    Quat() : w(1), x(0), y(0), z(0) {}
    Quat(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {}

    // Convert to Euler angles (in radians)
    Vec3 toEuler() const {
        Vec3 euler;

        // Roll (x-axis rotation)
        float sinr_cosp = 2 * (w * x + y * z);
        float cosr_cosp = 1 - 2 * (x * x + y * y);
        euler.x = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        float sinp = 2 * (w * y - z * x);
        if (std::abs(sinp) >= 1)
            euler.y = std::copysign(3.14159265f / 2, sinp);
        else
            euler.y = std::asin(sinp);

        // Yaw (z-axis rotation)
        float siny_cosp = 2 * (w * z + x * y);
        float cosy_cosp = 1 - 2 * (y * y + z * z);
        euler.z = std::atan2(siny_cosp, cosy_cosp);

        return euler;
    }

    // Normalize quaternion
    Quat normalized() const {
        float len = std::sqrt(w*w + x*x + y*y + z*z);
        if (len > 0.0001f) {
            return Quat(w/len, x/len, y/len, z/len);
        }
        return Quat(1, 0, 0, 0);
    }

    // Quaternion multiplication
    Quat operator*(const Quat& other) const {
        return Quat(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y + y * other.w + z * other.x - x * other.z,
            w * other.z + z * other.w + x * other.y - y * other.x
        );
    }

    // Rotate a vector
    Vec3 rotate(const Vec3& v) const {
        Quat p(0, v.x, v.y, v.z);
        Quat conj(w, -x, -y, -z);
        Quat result = *this * p * conj;
        return Vec3(result.x, result.y, result.z);
    }
};

/**
 * @brief VR pose containing position and orientation
 */
struct VRPose {
    Vec3 position;
    Quat orientation;
    Vec3 velocity;
    Vec3 angularVelocity;
    bool isValid;
    double timestamp;
    float trigger;

    VRPose() : isValid(false), timestamp(0), trigger(0.0f) {}

    // Get forward direction
    Vec3 forward() const {
        return orientation.rotate(Vec3(0, 0, -1));
    }

    // Get up direction
    Vec3 up() const {
        return orientation.rotate(Vec3(0, 1, 0));
    }

    // Get right direction
    Vec3 right() const {
        return orientation.rotate(Vec3(1, 0, 0));
    }

    // Transform a point from local to world space
    Vec3 transformPoint(const Vec3& localPoint) const {
        return position + orientation.rotate(localPoint);
    }

    // Transform a point from world to local space
    Vec3 inverseTransformPoint(const Vec3& worldPoint) const {
        Vec3 relative = worldPoint - position;
        Quat invRot(orientation.w, -orientation.x, -orientation.y, -orientation.z);
        return invRot.rotate(relative);
    }
};

/**
 * @brief Controller button states
 */
enum class VRButton {
    Trigger,
    Grip,
    Touchpad,
    Menu,
    System,
    A,
    B,
    X,
    Y
};

/**
 * @brief Controller input state
 */
struct VRControllerState {
    bool buttons[16];
    float trigger;
    float grip;
    Vec3 touchpadPos;
    Vec3 thumbstickPos;
    bool isConnected;
    int deviceIndex;

    VRControllerState() : trigger(0), grip(0), isConnected(false), deviceIndex(-1) {
        for (int i = 0; i < 16; ++i) buttons[i] = false;
    }
};

/**
 * @brief VR system capabilities
 */
struct VRCapabilities {
    bool hasPositionalTracking;
    bool hasRotationalTracking;
    bool hasHandTracking;
    bool hasEyeTracking;
    bool hasFaceTracking;
    bool hasHapticFeedback;
    int maxControllers;
    float playAreaWidth;
    float playAreaDepth;
    float trackingBoundsMin[3];
    float trackingBoundsMax[3];
};

} // namespace vrb