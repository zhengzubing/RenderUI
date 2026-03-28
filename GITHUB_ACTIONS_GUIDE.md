# GitHub Actions 远程构建指南

## 📋 概述

由于本地未安装 Docker，我们已配置 GitHub Actions 自动在云端 Ubuntu 环境中构建和测试项目。

---

## 🔍 查看构建状态

### 方式一：GitHub 网页（推荐）

1. 访问：https://github.com/zhengzubing/RenderUI/actions
2. 查看最新的工作流运行
3. 点击 "Build on Ubuntu" 查看详细日志
4. 等待所有步骤变为绿色 ✓

### 方式二：使用检查脚本

```bash
# 安装 GitHub CLI（如果未安装）
brew install gh

# 认证（首次需要）
gh auth login

# 运行检查脚本
./check_github_actions.sh
```

### 方式三：GitHub API

```bash
# 设置 token（可选，获取更多信息）
export GITHUB_TOKEN="your_token"

# 检查最近运行
curl -s https://api.github.com/repos/zhengzubing/RenderUI/actions/runs \
  | jq '.workflow_runs[0] | {status, conclusion, html_url}'
```

---

## ⏱️ 预计时间

| 阶段 | 时间 |
|------|------|
| 队列等待 | 0-2 分钟 |
| 环境准备 | 1-2 分钟 |
| 依赖下载 | 5-8 分钟 |
| 编译构建 | 3-5 分钟 |
| 验证上传 | 1 分钟 |
| **总计** | **10-18 分钟** |

---

## 📦 构建产物

成功后会生成以下 artifacts（保留 7 天）：

- `libRenderUICore.a` - 静态库
- `renderui_demo` - 示例程序（如果有）
- `compile_commands.json` - 编译命令数据库

### 下载产物

**方式一：GitHub 网页**
1. 进入成功的构建页面
2. 滚动到 "Artifacts" 部分
3. 点击 `renderui-linux-build` 下载

**方式二：GitHub CLI**
```bash
# 获取最近的运行 ID
RUN_ID=$(gh run list --repo zhengzubing/RenderUI --limit 1 --json databaseId --jq '.[0].databaseId')

# 下载 artifacts
gh run download ${RUN_ID} --repo zhengzubing/RenderUI
```

**方式三：自动下载**
```bash
# 运行验证脚本（会自动下载）
./verify_build.sh
```

---

## ✅ 验证构建成功

下载 artifacts 后，运行验证脚本：

```bash
./verify_build.sh ./build-artifacts/renderui-linux-build
```

**预期输出**:
```
======================================
RenderUI 构建产物验证
======================================

使用 GitHub Actions 构建产物...
检查目录：./build-artifacts/renderui-linux-build

1. 检查库文件
----------------------------------------
✓ 静态库 libRenderUICore.a
  大小：1.2M

2. 检查可执行文件
----------------------------------------
✓ 可执行文件目录
  文件数：1

3. 检查头文件
----------------------------------------
✓ 公共头文件目录
  文件数：5

...

======================================
验证总结
======================================
通过：8
失败：0

✓ 所有关键组件验证通过！
```

---

## 🔧 故障排查

### 问题 1: 构建一直处于 "queued" 状态

**原因**: GitHub Actions 队列繁忙

**解决**:
- 等待几分钟
- 检查是否有其他运行占用资源
- 如果是私有仓库，检查并发限制

### 问题 2: 构建失败

**查看日志**:
```bash
# 使用 CLI 查看失败原因
gh run view --log --repo zhengzubing/RenderUI
```

**常见原因**:
- 依赖下载超时 → 重试即可
- 编译错误 → 检查代码
- 内存不足 → 联系维护者

### 问题 3: Artifacts 不存在

**原因**:
- 构建尚未完成
- 构建失败
- 配置问题

**解决**:
1. 确认构建状态为 "success"
2. 检查 workflow 文件是否包含 upload-artifact 步骤
3. 重新触发构建

---

## 🚀 手动触发构建

如果需要重新运行：

```bash
# 方式一：推送空提交
git commit --allow-empty -m "ci: trigger build"
git push

# 方式二：使用 GitHub CLI
gh workflow run ci.yml --repo zhengzubing/RenderUI

# 方式三：GitHub 网页
# 访问 https://github.com/zhengzubing/RenderUI/actions/workflows/ci.yml
# 点击 "Run workflow"
```

---

## 📊 当前工作流配置

查看 `.github/workflows/ci.yml`:

```yaml
name: CI Build Test

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-ubuntu:
    runs-on: ubuntu-22.04
    # ... 完整配置见文件
    
  build-docker:
    runs-on: ubuntu-22.04
    # ... 完整配置见文件
```

---

## 📞 需要帮助？

1. 查看完整日志：https://github.com/zhengzubing/RenderUI/actions
2. 下载 artifacts 失败？运行 `./check_github_actions.sh`
3. 验证失败？运行 `./verify_build.sh` 查看详细错误
4. 仍有问题？提交 Issue 或联系维护者

---

**最后更新**: 2026-03-29
