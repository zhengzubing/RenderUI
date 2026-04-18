#include "Logger.hpp"

namespace Component {

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

bool Logger::init(plog::Severity level, const std::string& logFile) {
    if (initialized_) {
        return true;
    }
    
    level_ = level;

    // 禁用标准输出缓冲，确保日志立即显示
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    
    // 初始化 plog - 同时输出到文件和控制台
    // TODO 输出到控制台未生效
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(logFile.c_str());
    static plog::ConsoleAppender<plog::TxtFormatter> consoleAppender(plog::streamStdOut);    
    // 初始化日志系统，先添加文件 appender
    plog::init(level, &fileAppender);
    
    // 添加控制台 appender（链式添加）
    plog::get()->addAppender(&consoleAppender);
    
    initialized_ = true;
    PLOG_INFO << "Logger initialized, level: " << plog::severityToString(level);
    PLOG_INFO << "Logging to file: " << logFile << " and console";
    
    return true;
}

void Logger::setLevel(plog::Severity level) {
    level_ = level;
    // 注意：plog 不支持动态修改级别，需要重新初始化
}

} // namespace Component