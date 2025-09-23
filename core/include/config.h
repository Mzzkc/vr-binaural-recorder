// config.h - Configuration management system
#pragma once

#include <string>
#include <filesystem>
#include <atomic>
#include <mutex>
#include <json/json.h>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>

namespace vrb {

/**
 * @brief Configuration manager with hot-reload support
 *
 * Manages all application settings from JSON configuration file
 * with automatic reload detection and validation.
 */
class Config {
public:
    /**
     * @brief Load configuration from JSON file
     * @param filename Path to configuration file
     */
    explicit Config(const std::string& filename = "config/vr_binaural_config.json");

    ~Config();

    // Audio configuration getters
    int GetSampleRate() const { return getInt("audio.sampleRate", 48000); }
    int GetBufferSize() const { return getInt("audio.bufferSize", 128); }
    std::string GetPreferredInputDevice() const { return getString("audio.preferredInputDevice", ""); }
    std::string GetVirtualOutputName() const { return getString("audio.virtualOutputName", "VR Binaural Output"); }
    bool GetUseASIO() const { return getBool("audio.useASIO", false); }
    bool GetWASAPIExclusive() const { return getBool("audio.wasapiExclusive", false); }
    int GetInputChannels() const {
        std::string ch = getString("audio.channels.input", "auto");
        return (ch == "auto") ? 0 : std::stoi(ch);
    }
    int GetOutputChannels() const { return getInt("audio.channels.output", 2); }
    bool GetPriorityBoost() const { return getBool("audio.priorityBoost", true); }

    // HRTF configuration getters
    std::string GetHRTFDataPath() const { return getString("hrtf.dataPath", "./hrtf_data"); }
    std::string GetHRTFDataset() const { return getString("hrtf.dataset", "auto"); }
    int GetHRTFFilterLength() const { return getInt("hrtf.filterLength", 512); }
    std::string GetConvolutionMethod() const { return getString("hrtf.convolutionMethod", "auto"); }
    bool GetEnableDistanceAttenuation() const { return getBool("hrtf.enableDistanceAttenuation", true); }
    float GetMaxDistance() const { return getFloat("hrtf.maxDistance", 10.0f); }
    float GetReferenceDistance() const { return getFloat("hrtf.referenceDistance", 1.0f); }
    float GetRolloffFactor() const { return getFloat("hrtf.rolloffFactor", 1.0f); }
    bool GetNearFieldCompensation() const { return getBool("hrtf.nearFieldCompensation", true); }
    bool GetMinimumPhase() const { return getBool("hrtf.minimumPhase", true); }
    int GetFFTSize() const { return getInt("hrtf.fftSize", 1024); }

    // VR configuration getters
    int GetTrackingRate() const { return getInt("vr.trackingRate", 90); }
    float GetSmoothingFactor() const { return getFloat("vr.smoothingFactor", 0.95f); }
    float GetPredictionTime() const { return getFloat("vr.predictionTime", 0.011f); }
    float GetOverlayScale() const { return getFloat("vr.overlayScale", 0.5f); }
    void GetOverlayPosition(float& x, float& y, float& z) const {
        x = getFloat("vr.overlayPosition.x", 0.0f);
        y = getFloat("vr.overlayPosition.y", 1.5f);
        z = getFloat("vr.overlayPosition.z", -1.0f);
    }
    void GetMicrophonePosition(float& x, float& y, float& z) const {
        x = getFloat("vr.microphonePosition.x", 0.0f);
        y = getFloat("vr.microphonePosition.y", 1.2f);
        z = getFloat("vr.microphonePosition.z", -1.0f);
    }
    bool GetResetMicOnStart() const { return getBool("vr.resetMicOnStart", false); }
    bool GetDisableChaperone() const { return getBool("vr.disableChaperone", false); }

    // Performance configuration getters
    std::string GetThreadPriority() const { return getString("performance.threadPriority", "realtime"); }
    std::string GetCPUAffinity() const { return getString("performance.cpuAffinity", "0xFFF0"); }
    std::string GetSIMDLevel() const { return getString("performance.simdLevel", "auto"); }
    bool GetPreallocateBuffers() const { return getBool("performance.preallocateBuffers", true); }
    bool GetUseMemoryPool() const { return getBool("performance.useMemoryPool", true); }
    int GetRingBufferSize() const { return getInt("performance.ringBufferSize", 4096); }
    bool GetUseHugePages() const { return getBool("performance.useHugePages", false); }

    // Logging configuration getters
    std::string GetLogLevel() const { return getString("logging.level", "info"); }
    std::string GetLogPath() const { return getString("logging.path", "./logs"); }
    bool GetAsyncLogging() const { return getBool("logging.async", true); }
    std::string GetLogMaxFileSize() const { return getString("logging.maxFileSize", "10MB"); }
    int GetLogMaxFiles() const { return getInt("logging.maxFiles", 5); }
    bool GetLogToConsole() const { return getBool("logging.console", true); }
    bool GetLogToFile() const { return getBool("logging.file", true); }

    // Debug configuration getters
    bool GetEnableMetrics() const { return getBool("debug.enableMetrics", false); }
    int GetMetricsInterval() const { return getInt("debug.metricsInterval", 1000); }
    bool GetProfiling() const { return getBool("debug.profiling", false); }
    bool GetVerboseAudio() const { return getBool("debug.verboseAudio", false); }
    bool GetVerboseVR() const { return getBool("debug.verboseVR", false); }
    bool GetTestMode() const { return getBool("debug.testMode", false); }

