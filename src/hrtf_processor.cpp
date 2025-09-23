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
    // In production, this would load actual HRTF data
    m_initialized = true;

    // Reset spatial parameters
    m_currentAzimuth = 0.0f;
    m_currentElevation = 0.0f;
    m_currentDistance = 1.0f;
    m_currentFilterIndex = 0;

    LOG_INFO("HRTF processor initialized successfully");
    return true;
}

void HRTFProcessor::UpdateSpatialPosition(const VRPose& headPose, const VRPose& micPose) {
    if (!m_initialized) {
        return;
    }

    float azimuth, elevation, distance;
    CalculateAngles(headPose, micPose, azimuth, elevation, distance);

    m_currentAzimuth = azimuth;
    m_currentElevation = elevation;
    m_currentDistance = distance;

    LOG_DEBUG("Spatial position updated - Az: {:.1f}°, El: {:.1f}°, Dist: {:.2f}m",
              azimuth, elevation, distance);
}

void HRTFProcessor::Process(const float* input, float* output, size_t frames, int inputChannels) {
    if (!m_initialized || !input || !output || frames == 0) {
        if (output) {
            std::memset(output, 0, frames * 2 * sizeof(float));
        }
        return;
    }

    // Simple pass-through processing for now
    // In production, this would apply HRTF convolution
    if (inputChannels == 1) {
        // Mono to stereo with simple spatalization
        for (size_t i = 0; i < frames; ++i) {
            float sample = input[i];

            // Apply distance attenuation
            float distance = m_currentDistance.load();
            if (distance > 0.1f) {
                sample /= (distance * distance);
            }

            // Simple left/right panning based on azimuth
            float azimuth = m_currentAzimuth.load();
            float panFactor = std::sin(azimuth * M_PI / 180.0f);

            output[i * 2] = sample * (1.0f - panFactor * 0.5f);     // Left
            output[i * 2 + 1] = sample * (1.0f + panFactor * 0.5f); // Right
        }
    } else if (inputChannels == 2) {
        // Stereo pass-through with distance attenuation
        float distance = m_currentDistance.load();
        float attenuation = (distance > 0.1f) ? (1.0f / (distance * distance)) : 1.0f;

        for (size_t i = 0; i < frames * 2; ++i) {
            output[i] = input[i] * attenuation;
        }
    } else {
        // Unsupported channel count, output silence
        std::memset(output, 0, frames * 2 * sizeof(float));
    }
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

} // namespace vrb