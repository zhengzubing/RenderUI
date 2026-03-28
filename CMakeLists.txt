cmake_minimum_required(VERSION 3.20)
project(RenderUI 
    VERSION 0.1.0 
    DESCRIPTION "Modern UI Widget Rendering Framework"
    LANGUAGES CXX C
)

# C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 构建选项
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_TESTS "Build test suite" ON)
option(USE_SKIA "Use Skia for GPU-accelerated 2D rendering (Stage 4+)" OFF)
option(ENABLE_PERFETTO "Enable Perfetto trace profiling" OFF)

# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# 第三方库引入
# ============================================================================

include(FetchContent)

# 1. nlohmann/json - JSON 解析
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

# 2. plog - 日志库
FetchContent_Declare(
    plog
    GIT_REPOSITORY https://github.com/SergiusTheBest/plog.git
    GIT_TAG        1.1.10
)
FetchContent_MakeAvailable(plog)

# 3. magic_enum - 枚举反射
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG        v0.9.5
)
FetchContent_MakeAvailable(magic_enum)

# 4. tinyobjloader - OBJ 模型加载
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG        v2.0.0-rc13
)
FetchContent_MakeAvailable(tinyobjloader)

# 5. tinyply - Ply 模型加载
FetchContent_Declare(
    tinyply
    GIT_REPOSITORY https://github.com/ddiakopoulos/tinyply.git
    GIT_TAG        2.3.4
)
FetchContent_MakeAvailable(tinyply)

# 6. stb_image - 图像加载
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
)
FetchContent_Populate(stb)
# stb 是单头文件库，直接包含路径
include_directories(${stb_SOURCE_DIR})

# 7. CppSoft_Signals - 信号槽机制 (VDK 实现)
FetchContent_Declare(
    cppsoft_signals
    GIT_REPOSITORY https://github.com/elliotgoodrich/cppsoft-signals.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(cppsoft_signals)

# 8. Cairo - 2D 绘图（阶段一至三）
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

# 9. Wayland - 显示协议
pkg_check_modules(WAYLAND REQUIRED 
    wayland-client
    wayland-protocols
)

# 10. EGL - OpenGL ES 上下文
pkg_check_modules(EGL REQUIRED egl)

# 11. OpenGL ES - 渲染引擎
pkg_check_modules(GLES REQUIRED glesv2)

# 12. DBus - IPC 通信
pkg_check_modules(DBUS REQUIRED dbus-1)

# ============================================================================
# 扩展库（阶段四启用）
# ============================================================================

if(USE_SKIA)
    # Skia - GPU 加速 2D 渲染
    find_package(skia CONFIG REQUIRED)
    message(STATUS "Skia support enabled")
endif()

if(ENABLE_PERFETTO)
    # Perfetto - 性能分析
    FetchContent_Declare(
        perfetto
        GIT_REPOSITORY https://android.googlesource.com/platform/external/perfetto/
        GIT_TAG        v34.0
    )
    FetchContent_MakeAvailable(perfetto)
    message(STATUS "Perfetto tracing enabled")
endif()

# ============================================================================
# RenderUI 核心库
# ============================================================================

# 收集所有源文件
file(GLOB_RECURSE CORE_SOURCES 
    src/core/*.cpp
    src/window/*.cpp
    src/widget/*.cpp
    src/widget/widgets/*.cpp
    src/widget/events/*.cpp
    src/layout/*.cpp
    src/render/*.cpp
    src/resource/*.cpp
    src/data/*.cpp
    src/ipc/*.cpp
    src/utils/*.cpp
)

# 创建静态库
add_library(RenderUICore STATIC ${CORE_SOURCES})

target_include_directories(RenderUICore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CAIRO_INCLUDE_DIRS}
        ${WAYLAND_INCLUDE_DIRS}
        ${EGL_INCLUDE_DIRS}
        ${GLES_INCLUDE_DIRS}
        ${DBUS_INCLUDE_DIRS}
)

target_link_libraries(RenderUICore
    PUBLIC
        nlohmann_json::nlohmann_json
        plog::plog
        magic_enum::magic_enum
        tinyobjloader
        tinyply
        cppsoft_signals
    PRIVATE
        ${CAIRO_LIBRARIES}
        ${WAYLAND_LIBRARIES}
        ${EGL_LIBRARIES}
        ${GLES_LIBRARIES}
        ${DBUS_LIBRARIES}
        dl
        pthread
)

if(USE_SKIA)
    target_link_libraries(RenderUICore PUBLIC skia::skia)
    target_compile_definitions(RenderUICore PUBLIC RENDERUI_USE_SKIA)
endif()

if(ENABLE_PERFETTO)
    target_link_libraries(RenderUICore PUBLIC perfetto::perfetto)
    target_compile_definitions(RenderUICore PUBLIC RENDERUI_ENABLE_TRACING)
endif()

# 编译器警告
target_compile_options(RenderUICore PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -fPIC
)

# ============================================================================
# 示例程序
# ============================================================================

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# 测试
# ============================================================================

if(BUILD_TESTS)
    # GoogleTest - 单元测试（阶段五）
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    add_subdirectory(tests)
endif()

# ============================================================================
# 安装配置
# ============================================================================

install(TARGETS RenderUICore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/Component
    DESTINATION include
)

# 生成 pkg-config 文件
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/renderui.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/renderui.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/renderui.pc
    DESTINATION lib/pkgconfig
)

# ============================================================================
# 打印配置信息
# ============================================================================

message(STATUS "")
message(STATUS "RenderUI Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  C++ Standard:      ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build Examples:    ${BUILD_EXAMPLES}")
message(STATUS "  Build Tests:       ${BUILD_TESTS}")
message(STATUS "  Use Skia:          ${USE_SKIA}")
message(STATUS "  Enable Perfetto:   ${ENABLE_PERFETTO}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Cairo:             ${CAIRO_VERSION}")
message(STATUS "  Wayland:           ${WAYLAND_VERSION}")
message(STATUS "  OpenGL ES:         ${GLES_VERSION}")
message(STATUS "  EGL:               ${EGL_VERSION}")
message(STATUS "  DBus:              ${DBUS_VERSION}")
message(STATUS "")
