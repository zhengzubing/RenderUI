#include "Model3D.hpp"
#include "RenderContext.hpp"
#include "Logger.hpp"
#include "ResourceManager.hpp"
#include <GLES3/gl3.h>
#include <tiny_obj_loader.h>
#include <cstring>
#include <fstream>
#include <sstream>

namespace Component {

// Shader 文件路径
static const std::string sVertexShaderPath = "shaders/model3d_vertex.glsl";
static const std::string sFragmentShaderPath = "shaders/model3d_fragment.glsl";

// 从文件加载 Shader 源码
static std::string loadShaderSource(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_E << "Failed to open shader file: " << path;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 编译 Shader 并检查错误
static GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    // 检查编译状态
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOG_E << "Shader compilation failed: " << log;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

// 链接 Shader 程序并检查错误
static GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // 检查链接状态
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOG_E << "Shader program linking failed: " << log;
        glDeleteProgram(program);
        return 0;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

Model3D::Model3D() = default;

Model3D::~Model3D() {
    cleanupOpenGL();
}

bool Model3D::loadModel(const std::string& path) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        LOG_E << "Failed to load OBJ model: " << path << " - " << err;
        return false;
    }
    
    if (!warn.empty()) {
        LOG_W << "OBJ loader warning: " << warn;
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
    
    loaded_ = true;
    initOpenGL();
    
    LOG_I << "3D model loaded: " << path << " (" << modelData_.vertices.size() << " vertices, " << modelData_.indices.size() << " indices)";
    
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
                 GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    modelData_.hasTexture = true;
    modelData_.texturePath = path;
    
    LOG_I << "3D model texture loaded: " << path;
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
    
    // 加载并编译 Shader
    std::string vertexSource = loadShaderSource(sVertexShaderPath);
    std::string fragmentSource = loadShaderSource(sFragmentShaderPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        LOG_E << "Failed to load shader sources";
        return;
    }
    
    // 编译 Shader
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        LOG_E << "Failed to compile shaders";
        return;
    }
    
    // 链接 Shader 程序
    shaderProgram_ = linkShaderProgram(vertexShader, fragmentShader);
    
    if (shaderProgram_ == 0) {
        LOG_E << "Failed to link shader program";
        return;
    }
    
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
    if (!initialized_ || !loaded_) {
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
