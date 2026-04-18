#pragma once

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <mutex>
#include <glib-2.0/glib.h>

// DBus GLib 头文件
struct _GDBusConnection;
struct _GVariant;

namespace Component {

/**
 * @brief CAN 信号数据类型
 */
enum class SignalType {
    Int32,      // 32 位整数
    UInt32,     // 32 位无符号整数
    Float,      // 浮点数
    Double,     // 双精度浮点
    Boolean,    // 布尔值
    String      // 字符串
};

/**
 * @brief CAN 信号值
 */
union SignalValue {
    int32_t i32;
    uint32_t u32;
    float f;
    double d;
    bool b;
    
    SignalValue() : i32(0) {}
};

/**
 * @brief CAN 信号回调类型
 */
using SignalCallback = std::function<void(const std::string& signalName, SignalValue value)>;

/**
 * @brief DBus 适配层
 * 
 * 负责连接 DBus 系统总线，订阅和接收 CAN 信号
 */
class DBusAdapter {
public:
    static DBusAdapter& instance();
    
    /**
     * @brief 初始化 DBus 连接
     * @return 是否成功
     */
    bool init();
    
    /**
     * @brief 订阅 CAN 信号
     * @param interface DBus 接口名
     * @param path DBus 路径
     * @param member 信号名
     * @return 订阅 ID
     */
    uint32_t subscribeSignal(const std::string& interface,
                             const std::string& path,
                             const std::string& member);
    
    /**
     * @brief 取消订阅
     * @param subscriptionId 订阅 ID
     */
    void unsubscribeSignal(uint32_t subscriptionId);
    
    /**
     * @brief 设置信号回调
     */
    void setSignalCallback(SignalCallback callback);
    
    /**
     * @brief 发送信号（用于模拟）
     */
    bool sendSignal(const std::string& interface,
                    const std::string& path,
                    const std::string& member,
                    SignalType type,
                    SignalValue value);
    
    /**
     * @brief 获取连接状态
     */
    bool isConnected() const { return connected_; }
    
    /**
     * @brief 清理资源
     */
    void cleanup();
    
private:
    DBusAdapter() = default;
    ~DBusAdapter();
    
    DBusAdapter(const DBusAdapter&) = delete;
    DBusAdapter& operator=(const DBusAdapter&) = delete;
    
    /**
     * @brief DBus 信号处理回调
     */
    static void onDBusSignal(_GDBusConnection* connection,
                             const gchar* sender_name,
                             const gchar* object_path,
                             const gchar* interface_name,
                             const gchar* signal_name,
                             _GVariant* parameters,
                             gpointer user_data);
    
    _GDBusConnection* connection_ = nullptr;
    std::map<uint32_t, std::tuple<std::string, std::string, std::string>> subscriptions_;
    uint32_t nextSubscriptionId_ = 1;
    SignalCallback signalCallback_;
    bool connected_ = false;
    
    mutable std::mutex mutex_;
};

} // namespace Component
