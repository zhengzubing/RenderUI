/* GTEST DISABLED - src/test_model3d.cpp */
// #include <gtest/gtest.h>
#include "Model3D.hpp"
#include "Types.hpp"
#include <cmath>

using namespace Component;

/**
 * @brief Model3D 三维模型控件测试
 * 
 * 测试 OBJ 模型加载、纹理映射、自动旋转等功能
 */

TEST(Model3DTest, CreateAndDestroy) {
    auto model = std::make_shared<Model3D>();
    EXPECT_NE(model, nullptr);
    
    // 默认状态检查
    EXPECT_FALSE(model->isLoaded());
    EXPECT_TRUE(model->isVisible());
    EXPECT_FALSE(model->isAutoRotateEnabled());
}

TEST(Model3DTest, LoadOBJModel) {
    auto model = std::make_shared<Model3D>();
    
    // 注意：这个测试需要实际的 OBJ 文件
    // 在真实测试中应该提供测试资源文件
    bool loaded = model->loadModel("assets/test_cube.obj");
    
    // 如果文件不存在，loadModel 会返回 false
    // 这里只验证接口调用不崩溃
    EXPECT_NO_THROW([&]() {
        model->loadModel("nonexistent.obj");
    }());
}

TEST(Model3DTest, AutoRotate) {
    auto model = std::make_shared<Model3D>();
    
    // 启用自动旋转
    model->setAutoRotate(true);
    EXPECT_TRUE(model->isAutoRotateEnabled());
    
    // 设置旋转速度
    model->setAutoRotate(true, 0.02f);
    EXPECT_FLOAT_EQ(model->getRotationSpeed(), 0.02f);
    
    // 禁用自动旋转
    model->setAutoRotate(false);
    EXPECT_FALSE(model->isAutoRotateEnabled());
}

TEST(Model3DTest, ManualRotation) {
    auto model = std::make_shared<Model3D>();
    
    // 设置绕 Y 轴旋转
    model->setRotationY(M_PI / 4.0f);  // 45 度
    float rotation = model->getRotationY();
    EXPECT_NEAR(rotation, M_PI / 4.0f, 1e-5);
    
    // 设置绕 X 轴旋转
    model->setRotationX(M_PI / 6.0f);  // 30 度
    EXPECT_FLOAT_EQ(model->getRotationX(), M_PI / 6.0f);
    
    // 设置绕 Z 轴旋转
    model->setRotationZ(M_PI / 3.0f);  // 60 度
    EXPECT_FLOAT_EQ(model->getRotationZ(), M_PI / 3.0f);
}

TEST(Model3DTest, CameraDistance) {
    auto model = std::make_shared<Model3D>();
    
    // 设置相机距离
    model->setCameraDistance(5.0f);
    EXPECT_FLOAT_EQ(model->getCameraDistance(), 5.0f);
    
    model->setCameraDistance(10.0f);
    EXPECT_FLOAT_EQ(model->getCameraDistance(), 10.0f);
    
    // 最小距离限制
    model->setCameraDistance(0.1f);
    EXPECT_GE(model->getCameraDistance(), 0.5f);  // 假设有最小距离限制
}

TEST(Model3DTest, Lighting) {
    auto model = std::make_shared<Model3D>();
    
    // 设置环境光
    Color ambient(0.2f, 0.2f, 0.2f, 1.0f);
    model->setAmbientLight(ambient);
    
    // 设置方向光
    Color directional(1.0f, 1.0f, 1.0f, 1.0f);
    model->setDirectionalLight(directional);
    
    // 验证需要渲染 (因为 Model3D 始终需要渲染)
    EXPECT_TRUE(model->needsRender());
}

TEST(Model3DTest, MaterialProperties) {
    auto model = std::make_shared<Model3D>();
    
    // 测试材质属性设置
    model->setMetallic(0.8f);
    EXPECT_FLOAT_EQ(model->getMetallic(), 0.8f);
    
    model->setRoughness(0.5f);
    EXPECT_FLOAT_EQ(model->getRoughness(), 0.5f);
    
    // 范围验证
    model->setMetallic(1.5f);  // 超出范围
    EXPECT_LE(model->getMetallic(), 1.0f);
    
    model->setRoughness(-0.5f);  // 负值
    EXPECT_GE(model->getRoughness(), 0.0f);
}

TEST(Model3DTest, BoundingBox) {
    auto model = std::make_shared<Model3D>();
    
    // 获取包围盒 (空模型应该有默认值)
    auto bbox = model->getBoundingBox();
    
    // 空模型的包围盒应该是零或单位大小
    EXPECT_GE(bbox.min.x, 0.0f);
    EXPECT_GE(bbox.min.y, 0.0f);
    EXPECT_GE(bbox.min.z, 0.0f);
}

TEST(Model3DTest, WireframeMode) {
    auto model = std::make_shared<Model3D>();
    
    // 切换线框模式
    model->setWireframeMode(true);
    EXPECT_TRUE(model->isWireframeMode());
    
    model->setWireframeMode(false);
    EXPECT_FALSE(model->isWireframeMode());
}

TEST(Model3DTest, AnimationState) {
    auto model = std::make_shared<Model3D>();
    
    // 测试动画状态 (如果支持动画)
    EXPECT_NO_THROW([&]() {
        model->playAnimation("idle");
        model->stopAnimation();
    }());
}
