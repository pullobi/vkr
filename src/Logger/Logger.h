#pragma once

#include <string>
#include <mutex>
#include <format>
#include <string>
#include <tuple>
#include <vector>
#include <string>
enum class LogLevel {
    INFO,
    WARN,
    ERROR,
    FATAL
};

struct LogEntry{
    LogLevel logLevel; 
    std::string message;
};

class Logger {
public:
    Logger() = default;
    ~Logger() = default;

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::INFO, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::WARN, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::ERROR, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void fatal(std::format_string<Args...> fmt, Args&&... args) {
        log(LogLevel::FATAL, std::format(fmt, std::forward<Args>(args)...));
    }

    

    // Helper method used specifically by the assert macro
    template <typename... Args>
    static void handle_assert_fail(const char* expr, const char* file, int line, std::format_string<Args...> fmt, Args&&... args) {
        std::string custom_msg = std::format(fmt, std::forward<Args>(args)...);
        std::string final_msg = std::format("Assertion '{}' failed at {}:{}. Context: {}", 
                                            expr, file, line, custom_msg);
        
        // Log it as a FATAL error (which automatically throws std::runtime_error)
        Logger().fatal("{}", final_msg);
    }
    const std::vector<LogEntry>& GetLogHistory() const
    {
        return logHistory;
    } 
private:
    void log(LogLevel level, const std::string& message);
    
    static std::vector<LogEntry> logHistory;
    std::string getTimestamp();
    std::string levelToString(LogLevel level);
    std::string getColorCode(LogLevel level);
    
    static std::mutex logMutex;
};

// --- CUSTOM ASSERT MACRO ---
// If NDEBUG is defined (standard Release build flag), assertions disappear completely from code
#ifdef NDEBUG
    #define LOGGER_ASSERT(expression, fmt, ...) ((void)0)
#else
    #define LOGGER_ASSERT(expression, fmt, ...) \
        do { \
            if (!(expression)) { \
                Logger::handle_assert_fail(#expression, __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
            } \
        } while (false)
#endif
