#pragma once
// VR Capabilities Manager - Compatibility Alias
//
// This header provides backwards compatibility for code that references
// the old VRCapabilitiesManager class. All functionality is now handled
// through the minimal VRHardwareDetection compatibility stub.

#include "vr_hardware_detection.h"

namespace vrb {
    // Compatibility alias - all capabilities managed by OpenVR
    using VRCapabilitiesManager = VRHardwareDetection;
}