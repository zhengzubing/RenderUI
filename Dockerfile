# RenderUI 开发环境
# 基于 Ubuntu 22.04，包含所有构建依赖

FROM ubuntu:22.04

# 避免交互式提示
ENV DEBIAN_FRONTEND=noninteractive

# 安装基础工具
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget \
    vim \
    && rm -rf /var/lib/apt/lists/*

# 安装 Wayland 依赖
RUN apt-get update && apt-get install -y \
    libwayland-dev \
    wayland-protocols \
    libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装 Cairo 和图形库
RUN apt-get update && apt-get install -y \
    libcairo2-dev \
    libpixman-1-dev \
    libpng-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装 OpenGL ES 和 EGL
RUN apt-get update && apt-get install -y \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    libgbm-dev \
    && rm -rf /var/lib/apt/lists/*

# 安装 DBus
RUN apt-get update && apt-get install -y \
    libdbus-1-dev \
    dbus-x11 \
    && rm -rf /var/lib/apt/lists/*

# 安装可选依赖（阶段四）
# RUN apt-get update && apt-get install -y \
#     libskia-dev \
#     libleveldb-dev \
#     libassimp-dev \
#     && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /workspace

# 克隆项目（或使用本地挂载）
# COPY . /workspace/RenderUI

# 默认命令
CMD ["/bin/bash"]
