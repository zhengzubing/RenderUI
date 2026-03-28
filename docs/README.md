# RenderUI 文档中心

本目录包含 RenderUI 控件渲染框架的完整设计文档。

## 📁 文档目录结构

```
docs/
├── README.md                 # 本文档（索引）
├── requirements/             # 需求文档
│   └── PRD.md               # 产品需求文档
├── architecture/             # 架构设计
│   └── SYSTEM_DESIGN.md     # 系统架构设计文档
└── diagrams/                 # PlantUML 图表
    ├── architecture_layers.plantuml   # 系统分层架构图
    ├── module_diagram.plantuml        # 模块依赖图
    ├── render_flow.plantuml           # 静态控件渲染流程图
    └── video_render_flow.plantuml     # 视频流渲染流程图
```

## 📖 文档说明

### 1. 产品需求文档 (PRD)

**位置**: [`requirements/PRD.md`](requirements/PRD.md)

**内容概览**:
- 项目概述与设计目标
- 完整功能需求列表（窗口管理、控件系统、布局、事件等）
- 非功能需求（性能指标、编码规范）
- 7 个开发阶段规划
- 车载全景 360 影像 Demo 定义
- 风险评估与验收标准

**适合读者**: 产品经理、架构师、开发人员

---

### 2. 系统架构设计文档

**位置**: [`architecture/SYSTEM_DESIGN.md`](architecture/SYSTEM_DESIGN.md)

**内容概览**:
- 分层架构设计（6 层架构）
- 核心模块划分与类设计
- 渲染流程详解（静态控件按需渲染、视频流每帧渲染）
- 数据结构设计（场景图、JSON 配置格式）
- 关键技术实现（Wayland 窗口、YUV 纹理上传、DBus 信号处理）
- 性能优化方案
- 调试工具设计
- 扩展性设计
- 构建系统说明

**适合读者**: 架构师、高级开发人员

---

### 3. PlantUML 图表集

**位置**: [`diagrams/`](diagrams/)

#### 3.1 系统分层架构图
**文件**: `architecture_layers.plantuml`

展示 RenderUI 的 6 层架构：
- Application Layer（应用层）
- Widget Layer（控件层）
- Layout Engine（布局引擎）
- Render Backend（渲染后端）
- Window System（窗口系统）
- Platform Layer（平台层）

#### 3.2 模块依赖图
**文件**: `module_diagram.plantuml`

展示各模块之间的依赖关系，包括：
- core、window、widget、layout、render、resource、data、ipc、utils 等模块

#### 3.3 渲染流程图
**文件**: `render_flow.plantuml`

展示静态控件的按需渲染完整流程：
```
应用更新 → 控件树 → 布局计算 → Cairo 绘制 → OpenGL 纹理上传 → EGL 交换 → Weston 合成
```

#### 3.4 视频流渲染流程图
**文件**: `video_render_flow.plantuml`

展示视频流每帧渲染的实时流程：
```
Camera 推帧 → YUV 纹理上传 → Shader 转换 → OpenGL 渲染 → EGL 提交 → Weston 显示
```

**查看方法**:
```bash
# 使用 PlantUML 生成 PNG
plantuml diagrams/*.plantuml

# 或使用 VSCode 插件预览
# 安装 PlantUML 插件后直接打开 .plantuml 文件
```

---

## 🎯 快速开始

### 新成员阅读顺序

1. **先读 PRD** → 了解项目目标和功能需求
2. **再读架构文档** → 理解系统设计和分层
3. **查看图表** → 直观理解渲染流程和模块关系

### 开发阶段参考

| 阶段 | 主题 | 关键文档 |
|------|------|----------|
| 一 | 基础窗口与渲染 | PRD 第四章、架构文档 3.1 节 |
| 二 | 控件系统与布局 | PRD 2.2-2.4 节、架构文档 2.2 节 |
| 三 | 高级功能 | PRD 2.2.2 节、架构文档 5.2 节 |
| 四 | 性能优化 | 架构文档第 6 章 |
| 五 | 工程化 | PRD 2.8-2.9 节 |
| 六 | 可视化工具 | - |
| 七 | 迭代维护 | - |

---

## 🛠️ 技术栈参考

### 基础技术栈（阶段一至三）
- **语言**: C++17/20
- **构建**: CMake + Ninja
- **显示**: Wayland/Weston
- **渲染**: OpenGL ES 3.0+
- **2D 绘图**: Cairo
- **信号槽**: CppSoft_Signals
- **JSON**: nlohmann/json
- **日志**: plog

### 扩展技术栈（阶段四起）
- **2D 渲染**: Skia（GPU 加速）
- **网络**: muduo
- **存储**: LevelDB
- **3D 模型**: assimp
- **测试**: GoogleTest
- **性能分析**: Perfetto Trace

---

## 📝 编码规范要点

- **代码风格**: Google C++ Style
- **注释语言**: 中文
- **命名规范**: Qt 风格（如 `QPushButton`, `addWidget()`）
- **智能指针**: 优先使用 `std::unique_ptr`, `std::shared_ptr`
- **头文件保护**: `#pragma once`
- **继承深度**: 不超过 3 层
- **模板复杂度**: 仅使用易理解的泛型

---

## 🔗 相关链接

- [GitHub 仓库](https://github.com/zhengzubing/RenderUI)
- [Wayland 协议文档](https://wayland.freedesktop.org/docs/html/)
- [OpenGL ES 参考](https://www.khronos.org/opengles/sdk/docs/man3/)
- [Cairo 图形库](https://www.cairographics.org/)
- [Skia 图形库](https://skia.org/)

---

## 📞 问题反馈

如有文档相关问题，请在 GitHub 提交 Issue。

**最后更新**: 2026-03-29
