#include "JsonParser.hpp"
#include "WidgetTree.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"
#include "widgets/ImageView.hpp"
#include "Container.hpp"
#include "Logger.hpp"
#include <fstream>

namespace Component {

// 辅助函数：根据类型创建控件
static std::shared_ptr<Widget> createWidgetFromType(const std::string& type) {
    LOG_I << "Creating widget of type: " << type;
    
    if (type == "Label") {
        return std::make_shared<Label>();
    } else if (type == "Button") {
        return std::make_shared<Button>();
    } else if (type == "ImageView") {
        return std::make_shared<ImageView>();
    } else if (type == "Container") {
        return std::make_shared<Container>();
    } else {
        LOG_W << "Unknown widget type: " << type;
        return std::make_shared<Widget>();
    }
}

// 辅助函数：递归解析并添加子控件到容器
static void parseAndAddChildren(const json& childrenConfig, std::shared_ptr<Container> parent) {
    LOG_I << "Parsing " << childrenConfig.size() << " children for container";
    
    int childCount = 0;
    for (const auto& childConfig : childrenConfig) {
        std::string type = childConfig.value("type", "Widget");
        std::string id = childConfig.value("id", "unnamed");
        
        LOG_I << "  [" << childCount << "] Parsing child: type=" << type << ", id=" << id;
        
        auto child = createWidgetFromType(type);
        
        if (child) {
            // 调用子控件的 fromJson 方法解析配置
            child->fromJson(childConfig);
            LOG_I << "      fromJson completed for: " << id;
            
            // 如果子控件也是 Container，递归解析其 children
            if (type == "Container" && childConfig.contains("children")) {
                auto childContainer = std::dynamic_pointer_cast<Container>(child);
                if (childContainer) {
                    size_t subChildCount = childConfig["children"].size();
                    LOG_I << "      Recursively parsing " << subChildCount << " sub-children for container: " << id;
                    parseAndAddChildren(childConfig["children"], childContainer);
                }
            }
            
            // 标记为 dirty
            child->markDirty();
            
            // 添加到父容器
            parent->Add(child, id);
            
            LOG_I << "      ✓ Added child: " << type << " (id: " << id << ")";
            childCount++;
        } else {
            LOG_E << "      ✗ Failed to create child widget: " << type;
        }
    }
    
    LOG_I << "Finished parsing " << childCount << " children";
}

json JsonParser::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_E << "Failed to open JSON file: " << path;
        return json();
    }
    
    try {
        json config;
        file >> config;
        LOG_I << "JSON loaded from: " << path;
        return config;
    } catch (const std::exception& e) {
        LOG_E << "Failed to parse JSON: " << e.what();
        return json();
    }
}

json JsonParser::loadFromString(const std::string& content) {
    try {
        return json::parse(content);
    } catch (const std::exception& e) {
        LOG_E << "Failed to parse JSON: " << e.what();
        return json();
    }
}

bool JsonParser::parseWindowConfig(const json& config,
                                    std::string& title,
                                    int& width,
                                    int& height) {
    if (!config.contains("window")) {
        LOG_W << "No window config found";
        return false;
    }
    
    const auto& window = config["window"];
    
    if (window.contains("title")) {
        title = window["title"].get<std::string>();
    }
    
    if (window.contains("width")) {
        width = window["width"].get<int>();
    }
    
    if (window.contains("height")) {
        height = window["height"].get<int>();
    }
    
    LOG_I << "Window config parsed: " << title << " " << width << "x" << height;
    return true;
}

bool JsonParser::parseWidgets(const json& config, WidgetTree& tree) {
    if (!config.contains("widgets")) {
        LOG_W << "No widgets config found";
        return false;
    }
    
    size_t widgetCount = config["widgets"].size();
    LOG_I << "========================================";
    LOG_I << "Starting to parse " << widgetCount << " root widget(s)";
    LOG_I << "========================================";
    
    int rootIndex = 0;
    for (const auto& widgetConfig : config["widgets"]) {
        std::string type = widgetConfig.value("type", "Widget");
        std::string id = widgetConfig.value("id", "root");
        
        LOG_I << "\n[Root " << rootIndex << "] Parsing root widget: type=" << type << ", id=" << id;
        
        auto widget = createWidgetFromType(type);
        
        if (widget) {
            // 调用控件的 fromJson 方法解析配置
            LOG_I << "  Calling fromJson for root widget: " << id;
            widget->fromJson(widgetConfig);
            LOG_I << "  fromJson completed for: " << id;
            
            // 如果是 Container，递归解析 children
            LOG_I << "  Checking if widget is a container: " << type;
            if (type == "Container" && widgetConfig.contains("children")) {
                LOG_I << "  Parsing children for container: " << id;
                auto container = std::dynamic_pointer_cast<Container>(widget);
                if (container) {
                    size_t childCount = widgetConfig["children"].size();
                    LOG_I << "  Container has " << childCount << " children, starting recursive parse...";
                    parseAndAddChildren(widgetConfig["children"], container);
                    LOG_I << "  Recursive parse completed for container: " << id;
                }
            }
            
            // 标记为 dirty，确保首次渲染
            widget->markDirty();
            
            // 添加到控件树
            tree.addRoot(widget, id);
            LOG_I << "  ✓ Root widget added to tree: " << type << " (id: " << id << ")\n";
            
            rootIndex++;
        } else {
            LOG_E << "  ✗ Failed to create root widget: " << type;
        }
    }
    
    LOG_I << "========================================";
    LOG_I << "Widgets parsed successfully: " << rootIndex << " root(s)";
    LOG_I << "========================================";
    return true;
}

} // namespace Component
