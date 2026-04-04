# RenderUI Examples

## 📋 示例列表

### 1. UI Complete Demo (ui_demo)

**完整的 UI 控件演示** - 通过 JSON 配置展示所有基础控件和布局系统。

#### 演示内容

- ✅ **Label** - 文本标签（标题、信息文本）
- ✅ **Button** - 按钮（启用/禁用状态）
- ✅ **CheckBox** - 复选框（选中/未选中）
- ✅ **ImageView** - 图片视图
- ✅ **Container** - 容器组件
- ✅ **FlexLayout** - 弹性布局（水平/垂直）
- ✅ **AbsoluteLayout** - 绝对定位布局

#### 编译

```bash
cd build
cmake .. -G Ninja -DBUILD_EXAMPLES=ON
ninja ui_demo
```

#### 运行

```bash
# 使用默认配置
./bin/examples/ui_demo

# 使用自定义配置文件
./bin/examples/ui_demo path/to/your/config.json
```

#### 配置文件结构

查看 `config/ui_demo.json` 了解完整的 JSON 配置格式：

```json
{
  "window": {
    "title": "窗口标题",
    "width": 1024,
    "height": 768
  },
  "widgets": [
    {
      "type": "container",
      "layout": "flex",
      "direction": "vertical",
      "children": [
        {
          "type": "label",
          "text": "Hello World",
          "fontSize": 16
        },
        {
          "type": "button",
          "text": "Click Me",
          "width": 120,
          "height": 40
        }
      ]
    }
  ]
}
```

#### 支持的控件类型

| 类型 | 说明 | 必需属性 |
|------|------|---------|
| `label` | 文本标签 | `text` |
| `button` | 按钮 | `text` |
| `checkbox` | 复选框 | `text` |
| `imageview` | 图片视图 | `imagePath` |
| `container` | 容器 | `layout` |

#### 支持的布局类型

| 布局 | 说明 | 配置项 |
|------|------|--------|
| `flex` | 弹性布局 | `direction`, `spacing`, `padding` |
| `absolute` | 绝对定位 | `x`, `y`（子控件） |

#### 特性

- 🎨 **声明式 UI** - 通过 JSON 定义界面
- 🔄 **热重载** - 修改 JSON 后重新运行即可看到效果
- 📱 **响应式布局** - FlexLayout 自动适应窗口大小
- 🎯 **ID 查找** - 每个控件可设置唯一 ID，方便后续操作

---

## 🚀 快速开始

### 1. 构建项目

```bash
mkdir build && cd build
cmake .. -G Ninja -DBUILD_EXAMPLES=ON
ninja
```

### 2. 运行示例

```bash
./bin/examples/ui_demo
```

### 3. 修改配置

编辑 `examples/config/ui_demo.json`，然后重新运行：

```bash
./bin/examples/ui_demo
```

---

## 💡 提示

- 按 **ESC** 键退出程序
- 查看控制台输出了解控件树结构
- 修改 JSON 配置文件可以实时看到不同的布局效果
- 所有控件都支持嵌套，可以创建复杂的 UI 层次

---

## 📝 扩展

要添加新的示例：

1. 在 `examples/` 目录创建 `.cpp` 文件
2. 在 `examples/CMakeLists.txt` 中添加新的 `add_executable`
3. 重新构建项目

例如：

```cmake
add_executable(my_example
    my_example.cpp
)

target_link_libraries(my_example
    RenderUICore
)

set_target_properties(my_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
)
```

---

**最后更新**: 2026-04-04
