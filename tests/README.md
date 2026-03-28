# RenderUI 单元测试套件

## 构建测试

### 依赖项

确保已安装以下系统依赖：

```bash
# Ubuntu/Debian
sudo apt-get install \
    libcairo2-dev \
    libwayland-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev \
    libdbus-1-dev \
    pkg-config \
    cmake \
    ninja-build
```

### 编译

```bash
cd /path/to/RenderUI
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
ninja
```

## 运行测试

### 运行所有测试

```bash
ctest --test-dir .
# 或者
./bin/tests/renderui_tests
```

### 运行特定测试套件

```bash
# 只运行 Widget 测试
./bin/tests/renderui_tests --gtest_filter="WidgetTest.*"

# 只运行布局测试
./bin/tests/renderui_tests --gtest_filter="LayoutTest.*"

# 只运行事件测试
./bin/tests/renderui_tests --gtest_filter="EventLoopTest.*"

# 排除某些测试
./bin/tests/renderui_tests --gtest_filter="-VideoStreamTest.*"
```

### 运行单个测试

```bash
# 运行特定的测试用例
./bin/tests/renderui_tests --gtest_filter="WidgetTest.BasicProperties"
./bin/tests/renderui_tests --gtest_filter="LayoutTest.AbsoluteLayoutPercentage"
```

### 详细输出

```bash
# 显示详细的测试输出
./bin/tests/renderui_tests --gtest_brief=1

# 显示 XML 格式输出 (用于 CI)
./bin/tests/renderui_tests --gtest_output=xml:test_results.xml
```

## 测试覆盖率

使用 gcov/lcov 生成覆盖率报告：

```bash
# 编译时启用覆盖率
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON \
      -DCMAKE_CXX_FLAGS="--coverage" \
      -DCMAKE_C_FLAGS="--coverage" ..
ninja

# 运行测试
./bin/tests/renderui_tests

# 生成覆盖率报告
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report

# 查看报告
xdg-open coverage_report/index.html
```

## 测试资源

测试所需的资源文件应放置在 `tests/assets/` 目录：

```
tests/
├── assets/
│   ├── test_cube.obj       # 3D 模型测试
│   ├── test_texture.png    # 纹理加载测试
│   └── test_config.json    # JSON 解析测试
├── test_widget.cpp
├── test_layout.cpp
└── ...
```

## 当前测试覆盖的模块

### 核心模块 (Stage 1-2)
- ✅ Widget 基类 (基础属性、位置、大小、可见性、父子关系)
- ✅ 布局引擎 (AbsoluteLayout, FlexLayout)
- ✅ 事件系统 (EventLoop, EventDispatcher)
- ✅ 资源管理 (ResourceManager, 图片加载)

### 高级功能 (Stage 3)
- ✅ VideoStream (YUV420/NV12 格式、帧推送、回调)
- ✅ Model3D (OBJ 加载、旋转、材质、包围盒)
- ✅ DBus/CAN信号 (信号注册、值转换、监听器)

### 性能优化 (Stage 4)
- ⏳ SkiaRenderer (待实现)
- ⏳ PboPool (待实现)
- ⏳ AsyncTextureUploader (待实现)

### 工程化 (Stage 5)
- ✅ Perfetto 追踪 (集成演示)
- ✅ GoogleTest 框架集成
- ⏳ 性能基准测试 (待实现)

## 编写新测试

### 测试模板

```cpp
#include <gtest/gtest.h>
#include "Component/YourComponent.hpp"

using namespace RenderUI;

TEST(YourComponentTest, FeatureName) {
    // Arrange - 准备测试数据
    auto component = std::make_shared<YourComponent>();
    
    // Act - 执行操作
    component->doSomething();
    
    // Assert - 验证结果
    EXPECT_TRUE(component->isDone());
    EXPECT_FLOAT_EQ(component->getValue(), 42.0f);
}

// 测试套件级别的设置和清理
class YourComponentTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前的准备工作
    }
    
    void TearDown() override {
        // 每个测试后的清理工作
    }
};

TEST_F(YourComponentTestFixture, AdvancedTest) {
    // 使用测试夹具的高级测试
}
```

### 测试约定

1. **命名规范**: `TestSuiteName.FeatureName`
2. **断言选择**:
   - `EXPECT_*` - 失败后继续执行 (推荐)
   - `ASSERT_*` - 失败后立即终止
3. **浮点数比较**: 使用 `EXPECT_FLOAT_EQ` 或 `EXPECT_NEAR` (带容差)
4. **资源清理**: 使用 RAII 智能指针，避免手动 delete

## 持续集成 (CI)

测试会自动在 GitHub Actions 中运行。查看 CI 状态：

```bash
# 本地验证
cmake --build build --target renderui_tests
cd build && ctest --output-on-failure
```

## 常见问题

### Q: 测试失败 "Failed to initialize EGL"
A: 某些测试需要 OpenGL 上下文。在无头环境中使用虚拟显示服务器：
```bash
xvfb-run -a ./bin/tests/renderui_tests
```

### Q: DBus 测试失败
A: 确保 DBus 服务正在运行：
```bash
systemctl --user start dbus
```

### Q: 如何调试失败的测试？
A: 使用 GDB 调试：
```bash
gdb --args ./bin/tests/renderui_tests --gtest_filter=FailingTest.Name
(gdb) run
```

## 参考资料

- [GoogleTest 文档](https://google.github.io/googletest/)
- [CTest 手册](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- [Perfetto 追踪可视化](https://ui.perfetto.dev)
