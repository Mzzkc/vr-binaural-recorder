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
#include <regex>

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
    explicit Config(const std::string& filename = "config/vr_binaural_config.json")
        : m_configPath(filename)
        , m_lastModified()
        , m_hasChanges(false) {

        Load();

        // Start file watcher thread for hot-reload
        m_watching = true;
        m_watcherThread = std::thread([this] { WatchConfigFile(); });
    }

    ~Config() {
        m_watching = false;
        if (m_watcherThread.joinable()) {
            m_watcherThread.join();
        }
    }

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
    bool Reload() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return Load();
    }

    /**
     * @brief Save current configuration to file
     * @return true if save successful
     */
    bool Save() const {
        std::lock_guard<std::mutex> lock(m_mutex);

        try {
            std::ofstream file(m_configPath);
            if (!file.is_open()) {
                return false;
            }

            Json::StreamWriterBuilder builder;
            builder["indentation"] = "  ";
            std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
            writer->write(m_root, &file);

            return true;
        } catch (...) {
            return false;
        }
    }

    /**
     * @brief Check if configuration has changed on disk
     * @return true if file has been modified
     */
    bool HasChanged() const {
        return m_hasChanges.load();
    }

    /**
     * @brief Set a configuration value
     */
    template<typename T>
    void Set(const std::string& path, const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Parse path
        std::vector<std::string> keys;
        std::string key;
        std::istringstream iss(path);
        while (std::getline(iss, key, '.')) {
            keys.push_back(key);
        }

        // Navigate to value location
        Json::Value* current = &m_root;
        for (size_t i = 0; i < keys.size() - 1; ++i) {
            if (!(*current)[keys[i]].isObject()) {
                (*current)[keys[i]] = Json::objectValue;
            }
            current = &(*current)[keys[i]];
        }

        // Set value
        (*current)[keys.back()] = value;
    }

