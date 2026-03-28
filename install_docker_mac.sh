#!/bin/bash

# macOS Docker 安装脚本
# 需要输入一次 sudo 密码

set -e

echo "======================================"
echo "Docker Desktop for Mac 安装脚本"
echo "======================================"
echo ""

# 检查是否是 Apple Silicon
if [[ $(uname -m) == 'arm64' ]]; then
    echo "检测到 Apple Silicon (M1/M2) 芯片"
    DOCKER_URL="https://desktop.docker.com/mac/main/arm64/Docker.dmg"
else
    echo "检测到 Intel 芯片"
    DOCKER_URL="https://desktop.docker.com/mac/main/amd64/Docker.dmg"
fi

echo ""
echo "下载地址：${DOCKER_URL}"
echo ""

# 创建临时目录
TEMP_DIR=$(mktemp -d)
DMG_PATH="${TEMP_DIR}/Docker.dmg"

echo "步骤 1/5: 下载 Docker Desktop..."
curl -L "${DOCKER_URL}" -o "${DMG_PATH}" --progress-bar

echo ""
echo "步骤 2/5: 挂载 DMG..."
hdiutil attach "${DMG_PATH}" -mountpoint /Volumes/Docker

echo ""
echo "步骤 3/5: 复制到 Applications..."
sudo cp -R /Volumes/Docker/Docker.app /Applications/

echo ""
echo "步骤 4/5: 卸载 DMG..."
hdiutil detach /Volumes/Docker

echo ""
echo "步骤 5/5: 清理临时文件..."
rm -rf "${TEMP_DIR}"

echo ""
echo "======================================"
echo "✓ Docker Desktop 安装完成！"
echo "======================================"
echo ""
echo "下一步操作："
echo "1. 打开 /Applications/Docker.app"
echo "2. 等待 Docker 启动完成（菜单栏出现小鲸鱼图标）"
echo "3. 回到终端，运行：./build_in_docker.sh"
echo ""

# 尝试打开 Docker
open /Applications/Docker.app || echo "请手动打开 Docker.app"
