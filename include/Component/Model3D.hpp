#pragma once

#include <string>
#include <memory>

namespace Component {

/**
 * @brief 3D 模型渲染控件
 * 
 * 支持 OBJ/glTF 格式加载
 */
class Model3D {
public:
    Model3D() = default;
    ~Model3D() = default;
    
    /**
     * @brief 加载 3D 模型文件
     * @param path 模型文件路径
     * @return 是否成功
     */
    bool loadModel(const std::string& path);
    
    /**
     * @brief 设置模型变换
     */
    void setTransform(float x, float y, float z, float rotation);
    
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Component
