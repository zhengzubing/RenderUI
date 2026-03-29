#pragma once

#include "DBusAdapter.hpp"
#include <string>
#include <map>
#include <functional>
#include <memory>

namespace Component {

/**
 * @brief CAN 信号定义
 */
struct CanSignalDef {
    std::string name;           // 信号名称
    std::string interface;      // DBus 接口
    std::string path;           // DBus 路径
    std::string member;         // DBus 信号成员
    SignalType type;            // 数据类型
    float scale = 1.0f;         // 缩放因子
    float offset = 0.0f;        // 偏移量
    float min = 0.0f;           // 最小值
    float max = 100.0f;         // 最大值
    std::string unit;           // 单位
};

/**
 * @brief CAN 信号管理器
 * 
 * 管理所有 CAN 信号的订阅、解析和通知
 */
class CanSignalManager {
public:
    static CanSignalManager& instance();
    
    /**
     * @brief 初始化信号管理器
     */
    bool init();
    
    /**
     * @brief 注册 CAN 信号
     * @param signalId 信号 ID
     * @param def 信号定义
     * @return 是否成功
     */
    bool registerSignal(const std::string& signalId, const CanSignalDef& def);
    
    /**
     * @brief 取消注册信号
     */
    void unregisterSignal(const std::string& signalId);
    
    /**
     * @brief 获取信号值（原始值）
     */
    SignalValue getRawValue(const std::string& signalId);
    
    /**
     * @brief 获取信号值（缩放后的物理值）
     */
    float getPhysicalValue(const std::string& signalId);
    
    /**
     * @brief 设置信号变化回调
     */
    using ValueChangedCallback = std::function<void(const std::string& signalId, float value)>;
    void addValueChangedListener(const std::string& signalId, ValueChangedCallback callback);
    
    /**
     * @brief 模拟信号值（用于测试）
     */
    void simulateValue(const std::string& signalId, float value);
    
    /**
     * @brief 获取所有注册的信号
     */
    const std::map<std::string, CanSignalDef>& getSignals() const { return signals_; }
    
private:
    CanSignalManager() = default;
    ~CanSignalManager();
    
    CanSignalManager(const CanSignalManager&) = delete;
    CanSignalManager& operator=(const CanSignalManager&) = delete;
    
    /**
     * @brief 处理 DBus 信号
     */
    void onSignalReceived(const std::string& signalName, SignalValue value);
    
    /**
     * @brief 应用缩放和偏移
     */
    float applyScaleOffset(const CanSignalDef& def, float rawValue);
    
    std::map<std::string, CanSignalDef> signals_;
    std::map<std::string, SignalValue> currentValues_;
    std::map<std::string, std::vector<ValueChangedCallback>> listeners_;
    std::map<std::string, uint32_t> subscriptionIds_;
    
    bool initialized_ = false;
};

} // namespace Component
