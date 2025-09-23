// config.cpp - Configuration Management Implementation

#include "config.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <sstream>

namespace vrb {

Config::Config(const std::string& filename)
    : m_configPath(filename)
    , m_lastModified()
    , m_hasChanges(false) {

    Load();

    // Start file watcher thread for hot-reload
    m_watching = true;
    m_watcherThread = std::thread([this] { WatchConfigFile(); });
}

Config::~Config() {
    m_watching = false;
    if (m_watcherThread.joinable()) {
        m_watcherThread.join();
    }
}

bool Config::Load() {
    try {
        if (!std::filesystem::exists(m_configPath)) {
            // Create default config if not exists
            CreateDefaultConfig();
            return Save();
        }

        std::ifstream file(m_configPath);
        if (!file.is_open()) {
            return false;
        }

        Json::CharReaderBuilder builder;
        std::string errors;
        if (!Json::parseFromStream(builder, file, &m_root, &errors)) {
            std::cerr << "Failed to parse config: " << errors << std::endl;
            return false;
        }

        m_lastModified = std::filesystem::last_write_time(m_configPath);
        m_hasChanges = false;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool Config::Reload() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return Load();
}

bool Config::Save() const {
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

bool Config::HasChanged() const {
    return m_hasChanges.load();
}

void Config::WatchConfigFile() {
    while (m_watching) {
        try {
            if (std::filesystem::exists(m_configPath)) {
                auto currentModified = std::filesystem::last_write_time(m_configPath);
                if (currentModified != m_lastModified) {
                    m_hasChanges = true;
                }
            }
        } catch (...) {
            // Ignore errors in watcher thread
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Config::CreateDefaultConfig() {
    // Create comprehensive default configuration
    m_root = Json::objectValue;

    // Audio settings
    m_root["audio"]["sampleRate"] = 48000;
    m_root["audio"]["bufferSize"] = 128;
    m_root["audio"]["preferredInputDevice"] = "";
    m_root["audio"]["virtualOutputName"] = "VR Binaural Output";
    m_root["audio"]["useASIO"] = false;
    m_root["audio"]["wasapiExclusive"] = false;
    m_root["audio"]["channels"]["input"] = "auto";
    m_root["audio"]["channels"]["output"] = 2;
    m_root["audio"]["priorityBoost"] = true;

    // HRTF settings
    m_root["hrtf"]["dataPath"] = "./hrtf_data";
    m_root["hrtf"]["dataset"] = "auto";
    m_root["hrtf"]["filterLength"] = 512;
    m_root["hrtf"]["convolutionMethod"] = "auto";
    m_root["hrtf"]["enableDistanceAttenuation"] = true;
    m_root["hrtf"]["maxDistance"] = 10.0;
    m_root["hrtf"]["referenceDistance"] = 1.0;
    m_root["hrtf"]["rolloffFactor"] = 1.0;
    m_root["hrtf"]["nearFieldCompensation"] = true;
    m_root["hrtf"]["minimumPhase"] = true;
    m_root["hrtf"]["fftSize"] = 1024;

    // VR settings
    m_root["vr"]["trackingRate"] = 90;
    m_root["vr"]["smoothingFactor"] = 0.95;
    m_root["vr"]["predictionTime"] = 0.011;
    m_root["vr"]["overlayScale"] = 0.5;
    m_root["vr"]["overlayPosition"]["x"] = 0.0;
    m_root["vr"]["overlayPosition"]["y"] = 1.5;
    m_root["vr"]["overlayPosition"]["z"] = -1.0;
    m_root["vr"]["microphonePosition"]["x"] = 0.0;
    m_root["vr"]["microphonePosition"]["y"] = 1.2;
    m_root["vr"]["microphonePosition"]["z"] = -1.0;
    m_root["vr"]["resetMicOnStart"] = false;
    m_root["vr"]["disableChaperone"] = false;

    // Performance settings
    m_root["performance"]["threadPriority"] = "realtime";
    m_root["performance"]["cpuAffinity"] = "0xFFF0";
    m_root["performance"]["simdLevel"] = "auto";
    m_root["performance"]["preallocateBuffers"] = true;
    m_root["performance"]["useMemoryPool"] = true;
    m_root["performance"]["ringBufferSize"] = 4096;
    m_root["performance"]["useHugePages"] = false;

    // Logging settings
    m_root["logging"]["level"] = "info";
    m_root["logging"]["path"] = "./logs";
    m_root["logging"]["async"] = true;
    m_root["logging"]["maxFileSize"] = "10MB";
    m_root["logging"]["maxFiles"] = 5;
    m_root["logging"]["console"] = true;
    m_root["logging"]["file"] = true;

    // Debug settings
    m_root["debug"]["enableMetrics"] = false;
    m_root["debug"]["metricsInterval"] = 1000;
    m_root["debug"]["profiling"] = false;
    m_root["debug"]["verboseAudio"] = false;
    m_root["debug"]["verboseVR"] = false;
    m_root["debug"]["testMode"] = false;

    // Network settings
    m_root["network"]["enableRemoteControl"] = false;
    m_root["network"]["controlPort"] = 8080;
    m_root["network"]["enableOSC"] = false;
    m_root["network"]["oscPort"] = 9000;
    m_root["network"]["oscAddress"] = "/vr/binaural";
    m_root["network"]["enableWebsocket"] = false;
    m_root["network"]["websocketPort"] = 8081;

    // UI settings
    m_root["ui"]["theme"] = "dark";
    m_root["ui"]["fontSize"] = 14;
    m_root["ui"]["showAdvanced"] = false;
    m_root["ui"]["autoHide"] = true;
    m_root["ui"]["autoHideDelay"] = 5000;
    m_root["ui"]["transparency"] = 0.95;

    // Experimental features
    m_root["experimental"]["enableGPUProcessing"] = false;
    m_root["experimental"]["enableAIEnhancement"] = false;
    m_root["experimental"]["enableRoomSimulation"] = false;
    m_root["experimental"]["roomSize"] = "medium";
    m_root["experimental"]["reverb"]["enabled"] = false;
    m_root["experimental"]["reverb"]["wetLevel"] = 0.1;
    m_root["experimental"]["reverb"]["roomSize"] = 0.5;
    m_root["experimental"]["reverb"]["damping"] = 0.5;
    m_root["experimental"]["enableHandTracking"] = false;

    // Automation settings
    m_root["automation"]["enableAutomation"] = false;
    m_root["automation"]["recordPath"] = "./automation";
    m_root["automation"]["recordFormat"] = "json";
    m_root["automation"]["playbackSpeed"] = 1.0;
    m_root["automation"]["loop"] = false;

    // Startup settings
    m_root["startup"]["autoStart"] = true;
    m_root["startup"]["minimized"] = false;
    m_root["startup"]["checkForUpdates"] = true;
    m_root["startup"]["loadLastConfig"] = true;

    // Fallback settings
    m_root["fallback"]["useDefaultDeviceOnError"] = true;
    m_root["fallback"]["reduceQualityOnOverload"] = true;
    m_root["fallback"]["disableEffectsOnOverload"] = false;
    m_root["fallback"]["emergencyBufferSize"] = 512;
}

// Template instantiations for Set method
template void Config::Set<int>(const std::string& path, const int& value);
template void Config::Set<float>(const std::string& path, const float& value);
template void Config::Set<bool>(const std::string& path, const bool& value);
template void Config::Set<std::string>(const std::string& path, const std::string& value);

template<typename T>
void Config::Set(const std::string& path, const T& value) {
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

} // namespace vrb