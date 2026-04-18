# 多纹理渲染架构重构说明

## 🎯 重构目标

从**单纹理架构**升级为**多纹理/图层系统**，实现真正的增量渲染和性能优化。

---

## 📊 架构对比

### **之前：单纹理架构**
```
所有 Widget → 单个 Cairo Surface → 单个 OpenGL 纹理 → 屏幕
```
**问题：**
- ❌ 任何 Widget 更新都需要重绘整个画布
- ❌ 每次都要上传整个纹理到 GPU
- ❌ 无法利用 GPU 的局部更新优势

### **现在：多纹理架构**
```
Widget A → Cairo Surface A → GL Texture A ┐
Widget B → Cairo Surface B → GL Texture B ├→ OpenGL 合成 → 屏幕
Widget C → Cairo Surface C → GL Texture C ┘
```
**优势：**
- ✅ 每个 Widget 独立渲染和缓存
- ✅ 只重绘变化的 Widget
- ✅ 只上传变化的纹理
- ✅ 支持灵活的 Z 序和空间布局

---

## 🔧 核心改动

### **1. CairoGlRenderer 重构**

#### **新增数据结构**
```cpp
struct WidgetTexture {
    GLuint textureId = 0;        // OpenGL 纹理 ID
    cairo_surface_t* surface = nullptr;  // Cairo 表面
    cairo_t* cairo = nullptr;    // Cairo 上下文
    int width = 0;
    int height = 0;
    bool dirty = true;           // 是否需要重新渲染
    
    // 屏幕位置（用于合成）
    float screenX = 0;
    float screenY = 0;
};

std::unordered_map<std::string, WidgetTexture> widgetTextures_;
```

#### **新增接口**
```cpp
// 为 Widget 创建或获取渲染上下文
cairo_t* getCairoContext(const std::string& widgetId, int width, int height);

// 标记 Widget 需要重新渲染
void markWidgetDirty(const std::string& widgetId);

// 获取 Widget 的纹理 ID（懒加载）
GLuint getWidgetTexture(const std::string& widgetId);

// 注册 Widget 的位置信息
void registerWidgetPosition(const std::string& widgetId, float x, float y, float width, float height);

// 清理特定 Widget
void cleanupWidget(const std::string& widgetId);
```

---

### **2. Widget 渲染流程修改**

#### **之前**
```cpp
void Widget::render(CairoGlRenderer& ctx) {
    auto cairo = ctx.getCairoContext();  // 共享的全局上下文
    Canvas canvas(cairo);
    cairo_translate(cairo, x_, y_);  // 手动平移
    onDraw(canvas);
}
```

#### **现在**
```cpp
void Widget::render(CairoGlRenderer& ctx) {
    // 获取独立的 Cairo 上下文
    auto cairo = ctx.getCairoContext(id_, width_, height_);
    
    if (!dirty_) return;  // 不需要重绘则跳过
    
    // 清除画布
    cairo_set_operator(cairo, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cairo);
    cairo_set_operator(cairo, CAIRO_OPERATOR_OVER);
    
    // 在 (0, 0) 绘制内容
    Canvas canvas(cairo);
    onDraw(canvas);
    
    // 标记纹理需要更新
    ctx.markWidgetDirty(id_);
}
```

---

### **3. WidgetTree 渲染逻辑**

```cpp
bool WidgetTree::updateNode(CairoGlRenderer& ctx, ...) {
    // 1. 注册位置信息（每帧都更新）
    ctx.registerWidgetPosition(widget->getId(), x, y, w, h);
    
    // 2. 只在需要时渲染
    if (widget->isDirty()) {
        widget->render(ctx);  // 渲染到独立纹理
    }
    
    // 3. 递归渲染子节点
    for (auto& child : node->children) {
        updateNode(ctx, child);
    }
}
```

---

### **4. endFrame 合成逻辑**

```cpp
void CairoGlRenderer::endFrame() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 遍历所有 Widget 纹理
    for (auto& [widgetId, wt] : widgetTextures_) {
        // 获取纹理（如果 dirty 会自动上传）
        GLuint texId = getWidgetTexture(widgetId);
        
        // 设置视口到 Widget 的位置
        glViewport(wt.screenX, screenHeight - wt.screenY - wt.height,
                   wt.width, wt.height);
        
        // 绘制纹理
        glBindTexture(GL_TEXTURE_2D, texId);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    eglSwapBuffers(display_, surface_);
}
```

