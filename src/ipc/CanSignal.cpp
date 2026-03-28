#include "Component/CanSignal.hpp"
#include "Component/Logger.hpp"

namespace Component {

CanSignalManager& CanSignalManager::instance() {
    static CanSignalManager instance;
    return instance;
}

CanSignalManager::~CanSignalManager() = default;

bool CanSignalManager::init() {
    if (initialized_) {
        return true;
    }
    
    // 初始化 DBus
    if (!DBusAdapter::instance().init()) {
        LOG_ERROR("Failed to initialize DBus");
        return false;
    }
    
    // 设置信号回调
    DBusAdapter::instance().setSignalCallback(
        [this](const std::string& signalName, SignalValue value) {
            onSignalReceived(signalName, value);
        }
    );
    
    initialized_ = true;
    LOG_INFO("CanSignalManager initialized");
    
    return true;
}

bool CanSignalManager::registerSignal(const std::string& signalId, const CanSignalDef& def) {
    if (signals_.find(signalId) != signals_.end()) {
        LOG_WARNING("Signal already registered: %s", signalId.c_str());
        return false;
    }
    
    signals_[signalId] = def;
    
    // 订阅 DBus 信号
    uint32_t subscriptionId = DBusAdapter::instance().subscribeSignal(
        def.interface, def.path, def.member);
    
    if (subscriptionId > 0) {
        subscriptionIds_[signalId] = subscriptionId;
        LOG_INFO("CAN signal registered: %s (%s.%s)", 
                 signalId.c_str(), def.interface.c_str(), def.member.c_str());
        return true;
    }
    
    return false;
}

void CanSignalManager::unregisterSignal(const std::string& signalId) {
    auto it = signals_.find(signalId);
    if (it != signals_.end()) {
        // 取消订阅
        auto subIt = subscriptionIds_.find(signalId);
        if (subIt != subscriptionIds_.end()) {
            DBusAdapter::instance().unsubscribeSignal(subIt->second);
            subscriptionIds_.erase(subIt);
        }
        
        signals_.erase(it);
        currentValues_.erase(signalId);
        listeners_.erase(signalId);
        
        LOG_INFO("CAN signal unregistered: %s", signalId.c_str());
    }
}

SignalValue CanSignalManager::getRawValue(const std::string& signalId) {
    auto it = currentValues_.find(signalId);
    if (it != currentValues_.end()) {
        return it->second;
    }
    
    SignalValue empty;
    return empty;
}

float CanSignalManager::getPhysicalValue(const std::string& signalId) {
    auto it = signals_.find(signalId);
    if (it == signals_.end()) {
        return 0.0f;
    }
    
    auto valIt = currentValues_.find(signalId);
    if (valIt == currentValues_.end()) {
        return 0.0f;
    }
    
    float rawValue = 0.0f;
    
    // 根据类型获取原始值
    switch (it->second.type) {
        case SignalType::Int32:
            rawValue = static_cast<float>(valIt->second.i32);
            break;
        case SignalType::UInt32:
            rawValue = static_cast<float>(valIt->second.u32);
            break;
        case SignalType::Float:
            rawValue = valIt->second.f;
            break;
        case SignalType::Double:
            rawValue = static_cast<float>(valIt->second.d);
            break;
        case SignalType::Boolean:
            rawValue = valIt->second.b ? 1.0f : 0.0f;
            break;
    }
    
    return applyScaleOffset(it->second, rawValue);
}

void CanSignalManager::addValueChangedListener(const std::string& signalId, 
                                                ValueChangedCallback callback) {
    listeners_[signalId].push_back(std::move(callback));
}

void CanSignalManager::simulateValue(const std::string& signalId, float value) {
    auto it = signals_.find(signalId);
    if (it == signals_.end()) {
        LOG_WARNING("Signal not found for simulation: %s", signalId.c_str());
        return;
    }
    
    // 反向应用缩放和偏移得到原始值
    float rawValue = (value - it->second.offset) / it->second.scale;
    
    SignalValue signalValue;
    
    switch (it->second.type) {
        case SignalType::Int32:
            signalValue.i32 = static_cast<int32_t>(rawValue);
            break;
        case SignalType::UInt32:
            signalValue.u32 = static_cast<uint32_t>(rawValue);
            break;
        case SignalType::Float:
            signalValue.f = rawValue;
            break;
        case SignalType::Double:
            signalValue.d = rawValue;
            break;
        case SignalType::Boolean:
            signalValue.b = rawValue > 0.5f;
            break;
    }
    
    // 更新当前值
    currentValues_[signalId] = signalValue;
    
    // 通知监听器
    auto listenerIt = listeners_.find(signalId);
    if (listenerIt != listeners_.end()) {
        for (auto& callback : listenerIt->second) {
            callback(signalId, value);
        }
    }
    
    LOG_DEBUG("Simulated signal: %s = %.2f", signalId.c_str(), value);
}

void CanSignalManager::onSignalReceived(const std::string& signalName, SignalValue value) {
    // 查找对应的信号 ID
    for (const auto& [signalId, def] : signals_) {
        if (def.member == signalName) {
            // 更新当前值
            currentValues_[signalId] = value;
            
            // 计算物理值
            float physicalValue = 0.0f;
            
            switch (def.type) {
                case SignalType::Int32:
                    physicalValue = applyScaleOffset(def, static_cast<float>(value.i32));
                    break;
                case SignalType::UInt32:
                    physicalValue = applyScaleOffset(def, static_cast<float>(value.u32));
                    break;
                case SignalType::Float:
                    physicalValue = applyScaleOffset(def, value.f);
                    break;
                case SignalType::Double:
                    physicalValue = applyScaleOffset(def, static_cast<float>(value.d));
                    break;
                case SignalType::Boolean:
                    physicalValue = value.b ? 1.0f : 0.0f;
                    break;
            }
            
            // 通知监听器
            auto listenerIt = listeners_.find(signalId);
            if (listenerIt != listeners_.end()) {
                for (auto& callback : listenerIt->second) {
                    callback(signalId, physicalValue);
                }
            }
            
            LOG_DEBUG("CAN signal received: %s = %.2f %s", 
                     signalId.c_str(), physicalValue, def.unit.c_str());
            break;
        }
    }
}

float CanSignalManager::applyScaleOffset(const CanSignalDef& def, float rawValue) {
    float value = rawValue * def.scale + def.offset;
    
    // 限制在最小/最大值范围内
    if (value < def.min) value = def.min;
    if (value > def.max) value = def.max;
    
    return value;
}

} // namespace Component
