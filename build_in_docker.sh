#!/bin/bash

# 在 Docker 中构建 RenderUI 的脚本

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

echo "======================================"
echo "RenderUI Docker 构建"
echo "======================================"
echo ""

# 检查 Docker 是否运行
if ! docker info >/dev/null 2>&1; then
    echo "错误：Docker 未运行或未安装"
    echo ""
    echo "请先："
    echo "1. 打开 Docker.app"
    echo "2. 等待 Docker 启动完成（菜单栏出现小鲸鱼图标）"
    echo "3. 重新运行此脚本"
    exit 1
fi

echo "✓ Docker 正在运行"
echo ""

# 检查是否已有镜像
if docker images renderui-dev:latest | grep -q renderui-dev; then
    echo "发现已有镜像，跳过构建..."
else
    echo "步骤 1/4: 构建 Docker 镜像..."
    docker-compose build
fi

echo ""
echo "步骤 2/4: 启动 Docker 容器..."
docker-compose up -d

echo ""
echo "步骤 3/4: 等待容器启动..."
sleep 3

echo ""
echo "步骤 4/4: 在容器内执行构建..."
docker-compose exec -T renderui-dev bash -c "
    cd /workspace/RenderUI
    echo ''
    echo '=========================================='
    echo 'RenderUI Build in Docker'
    echo '=========================================='
    echo ''
    
    # 执行构建
    ./build.sh
    
    # 验证构建结果
    echo ''
    echo '=========================================='
    echo '验证构建产物'
    echo '=========================================='
    
    if [ -f build/lib/libRenderUICore.a ]; then
        echo '✓ 静态库创建成功:'
        ls -lh build/lib/libRenderUICore.a
    else
        echo '✗ 静态库创建失败'
        exit 1
    fi
    
    echo ''
    echo '构建产物列表:'
    find build/lib -type f -name '*.a' -o -name '*.so' 2>/dev/null | head -10
    find build/bin -type f -executable 2>/dev/null | head -10
    
    echo ''
    echo '=========================================='
    echo '✓ Docker 内构建成功！'
    echo '=========================================='
"

echo ""
echo "=========================================="
echo "构建完成！"
echo "=========================================="
echo ""
echo "容器状态：运行中"
echo "查看容器日志：docker-compose logs"
echo "进入容器交互：docker-compose exec renderui-dev bash"
echo "停止容器：docker-compose down"
echo ""
