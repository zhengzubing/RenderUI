#include "PerfettoTracker.hpp"
#include "Logger.hpp"
#include <fstream>

// 引入 Perfetto SDK
#if ENABLE_PERFETTO
#include <perfetto.h>
using namespace perfetto;
#endif

// 注意：Perfetto 功能已禁用，ENABLE_PERFETTO=OFF
// 以下代码需要 perfetto 库支持

#if ENABLE_PERFETTO

namespace RenderUI {

// Perfetto 追踪类别定义
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

// 异步操作追踪映射
static std::map<uint64_t, perfetto::Track> g_asyncTracks;

PerfettoTracker::PerfettoTracker() : initialized_(false), tracingActive_(false) {}

PerfettoTracker::~PerfettoTracker() {
    if (tracingActive_) {
        stopTracing();
    }
}

PerfettoTracker& PerfettoTracker::instance() {
    static PerfettoTracker instance;
    return instance;
}

bool PerfettoTracker::init(uint32_t maxBufferSizeMB, uint32_t durationMs) {
    if (initialized_) {
        LOGW("Perfetto already initialized");
        return true;
    }
    
    maxBufferSizeMB_ = maxBufferSizeMB;
    durationMs_ = durationMs;
    
    // 初始化 Perfetto SDK
    perfetto::TracingInitArgs args;
    args.backends |= perfetto::kSystemBackend;
    
    perfetto::TracingInitialize(args);
    
    // 配置追踪会话
    perfetto::TraceConfig traceConfig;
    traceConfig.set_duration_ms(durationMs);
    traceConfig.add_buffers()->set_size_kb(maxBufferSizeMB * 1024);
    
    // 添加追踪配置
    auto* cfg = traceConfig.add_data_sources()->mutable_config();
    cfg->set_name("track_event");
    
    // 启用 Chrome 风格的追踪
    cfg->set_trace_packet_mode(perfetto::protos::gen::DataSourceConfig::TRACE_PACKET_MODE_STREAMING);
    
    initialized_ = true;
    
    LOGI("Perfetto initialized: buffer=%uMB, duration=%ums", 
         maxBufferSizeMB_, durationMs_);
    
    return true;
}

bool PerfettoTracker::startTracing() {
    if (!initialized_) {
        LOGE("Perfetto not initialized");
        return false;
    }
    
    if (tracingActive_) {
        LOGW("Perfetto tracing already active");
        return true;
    }
    
    // 创建追踪会话
    tracingSession_ = perfetto::NewTracingSession();
    if (!tracingSession_) {
        LOGE("Failed to create Perfetto tracing session");
        return false;
    }
    
    // 配置追踪
    perfetto::TraceConfig traceConfig;
    traceConfig.set_duration_ms(durationMs_);
    traceConfig.add_buffers()->set_size_kb(maxBufferSizeMB_ * 1024);
    
    auto* cfg = traceConfig.add_data_sources()->mutable_config();
    cfg->set_name("track_event");
    
    // 启动追踪
    tracingSession_->Setup(traceConfig);
    tracingSession_->Start();
    
    tracingActive_ = true;
    LOGI("Perfetto tracing started");
    
    return true;
}

void PerfettoTracker::stopTracing() {
    if (!tracingActive_ || !tracingSession_) {
        return;
    }
    
    LOGI("Stopping Perfetto tracing...");
    
    // 停止追踪并等待数据刷新
    tracingSession_->Stop();
    tracingSession_->FlushBlocking();
    
    tracingActive_ = false;
    LOGI("Perfetto tracing stopped");
}

bool PerfettoTracker::exportTrace(const std::string& filePath) {
    if (!tracingSession_) {
        LOGE("No tracing session to export");
        return false;
    }
    
    // 读取追踪数据
    auto traceData = tracingSession_->ReadTraceBlocking();
    if (traceData.empty()) {
        LOGE("Empty trace data");
        return false;
    }
    
    // 写入文件
    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        LOGE("Failed to open file for writing: %s", filePath.c_str());
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(traceData.data()), traceData.size());
    file.close();
    
    LOGI("Perfetto trace exported to: %s (%zu bytes)", filePath.c_str(), traceData.size());
    return true;
}

// PerfettoScope 实现
PerfettoScope::PerfettoScope(const char* name) : name_(name) {
    // 使用新 API 记录瞬时事件（自动作用域）
    PERFETTO_TRACE_EVENT(name_);
}

PerfettoScope::~PerfettoScope() {
    // 析构时自动结束追踪（通过 RAII）
}

// 异步追踪函数
void perfettoTrackAsyncBegin(const char* name, uint64_t id) {
    auto track = perfetto::Track(id);
    g_asyncTracks[id] = track;
    PERFETTO_TRACE_INSTANT_ASYNC(name, track);
}

void perfettoTrackAsyncEnd(const char* name, uint64_t id) {
    auto it = g_asyncTracks.find(id);
    if (it != g_asyncTracks.end()) {
        PERFETTO_TRACE_INSTANT_ASYNC(name, it->second);
        g_asyncTracks.erase(it);
    }
}

// 计数器追踪函数
void perfettoTrackCounter(const char* name, int64_t value) {
    PERFETTO_COUNTER(name, value);
}

} // namespace RenderUI

#else

// ENABLE_PERFETTO=OFF 时的空实现
namespace RenderUI {

bool PerfettoTracker::init(uint32_t, uint32_t) { return false; }
bool PerfettoTracker::startTracing() { return false; }
void PerfettoTracker::stopTracing() {}
bool PerfettoTracker::exportTrace(const std::string&) { return false; }

PerfettoScope::PerfettoScope(const char*) {}
PerfettoScope::~PerfettoScope() = default;

void perfettoTrackAsyncBegin(const char*, uint64_t) {}
void perfettoTrackAsyncEnd(const char*, uint64_t) {}
void perfettoTrackCounter(const char*, int64_t) {}

} // namespace RenderUI

#endif
