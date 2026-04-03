#include "JsonParser.hpp"
#include "WidgetTree.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"
#include "widgets/ImageView.hpp"
#include "Container.hpp"
#include "Logger.hpp"
#include <fstream>

namespace Component {

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
    
    for (const auto& widgetConfig : config["widgets"]) {
        std::string type = widgetConfig.value("type", "Widget");
        std::shared_ptr<Widget> widget;
        
        // 根据类型创建控件
        if (type == "Label") {
            widget = std::make_shared<Label>();
        } else if (type == "Button") {
            widget = std::make_shared<Button>();
        } else if (type == "ImageView") {
            widget = std::make_shared<ImageView>();
        } else if (type == "Container") {
            widget = std::make_shared<Container>();
        } else {
            widget = std::make_shared<Widget>();
        }
        
        if (widget) {
            // 调用控件的 fromJson 方法解析配置
            widget->fromJson(widgetConfig);
            
            // 添加到控件树
            std::string id = widgetConfig.value("id", "root");
            tree.addRoot(widget, id);
        }
    }
    
    LOG_I << "Widgets parsed: " << config["widgets"].size() << " roots";
    return true;
}

} // namespace Component
