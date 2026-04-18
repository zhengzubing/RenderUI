#!/bin/bash

# RenderUI 物理机环境配置脚本
# 用于在 Ubuntu 22.04+ 上安装所有构建依赖

set -e

echo "======================================"
echo "RenderUI 物理机环境配置"
echo "======================================"
echo ""

# 检查是否为 root 用户
if [ "$EUID" -ne 0 ]; then 
    echo "错误：请使用 sudo 运行此脚本"
    echo "用法: sudo $0"
    exit 1
fi

# 检测系统版本
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VERSION=$VERSION_ID
    echo "检测到操作系统: $OS $VERSION"
else
    echo "警告: 无法检测操作系统版本"
fi

echo ""
echo "步骤 1/6: 更新软件包列表..."
apt-get update

echo ""
echo "步骤 2/6: 安装基础构建工具..."
apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    curl \
    wget \
    vim

echo ""
echo "步骤 3/6: 安装 Wayland 显示协议依赖..."
apt-get install -y \
    libwayland-dev \
    wayland-protocols \
    libxkbcommon-dev

echo ""
echo "步骤 4/6: 安装 Cairo 和图形库..."
apt-get install -y \
    libcairo2-dev \
    libpixman-1-dev \
    libpng-dev \
    libfreetype6-dev

echo ""
echo "步骤 5/6: 安装 OpenGL ES 和 EGL..."
apt-get install -y \
    libgles2-mesa-dev \
    libegl1-mesa-dev \
    libgbm-dev

echo ""
echo "步骤 6/6: 安装 DBus IPC 通信..."
apt-get install -y \
    libdbus-1-dev \
    dbus-x11 \
    libglib2.0-dev

# 清理 apt 缓存
apt-get clean
rm -rf /var/lib/apt/lists/*

echo ""
echo "======================================"
echo "✓ 所有依赖安装完成！"
echo "======================================"
echo ""
echo "验证安装:"
echo ""

# 验证关键组件
check_package() {
    local pkg=$1
    local desc=$2
    
    if dpkg -l | grep -q "^ii.*${pkg}"; then
        echo "✓ ${desc}"
    else
        echo "✗ ${desc} - 未找到"
    fi
}

check_package "cmake" "CMake"
check_package "ninja-build" "Ninja"
check_package "libwayland-dev" "Wayland"
check_package "libcairo2-dev" "Cairo"
check_package "libgles2-mesa-dev" "OpenGL ES"
check_package "libegl1-mesa-dev" "EGL"
check_package "libdbus-1-dev" "DBus"

echo ""
echo "下一步操作:"
echo "1. 克隆项目（如果还没有）:"
echo "   git clone https://github.com/zhengzubing/RenderUI.git"
echo "   cd RenderUI"
echo ""
echo "2. 构建项目:"
echo "   ./build.sh"
echo ""
echo "3. 或者手动构建:"
echo "   mkdir build && cd build"
echo "   cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release"
echo "   ninja"
echo ""
echo "4. 运行示例程序:"
echo "   ./build/bin/renderui_demo"
echo ""
