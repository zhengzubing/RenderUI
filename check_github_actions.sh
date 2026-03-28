#!/bin/bash

# GitHub Actions 构建状态检查脚本

REPO="zhengzubing/RenderUI"
WORKFLOW="ci.yml"

echo "======================================"
echo "GitHub Actions 构建状态检查"
echo "======================================"
echo ""

# 使用 GitHub CLI 检查（如果已安装）
if command -v gh &> /dev/null; then
    echo "使用 GitHub CLI 检查..."
    echo ""
    
    # 获取最近的运行记录
    RUNS=$(gh run list --repo ${REPO} --workflow ${WORKFLOW} --limit 5 --json status,conclusion,startedAt,databaseId --jq '.[0]')
    
    if [ -z "$RUNS" ]; then
        echo "未找到运行记录"
        exit 1
    fi
    
    STATUS=$(echo $RUNS | jq -r '.status')
    CONCLUSION=$(echo $RUNS | jq -r '.conclusion')
    RUN_ID=$(echo $RUNS | jq -r '.databaseId')
    STARTED=$(echo $RUNS | jq -r '.startedAt')
    
    echo "运行 ID: ${RUN_ID}"
    echo "开始时间：${STARTED}"
    echo "状态：${STATUS}"
    echo "结论：${CONCLUSION:-进行中}"
    echo ""
    
    # 查看具体任务
    echo "任务详情:"
    gh run view ${RUN_ID} --repo ${REPO} --jobs || true
    
    # 如果正在运行，提供日志链接
    if [ "$STATUS" == "in_progress" ] || [ "$STATUS" == "queued" ]; then
        echo ""
        echo "🔗 查看实时日志："
        echo "https://github.com/${REPO}/actions/runs/${RUN_ID}"
    fi
    
    # 如果完成，下载 artifacts
    if [ "$CONCLUSION" == "success" ]; then
        echo ""
        echo "✓ 构建成功！"
        echo ""
        echo "下载构建产物:"
        gh run download ${RUN_ID} --repo ${REPO} --dir ./build-artifacts
        
        if [ -d "./build-artifacts/renderui-linux-build" ]; then
            echo ""
            echo "构建产物内容:"
            ls -lh ./build-artifacts/renderui-linux-build/
            
            echo ""
            echo "验证静态库:"
            if [ -f "./build-artifacts/renderui-linux-build/lib/libRenderUICore.a" ]; then
                echo "✓ libRenderUICore.a 存在"
                file ./build-artifacts/renderui-linux-build/lib/libRenderUICore.a
            fi
        fi
    elif [ "$CONCLUSION" == "failure" ]; then
        echo ""
        echo "✗ 构建失败"
        echo "查看失败原因：https://github.com/${REPO}/actions/runs/${RUN_ID}"
    fi
    
else
    echo "GitHub CLI (gh) 未安装"
    echo ""
    echo "请手动访问以下链接查看构建状态:"
    echo "https://github.com/${REPO}/actions"
    echo ""
    
    # 使用 curl 检查（需要 token）
    if [ -n "$GITHUB_TOKEN" ]; then
        echo "使用 API 检查..."
        RESPONSE=$(curl -s -H "Authorization: token ${GITHUB_TOKEN}" \
            "https://api.github.com/repos/${REPO}/actions/workflows/${WORKFLOW}/runs?per_page=1")
        
        STATUS=$(echo $RESPONSE | jq -r '.workflow_runs[0].status')
        CONCLUSION=$(echo $RESPONSE | jq -r '.workflow_runs[0].conclusion')
        RUN_ID=$(echo $RESPONSE | jq -r '.workflow_runs[0].id')
        
        echo "运行 ID: ${RUN_ID}"
        echo "状态：${STATUS}"
        echo "结论：${CONCLUSION:-进行中}"
        echo ""
        echo "查看详情：https://github.com/${REPO}/actions/runs/${RUN_ID}"
    else
        echo "提示：设置 GITHUB_TOKEN 环境变量可获取详细信息"
    fi
fi

echo ""
echo "======================================"
