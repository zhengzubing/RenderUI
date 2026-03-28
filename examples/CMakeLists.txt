# 示例程序 CMakeLists.txt

# 360 车载全景影像 Demo
add_executable(360_camera_demo
    360_camera_demo.cpp
)
target_link_libraries(360_camera_demo PRIVATE RenderUICore)

# Perfetto 性能分析演示
if(ENABLE_PERFETTO)
    add_executable(perfetto_demo
        perfetto_demo.cpp
    )
    target_link_libraries(perfetto_demo PRIVATE RenderUICore)
endif()

# 设置输出目录
set_target_properties(360_camera_demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
)

if(ENABLE_PERFETTO)
    set_target_properties(perfetto_demo
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
    )
endif()

# 复制示例配置文件到构建目录
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_BINARY_DIR}/bin/examples/config/)

message(STATUS "Examples configured:")
message(STATUS "  - 360_camera_demo: ${BUILD_EXAMPLES}")
if(ENABLE_PERFETTO)
    message(STATUS "  - perfetto_demo: ON")
else()
    message(STATUS "  - perfetto_demo: OFF (requires ENABLE_PERFETTO=ON)")
endif()
