// hrtf_processor.cpp - Head-Related Transfer Function Processing Implementation

#include "hrtf_processor.h"
#include "logger.h"
#include <algorithm>
#include <cstring>
#include <cmath>

namespace vrb {

HRTFProcessor::HRTFProcessor()
    : m_initialized(false) {
    LOG_DEBUG("HRTFProcessor constructor");
}

HRTFProcessor::~HRTFProcessor() {
    LOG_DEBUG("HRTFProcessor destructor");
}

bool HRTFProcessor::Initialize(const std::string& hrtfDataPath) {
    LOG_INFO("Initializing HRTF processor with data path: {}", hrtfDataPath);

    // For testing purposes, create a simple synthetic HRTF
    // In production, this would load actual HRTF data from the specified path
    m_initialized = true;

    // Reset spatial parameters to defaults
    m_currentAzimuth = 0.0f;
    m_currentElevation = 0.0f;
    m_currentDistance = 1.0f;
    m_currentFilterIndex = 0;

    LOG_INFO("HRTF processor initialized successfully (stub implementation)");
    return true;
}

// NOTE: InitializeWithHeadset method removed - not declared in header
// Advanced headset-specific optimizations will be implemented in future iterations

void HRTFProcessor::UpdateSpatialPosition(const VRPose& hmdPose, const std::vector<VRPose>& controllerPoses) {
    if (!m_initialized) {
        return;
    }

    // Use the first valid controller as the primary microphone position
    // In a binaural recording setup, this represents the microphone location
    VRPose micPose;
    micPose.isValid = false;

    for (const auto& controller : controllerPoses) {
        if (controller.isValid) {
            micPose = controller;
            break;
        }
    }

    // If no controllers available, fall back to a default position in front of the user
    if (!micPose.isValid) {
        micPose.position = {0.0f, hmdPose.position.y - 0.2f, hmdPose.position.z - 0.3f}; // Chest level, in front
        micPose.orientation = hmdPose.orientation;
        micPose.isValid = true;
        LOG_DEBUG("No controllers detected - using default microphone position relative to HMD");
    }

    // Calculate angles between HMD and microphone position
    Vec3 relativePos = micPose.position - hmdPose.position;
    float distance = relativePos.length();

    // Simple azimuth calculation for spatial audio
    float azimuth = std::atan2(relativePos.x, relativePos.z) * 180.0f / M_PI;
    float elevation = std::atan2(relativePos.y, std::sqrt(relativePos.x*relativePos.x + relativePos.z*relativePos.z)) * 180.0f / M_PI;

    m_currentAzimuth = azimuth;
    m_currentElevation = elevation;
    m_currentDistance = distance;

    LOG_DEBUG("Updated spatial position - Az: {:.1f}°, El: {:.1f}°, Dist: {:.2f}m (from {} controllers)",
              azimuth, elevation, distance, controllerPoses.size());
}

void HRTFProcessor::Process(const float* input, float* output, size_t frames, int inputChannels) {
    if (!m_initialized || !input || !output || frames == 0) {
        if (output) {
            std::memset(output, 0, frames * 2 * sizeof(float));
        }
        return;
    }

    // Basic HRTF processing implementation (stub)
    if (inputChannels == 1) {
        // Mono to stereo with simple spatialization
        for (size_t i = 0; i < frames; ++i) {
            float sample = input[i];

            // Apply distance attenuation
            float distance = m_currentDistance.load();
            if (distance > 0.1f) {
                float attenuation = 1.0f / (distance * distance);
                sample *= attenuation;
            }

            // Simple left/right panning based on azimuth
            float azimuth = m_currentAzimuth.load();
            float panFactor = std::sin(azimuth * M_PI / 180.0f);

            float leftSample = sample * (1.0f - panFactor * 0.5f);
            float rightSample = sample * (1.0f + panFactor * 0.5f);

            output[i * 2] = leftSample;
            output[i * 2 + 1] = rightSample;
        }
    } else if (inputChannels == 2) {
        // Stereo processing with basic distance attenuation
        float distance = m_currentDistance.load();
        float attenuation = (distance > 0.1f) ? (1.0f / (distance * distance)) : 1.0f;

        for (size_t i = 0; i < frames * 2; ++i) {
            output[i] = input[i] * attenuation;
        }
    } else {
        // Unsupported channel count, output silence
        std::memset(output, 0, frames * 2 * sizeof(float));
    }

    // Basic processing complete (stub implementation)
}

void HRTFProcessor::Reset() {
    m_currentAzimuth = 0.0f;
    m_currentElevation = 0.0f;
    m_currentDistance = 1.0f;
    m_currentFilterIndex = 0;

    LOG_DEBUG("HRTF processor reset");
}

HRTFProcessor::ProcessingStats HRTFProcessor::GetStats() const {
    ProcessingStats stats;
    stats.azimuth = m_currentAzimuth.load();
    stats.elevation = m_currentElevation.load();
    stats.distance = m_currentDistance.load();
    stats.hrtfIndex = m_currentFilterIndex.load();
    return stats;
}

void HRTFProcessor::CalculateAngles(const VRPose& headPose, const VRPose& micPose,
                                   float& azimuth, float& elevation, float& distance) {
    // Calculate relative position vector from head to mic
    float dx = micPose.position.x - headPose.position.x;
    float dy = micPose.position.y - headPose.position.y;
    float dz = micPose.position.z - headPose.position.z;

    // Calculate distance
    distance = std::sqrt(dx*dx + dy*dy + dz*dz);

    // Calculate azimuth (horizontal angle)
    azimuth = std::atan2(dx, -dz) * 180.0f / M_PI;

    // Calculate elevation (vertical angle)
    float horizontalDistance = std::sqrt(dx*dx + dz*dz);
    elevation = std::atan2(dy, horizontalDistance) * 180.0f / M_PI;

    // Normalize angles
    while (azimuth < -180.0f) azimuth += 360.0f;
    while (azimuth > 180.0f) azimuth -= 360.0f;

    elevation = std::clamp(elevation, -90.0f, 90.0f);

    // Ensure minimum distance
    distance = std::max(distance, 0.1f);
}

// Stub implementations for ConvolutionEngine to fix linking errors
HRTFProcessor::ConvolutionEngine::ConvolutionEngine(int filterLength)
    : m_filterLength(filterLength), m_historyIndex(0), m_fftSize(filterLength * 2) {
    LOG_DEBUG("ConvolutionEngine constructor (stub) - filter length: {}", filterLength);
}

HRTFProcessor::ConvolutionEngine::~ConvolutionEngine() {
    LOG_DEBUG("ConvolutionEngine destructor (stub)");
}

void HRTFProcessor::ConvolutionEngine::Process(const float* input, float* outputLeft, float* outputRight,
                                              size_t frames, const HRTFData::Filter& filter) {
    // Stub implementation - just pass through with basic processing
    (void)filter; // Suppress unused parameter warning
    for (size_t i = 0; i < frames; ++i) {
        outputLeft[i] = input[i];
        outputRight[i] = input[i];
    }
}

void HRTFProcessor::ConvolutionEngine::Reset() {
    // Stub implementation
    m_historyIndex = 0;
}

// Stub implementations for InterpolationEngine
HRTFProcessor::InterpolationEngine::InterpolationEngine() {
    LOG_DEBUG("InterpolationEngine constructor (stub)");
}

void HRTFProcessor::InterpolationEngine::UpdateTarget(float azimuth, float elevation, float distance) {
    m_targetAzimuth = azimuth;
    m_targetElevation = elevation;
    m_targetDistance = distance;
}

void HRTFProcessor::InterpolationEngine::GetSmoothedValues(float& azimuth, float& elevation, float& distance) {
    // Simple smoothing
    m_currentAzimuth += (m_targetAzimuth - m_currentAzimuth) * (1.0f - SMOOTHING_FACTOR);
    m_currentElevation += (m_targetElevation - m_currentElevation) * (1.0f - SMOOTHING_FACTOR);
    m_currentDistance += (m_targetDistance - m_currentDistance) * (1.0f - SMOOTHING_FACTOR);

    azimuth = m_currentAzimuth;
    elevation = m_currentElevation;
    distance = m_currentDistance;
}

} // namespace vrb
