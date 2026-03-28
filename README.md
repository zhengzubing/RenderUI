# RenderUI

现代化控件渲染框架 - Modern UI Widget Rendering Framework

## 🎯 项目简介

RenderUI 是一个面向车载嵌入式系统的高性能控件渲染框架，采用现代 C++20 开发，基于 Wayland/Weston 显示协议，使用 OpenGL ES 硬件加速渲染。

### 核心特性

- **简洁易用**: API 设计参考 Qt 风格，组件采用 `Component/ClassName` 命名
- **高性能**: GPU 加速、异步纹理上传、PBO 双缓冲
- **按需渲染**: 静态控件仅在更新时渲染，视频流每帧渲染
- **现代 C++**: 智能指针、避免复杂模板和宏
- **分层清晰**: 6 层架构设计，问题内部消化

## 📋 技术栈

### 基础技术栈（阶段一至三）

| 类别 | 技术 |
|------|------|
| 语言 | C++20 |
| 构建 | CMake + Ninja |
| 显示 | Wayland/Weston |
| 渲染 | OpenGL ES 3.0+ |
| 2D 绘图 | Cairo |
| 信号槽 | sigslot (轻量级 C++14 实现) |
| JSON | nlohmann/json |
| 日志 | plog |
| 模型加载 | tinyobjloader / tinyply |
| 图像加载 | stb_image |
| IPC | DBus |

### 扩展技术栈（阶段四起）

- **2D 渲染**: Skia（GPU 加速）
- **网络**: muduo
- **存储**: LevelDB
- **3D 模型**: assimp
- **测试**: GoogleTest
- **性能分析**: Perfetto Trace

## 🚀 快速开始

### 环境要求

- **操作系统**: Linux (ARM64/x86_64)
- **编译器**: GCC 10+ 或 Clang 12+
- **CMake**: 3.20+
- **依赖**: Wayland, Cairo, OpenGL ES, EGL, DBus

### 安装依赖

**Ubuntu 22.04 / Debian 11+**

```bash
sudo apt install cmake ninja-build pkg-config \
  libwayland-dev wayland-protocols \
  libcairo2-dev \
  libgles2-mesa-dev libegl1-mesa-dev \
  libdbus-1-dev
```

**Fedora 36+**

```bash
sudo dnf install cmake ninja-build pkg-config \
  wayland-devel wayland-protocols-devel \
  cairo-devel \
  mesa-libGLES-devel mesa-libEGL-devel \
  dbus-devel
```

### 构建项目

#### 方法一：使用构建脚本（推荐）

```bash
# 基础构建
./build.sh

# Debug 模式
./build.sh --debug

# 启用 Skia GPU 加速（阶段四）
./build.sh --skia

# 清理构建
./build.sh --clean
```

#### 方法二：手动构建

```bash
mkdir build && cd build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release

ninja
```

### Docker 开发环境

```bash
# 构建开发镜像
docker-compose build

# 启动容器
docker-compose up -d

# 进入容器开发
docker-compose exec renderui-dev bash

# 在容器内构建
cd /workspace/RenderUI
./build.sh
```

## 📁 项目结构

```
RenderUI/
├── CMakeLists.txt          # CMake 配置
├── build.sh                # 构建脚本
├── Dockerfile              # Docker 开发环境
├── docker-compose.yml      # Docker Compose 配置
├── include/Component/      # 公共头文件
│   ├── Application.hpp
│   ├── Widget.hpp
│   ├── Button.hpp
│   └── ...
├── src/                    # 源代码
│   ├── core/               # 核心模块
│   ├── window/             # 窗口管理
│   ├── widget/             # 控件系统
│   │   ├── widgets/        # 具体控件实现
│   │   └── events/         # 事件处理
│   ├── layout/             # 布局引擎
│   ├── render/             # 渲染后端
│   ├── resource/           # 资源管理
│   ├── data/               # 数据存储
│   ├── ipc/                # IPC 通信
│   └── utils/              # 工具函数
├── examples/               # 示例程序
├── tests/                  # 单元测试
└── docs/                   # 文档
    ├── requirements/       # 需求文档
    ├── architecture/       # 架构设计
    └── diagrams/           # PlantUML 图表
```

## 📖 开发阶段

| 阶段 | 主题 | 状态 |
|------|------|------|
| 一 | 基础窗口与渲染 | 🟢 进行中 |
| 二 | 控件系统与布局 | ⚪ 待开始 |
| 三 | 高级功能（3D/多窗口） | ⚪ 待开始 |
| 四 | GPU 性能优化 | ⚪ 待开始 |
| 五 | 工程化与测试 | ⚪ 待开始 |
| 六 | 可视化工具 | ⚪ 待开始 |
| 七 | 迭代与维护 | ⚪ 待开始 |

详细规划见 [PRD 文档](docs/requirements/PRD.md)

## 📚 文档

- [产品需求文档 (PRD)](docs/requirements/PRD.md)
- [系统架构设计](docs/architecture/SYSTEM_DESIGN.md)
- [渲染流程图](docs/diagrams/)

## 🔧 编码规范

- **代码风格**: Google C++ Style
- **命名规范**: `Component/ClassName` 形式（如 `Component/Button`）
- **注释语言**: 中文
- **智能指针**: 优先使用 `std::unique_ptr`, `std::shared_ptr`
- **头文件保护**: `#pragma once`
- **继承深度**: 不超过 3 层

## 📝 示例代码

### 创建简单窗口

```cpp
#include <Component/Application.hpp>
#include <Component/Widget.hpp>
#include <Component/Label.hpp>

int main() {
    auto app = Component::Application::instance();
    
    if (!app->init("Hello RenderUI", 1920, 1080)) {
        return -1;
    }
    
    // TODO: 添加控件
    
    return app->run();
}
```

## 🤝 参与贡献

欢迎提交 Issue 和 Pull Request！

## 📄 许可证

本项目采用**双许可模式**：

- **个人非商业用途**: 免费使用
- **商业用途**: 需购买商业授权（非企业版售卖，仅为授权许可）

详细条款请查看 [LICENSE](LICENSE) 文件。

**注意**: 项目开发初期（v1.0 前）仅接受 Bug 修复类贡献，暂不接受功能需求类代码合入。

---

**最后更新**: 2026-03-29
