// logger.h - Logging system wrapper for spdlog
#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/async.h>
#include <memory>
#include <string>
#include <mutex>
#include <filesystem>
#include <iostream>

namespace vrb {

/**
 * @brief Global logging system using spdlog
 */
class Logger {
public:
    /**
     * @brief Initialize the logging system
     * @param level Log level (trace, debug, info, warn, error, critical)
     * @param logPath Directory for log files
     * @param async Use async logging for better performance
     */
    static void Initialize(const std::string& level = "info",
                          const std::string& logPath = "./logs",
                          bool async = true) {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (s_initialized) {
            return;
        }

        try {
            // Create log directory if it doesn't exist
            std::filesystem::create_directories(logPath);

            std::vector<spdlog::sink_ptr> sinks;

            // Console sink with colors
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
            sinks.push_back(console_sink);

            // Rotating file sink (10MB max, 5 files)
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logPath + "/vr_binaural.log", 1024 * 1024 * 10, 5);
            file_sink->set_level(spdlog::level::trace);
            file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%@] %v");
            sinks.push_back(file_sink);

            // Create logger
            if (async) {
                spdlog::init_thread_pool(8192, 1);
                s_logger = std::make_shared<spdlog::async_logger>(
                    "vrb", sinks.begin(), sinks.end(),
                    spdlog::thread_pool(), spdlog::async_overflow_policy::block);
            } else {
                s_logger = std::make_shared<spdlog::logger>("vrb", sinks.begin(), sinks.end());
            }

            // Set log level
            s_logger->set_level(StringToLevel(level));

            // Register as default logger
            spdlog::set_default_logger(s_logger);
            spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

            s_initialized = true;
            s_logger->info("Logger initialized - Level: {}, Path: {}", level, logPath);

        } catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log initialization failed: " << ex.what() << std::endl;
        }
    }

    /**
     * @brief Shutdown the logging system
     */
    static void Shutdown() {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (s_logger) {
            s_logger->flush();
            spdlog::drop_all();
            spdlog::shutdown();
            s_logger.reset();
            s_initialized = false;
        }
    }

    /**
     * @brief Get the logger instance
     */
    static std::shared_ptr<spdlog::logger> Get() {
        if (!s_initialized) {
            Initialize();
        }
        return s_logger;
    }

    /**
     * @brief Set log level at runtime
     */
    static void SetLevel(const std::string& level) {
        if (s_logger) {
            s_logger->set_level(StringToLevel(level));
        }
    }

    /**
     * @brief Flush all log buffers
     */
    static void Flush() {
        if (s_logger) {
            s_logger->flush();
        }
    }

private:
    static spdlog::level::level_enum StringToLevel(const std::string& level) {
        if (level == "trace") return spdlog::level::trace;
        if (level == "debug") return spdlog::level::debug;
        if (level == "info") return spdlog::level::info;
        if (level == "warn" || level == "warning") return spdlog::level::warn;
        if (level == "error") return spdlog::level::err;
        if (level == "critical") return spdlog::level::critical;
        return spdlog::level::info;
    }

    static std::shared_ptr<spdlog::logger> s_logger;
    static bool s_initialized;
    static std::mutex s_mutex;
};

// Static member definitions
inline std::shared_ptr<spdlog::logger> Logger::s_logger = nullptr;
inline bool Logger::s_initialized = false;
inline std::mutex Logger::s_mutex;

} // namespace vrb

// Convenience macros for logging
#define LOG_TRACE(...) if (vrb::Logger::Get()) vrb::Logger::Get()->trace(__VA_ARGS__)
#define LOG_DEBUG(...) if (vrb::Logger::Get()) vrb::Logger::Get()->debug(__VA_ARGS__)
#define LOG_INFO(...) if (vrb::Logger::Get()) vrb::Logger::Get()->info(__VA_ARGS__)
#define LOG_WARN(...) if (vrb::Logger::Get()) vrb::Logger::Get()->warn(__VA_ARGS__)
#define LOG_ERROR(...) if (vrb::Logger::Get()) vrb::Logger::Get()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) if (vrb::Logger::Get()) vrb::Logger::Get()->critical(__VA_ARGS__)