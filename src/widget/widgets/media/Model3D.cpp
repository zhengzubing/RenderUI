#include "Model3D.hpp"
#include "RenderContext.hpp"
#include "Logger.hpp"
#include "ResourceManager.hpp"
#include <GLES2/gl2.h>
#include <tiny_obj_loader.h>
#include <cstring>

namespace Component {

// 3D 模型渲染 Shader
static const char* sVertexShaderSource = R"(
    attribute vec3 aPosition;
    attribute vec3 aNormal;
    attribute vec2 aTexCoord;
    
    uniform mat4 uModelMatrix;
    uniform mat4 uViewMatrix;
    uniform mat4 uProjectionMatrix;
    uniform mat4 uNormalMatrix;
    
    varying vec3 vNormal;
    varying vec2 vTexCoord;
    varying vec3 vFragPos;
    
    void main() {
        vec4 worldPos = uModelMatrix * vec4(aPosition, 1.0);
        vFragPos = vec3(worldPos);
        vNormal = mat3(uNormalMatrix) * aNormal;
        vTexCoord = aTexCoord;
        
        gl_Position = uProjectionMatrix * uViewMatrix * worldPos;
    }
)";

static const char* sFragmentShaderSource = R"(
    precision mediump float;
    
    varying vec3 vNormal;
    varying vec2 vTexCoord;
    varying vec3 vFragPos;
    
    uniform sampler2D uTexture;
    uniform bool uHasTexture;
    uniform vec3 uLightDir;
    uniform vec3 uColor;
    
    void main() {
        // 环境光
        vec3 ambient = 0.2 * uColor;
        
        // 漫反射
        vec3 norm = normalize(vNormal);
        vec3 lightDir = normalize(uLightDir);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * uColor;
        
        vec3 result = ambient + diffuse;
        
        if (uHasTexture) {
            vec3 texColor = texture2D(uTexture, vTexCoord).rgb;
            result *= texColor;
        }
        
        gl_FragColor = vec4(result, 1.0);
    }
)";

struct Model3D::Impl {
    bool loaded = false;
};

Model3D::Model3D() {
    impl_ = std::make_unique<Impl>();
}

Model3D::~Model3D() {
    cleanupOpenGL();
}

bool Model3D::loadModel(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        LOG_ERROR << "Failed to load OBJ model: " << path << " - " << err;
        return false;
    }
    
    if (!warn.empty()) {
        LOG_WARNING << "OBJ loader warning: " << warn;
    }
    
    // 转换顶点数据
    modelData_.vertices.clear();
    modelData_.indices.clear();
    
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex3D vertex;
            
            vertex.x = attrib.vertices[3 * index.vertex_index + 0];
            vertex.y = attrib.vertices[3 * index.vertex_index + 1];
            vertex.z = attrib.vertices[3 * index.vertex_index + 2];
            
            if (index.normal_index >= 0) {
                vertex.nx = attrib.normals[3 * index.normal_index + 0];
                vertex.ny = attrib.normals[3 * index.normal_index + 1];
                vertex.nz = attrib.normals[3 * index.normal_index + 2];
            } else {
                vertex.nx = vertex.ny = vertex.nz = 0;
            }
            
            if (index.texcoord_index >= 0) {
                vertex.u = attrib.texcoords[2 * index.texcoord_index + 0];
                vertex.v = attrib.texcoords[2 * index.texcoord_index + 1];
            } else {
                vertex.u = vertex.v = 0;
            }
            
            modelData_.vertices.push_back(vertex);
            modelData_.indices.push_back(static_cast<uint32_t>(modelData_.vertices.size()) - 1);
        }
    }
    
    impl_->loaded = true;
    initOpenGL();
    
    LOG_INFO << "3D model loaded: " << path << " (" << modelData_.vertices.size() << " vertices, " << modelData_.indices.size() << " indices)";
    
    return true;
}

