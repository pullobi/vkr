#include "Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>

std::mutex Logger::logMutex;
std::vector<LogEntry> Logger::logHistory;

void Logger::log(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(logMutex);

    std::ostream& output =
        (level == LogLevel::ERROR || level == LogLevel::FATAL)
            ? std::cerr
            : std::cout;

    // ANSI color escape code configuration
    std::string color = getColorCode(level);
    std::string reset = "\033[0m";

    output << color
           << "[" << getTimestamp() << "] "
           << "[" << levelToString(level) << "] "
           << message
           << reset
           << "\n";

    logHistory.push_back({
        .logLevel = level,
        .message = message
    });

    if (level == LogLevel::FATAL)
    {
        throw std::runtime_error(message);
    }
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeTime = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    std::tm timeInfo;
#if defined(_WIN32)
    localtime_s(&timeInfo, &timeTime);
#else
    localtime_r(&timeTime, &timeInfo);
#endif

    ss << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        default:              return "UNKNOWN";
    }
}

std::string Logger::getColorCode(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:  return "\033[90m";      // Gray
        case LogLevel::WARN:  return "\033[33m";      // Yellow
        case LogLevel::ERROR: return "\033[31m";      // Red
        case LogLevel::FATAL: return "\033[1;31;5m";  // Bold, Red, Blinking
        default:              return "\033[0m";       // Reset
    }
}
