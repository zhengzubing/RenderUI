#include "Component/Model3D.hpp"

namespace Component {

struct Model3D::Impl {
    std::string modelPath;
    float position[3] = {0, 0, 0};
    float rotation = 0;
    bool loaded = false;
};

bool Model3D::loadModel(const std::string& path) {
    impl_ = std::make_unique<Impl>();
    impl_->modelPath = path;
    
    // TODO: 使用 tinyobjloader 或 tinyply 加载模型
    
    impl_->loaded = true;
    return true;
}

void Model3D::setTransform(float x, float y, float z, float rotation) {
    if (impl_) {
        impl_->position[0] = x;
        impl_->position[1] = y;
        impl_->position[2] = z;
        impl_->rotation = rotation;
    }
}

} // namespace Component