---

## 🎨 渲染流程图

```
┌─────────────────────────────────────────────┐
│          WidgetTree::render()               │
│                                             │
│  for each Widget:                           │
│    1. registerWidgetPosition(id, x, y, w, h)│
│    2. if (dirty) widget->render(ctx)        │
│       └─> getCairoContext(id, w, h)        │
│       └─> onDraw(canvas)                    │
│       └─> markWidgetDirty(id)               │
└─────────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────────┐
│         CairoGlRenderer::endFrame()         │
│                                             │
│  glClear(COLOR_BUFFER_BIT)                  │
│                                             │
│  for each WidgetTexture:                    │
│    1. getWidgetTexture(id)  ← 懒加载上传    │
│       └─> if (dirty) glTexImage2D(...)      │
│    2. glViewport(x, y, w, h)                │
│    3. glDrawArrays(...)                     │
│                                             │
│  eglSwapBuffers()                           │
└─────────────────────────────────────────────┘
```

---

## ⚡ 性能优化点

### **1. 增量渲染**
- Widget 未变化时，跳过 `onDraw()` 调用
- Cairo Surface 保留上一帧内容
- 不执行任何绘制操作

### **2. 懒加载纹理上传**
- `getWidgetTexture()` 检查 `dirty` 标志
- 只有 dirty 的 Widget 才执行 `glTexImage2D`
- 静态 Widget 零 GPU 上传开销

### **3. 局部视口更新**
- `glViewport` 设置为 Widget 的实际区域
- 避免全屏绘制的浪费

---

## 📝 使用示例

### **JSON 配置（无需修改）**
```json
{
  "type": "ImageView",
  "id": "logo",
  "imagePath": "assets/logo.png",
  "x": 100,
  "y": 100,
  "width": 200,
  "height": 200
}
```

### **代码使用（无需修改）**
```cpp
auto imageView = std::make_shared<ImageView>("assets/logo.png");
imageView->setPosition(100, 100);
imageView->setSize(200, 200);
widgetTree->addRoot(imageView, "logo");
```

**完全向后兼容！** 上层代码不需要任何改动。

---

## ⚠️ 注意事项

### **1. Widget ID 必须唯一**
- 每个 Widget 必须有唯一的 `id_`
- ID 用作纹理缓存的键值

### **2. 内存占用增加**
- 每个 Widget 都有独立的 Cairo Surface 和 GL 纹理
- 对于大量小控件，需要考虑内存优化

### **3. Z 序管理**
- 当前按 `unordered_map` 的遍历顺序绘制
- 如需精确控制 Z 序，需要额外排序逻辑

---

## 🚀 后续优化方向

1. **纹理图集（Texture Atlas）**
   - 将多个小 Widget 合并到一个纹理
   - 减少 GPU 状态切换

2. **脏矩形合并**
   - 相邻的 dirty Widget 合并上传
   - 减少 `glTexImage2D` 调用次数

3. **离屏渲染缓存**
   - 复杂 Widget 预渲染到 FBO
   - 避免每帧重复绘制

4. **异步纹理上传**
   - 使用 PBO（Pixel Buffer Object）
   - 不阻塞渲染线程

---

## ✅ 测试建议

1. **功能测试**
   - 验证所有 Widget 正常显示
   - 验证位置、尺寸正确

2. **性能测试**
   - 对比单纹理和多纹理的 FPS
   - 监控 GPU 上传带宽

3. **内存测试**
   - 监控内存占用增长
   - 验证 `cleanupWidget()` 正确释放资源

---

## 📚 相关文件

- `/home/parallels/RenderUI/src/core/CairoGlRenderer.hpp`
- `/home/parallels/RenderUI/src/core/CairoGlRenderer.cpp`
- `/home/parallels/RenderUI/src/widget/Widget.cpp`
- `/home/parallels/RenderUI/src/widget/WidgetTree.cpp`

---

**重构完成日期：** 2026-04-08  
**作者：** AI Assistant
