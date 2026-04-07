#!/bin/bash

# RenderUI 构建脚本
# 支持本地构建和 Docker 环境构建

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="Release"
CMAKE_EXTRA_ARGS=""

# 解析参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --skia)
            CMAKE_EXTRA_ARGS+=" -DUSE_SKIA=ON"
            shift
            ;;
        --perfetto)
            CMAKE_EXTRA_ARGS+=" -DENABLE_PERFETTO=ON"
            shift
            ;;
        --clean)
            rm -rf "${BUILD_DIR}"
            echo "Build directory cleaned"
            exit 0
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug      Build in Debug mode (default: Release)"
            echo "  --skia       Enable Skia GPU acceleration (Stage 4+)"
            echo "  --perfetto   Enable Perfetto tracing"
            echo "  --clean      Clean build directory"
            echo "  --help       Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# 检查依赖
check_dependencies() {
    echo "Checking dependencies..."
    
    local MISSING_DEPS=()
    
    # 检查基础依赖
    command -v cmake >/dev/null 2>&1 || MISSING_DEPS+=("cmake")
    command -v ninja >/dev/null 2>&1 || MISSING_DEPS+=("ninja")
    command -v pkg-config >/dev/null 2>&1 || MISSING_DEPS+=("pkg-config")
    
    # 检查 Wayland 依赖
    pkg-config --exists wayland-client || MISSING_DEPS+=("wayland-dev")
    pkg-config --exists wayland-protocols || MISSING_DEPS+=("wayland-protocols")
    
    # 检查 Cairo
    pkg-config --exists cairo || MISSING_DEPS+=("libcairo2-dev")
    
    # 检查 OpenGL ES
    pkg-config --exists glesv2 || MISSING_DEPS+=("libgles2-mesa-dev")
    
    # 检查 EGL
    pkg-config --exists egl || MISSING_DEPS+=("libegl1-mesa-dev")
    
    # 检查 DBus
    pkg-config --exists dbus-1 || MISSING_DEPS+=("libdbus-1-dev")
    
    if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
        echo "Missing dependencies: ${MISSING_DEPS[*]}"
        echo ""
        echo "Install on Ubuntu/Debian:"
        echo "  sudo apt install cmake ninja-build pkg-config \\"
        echo "    wayland-protocols libwayland-dev \\"
        echo "    libcairo2-dev \\"
        echo "    libgles2-mesa-dev libegl1-mesa-dev \\"
        echo "    libdbus-1-dev"
        echo ""
        echo "Install on Fedora:"
        echo "  sudo dnf install cmake ninja-build pkg-config \\"
        echo "    wayland-devel wayland-protocols-devel \\"
        echo "    cairo-devel \\"
        echo "    mesa-libGLES-devel mesa-libEGL-devel \\"
        echo "    dbus-devel"
        exit 1
    fi
    
    echo "All dependencies found."
}

# 配置和构建
build() {
    echo ""
    echo "=========================================="
    echo "RenderUI Build Configuration"
    echo "=========================================="
    echo "Build Type: ${BUILD_TYPE}"
    echo "Build Dir:  ${BUILD_DIR}"
    echo "Extra Args: ${CMAKE_EXTRA_ARGS:-none}"
    echo ""
    
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    
    echo "Running CMake..."
    cmake .. \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        ${CMAKE_EXTRA_ARGS}
    
    echo ""
    echo "Building..."
    ninja -j$(nproc)
    
    echo ""
    echo "=========================================="
    echo "Build completed successfully!"
    echo "=========================================="
    echo ""
    echo "Binaries location: ${BUILD_DIR}/bin/"
    echo "Libraries location: ${BUILD_DIR}/lib/"
    echo ""
    echo "To run examples:"
    echo "  cd ${BUILD_DIR}/bin && ./ui_demo"
    echo ""
}

# 主流程
main() {
    check_dependencies
    build
}

main "$@"
