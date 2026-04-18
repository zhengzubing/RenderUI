# Shaders 目录

存放所有 Shader 源代码文件，便于维护和扩展。

## 文件结构

```
shaders/
├── model3d_vertex.glsl    # 3D 模型顶点 Shader
├── model3d_fragment.glsl  # 3D 模型片段 Shader
└── ...                    # 其他 Shader
```

## Shader 规范

### 版本
- OpenGL ES 3.0: `#version 300 es`
- 使用 explicit layout location（避免绑定冲突）

### 命名约定
- Vertex Shader: `{name}_vertex.glsl`
- Fragment Shader: `{name}_fragment.glsl`

### 可用 Shader 列表

#### 1. Model3D Shader (model3d_*.glsl)
- **用途**: 3D 模型渲染
- **特性**: 
  - 支持纹理贴图
  - 漫反射光照
  - 自动旋转
  
**顶点属性:**
- `aPosition` (vec3): 顶点位置
- `aNormal` (vec3): 法线
- `aTexCoord` (vec2): 纹理坐标

**Uniforms:**
- `uModelMatrix`: 模型矩阵
- `uViewMatrix`: 视图矩阵
- `uProjectionMatrix`: 投影矩阵
- `uNormalMatrix`: 法线变换矩阵
- `uTexture`: 纹理采样器
- `uHasTexture`: 是否有纹理
- `uLightDir`: 光照方向
- `uColor`: 基础颜色

#### 2. VideoStream Shader (videostream_*.glsl)
- **用途**: 视频流渲染（YUV 转 RGB）
- **特性**:
  - YUV420/YUV422 颜色转换
  - GPU 加速处理
  - 实时视频渲染
  
**顶点属性:**
- `aPosition` (vec4): 屏幕空间位置
- `aTexCoord` (vec2): 纹理坐标

**Uniforms:**
- `uTextureY`: Y 分量纹理
- `uTextureU`: U 分量纹理
- `uTextureV`: V 分量纹理

---

## 添加新 Shader

1. 在 `shaders/` 目录创建新的 `.glsl` 文件
2. 在对应的 C++ 类中加载使用
3. 更新此 README

### 示例：创建 UI 专用 Shader

**shaders/ui_vertex.glsl:**
```glsl
#version 300 es
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;

uniform mat4 uMVPMatrix;
out vec4 vColor;

void main() {
    gl_Position = uMVPMatrix * vec4(aPosition, 0.0, 1.0);
    vColor = aColor;
}
```

**shaders/ui_fragment.glsl:**
```glsl
#version 300 es
precision mediump float;
in vec4 vColor;
out vec4 fragColor;

void main() {
    fragColor = vColor;
}
```

---

## Shader 编译错误排查

如果 Shader 编译失败，日志会输出详细错误信息：

```
[ERROR] Shader compilation failed: 0:10(101): error: syntax error, unexpected ')'
```

常见问题：
1. GLSL 版本不匹配
2. 语法错误（缺少分号、括号等）
3. 使用了不支持的特性
4. Uniform/Attribute 名称不匹配

---

## 未来扩展方向

- [ ] UI 控件专用 Shader（扁平化渲染）
- [ ] 视频流 Shader（YUV 转 RGB）
- [ ] 特效 Shader（模糊、阴影、发光）
- [ ] PBR 材质 Shader（真实感渲染）
- [ ] 后处理 Shader（色调映射、抗锯齿）
