// config.h - Configuration Management
// Handles application settings with hot-reload support

#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <json/json.h>

namespace vrb {

/**
 * @brief Configuration manager with JSON file support
 * 
 * Manages all application settings with automatic reload detection
 */
class Config {
public:
    explicit Config(const std::string& configPath);
    ~Config();

    /**
     * @brief Load configuration from file
     * @return true if configuration loaded successfully
     */
    bool Load();

    /**
     * @brief Reload configuration from file
     * @return true if reload successful
     */
    bool Reload();

    /**
     * @brief Check if configuration file has changed
     * @return true if file has been modified since last load
     */
    bool HasChanged() const;

    // Audio settings
    int GetSampleRate() const;
    int GetBufferSize() const;
    std::string GetPreferredInputDevice() const;
    std::string GetVirtualOutputName() const;

    // HRTF settings
    std::string GetHRTFDataPath() const;
    
    // Logging settings
    std::string GetLogLevel() const;
    std::string GetLogPath() const;

    // VR settings
    float GetOverlayScale() const;
    
private:
    bool ParseConfig();
    
    std::string m_configPath;
    Json::Value m_root;
    mutable std::mutex m_mutex;
    std::atomic<time_t> m_lastModified{0};
    
    // Cached values
    int m_sampleRate{48000};
    int m_bufferSize{128};
    std::string m_preferredInputDevice;
    std::string m_virtualOutputName{"VR Binaural Output"};
    std::string m_hrtfDataPath{"./hrtf_data"};
    std::string m_logLevel{"info"};
    std::string m_logPath{"./logs"};
    float m_overlayScale{0.5f};
};

// logger.h - Logging System

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace vrb {

/**
 * @brief Thread-safe logging system using spdlog
 */
class Logger {
public:
    /**
     * @brief Initialize logging system
     * @param level Log level (trace, debug, info, warn, error, critical)
     * @param logPath Directory for log files
     */
    static void Initialize(const std::string& level, const std::string& logPath);

    /**
     * @brief Get logger instance
     * @return Shared pointer to logger
     */
    static std::shared_ptr<spdlog::logger> Get();

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

// Convenience macros
#define LOG_TRACE(...)    if(vrb::Logger::Get()) vrb::Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    if(vrb::Logger::Get()) vrb::Logger::Get()->debug(__VA_ARGS__)
#define LOG_INFO(...)     if(vrb::Logger::Get()) vrb::Logger::Get()->info(__VA_ARGS__)
#define LOG_WARN(...)     if(vrb::Logger::Get()) vrb::Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    if(vrb::Logger::Get()) vrb::Logger::Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) if(vrb::Logger::Get()) vrb::Logger::Get()->critical(__VA_ARGS__)

} // namespace vrb

// vr_types.h - Common VR data types

#pragma once

namespace vrb {

/**
 * @brief 3D vector
 */
struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

/**
 * @brief Quaternion for orientation
 */
struct Quat {
    float x, y, z, w;
    
    Quat() : x(0), y(0), z(0), w(1) {}
    Quat(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

/**
 * @brief VR pose (position and orientation)
 */
struct VRPose {
    Vec3 position;
    Quat orientation;
};

} // namespace vrb

// ring_buffer.h - Lock-free ring buffer for audio

#pragma once

#include <atomic>
#include <vector>
#include <cstring>

namespace vrb {

/**
 * @brief Lock-free ring buffer for audio processing
 * 
 * Thread-safe single producer, single consumer ring buffer
 */
template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity)
        : m_buffer(capacity + 1)  // +1 to distinguish full from empty
        , m_capacity(capacity + 1)
        , m_writeIndex(0)
        , m_readIndex(0) {
    }

    /**
     * @brief Write data to buffer
     * @param data Input data
     * @param count Number of elements to write
     * @return Number of elements actually written
     */
    size_t Write(const T* data, size_t count) {
        size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
        size_t readIdx = m_readIndex.load(std::memory_order_acquire);
        
        size_t available = (m_capacity + readIdx - writeIdx - 1) % m_capacity;
        size_t toWrite = std::min(count, available);
        
        for (size_t i = 0; i < toWrite; ++i) {
            m_buffer[writeIdx] = data[i];
            writeIdx = (writeIdx + 1) % m_capacity;
        }
        
        m_writeIndex.store(writeIdx, std::memory_order_release);
        return toWrite;
    }

    /**
     * @brief Read data from buffer
     * @param data Output buffer
     * @param count Number of elements to read
     * @return Number of elements actually read
     */
    size_t Read(T* data, size_t count) {
        size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);
        
        size_t available = (m_capacity + writeIdx - readIdx) % m_capacity;
        size_t toRead = std::min(count, available);
        
        for (size_t i = 0; i < toRead; ++i) {
            data[i] = m_buffer[readIdx];
            readIdx = (readIdx + 1) % m_capacity;
        }
        
        m_readIndex.store(readIdx, std::memory_order_release);
        return toRead;
    }

    /**
     * @brief Get available space for writing
     * @return Number of elements that can be written
     */
    size_t GetWriteAvailable() const {
        size_t writeIdx = m_writeIndex.load(std::memory_order_relaxed);
        size_t readIdx = m_readIndex.load(std::memory_order_acquire);
        return (m_capacity + readIdx - writeIdx - 1) % m_capacity;
    }

    /**
     * @brief Get available data for reading
     * @return Number of elements that can be read
     */
    size_t GetReadAvailable() const {
        size_t readIdx = m_readIndex.load(std::memory_order_relaxed);
        size_t writeIdx = m_writeIndex.load(std::memory_order_acquire);
        return (m_capacity + writeIdx - readIdx) % m_capacity;
    }

    /**
     * @brief Reset buffer to empty state
     */
    void Reset() {
        m_writeIndex.store(0, std::memory_order_release);
        m_readIndex.store(0, std::memory_order_release);
    }

private:
    std::vector<T> m_buffer;
    size_t m_capacity;
    std::atomic<size_t> m_writeIndex;
    std::atomic<size_t> m_readIndex;
};

} // namespace vrb

// config.cpp - Implementation

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

// logger.cpp - Implementation

#include "logger.h"
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace vrb {

std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;

void Logger::Initialize(const std::string& level, const std::string& logPath) {
    // Create log directory if it doesn't exist
    std::filesystem::create_directories(logPath);
    
    // Generate log filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << logPath << "/vr_binaural_" 
       << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") 
       << ".log";
    
    // Create sinks
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(ss.str(), true));
    
    // Create logger
    s_logger = std::make_shared<spdlog::logger>("VRB", sinks.begin(), sinks.end());
    
    // Set level
    if (level == "trace") {
        s_logger->set_level(spdlog::level::trace);
    } else if (level == "debug") {
        s_logger->set_level(spdlog::level::debug);
    } else if (level == "info") {
        s_logger->set_level(spdlog::level::info);
    } else if (level == "warn") {
        s_logger->set_level(spdlog::level::warn);
    } else if (level == "error") {
        s_logger->set_level(spdlog::level::err);
    } else if (level == "critical") {
        s_logger->set_level(spdlog::level::critical);
    }
    
    // Set pattern
    s_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
    
    // Register as default logger
    spdlog::set_default_logger(s_logger);
    
    s_logger->info("Logger initialized - Level: {}, File: {}", level, ss.str());
}

std::shared_ptr<spdlog::logger> Logger::Get() {
    return s_logger;
}

} // namespace vrb