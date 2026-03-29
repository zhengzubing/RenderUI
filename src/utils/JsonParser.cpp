#include "JsonParser.hpp"
#include "WidgetTree.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"
#include "widgets/ImageView.hpp"
#include "Logger.hpp"
#include <fstream>

namespace Component {

json JsonParser::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open JSON file: %s", path.c_str());
        return json();
    }
    
    try {
        json config;
        file >> config;
        LOG_INFO("JSON loaded from: %s", path.c_str());
        return config;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse JSON: %s", e.what());
        return json();
    }
}

json JsonParser::loadFromString(const std::string& content) {
    try {
        return json::parse(content);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to parse JSON: %s", e.what());
        return json();
    }
}

bool JsonParser::parseWindowConfig(const json& config,
                                    std::string& title,
                                    int& width,
                                    int& height) {
    if (!config.contains("window")) {
        LOG_WARNING("No window config found");
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
    
    LOG_INFO("Window config parsed: %s %dx%d", title.c_str(), width, height);
    return true;
}

std::shared_ptr<Widget> JsonParser::parseWidget(const json& config) {
    if (!config.contains("type")) {
        LOG_WARNING("Widget type not specified");
        return nullptr;
    }
    
    std::string type = config["type"].get<std::string>();
    std::shared_ptr<Widget> widget;
    
    // 根据类型创建控件
    if (type == "Label") {
        std::string text = config.value("text", "");
        auto label = std::make_shared<Label>(text);
        
        if (config.contains("fontSize")) {
            label->setFontSize(config["fontSize"].get<float>());
        }
        if (config.contains("fontFamily")) {
            label->setFontFamily(config["fontFamily"].get<std::string>());
        }
        if (config.contains("color")) {
            std::string colorStr = config["color"].get<std::string>();
            label->setTextColor(Color::fromHex(colorStr.c_str()));
        }
        
        widget = label;
        LOG_DEBUG("Label widget created: %s", text.c_str());
    }
    else if (type == "Button") {
        std::string text = config.value("text", "");
        auto button = std::make_shared<Button>(text);
        
        if (config.contains("fontSize")) {
            button->setFontSize(config["fontSize"].get<float>());
        }
        if (config.contains("backgroundColor")) {
            std::string colorStr = config["backgroundColor"].get<std::string>();
            button->setBackgroundColor(Color::fromHex(colorStr.c_str()));
        }
        if (config.contains("textColor")) {
            std::string colorStr = config["textColor"].get<std::string>();
            button->setTextColor(Color::fromHex(colorStr.c_str()));
        }
        
        widget = button;
        LOG_DEBUG("Button widget created: %s", text.c_str());
    }
    else if (type == "ImageView") {
        std::string image = config.value("image", "");
        auto imageView = std::make_shared<ImageView>(image);
        
        widget = imageView;
        LOG_DEBUG("ImageView widget created: %s", image.c_str());
    }
    else if (type == "Container") {
        widget = std::make_shared<Widget>();
        LOG_DEBUG("Container widget created");
    }
    else {
        LOG_WARNING("Unknown widget type: %s", type.c_str());
        return nullptr;
    }
    
    // 应用位置和大小
    if (config.contains("position")) {
        const auto& pos = config["position"];
        float x = pos.value("x", 0.0f);
        float y = pos.value("y", 0.0f);
        widget->setPosition(x, y);
    }
    
    if (config.contains("size")) {
        const auto& size = config["size"];
        float w = size.value("width", 0.0f);
        float h = size.value("height", 0.0f);
        widget->setSize(w, h);
    }
    
    // 应用 ID
    if (config.contains("id")) {
        widget->setId(config["id"].get<std::string>());
    }
    
    // 应用 Z 序
    if (config.contains("zIndex")) {
        widget->setZIndex(config["zIndex"].get<int>());
    }
    
    // 递归解析子控件
    if (config.contains("children")) {
        for (const auto& childConfig : config["children"]) {
            auto child = parseWidget(childConfig);
            if (child) {
                widget->addChild(child);
            }
        }
    }
    
    return widget;
}

bool JsonParser::parseWidgets(const json& config, WidgetTree& tree) {
    if (!config.contains("widgets")) {
        LOG_WARNING("No widgets config found");
        return false;
    }
    
    for (const auto& widgetConfig : config["widgets"]) {
        auto widget = parseWidget(widgetConfig);
        if (widget) {
            std::string id = widgetConfig.value("id", "root");
            tree.addRoot(widget, id);
        }
    }
    
    LOG_INFO("Widgets parsed: %zu roots", config["widgets"].size());
    return true;
}

void JsonParser::applyStyles(Widget* widget, const json& styles) {
    // TODO: 实现样式应用
}

void JsonParser::applyLayout(Widget* widget, const json& layout) {
    // TODO: 实现布局属性应用
}

} // namespace Component
