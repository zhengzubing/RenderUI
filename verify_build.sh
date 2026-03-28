#!/bin/bash

# 验证构建产物脚本（本地或远程下载）

set -e

echo "======================================"
echo "RenderUI 构建产物验证"
echo "======================================"
echo ""

BUILD_DIR="${1:-./build}"
ARTIFACTS_DIR="${2:-./build-artifacts/renderui-linux-build}"

# 检查目录
if [ ! -d "${BUILD_DIR}" ] && [ ! -d "${ARTIFACTS_DIR}" ]; then
    echo "错误：未找到构建目录"
    echo "请先运行 ./build.sh 或从 GitHub Actions 下载 artifacts"
    exit 1
fi

# 优先使用 artifacts 目录（GitHub Actions 下载）
if [ -d "${ARTIFACTS_DIR}" ]; then
    echo "使用 GitHub Actions 构建产物..."
    CHECK_DIR="${ARTIFACTS_DIR}"
else
    echo "使用本地构建产物..."
    CHECK_DIR="${BUILD_DIR}"
fi

echo "检查目录：${CHECK_DIR}"
echo ""

# 验证清单
PASS_COUNT=0
FAIL_COUNT=0

check_file() {
    local file=$1
    local desc=$2
    
    if [ -f "${file}" ]; then
        echo "✓ ${desc}"
        ls -lh "${file}" | awk '{print "  大小:", $5}'
        ((PASS_COUNT++))
    else
        echo "✗ ${desc} - 文件不存在"
        ((FAIL_COUNT++))
    fi
}

check_dir() {
    local dir=$1
    local desc=$2
    
    if [ -d "${dir}" ] && [ "$(ls -A ${dir} 2>/dev/null)" ]; then
        echo "✓ ${desc}"
        local count=$(find "${dir}" -type f | wc -l)
        echo "  文件数：${count}"
        ((PASS_COUNT++))
    else
        echo "✗ ${desc} - 目录为空或不存在"
        ((FAIL_COUNT++))
    fi
}

echo "1. 检查库文件"
echo "----------------------------------------"
check_file "${CHECK_DIR}/lib/libRenderUICore.a" "静态库 libRenderUICore.a"
check_file "${CHECK_DIR}/lib/libRenderUICore.so" "共享库 libRenderUICore.so (可选)"

echo ""
echo "2. 检查可执行文件"
echo "----------------------------------------"
if [ -d "${CHECK_DIR}/bin" ]; then
    check_dir "${CHECK_DIR}/bin" "可执行文件目录"
    
    echo ""
    echo "可执行文件列表:"
    find "${CHECK_DIR}/bin" -type f -executable -exec basename {} \; 2>/dev/null | while read exe; do
        echo "  - ${exe}"
    done
else
    echo "ℹ bin 目录不存在（可能未编译示例程序）"
fi

echo ""
echo "3. 检查头文件"
echo "----------------------------------------"
check_dir "${CHECK_DIR}/include/Component" "公共头文件目录"

if [ -d "${CHECK_DIR}/include/Component" ]; then
    echo ""
    echo "头文件列表:"
    find "${CHECK_DIR}/include/Component" -name "*.hpp" -exec basename {} \; 2>/dev/null | sort | while read header; do
        echo "  - ${header}"
    done
fi

echo ""
echo "4. 检查 CMake 配置"
echo "----------------------------------------"
check_file "${CHECK_DIR}/lib/cmake/RenderUI/RenderUIConfig.cmake" "CMake 配置文件 (可选)"
check_file "${CHECK_DIR}/lib/pkgconfig/renderui.pc" "pkg-config 文件"

echo ""
echo "5. 检查编译数据库"
echo "----------------------------------------"
if [ -f "${CHECK_DIR}/compile_commands.json" ]; then
    check_file "${CHECK_DIR}/compile_commands.json" "编译命令数据库"
else
    # 可能在 build 子目录
    if [ -f "${CHECK_DIR}/compile_commands.json" ]; then
        check_file "${CHECK_DIR}/compile_commands.json" "编译命令数据库"
    else
        echo "ℹ compile_commands.json 不存在（非致命）"
    fi
fi

echo ""
echo "6. 文件类型分析"
echo "----------------------------------------"
if [ -f "${CHECK_DIR}/lib/libRenderUICore.a" ]; then
    echo "静态库类型:"
    file "${CHECK_DIR}/lib/libRenderUICore.a" | cut -d: -f2
    
    echo ""
    echo "符号表统计:"
    nm "${CHECK_DIR}/lib/libRenderUICore.a" 2>/dev/null | grep -c " T " || echo "N/A"
    echo "  (T = 代码段符号数量)"
fi

echo ""
echo "======================================"
echo "验证总结"
echo "======================================"
echo "通过：${PASS_COUNT}"
echo "失败：${FAIL_COUNT}"
echo ""

if [ ${FAIL_COUNT} -eq 0 ]; then
    echo "✓ 所有关键组件验证通过！"
    exit 0
elif [ ${FAIL_COUNT} -le 2 ]; then
    echo "⚠ 大部分组件验证通过，部分可选组件缺失"
    exit 0
else
    echo "✗ 验证失败，缺少关键组件"
    exit 1
fi
