#include "DBusAdapter.hpp"
#include "Logger.hpp"
#include <gio/gio.h>
#include <cstring>

namespace Component {

DBusAdapter& DBusAdapter::instance() {
    static DBusAdapter instance;
    return instance;
}

DBusAdapter::~DBusAdapter() {
    cleanup();
}

bool DBusAdapter::init() {
    if (connected_) {
        return true;
    }
    
    GError* error = nullptr;
    
    // 连接到系统总线
    connection_ = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    
    if (!connection_) {
        LOG_ERROR("Failed to connect to DBus: %s", error->message);
        g_error_free(error);
        return false;
    }
    
    connected_ = true;
    LOG_INFO("Connected to DBus system bus");
    
    return true;
}

uint32_t DBusAdapter::subscribeSignal(const std::string& interface,
                                       const std::string& path,
                                       const std::string& member) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!connected_) {
        LOG_ERROR("DBus not connected");
        return 0;
    }
    
    // 生成订阅 ID
    uint32_t subscriptionId = nextSubscriptionId_++;
    
    // 保存订阅信息
    subscriptions_[subscriptionId] = std::make_tuple(interface, path, member);
    
    // 构建 DBus 信号匹配规则
    std::string rule = "type='signal', interface='" + interface + 
                       "', member='" + member + "'";
    
    // 订阅信号
    g_dbus_connection_signal_subscribe(
        connection_,
        nullptr,                    // sender (any)
        interface.c_str(),          // interface
        member.c_str(),             // member
        path.empty() ? nullptr : path.c_str(),  // object path
        nullptr,                    // arg0 (any)
        G_DBUS_SIGNAL_FLAGS_NONE,
        onDBusSignal,
        this,
        nullptr
    );
    
    LOG_INFO("Subscribed to DBus signal: %s.%s", interface.c_str(), member.c_str());
    return subscriptionId;
}

void DBusAdapter::unsubscribeSignal(uint32_t subscriptionId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = subscriptions_.find(subscriptionId);
    if (it != subscriptions_.end()) {
        subscriptions_.erase(it);
        LOG_DEBUG("Unsubscribed from signal: %u", subscriptionId);
    }
}

void DBusAdapter::setSignalCallback(SignalCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    signalCallback_ = std::move(callback);
}

bool DBusAdapter::sendSignal(const std::string& interface,
                              const std::string& path,
                              const std::string& member,
                              SignalType type,
                              SignalValue value) {
    if (!connected_) {
        LOG_ERROR("DBus not connected");
        return false;
    }
    
    GVariant* parameters = nullptr;
    
    // 根据类型创建 GVariant
    switch (type) {
        case SignalType::Int32:
            parameters = g_variant_new("(i)", value.i32);
            break;
        case SignalType::UInt32:
            parameters = g_variant_new("(u)", value.u32);
            break;
        case SignalType::Float:
            parameters = g_variant_new("(f)", value.f);
            break;
        case SignalType::Double:
            parameters = g_variant_new("(d)", value.d);
            break;
        case SignalType::Boolean:
            parameters = g_variant_new("(b)", value.b ? TRUE : FALSE);
            break;
        default:
            LOG_ERROR("Unsupported signal type");
            return false;
    }
    
    // 发送信号
    g_dbus_connection_emit_signal(
        connection_,
        nullptr,                    // destination (any)
        path.c_str(),
        interface.c_str(),
        member.c_str(),
        parameters,
        nullptr
    );
    
    LOG_DEBUG("Sent DBus signal: %s.%s", interface.c_str(), member.c_str());
    return true;
}

void DBusAdapter::onDBusSignal(_GDBusConnection* connection,
                                const gchar* sender_name,
                                const gchar* object_path,
                                const gchar* interface_name,
                                const gchar* signal_name,
                                _GVariant* parameters,
                                gpointer user_data) {
    auto adapter = static_cast<DBusAdapter*>(user_data);
    
    std::lock_guard<std::mutex> lock(adapter->mutex_);
    
    if (!adapter->signalCallback_) {
        return;
    }
    
    SignalValue value;
    
    // 解析参数
    if (g_variant_n_children(parameters) > 0) {
        GVariant* child = g_variant_get_child_value(parameters, 0);
        const gchar* type_string = g_variant_get_type_string(child);
        
        if (strcmp(type_string, "i") == 0) {
            g_variant_get(child, "i", &value.i32);
        } else if (strcmp(type_string, "u") == 0) {
            g_variant_get(child, "u", &value.u32);
        } else if (strcmp(type_string, "f") == 0) {
            g_variant_get(child, "f", &value.f);
        } else if (strcmp(type_string, "d") == 0) {
            g_variant_get(child, "d", &value.d);
        } else if (strcmp(type_string, "b") == 0) {
            value.b = g_variant_get_boolean(child);
        }
        
        g_variant_unref(child);
    }
    
    // 调用回调
    adapter->signalCallback_(std::string(signal_name), value);
    
    LOG_DEBUG("Received DBus signal: %s", signal_name);
}

void DBusAdapter::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (connection_) {
        g_object_unref(connection_);
        connection_ = nullptr;
    }
    
    subscriptions_.clear();
    connected_ = false;
    
    LOG_INFO("DBus connection cleaned up");
}

} // namespace Component
