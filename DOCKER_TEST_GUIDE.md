# Docker 开发环境测试指南

## 📋 前提条件

### 1. 安装 Docker

#### macOS / Windows
下载并安装 [Docker Desktop](https://www.docker.com/products/docker-desktop/)

#### Ubuntu / Debian
```bash
# 添加 Docker 官方仓库
sudo apt-get update
sudo apt-get install \
    ca-certificates \
    curl \
    gnupg \
    lsb-release

# 添加 GPG 密钥
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

# 添加仓库
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 安装 Docker Engine
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io

# 启动 Docker
sudo systemctl start docker

# 验证安装
docker --version
```

#### Fedora
```bash
sudo dnf -y install dnf-plugins-core
sudo dnf config-manager --add-repo https://download.docker.com/linux/fedora/docker-ce.repo
sudo dnf install docker-ce docker-ce-cli containerd.io
sudo systemctl start docker
```

### 2. 安装 Docker Compose

#### 方法一：使用 Docker Desktop（推荐）
Docker Desktop 已包含 Docker Compose

#### 方法二：单独安装
```bash
# Linux
sudo curl -L "https://github.com/docker/compose/releases/latest/download/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
sudo chmod +x /usr/local/bin/docker-compose

# 验证安装
docker-compose --version
```

**注意**: 新版本的 Docker 使用 `docker compose` (无连字符) 命令

---

## 🚀 测试步骤

### 步骤 1: 克隆项目

```bash
git clone https://github.com/zhengzubing/RenderUI.git
cd RenderUI
```

### 步骤 2: 构建 Docker 镜像

```bash
# 使用 docker-compose (旧版本)
docker-compose build

# 或使用 docker compose (新版本)
docker compose build
```

**预期输出**:
```
[+] Building 0.5s (15/15) FINISHED
 => [internal] load build definition from Dockerfile
 => => transferring dockerfile: 1.23kB
 => [internal] load .dockerignore
 => => transferring context: 123B
 => [1/10] FROM ubuntu:22.04
 => CACHED [2/10] RUN apt-get update && apt-get install -y ...
 => ...
 => naming to sha256:xxxxxxxxxxxx
Successfully built xxxxxxxxxxxx
```

### 步骤 3: 启动开发容器

```bash
# 后台启动容器
docker-compose up -d

# 或前台启动（可以看到日志）
docker-compose up
```

**预期输出**:
```
[+] Running 1/1
 ✔ Container renderui-dev  Started
```

### 步骤 4: 进入容器开发环境

```bash
# 进入容器
docker-compose exec renderui-dev bash
```

现在你应该在容器内的 `/workspace/RenderUI` 目录中：
```bash
root@container-id:/workspace/RenderUI#
```

### 步骤 5: 在容器内构建项目

```bash
# 检查依赖
./build.sh

# 执行构建
./build.sh
```

**预期输出**:
```
Checking dependencies...
All dependencies found.

==========================================
RenderUI Build Configuration
==========================================
Build Type: Release
Build Dir:  /workspace/RenderUI/build
Extra Args: none

Running CMake...
-- The CXX compiler identification is GNU 11.4.0
-- FetchContent_Declare(json)
-- Populating json
-- Configuring done
-- Generating done
-- Build files have been written to: /workspace/RenderUI/build

Dependencies:
  Cairo:             1.17.6
  Wayland:           1.21.0
  OpenGL ES:         3.0
  EGL:               1.5
  DBus:              1.14.0

Building...
[1/10] Building CXX object CMakeFiles/RenderUICore.dir/src/core/Application.cpp.o
[2/10] Building CXX object CMakeFiles/RenderUICore.dir/src/core/ApplicationContext.cpp.o
...
[9/10] Linking CXX static library lib/libRenderUICore.a
[10/10] Linking CXX executable bin/renderui_demo

==========================================
Build completed successfully!
==========================================

Binaries location: /workspace/RenderUI/build/bin/
Libraries location: /workspace/RenderUI/build/lib/
```

### 步骤 6: 运行示例程序（可选）

由于是命令行环境，暂时无法运行 GUI 程序，但可以检查编译产物：

```bash
# 查看构建产物
ls -la build/bin/
ls -la build/lib/

# 检查可执行文件
file build/bin/*

# 查看库文件
file build/lib/*
```

**预期输出**:
```
build/bin/:
renderui_demo

build/lib/:
libRenderUICore.a

# file 命令输出:
renderui_demo: ELF 64-bit LSB pie executable, x86-64
libRenderUICore.a: current ar archive
```

### 步骤 7: 清理和退出

```bash
# 退出容器
exit

# 停止容器
docker-compose down

# 清理容器和镜像（可选）
docker-compose down --rmi all --volumes
```

---

## 🔧 常见问题排查

### 问题 1: Docker 服务未启动

**错误信息**:
```
Cannot connect to the Docker daemon at unix:///var/run/docker.sock. Is the docker daemon running?
```

**解决方案**:
```bash
# Linux
sudo systemctl start docker
sudo systemctl enable docker  # 开机自启

# macOS / Windows
# 启动 Docker Desktop 应用程序
```

### 问题 2: 权限不足

**错误信息**:
```
Got permission denied while trying to connect to the Docker daemon socket
```

**解决方案**:
```bash
# 将用户添加到 docker 组
sudo usermod -aG docker $USER

# 重新登录或执行
newgrp docker

# 验证
docker ps
```

### 问题 3: 内存不足

**错误信息**:
```
OCI runtime create failed: container_linux.go:380: starting container process caused: process_linux.go:545: container init caused: Rootfs mount failed: signal: killed
```

**解决方案**:
```bash
# Docker Desktop -> Settings -> Resources
# 增加内存分配（建议至少 4GB）
```

### 问题 4: 网络问题导致依赖下载失败

**错误信息**:
```
error: RPC failed; curl 92 HTTP/2 stream 5 was not closed cleanly: CANCEL (err 8)
```

**解决方案**:
```bash
# 在容器内手动设置 Git 配置
git config --global http.postBuffer 524288000
git config --global http.lowSpeedLimit 0
git config --global http.lowSpeedTime 999999

# 或使用国内镜像
# 修改 CMakeLists.txt 中的 GIT_REPOSITORY 为国内镜像地址
```

### 问题 5: 构建时间过长

**说明**: 首次构建需要下载所有依赖，可能需要 10-20 分钟

**优化方案**:
```bash
# 使用构建缓存
docker-compose build --no-cache  # 不使用缓存（不推荐）

# 后续构建会使用缓存，速度会快很多
```

---

## 💡 高级用法

### 1. 挂载当前目录

编辑 `docker-compose.yml`，确保有这一行：
```yaml
volumes:
  - .:/workspace/RenderUI
```

这样你在本地的代码修改会实时同步到容器中。

### 2. 启用 GPU 加速（阶段四）

```bash
# 进入容器
docker-compose exec renderui-dev bash

# 启用 Skia 构建
./build.sh --skia
```

### 3. 运行测试（阶段五）

```bash
# 进入容器
docker-compose exec renderui-dev bash

# 运行测试
cd build
ctest --verbose
```

### 4. 性能分析（阶段四）

```bash
# 启用 Perfetto 构建
./build.sh --perfetto

# 运行带性能分析的示例
./build/bin/renderui_demo --trace
```

---

## 📝 开发工作流建议

### 日常开发流程

1. **本地编辑代码**
   ```bash
   # 在你喜欢的 IDE 中编辑代码
   # VS Code / CLion / Vim 等
   ```

2. **在容器中构建**
   ```bash
   docker-compose exec renderui-dev bash
   cd /workspace/RenderUI
   ./build.sh
   ```

3. **测试和调试**
   ```bash
   # 在容器内运行测试
   cd build && ctest
   
   # 查看日志
   tail -f /tmp/renderui.log
   ```

4. **提交代码**
   ```bash
   # 在本地 git 仓库提交
   git add .
   git commit -m "feat: ..."
   git push
   ```

### 清理和维护

```bash
# 清理构建产物
./build.sh --clean

# 清理容器
docker-compose down

# 清理悬空镜像
docker image prune -f

# 完全清理（删除所有相关容器和镜像）
docker-compose down --rmi all --volumes
```

---

## 🎯 验证清单

完成上述步骤后，你应该能够确认：

- [ ] Docker 和 Docker Compose 已安装并可正常运行
- [ ] 成功构建 Docker 镜像
- [ ] 成功启动容器并进入交互式 shell
- [ ] 容器内所有依赖都已正确安装
- [ ] 项目成功编译，生成静态库和可执行文件
- [ ] 了解如何在本地和容器之间同步代码

如果以上所有项都打勾，恭喜！你的 Docker 开发环境已经就绪！🎉

---

## 📞 需要帮助？

如果遇到其他问题：

1. 查看 Docker 日志：
   ```bash
   docker-compose logs
   ```

2. 检查容器状态：
   ```bash
   docker-compose ps
   ```

3. 重启容器：
   ```bash
   docker-compose restart
   ```

4. 重建容器：
   ```bash
   docker-compose down
   docker-compose up -d --build
   ```

---

**最后更新**: 2026-03-29
