/**
 * @file 360_camera_demo.cpp
 * @brief 车载全景 360 影像 Demo
 * 
 * 演示功能:
 * 1. 前视图 + 后视图视频流显示
 * 2. 3D 车身模型叠加
 * 3. 触摸切换视角
 * 4. 动态轨迹线绘制
 * 5. 车速/档位显示
 * 6. DBus 接收 CAN 信号
 * 7. 多视图切换（正常/俯视/侧视）
 */

#include "Application.hpp"
#include "WidgetTree.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"
#include "widgets/ImageView.hpp"
#include "VideoStream.hpp"
#include "Model3D.hpp"
#include "CanSignal.hpp"
#include "JsonParser.hpp"
#include "AbsoluteLayout.hpp"
#include "Logger.hpp"

using namespace Component;

int main(int argc, char* argv[]) {
    // 初始化日志
    Logger::instance().init(plog::verbose, "360_camera.log");
    
    PLOG_INFO << "=== 360 Camera Demo Started ===";
    
    // 创建应用
    auto app = Application::instance();
    
    // 从 JSON 加载配置
    auto config = JsonParser::loadFromFile("config/360_camera.json");
    
    std::string title = "360 Camera Demo";
    int width = 1920, height = 1080;
    
    if (!config.is_null()) {
        JsonParser::parseWindowConfig(config, title, width, height);
    }
    
    // 初始化应用
    if (!app->init(title, width, height)) {
        PLOG_ERROR << "Failed to initialize application";
        return -1;
    }
    
    // 初始化 CAN 信号管理器
    CanSignalManager::instance().init();
    
    // 注册 CAN 信号
    CanSignalDef speedDef;
    speedDef.name = "VehicleSpeed";
    speedDef.interface = "com.vehicle.can.Signal";
    speedDef.path = "/com/vehicle/can/speed";
    speedDef.member = "SpeedUpdate";
    speedDef.type = SignalType::Int32;
    speedDef.scale = 0.01f;  // 0.01 km/h per bit
    speedDef.unit = "km/h";
    CanSignalManager::instance().registerSignal("speed", speedDef);
    
    // 创建控件树
    WidgetTree tree;
    
    // 背景
    auto background = std::make_shared<ImageView>("assets/bg_dark.png");
    background->setId("background");
    background->setPosition(0, 0);
    background->setSize(1920, 1080);
    tree.addRoot(background, "background");
    
    // 前视图视频流容器
    auto frontContainer = std::make_shared<Widget>();
    frontContainer->setId("front_container");
    frontContainer->setPosition(100, 100);
    frontContainer->setSize(640, 480);
    tree.addRoot(frontContainer, "front_container");
    
    // 前视图 VideoStream
    auto frontCamera = std::make_shared<VideoStream>("camera://front");
    frontCamera->setId("front_camera");
    frontCamera->setPosition(0, 0);
    frontCamera->setSize(640, 480);
    frontContainer->addChild(frontCamera);
    frontCamera->start();
    
    // 后视图视频流容器
    auto rearContainer = std::make_shared<Widget>();
    rearContainer->setId("rear_container");
    rearContainer->setPosition(1180, 100);
    rearContainer->setSize(640, 480);
    tree.addRoot(rearContainer, "rear_container");
    
    // 后视图 VideoStream
    auto rearCamera = std::make_shared<VideoStream>("camera://rear");
    rearCamera->setId("rear_camera");
    rearCamera->setPosition(0, 0);
    rearCamera->setSize(640, 480);
    rearContainer->addChild(rearCamera);
    rearCamera->start();
    
    // 3D 车身模型
    auto carModel = std::make_shared<Model3D>();
    carModel->setId("car_model");
    carModel->setPosition(960, 540, 0);  // 屏幕中央
    carModel->loadModel("assets/car.obj");
    carModel->setScale(2.0f, 2.0f, 2.0f);
    carModel->setAutoRotate(true, 0.005f);
    tree.addRoot(carModel, "car_model");
    
    // 车速显示 Label
    auto speedLabel = std::make_shared<Label>("0 km/h");
    speedLabel->setId("speed_label");
    speedLabel->setPosition(50, 50);
    speedLabel->setFontSize(32.0f);
    speedLabel->setTextColor(Color(1.0f, 1.0f, 0.0f, 1.0f));  // 黄色
    tree.addRoot(speedLabel, "speed_label");
    
    // 监听车速信号
    CanSignalManager::instance().addValueChangedListener("speed",
        [speedLabel](const std::string& signalId, float value) {
            char buffer[64];
            snprintf(buffer, sizeof(buffer), "%.1f km/h", value);
            speedLabel->setText(buffer);
        }
    );
    
    // 档位显示 Label
    auto gearLabel = std::make_shared<Label>("D");
    gearLabel->setId("gear_label");
    gearLabel->setPosition(50, 100);
    gearLabel->setFontSize(24.0f);
    gearLabel->setTextColor(Color(0.0f, 1.0f, 0.0f, 1.0f));  // 绿色
    tree.addRoot(gearLabel, "gear_label");
    
    // 视图切换按钮
    auto btnNormal = std::make_shared<Button>("正常视图");
    btnNormal->setId("btn_normal");
    btnNormal->setPosition(1700, 50);
    btnNormal->setSize(150, 40);
    btnNormal->setOnClick([&tree]() {
        PLOG_INFO << "Switched to normal view";
        // TODO: 切换视图逻辑
    });
    tree.addRoot(btnNormal, "btn_normal");
    
    auto btnTop = std::make_shared<Button>("俯视图");
    btnTop->setId("btn_top");
    btnTop->setPosition(1700, 100);
    btnTop->setSize(150, 40);
    btnTop->setOnClick([&tree]() {
        PLOG_INFO << "Switched to top view";
        // TODO: 切换视图逻辑
    });
    tree.addRoot(btnTop, "btn_top");
    
    // 更新布局
    tree.updateLayout(width, height);
    
    PLOG_INFO << "=== Demo Setup Complete ===";
    
    // 运行主循环
    int result = app->run();
    
    // 清理
    app->shutdown();
    
    return result;
}
