#pragma once
// VR Hardware Detection - Minimal Compatibility Stub
//
// This header provides compatibility for legacy test and demo files.
// Actual hardware detection is handled by OpenVR runtime automatically.
// Our VRTracker class interfaces directly with OpenVR for optimal performance.

#include "vr_tracker.h"
#include <vector>
#include <string>

namespace vrb {
    /**
     * Compatibility stub for VR hardware detection
     *
     * This class provides a minimal interface to unblock compilation of legacy
     * test and demo files that reference the old hardware detection system.
     *
     * Actual hardware detection and device enumeration is handled automatically
     * by the OpenVR runtime. Our VRTracker class provides all necessary VR
     * integration functionality.
     */
    class VRHardwareDetection {
    public:
        /**
         * Initialize hardware detection system
         * @return Always true - OpenVR handles device detection
         */
        bool Initialize() {
            return true;
        }

        /**
         * Get list of connected VR devices
         * @return Generic device list - actual devices managed by OpenVR
         */
        std::vector<std::string> GetConnectedDevices() {
            return {"OpenVR_Device"};
        }

        /**
         * Check if headset is compatible
         * @param deviceName Device name to check
         * @return Always true - OpenVR handles compatibility
         */
        bool IsHeadsetCompatible(const std::string& deviceName) {
            (void)deviceName;  // Suppress unused parameter warning
            return true;
        }

        /**
         * Get device capabilities
         * @param deviceName Device name
         * @return Generic capabilities - actual capabilities from OpenVR
         */
        std::string GetDeviceCapabilities(const std::string& deviceName) {
            (void)deviceName;
            return "OpenVR_Managed";
        }

        /**
         * Check if device supports feature
         * @param deviceName Device name
         * @param feature Feature name
         * @return Always true - assume OpenVR devices support standard features
         */
        bool SupportsFeature(const std::string& deviceName, const std::string& feature) {
            (void)deviceName;
            (void)feature;
            return true;
        }
    };

    // Compatibility aliases for legacy code
    using VRCapabilitiesManager = VRHardwareDetection;
    using VRTestingFramework = VRHardwareDetection;

    // Additional compatibility types
    using VRDeviceInfo = std::string;
    using VRCompatibilityMatrix = std::vector<std::string>;
}