    // Network configuration getters
    bool GetEnableRemoteControl() const { return getBool("network.enableRemoteControl", false); }
    int GetControlPort() const { return getInt("network.controlPort", 8080); }
    bool GetEnableOSC() const { return getBool("network.enableOSC", false); }
    int GetOSCPort() const { return getInt("network.oscPort", 9000); }
    std::string GetOSCAddress() const { return getString("network.oscAddress", "/vr/binaural"); }
    bool GetEnableWebsocket() const { return getBool("network.enableWebsocket", false); }
    int GetWebsocketPort() const { return getInt("network.websocketPort", 8081); }

    // UI configuration getters
    std::string GetUITheme() const { return getString("ui.theme", "dark"); }
    int GetUIFontSize() const { return getInt("ui.fontSize", 14); }
    bool GetShowAdvanced() const { return getBool("ui.showAdvanced", false); }
    bool GetAutoHide() const { return getBool("ui.autoHide", true); }
    int GetAutoHideDelay() const { return getInt("ui.autoHideDelay", 5000); }
    float GetUITransparency() const { return getFloat("ui.transparency", 0.95f); }

    // Experimental features
    bool GetEnableGPUProcessing() const { return getBool("experimental.enableGPUProcessing", false); }
    bool GetEnableAIEnhancement() const { return getBool("experimental.enableAIEnhancement", false); }
    bool GetEnableRoomSimulation() const { return getBool("experimental.enableRoomSimulation", false); }
    std::string GetRoomSize() const { return getString("experimental.roomSize", "medium"); }
    bool GetEnableReverb() const { return getBool("experimental.reverb.enabled", false); }
    float GetReverbWetLevel() const { return getFloat("experimental.reverb.wetLevel", 0.1f); }
    float GetReverbRoomSize() const { return getFloat("experimental.reverb.roomSize", 0.5f); }
    float GetReverbDamping() const { return getFloat("experimental.reverb.damping", 0.5f); }
    bool GetEnableHandTracking() const { return getBool("experimental.enableHandTracking", false); }

    // Automation configuration
    bool GetEnableAutomation() const { return getBool("automation.enableAutomation", false); }
    std::string GetRecordPath() const { return getString("automation.recordPath", "./automation"); }
    std::string GetRecordFormat() const { return getString("automation.recordFormat", "json"); }
    float GetPlaybackSpeed() const { return getFloat("automation.playbackSpeed", 1.0f); }
    bool GetLoopPlayback() const { return getBool("automation.loop", false); }

    // Startup configuration
    bool GetAutoStart() const { return getBool("startup.autoStart", true); }
    bool GetMinimized() const { return getBool("startup.minimized", false); }
    bool GetCheckForUpdates() const { return getBool("startup.checkForUpdates", true); }
    bool GetLoadLastConfig() const { return getBool("startup.loadLastConfig", true); }

    // Fallback configuration
    bool GetUseDefaultDeviceOnError() const { return getBool("fallback.useDefaultDeviceOnError", true); }
    bool GetReduceQualityOnOverload() const { return getBool("fallback.reduceQualityOnOverload", true); }
    bool GetDisableEffectsOnOverload() const { return getBool("fallback.disableEffectsOnOverload", false); }
    int GetEmergencyBufferSize() const { return getInt("fallback.emergencyBufferSize", 512); }

    /**
     * @brief Reload configuration from file
     * @return true if reload successful
     */
    bool Reload();

    /**
     * @brief Save current configuration to file
     * @return true if save successful
     */
    bool Save() const;

    /**
     * @brief Check if configuration has changed on disk
     * @return true if file has been modified
     */
    bool HasChanged() const;

    /**
     * @brief Set a configuration value
     */
    template<typename T>
    void Set(const std::string& path, const T& value);

private:
    bool Load();
    void WatchConfigFile();
    void CreateDefaultConfig();

    // Helper methods for safe value access
    int getInt(const std::string& path, int defaultValue) const {
        const Json::Value* val = getValueByPath(path);
        return (val && val->isInt()) ? val->asInt() : defaultValue;
    }

    float getFloat(const std::string& path, float defaultValue) const {
        const Json::Value* val = getValueByPath(path);
        return (val && val->isNumeric()) ? val->asFloat() : defaultValue;
    }

    bool getBool(const std::string& path, bool defaultValue) const {
        const Json::Value* val = getValueByPath(path);
        return (val && val->isBool()) ? val->asBool() : defaultValue;
    }

    std::string getString(const std::string& path, const std::string& defaultValue) const {
        const Json::Value* val = getValueByPath(path);
        return (val && val->isString()) ? val->asString() : defaultValue;
    }

    const Json::Value* getValueByPath(const std::string& path) const {
        std::lock_guard<std::mutex> lock(m_mutex);

        std::vector<std::string> keys;
        std::string key;
        std::istringstream iss(path);
        while (std::getline(iss, key, '.')) {
            keys.push_back(key);
        }

        const Json::Value* current = &m_root;
        for (const auto& k : keys) {
            if (!current->isMember(k)) {
                return nullptr;
            }
            current = &(*current)[k];
        }

        return current;
    }

private:
    std::string m_configPath;
    Json::Value m_root;
    mutable std::mutex m_mutex;

    std::filesystem::file_time_type m_lastModified;
    std::atomic<bool> m_hasChanges;
    std::atomic<bool> m_watching;
    std::thread m_watcherThread;
};

} // namespace vrb