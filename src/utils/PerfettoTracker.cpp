#include "Component/PerfettoTracker.hpp"
#include "core/Logger.hpp"
#include <perfetto/perfetto.h>
#include <fstream>

namespace RenderUI {

// Perfetto 追踪类别定义
PERFETTO_TRACK_EVENT_STATIC_STORAGE();

// 异步操作追踪映射
static std::map<uint64_t, perfetto::Track> g_asyncTracks;

struct PerfettoTracker::Impl {
    std::unique_ptr<perfetto::TracingSession> tracingSession;
    bool isInitialized = false;
};

PerfettoTracker::PerfettoTracker() : impl_(std::make_unique<Impl>()) {
}

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
    
    impl_->isInitialized = true;
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
    impl_->tracingSession = perfetto::NewTracingSession();
    if (!impl_->tracingSession) {
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
    impl_->tracingSession->Setup(traceConfig);
    impl_->tracingSession->Start();
    
    tracingActive_ = true;
    LOGI("Perfetto tracing started");
    
    return true;
}

void PerfettoTracker::stopTracing() {
    if (!tracingActive_ || !impl_->tracingSession) {
        return;
    }
    
    LOGI("Stopping Perfetto tracing...");
    
    // 停止追踪并等待数据刷新
    impl_->tracingSession->Stop();
    impl_->tracingSession->FlushBlocking();
    
    tracingActive_ = false;
    LOGI("Perfetto tracing stopped");
}

bool PerfettoTracker::exportTrace(const std::string& filePath) {
    if (!impl_->tracingSession) {
        LOGE("No tracing session to export");
        return false;
    }
    
    // 读取追踪数据
    auto traceData = impl_->tracingSession->ReadTraceBlocking();
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
struct PerfettoScope::Impl {
    perfetto::TrackEvent::LegacyScope scope;
};

PerfettoScope::PerfettoScope(const char* name) : impl_(new Impl()) {
    PERFETTO_TRACK_EVENT(TRACE_EVENT_CATEGORY_GROUP_ENABLED, "perfetto", "track_event");
    impl_->scope = perfetto::TrackEvent::LegacyScope(name);
}

PerfettoScope::~PerfettoScope() {
    delete impl_;
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
