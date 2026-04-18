# Examples CMakeLists.txt

# UI 完整控件演示
add_executable(ui_demo
    ui_demo.cpp
)

target_link_libraries(ui_demo
    RenderUICore
)

target_include_directories(ui_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# 设置输出目录
set_target_properties(ui_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
)

# 复制配置文件到构建目录
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR}/bin/examples/)

message(STATUS "Examples configured:")
message(STATUS "  - ui_demo (Complete UI Demo with JSON config)")
