// config.cpp - Configuration Management Implementation

#include "config.h"
#include "logger.h"
#include <fstream>
#include <filesystem>
#include <sys/stat.h>

namespace vrb {

Config::Config(const std::string& configPath) : m_configPath(configPath) {
    Load();
}

Config::~Config() {
    // Nothing to clean up
}

bool Config::Load() {
    std::lock_guard<std::mutex> lock(m_mutex);

    // Check if file exists
    if (!std::filesystem::exists(m_configPath)) {
        LOG_WARN("Config file not found: {}, using defaults", m_configPath);

        // Create default config file
        Json::Value defaultConfig;
        defaultConfig["audio"]["sampleRate"] = 48000;
        defaultConfig["audio"]["bufferSize"] = 128;
        defaultConfig["audio"]["preferredInputDevice"] = "";
        defaultConfig["audio"]["virtualOutputName"] = "VR Binaural Output";
        defaultConfig["hrtf"]["dataPath"] = "./hrtf_data";
        defaultConfig["logging"]["level"] = "info";
        defaultConfig["logging"]["path"] = "./logs";
        defaultConfig["vr"]["overlayScale"] = 0.5;

        std::ofstream file(m_configPath);
        Json::StreamWriterBuilder builder;
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(defaultConfig, &file);

        m_root = defaultConfig;
        ParseConfig();

        LOG_INFO("Created default config file: {}", m_configPath);
        return true;
    }

    // Load existing config
    std::ifstream file(m_configPath);
    Json::CharReaderBuilder builder;
    Json::String errors;

    if (!Json::parseFromStream(builder, file, &m_root, &errors)) {
        LOG_ERROR("Failed to parse config file: {}", errors);
        return false;
    }

    // Update last modified time
    struct stat fileStat;
    if (stat(m_configPath.c_str(), &fileStat) == 0) {
        m_lastModified = fileStat.st_mtime;
    }

    ParseConfig();
    LOG_INFO("Configuration loaded from: {}", m_configPath);
    return true;
}

bool Config::Reload() {
    LOG_INFO("Reloading configuration");
    return Load();
}

bool Config::HasChanged() const {
    struct stat fileStat;
    if (stat(m_configPath.c_str(), &fileStat) == 0) {
        return fileStat.st_mtime > m_lastModified;
    }
    return false;
}

bool Config::ParseConfig() {
    try {
        // Audio settings
        m_sampleRate = m_root["audio"]["sampleRate"].asInt();
        m_bufferSize = m_root["audio"]["bufferSize"].asInt();
        m_preferredInputDevice = m_root["audio"]["preferredInputDevice"].asString();
        m_virtualOutputName = m_root["audio"]["virtualOutputName"].asString();

        // HRTF settings
        m_hrtfDataPath = m_root["hrtf"]["dataPath"].asString();

        // Logging settings
        m_logLevel = m_root["logging"]["level"].asString();
        m_logPath = m_root["logging"]["path"].asString();

        // VR settings
        m_overlayScale = m_root["vr"]["overlayScale"].asFloat();

        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Error parsing config: {}", e.what());
        return false;
    }
}

// Getters
int Config::GetSampleRate() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sampleRate;
}

int Config::GetBufferSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bufferSize;
}

std::string Config::GetPreferredInputDevice() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_preferredInputDevice;
}

std::string Config::GetVirtualOutputName() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_virtualOutputName;
}

std::string Config::GetHRTFDataPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_hrtfDataPath;
}

std::string Config::GetLogLevel() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logLevel;
}

std::string Config::GetLogPath() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logPath;
}

float Config::GetOverlayScale() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_overlayScale;
}

} // namespace vrb