// Minimal logger stub for testing
#pragma once
#include <iostream>

#define LOG_INFO(msg, ...) std::cout << "[INFO] " << msg << std::endl
#define LOG_WARN(msg, ...) std::cout << "[WARN] " << msg << std::endl
#define LOG_ERROR(msg, ...) std::cout << "[ERROR] " << msg << std::endl
#define LOG_DEBUG(msg, ...) std::cout << "[DEBUG] " << msg << std::endl