# 测试配置 CMakeLists.txt

# 测试可执行文件
add_executable(renderui_tests
    test_main.cpp
    test_basic.cpp
)

target_link_libraries(renderui_tests
    RenderUICore
    GTest::gtest
)

# 包含头文件路径
target_include_directories(renderui_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# 编译器选项
target_compile_options(renderui_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# 注册测试（让 CTest 自动发现 GoogleTest 测试）
include(GoogleTest)
gtest_discover_tests(renderui_tests)

# 设置测试属性
set_target_properties(renderui_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/assets
)