private:
    bool Load() {
        try {
            if (!std::filesystem::exists(m_configPath)) {
                std::cout << "📄 Creating default config at: " << m_configPath << std::endl;
                CreateDefaultConfig();
                return Save();
            }

            std::ifstream file(m_configPath);
            if (!file.is_open()) {
                std::cerr << "❌ Cannot open config file: " << m_configPath << std::endl;
                return false;
            }

            // Read entire file to string for potential auto-repair
            std::string configContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();

            Json::CharReaderBuilder builder;
            std::string errors;
            std::istringstream configStream(configContent);

            if (!Json::parseFromStream(builder, configStream, &m_root, &errors)) {
                std::cerr << "🔧 JSON parse error detected: " << errors << std::endl;
                std::cout << "🛠️  Attempting auto-repair of common JSON issues..." << std::endl;

                // Alex Kim's clever auto-repair system!
                std::string repairedContent = AutoRepairJson(configContent);

                if (!repairedContent.empty()) {
                    std::istringstream repairedStream(repairedContent);
                    if (Json::parseFromStream(builder, repairedStream, &m_root, &errors)) {
                        std::cout << "✅ JSON auto-repair successful!" << std::endl;
                        std::cout << "💾 Saving repaired config..." << std::endl;
                        // Save the repaired version
                        BackupBrokenConfig();
                        Save();
                    } else {
                        std::cerr << "❌ Auto-repair failed: " << errors << std::endl;
                        return FallbackToDefaults();
                    }
                } else {
                    std::cerr << "❌ Auto-repair not possible, using defaults" << std::endl;
                    return FallbackToDefaults();
                }
            }

            m_lastModified = std::filesystem::last_write_time(m_configPath);
            m_hasChanges = false;
            std::cout << "✅ Config loaded successfully from: " << m_configPath << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "❌ Config loading exception: " << e.what() << std::endl;
            return FallbackToDefaults();
        }
    }

    void WatchConfigFile() {
        while (m_watching) {
            try {
                if (std::filesystem::exists(m_configPath)) {
                    auto currentModified = std::filesystem::last_write_time(m_configPath);
                    if (currentModified != m_lastModified) {
                        m_lastModified = currentModified;
                        m_hasChanges = true;
                    }
                }
            } catch (...) {
                // Ignore errors in watcher thread
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void CreateDefaultConfig() {
        // Alex Kim's comprehensive default config - ready for ASMRtists!
        m_root = Json::objectValue;

        // Audio settings - optimized for ASMR recording
        m_root["audio"]["sampleRate"] = 48000;
        m_root["audio"]["bufferSize"] = 128;
        m_root["audio"]["preferredInputDevice"] = "";
        m_root["audio"]["virtualOutputName"] = "VR Binaural Output";
        m_root["audio"]["useASIO"] = false;
        m_root["audio"]["wasapiExclusive"] = false;
        m_root["audio"]["channels"]["input"] = "auto";
        m_root["audio"]["channels"]["output"] = 2;
        m_root["audio"]["priorityBoost"] = true;

        // HRTF settings - spatial audio magic
        m_root["hrtf"]["dataPath"] = "./hrtf_data";
        m_root["hrtf"]["dataset"] = "auto";
        m_root["hrtf"]["filterLength"] = 512;
        m_root["hrtf"]["convolutionMethod"] = "auto";
        m_root["hrtf"]["enableDistanceAttenuation"] = true;
        m_root["hrtf"]["maxDistance"] = 10.0f;
        m_root["hrtf"]["referenceDistance"] = 1.0f;
        m_root["hrtf"]["rolloffFactor"] = 1.0f;
        m_root["hrtf"]["nearFieldCompensation"] = true;
        m_root["hrtf"]["minimumPhase"] = true;
        m_root["hrtf"]["fftSize"] = 1024;

        // VR settings - ASMRtist-friendly defaults
        m_root["vr"]["trackingRate"] = 90;
        m_root["vr"]["smoothingFactor"] = 0.95f;
        m_root["vr"]["predictionTime"] = 0.011f;
        m_root["vr"]["overlayScale"] = 0.5f;
        m_root["vr"]["overlayPosition"]["x"] = 0.0f;
        m_root["vr"]["overlayPosition"]["y"] = 1.5f;
        m_root["vr"]["overlayPosition"]["z"] = -1.0f;
        m_root["vr"]["microphonePosition"]["x"] = 0.0f;
        m_root["vr"]["microphonePosition"]["y"] = 1.2f;
        m_root["vr"]["microphonePosition"]["z"] = -1.0f;
        m_root["vr"]["resetMicOnStart"] = false;

        // Performance optimizations
        m_root["performance"]["threadPriority"] = "realtime";
        m_root["performance"]["simdLevel"] = "auto";
        m_root["performance"]["preallocateBuffers"] = true;
        m_root["performance"]["ringBufferSize"] = 4096;

        // Logging for debugging
        m_root["logging"]["level"] = "info";
        m_root["logging"]["path"] = "./logs";
        m_root["logging"]["console"] = true;
        m_root["logging"]["file"] = true;

        std::cout << "📝 Created comprehensive default configuration" << std::endl;
    }

    // Alex Kim's brilliant JSON auto-repair system!
    std::string AutoRepairJson(const std::string& brokenJson) {
        std::string repaired = brokenJson;
        bool hasRepairs = false;

        // Fix common JSON issues that break parsing

        // 1. Remove trailing commas - the classic killer!
        std::regex trailingCommaRegex(R"(,\s*([}\]]))", std::regex_constants::icase);
        if (std::regex_search(repaired, trailingCommaRegex)) {
            repaired = std::regex_replace(repaired, trailingCommaRegex, "$1");
            hasRepairs = true;
            std::cout << "🔧 Fixed trailing commas" << std::endl;
        }

        // 2. Fix single quotes to double quotes
        std::regex singleQuoteRegex(R"('([^']*)')", std::regex_constants::icase);
        if (std::regex_search(repaired, singleQuoteRegex)) {
            repaired = std::regex_replace(repaired, singleQuoteRegex, R"("$1")");
            hasRepairs = true;
            std::cout << "🔧 Fixed single quotes" << std::endl;
        }

        // 3. Add missing quotes around unquoted keys
        std::regex unquotedKeyRegex(R"((\n\s*)([a-zA-Z_][a-zA-Z0-9_]*)\s*:)", std::regex_constants::icase);
        if (std::regex_search(repaired, unquotedKeyRegex)) {
            repaired = std::regex_replace(repaired, unquotedKeyRegex, R"($1"$2":)");
            hasRepairs = true;
            std::cout << "🔧 Added quotes around keys" << std::endl;
        }

        // 4. Remove comments (// and /* */)
        std::regex singleLineCommentRegex(R"(//.*)", std::regex_constants::icase);
        std::regex multiLineCommentRegex(R"(/\*.*?\*/)", std::regex_constants::icase);
        if (std::regex_search(repaired, singleLineCommentRegex) || std::regex_search(repaired, multiLineCommentRegex)) {
            repaired = std::regex_replace(repaired, singleLineCommentRegex, "");
            repaired = std::regex_replace(repaired, multiLineCommentRegex, "");
            hasRepairs = true;
            std::cout << "🔧 Removed comments" << std::endl;
        }

        return hasRepairs ? repaired : "";
    }

    bool FallbackToDefaults() {
        std::cout << "🛡️  Activating fallback: using default configuration" << std::endl;
        CreateDefaultConfig();

        // Try to save defaults to repair the broken file
        try {
            if (Save()) {
                std::cout << "💾 Default config saved successfully" << std::endl;
            } else {
                std::cout << "⚠️  Could not save default config to file" << std::endl;
            }
        } catch (...) {
            std::cout << "⚠️  Exception while saving default config" << std::endl;
        }

        return true; // Always succeed with defaults
    }

    void BackupBrokenConfig() {
        try {
            std::string backupPath = m_configPath + ".broken." +
                std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());

            std::filesystem::copy_file(m_configPath, backupPath);
            std::cout << "💾 Backed up broken config to: " << backupPath << std::endl;
        } catch (...) {
            std::cout << "⚠️  Could not backup broken config file" << std::endl;
        }
    }

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