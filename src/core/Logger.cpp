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
    
    // 初始化 plog
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(logFile.c_str());
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    
    plog::init(level, &fileAppender);
    plog::addAppender(&consoleAppender);
    
    initialized_ = true;
    PLOG_INFO << "Logger initialized, level: " << plog::severityToString(level);
    
    return true;
}

void Logger::setLevel(plog::Severity level) {
    level_ = level;
    // 注意：plog 不支持动态修改级别，需要重新初始化
}

} // namespace Component
