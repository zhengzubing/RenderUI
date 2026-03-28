#pragma once

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Initializers/ConsoleInitializer.h>
#include <string>

namespace Component {

/**
 * @brief 日志系统
 * 
 * 基于 plog 封装，提供统一的日志接口
 */
class Logger {
public:
    static Logger& instance();
    
    /**
     * @brief 初始化日志系统
     * @param level 日志级别
     * @param logFile 日志文件路径
     * @return 是否成功
     */
    bool init(plog::Severity level = plog::info, const std::string& logFile = "renderui.log");
    
    /**
     * @brief 获取日志级别
     */
    plog::Severity getLevel() const { return level_; }
    
    /**
     * @brief 设置日志级别
     */
    void setLevel(plog::Severity level);
    
private:
    Logger() = default;
    ~Logger() = default;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    plog::Severity level_ = plog::info;
    bool initialized_ = false;
};

} // namespace Component

// 便捷宏定义
#define LOG_DEBUG PLOG_DEBUG
#define LOG_INFO PLOG_INFO
#define LOG_WARNING PLOG_WARNING
#define LOG_ERROR PLOG_ERROR
#define LOG_VERBOSE PLOG_VERBOSE
