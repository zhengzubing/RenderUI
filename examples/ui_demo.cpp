/**
 * @file ui_demo.cpp
 * @brief 完整 UI 控件演示 - 通过 JSON 配置加载所有控件和布局
 * 
 * 演示内容：
 * - Label（标签）
 * - Button（按钮）
 * - CheckBox（复选框）
 * - ImageView（图片视图）
 * - Container（容器）
 */

#include "Application.hpp"
#include "WidgetTree.hpp"
#include "JsonParser.hpp"
#include "Logger.hpp"
#include "Widget.hpp"
#include "Label.hpp"
#include "Button.hpp"
#include "CheckBox.hpp"
#include "ImageView.hpp"
#include "Container.hpp"

#include <iostream>
#include <filesystem>

using namespace Component;

/**
 * @brief 打印控件树结构（调试用）
 */
void printWidgetTree(Widget* widget, int depth = 0) {
    if (!widget) return;
     
    if (!widget->getId().empty()) {
        LOG_I << " (id: " << widget->getId() << ")";
    }
    
    // 递归打印子控件
    if (auto container = dynamic_cast<Container*>(widget)) {
        LOG_I << container->getId() << " (Container)";
        LOG_I <<  "Children (" << container->GetChildren().size() << "):";
        for (const auto& child : container->GetChildren()) {
            printWidgetTree(child.get(), depth + 1);
        }
    }
}

int main(int argc, char** argv) {
    LOG_I << "========================================";
    LOG_I << "RenderUI Complete Demo Starting...";
    LOG_I << "========================================";
    
    // 获取应用程序实例
    auto app = Application::instance();
    
    // 默认配置文件路径
    std::string configPath = "examples/config/ui_demo.json";
    
    // 如果命令行提供了配置文件路径，使用它
    if (argc > 1) {
        configPath = argv[1];
    }
    
    LOG_I << "Loading configuration from: " << configPath;
    
    // 检查文件是否存在
    if (!std::filesystem::exists(configPath)) {
        LOG_E << "Configuration file not found: " << configPath;
        return -1;
    }
    
    try {
        // 1. 加载 JSON 配置
        auto config = JsonParser::loadFromFile(configPath);
        
        // 2. 解析窗口配置
        std::string title;
        int width;
        int height;
        
        if (JsonParser::parseWindowConfig(config, title, width, height)) {
            LOG_I << "Window: " << title << " (" << width << "x" << height << ")";
        }
        
        // 3. 初始化应用程序
        if (!app->init(title, width, height)) {
            LOG_E << "Failed to initialize application";
            return -1;
        }
        
        // 4. 解析配置到 Application 的控件树（自动渲染）
        WidgetTree& tree = app->getWidgetTree();
        
        if (JsonParser::parseWidgets(config, tree)) {
            LOG_I << "Successfully parsed widgets from JSON";
            
            // 打印控件树结构（调试）
            LOG_I << "Widget Tree Structure:";
            const auto& roots = tree.getTopLevelWidgets();
            for (const auto& root : roots) {
                LOG_I << "Root Widget: " << root->widget->getId();
                printWidgetTree(root->widget.get());
            }
        } else {
            LOG_W << "Failed to parse widgets, using default layout";
        }
        
        LOG_I << "========================================";
        LOG_I << "Demo Ready! Press ESC to exit.";
        LOG_I << "========================================";
        
        // 5. 运行应用程序（自动渲染控件树）
        return app->run();
        
    } catch (const std::exception& e) {
        LOG_E << "Exception: " << e.what();
        return -1;
    }
}
