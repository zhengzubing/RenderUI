#pragma once

#include "Widget.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace Component {

/**
 * @brief 3D 顶点数据结构
 */
struct Vertex3D {
    float x, y, z;      // 位置
    float nx, ny, nz;   // 法线
    float u, v;         // 纹理坐标
};

/**
 * @brief 3D 模型数据
 */
struct ModelData {
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    std::string texturePath;
    uint32_t textureId = 0;
    bool hasTexture = false;
};

/**
 * @brief 3D 模型渲染控件
 * 
 * 支持 OBJ/glTF 格式加载，使用 OpenGL ES 渲染
 */
class Model3D : public Widget {
public:
    Model3D();
    ~Model3D() override;
    
    /**
     * @brief 加载 3D 模型文件
     * @param path 模型文件路径（.obj 或 .gltf）
     * @return 是否成功
     */
    bool loadModel(const std::string& path);
    
    /**
     * @brief 加载纹理贴图
     */
    bool loadTexture(const std::string& path);
    
    /**
     * @brief 设置模型位置
     */
    void setPosition(float x, float y, float z);
    
    /**
     * @brief 设置模型旋转（弧度）
     */
    void setRotation(float rotationX, float rotationY, float rotationZ);
    
    /**
     * @brief 设置模型缩放
     */
    void setScale(float scaleX, float scaleY, float scaleZ);
    
    /**
     * @brief 设置自动旋转
     */
    void setAutoRotate(bool enable, float speed = 0.01f);
    
    /**
     * @brief 是否需要渲染
     */
    bool needsRender() const override { return true; }  // 3D 模型每帧渲染
    
    /**
     * @brief 获取模型数据
     */
    const ModelData& getModelData() const { return modelData_; }
    
protected:
    void onDraw(Canvas& canvas) override;
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    
    ModelData modelData_;
    
    // 变换矩阵
    float position_[3] = {0, 0, 0};
    float rotation_[3] = {0, 0, 0};
    float scale_[3] = {1, 1, 1};
    
    // OpenGL 资源
    uint32_t vao_ = 0;
    uint32_t vbo_ = 0;
    uint32_t ebo_ = 0;
    uint32_t shaderProgram_ = 0;
    
    // 自动旋转
    bool autoRotate_ = false;
    float rotateSpeed_ = 0.01f;
    float currentAngle_ = 0;
    
    bool initialized_ = false;
    
    void initOpenGL();
    void updateBuffers();
    void renderOpenGL(Canvas& canvas);
    void cleanupOpenGL();
};

} // namespace Component
