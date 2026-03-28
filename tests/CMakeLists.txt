# 测试配置 CMakeLists.txt

# GoogleTest 已经由主 CMakeLists.txt 引入

# 测试可执行文件
add_executable(renderui_tests
    main.cpp
    test_widget.cpp
    test_layout.cpp
    test_event.cpp
    test_resource.cpp
)

target_link_libraries(renderui_tests
    RenderUICore
    GTest::gtest
    GTest::gtest_main
)

# 包含头文件路径
target_include_directories(renderui_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
)

# 编译器选项
target_compile_options(renderui_tests PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# 注册测试
include(GoogleTest)
gtest_discover_tests(renderui_tests)

# 设置测试属性
set_target_properties(renderui_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests
)
