#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Component {

class WidgetTree;
class Container;

/**
 * @brief JSON 配置解析器
 * 
 * 从 JSON 配置加载 UI 布局
 */
class JsonParser {
public:
    /**
     * @brief 从文件加载 JSON
     * @param path JSON 文件路径
     * @return JSON 对象
     */
    static json loadFromFile(const std::string& path);
    
    /**
     * @brief 从字符串加载 JSON
     * @param content JSON 字符串
     * @return JSON 对象
     */
    static json loadFromString(const std::string& content);
    
    /**
     * @brief 解析窗口配置
     * @param config JSON 配置
     * @return 是否成功
     */
    static bool parseWindowConfig(const json& config, 
                                   std::string& title,
                                   int& width,
                                   int& height);
    
    /**
     * @brief 解析控件配置到控件树
     * @param config JSON 配置
     * @param tree 控件树
     * @return 是否成功
     */
    static bool parseWidgets(const json& config, WidgetTree& tree);
};

} // namespace Component