bool Model3D::loadTexture(const std::string& path) {
    auto surface = ResourceManager::instance().loadImage(path);
    if (!surface) {
        return false;
    }
    
    // 创建 OpenGL 纹理
    glGenTextures(1, &modelData_.textureId);
    glBindTexture(GL_TEXTURE_2D, modelData_.textureId);
    
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    unsigned char* data = cairo_image_surface_get_data(surface);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, 
                 GL_BGRA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    modelData_.hasTexture = true;
    modelData_.texturePath = path;
    
    LOG_INFO << "3D model texture loaded: " << path;
    return true;
}

void Model3D::setPosition(float x, float y, float z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
    markDirty();
}

void Model3D::setRotation(float rotationX, float rotationY, float rotationZ) {
    rotation_[0] = rotationX;
    rotation_[1] = rotationY;
    rotation_[2] = rotationZ;
    markDirty();
}

void Model3D::setScale(float scaleX, float scaleY, float scaleZ) {
    scale_[0] = scaleX;
    scale_[1] = scaleY;
    scale_[2] = scaleZ;
    markDirty();
}

void Model3D::setAutoRotate(bool enable, float speed) {
    autoRotate_ = enable;
    rotateSpeed_ = speed;
}

void Model3D::initOpenGL() {
    if (initialized_) {
        return;
    }
    
    // 创建 VAO/VBO/EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    
    glBindVertexArray(vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, 
                 modelData_.vertices.size() * sizeof(Vertex3D),
                 modelData_.vertices.data(),
                 GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 modelData_.indices.size() * sizeof(uint32_t),
                 modelData_.indices.data(),
                 GL_STATIC_DRAW);
    
    // 设置顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    // 编译 Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &sVertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &sFragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    
    shaderProgram_ = glCreateProgram();
    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    initialized_ = true;
    updateBuffers();
}

void Model3D::updateBuffers() {
    if (!initialized_) {
        return;
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, 
                    modelData_.vertices.size() * sizeof(Vertex3D),
                    modelData_.vertices.data());
}

void Model3D::renderOpenGL(Canvas& canvas) {
    if (!initialized_ || !impl_->loaded) {
        return;
    }
    
    // 自动旋转
    if (autoRotate_) {
        currentAngle_ += rotateSpeed_;
        rotation_[1] = currentAngle_;
    }
    
    glUseProgram(shaderProgram_);
    
    // 设置变换矩阵（简化版本，实际需要使用 glm 库）
    // TODO: 使用 glm 构建完整的模型 - 视图 - 投影矩阵
    
    // 绑定 VAO
    glBindVertexArray(vao_);
    
    // 绑定纹理
    if (modelData_.hasTexture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, modelData_.textureId);
        glUniform1i(glGetUniformLocation(shaderProgram_, "uTexture"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram_, "uHasTexture"), GL_TRUE);
    } else {
        glUniform1i(glGetUniformLocation(shaderProgram_, "uHasTexture"), GL_FALSE);
    }
    
    // 设置光照方向
    glUniform3f(glGetUniformLocation(shaderProgram_, "uLightDir"), 0.5f, 1.0f, 0.3f);
    
    // 设置颜色
    glUniform3f(glGetUniformLocation(shaderProgram_, "uColor"), 0.8f, 0.8f, 0.8f);
    
    // 绘制
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(modelData_.indices.size()), 
                   GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

void Model3D::onDraw(Canvas& canvas) {
    renderOpenGL(canvas);
}

void Model3D::cleanupOpenGL() {
    if (vao_ != 0) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    
    if (vbo_ != 0) {
        glDeleteBuffers(1, &vbo_);
        vbo_ = 0;
    }
    
    if (ebo_ != 0) {
        glDeleteBuffers(1, &ebo_);
        ebo_ = 0;
    }
    
    if (shaderProgram_ != 0) {
        glDeleteProgram(shaderProgram_);
        shaderProgram_ = 0;
    }
    
    initialized_ = false;
}

} // namespace Component
