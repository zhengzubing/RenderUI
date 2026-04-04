# RenderUI 测试指南

## 📋 概述

RenderUI 使用 **GoogleTest** 作为单元测试框架，通过 **CTest** 进行统一管理。

---

## 🚀 快速开始

### 1. 构建项目（包含测试）

```bash
mkdir build && cd build
cmake .. -G Ninja -DBUILD_TESTS=ON
ninja
```

### 2. 运行所有测试

```bash
# 方法一：使用 CTest（推荐）
ctest --verbose

# 方法二：直接运行测试可执行文件
./bin/tests/renderui_tests
```

---

## 📊 测试结构

```
tests/
├── CMakeLists.txt      # 测试配置
├── test_basic.cpp      # 基础功能测试
├── test_widget.cpp     # Widget 控件测试（待添加）
└── test_layout.cpp     # 布局系统测试（待添加）
```

---

## 🔧 CTest 常用命令

### 基本用法

```bash
ctest                    # 运行所有测试
ctest -N                 # 列出所有测试（不运行）
ctest -V                 # 详细输出
ctest -VV                # 更详细输出
```

### 过滤测试

```bash
ctest -R Basic           # 只运行名称包含 "Basic" 的测试
ctest -E Slow            # 排除名称包含 "Slow" 的测试
```

### 并行执行

```bash
ctest -j4                # 使用 4 个线程并行运行
ctest -j$(nproc)         # 使用所有 CPU 核心
```

### 生成报告

```bash
ctest --output-on-failure    # 失败时显示详细输出
ctest -T Test                # 生成测试结果报告
```

---

## 🎯 GoogleTest 常用命令

### 直接运行测试程序

```bash
./bin/tests/renderui_tests
```

### 列出所有测试

```bash
./bin/tests/renderui_tests --gtest_list_tests
```

**输出示例**：
```
BasicTest.
  Addition
  Subtraction
  SmartPointer
MathTest.
  Multiply
  Divide
```

### 过滤测试

```bash
# 运行特定测试套件
./bin/tests/renderui_tests --gtest_filter="BasicTest.*"

# 运行特定测试用例
./bin/tests/renderui_tests --gtest_filter="BasicTest.Addition"

# 排除某些测试
./bin/tests/renderui_tests --gtest_filter="-MathTest.*"
```

### 重复运行（检测不稳定测试）

```bash
./bin/tests/renderui_tests --gtest_repeat=10
./bin/tests/renderui_tests --gtest_repeat=-1  # 无限重复，直到 Ctrl+C
```

### 输出格式

```bash
# XML 格式（用于 CI/CD）
./bin/tests/renderui_tests --gtest_output=xml:test_results.xml

# JSON 格式
./bin/tests/renderui_tests --gtest_output=json:test_results.json
```

---

## 📝 编写测试

### 简单测试

```cpp
#include <gtest/gtest.h>

TEST(WidgetTest, CreateButton) {
    auto button = std::make_shared<Button>("Click Me");
    
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->getText(), "Click Me");
}
```

### 使用 Test Fixture

```cpp
class ApplicationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 每个测试前执行
        app_ = Application::instance();
    }
    
    void TearDown() override {
        // 每个测试后执行
    }
    
    Application* app_;
};

TEST_F(ApplicationTest, Initialize) {
    bool result = app_->init("Test", 800, 600);
    EXPECT_TRUE(result);
}
```

---

## ⚠️ 常用断言宏

| 宏 | 说明 | 失败行为 |
|----|------|---------|
| `EXPECT_EQ(a, b)` | 期望 a == b | 继续执行 |
| `ASSERT_EQ(a, b)` | 断言 a == b | **立即停止** |
| `EXPECT_TRUE(x)` | 期望 x 为真 | 继续执行 |
| `EXPECT_FALSE(x)` | 期望 x 为假 | 继续执行 |
| `EXPECT_NE(a, b)` | 期望 a != b | 继续执行 |
| `EXPECT_STREQ(s1, s2)` | 字符串相等 | 继续执行 |
| `EXPECT_NEAR(a, b, tol)` | 浮点数接近 | 继续执行 |

---

## 🔍 调试技巧

### 1. 查看详细输出

```bash
ctest -VV
```

### 2. 只运行失败的测试

```bash
ctest --rerun-failed
```

### 3. 使用 GDB 调试

```bash
gdb ./bin/tests/renderui_tests
(gdb) run --gtest_filter="WidgetTest.CreateButton"
```

### 4. 启用日志

```cpp
// 在测试中添加日志
TEST(WidgetTest, Debug) {
    std::cout << "Debug info: " << some_value << std::endl;
    EXPECT_TRUE(condition);
}
```

---

## 📈 代码覆盖率

### 生成覆盖率报告

```bash
# 1. 配置时启用覆盖率
cmake .. -DCMAKE_CXX_FLAGS="--coverage"

# 2. 构建
ninja

# 3. 运行测试
ctest

# 4. 生成报告
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report

# 5. 查看报告
open coverage_report/index.html
```

---

## 🎯 最佳实践

1. ✅ **每个测试独立** - 不依赖其他测试的状态
2. ✅ **使用有意义的名称** - `TEST(WidgetTest, CreateButton)` 
3. ✅ **优先使用 EXPECT** - 失败后继续执行，看到更多错误
4. ✅ **保持测试快速** - 单个测试应在毫秒级完成
5. ✅ **定期运行测试** - 每次提交前运行 `ctest`
6. ✅ **测试边界条件** - 空值、极端值、异常情况

---

## 🚨 常见问题

### Q: 测试找不到？

**A**: 确保已重新运行 CMake：
```bash
cd build
cmake ..
ninja
```

### Q: 测试编译失败？

**A**: 检查是否包含了正确的头文件：
```cpp
#include <gtest/gtest.h>
```

### Q: 如何跳过某个测试？

**A**: 使用 `DISABLED_` 前缀：
```cpp
TEST(WidgetTest, DISABLED_SlowTest) {
    // 这个测试会被跳过
}
```

---

## 📞 需要帮助？

- 查看 [GoogleTest 文档](https://google.github.io/googletest/)
- 查看 [CTest 文档](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- 查看项目 README.md

---

**最后更新**: 2026-04-04
