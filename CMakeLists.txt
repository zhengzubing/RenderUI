cmake_minimum_required(VERSION 3.20)

# 设置 CMake 策略以兼容旧版本依赖
if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()
if(POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

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
    GIT_TAG        v2.0.0rc13
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

# 7. signals - 信号槽机制 (轻量级 C++14 实现)
# 这是一个 header-only 库，只需要包含头文件路径
FetchContent_Declare(
    signals
    GIT_REPOSITORY https://github.com/vdksoft/signals
    GIT_TAG        master
)
FetchContent_GetProperties(signals)
if(NOT signals_POPULATED)
    FetchContent_Populate(signals)
endif()
# 添加头文件路径
include_directories(${signals_SOURCE_DIR})

# 8. Cairo - 2D 绘图（阶段一至三）
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)

# 9. Wayland - 显示协议
pkg_check_modules(WAYLAND REQUIRED 
    wayland-client
    wayland-protocols
)

# 生成 Wayland XDG Shell 协议头文件
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)
execute_process(
    COMMAND pkg-config --variable=pkgdatadir wayland-protocols
    OUTPUT_VARIABLE WAYLAND_PROTOCOLS_DATADIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PKGCONFIG_RESULT
)
if(PKGCONFIG_RESULT EQUAL 0 AND EXISTS "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml")
    # 新版 wayland-protocols (1.15+) 使用 stable/xdg-shell/xdg-shell.xml
    set(XDG_SHELL_XML ${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml)
elseif(EXISTS "${WAYLAND_PROTOCOLS_DATADIR}/wayland/xdg-shell.xml")
    # 旧版路径
    set(XDG_SHELL_XML ${WAYLAND_PROTOCOLS_DATADIR}/wayland/xdg-shell.xml)
else()
    message(FATAL_ERROR "Could not find xdg-shell.xml in wayland-protocols datadir: ${WAYLAND_PROTOCOLS_DATADIR}")
endif()
set(XDG_SHELL_HEADER ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h)
set(XDG_SHELL_CODE ${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c)

add_custom_command(
    OUTPUT ${XDG_SHELL_HEADER}
    COMMAND ${WAYLAND_SCANNER} client-header ${XDG_SHELL_XML} ${XDG_SHELL_HEADER}
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generating XDG Shell header"
)

add_custom_command(
    OUTPUT ${XDG_SHELL_CODE}
    COMMAND ${WAYLAND_SCANNER} private-code ${XDG_SHELL_XML} ${XDG_SHELL_CODE}
    DEPENDS ${XDG_SHELL_XML}
    COMMENT "Generating XDG Shell code"
)

add_custom_target(wayland_protocols DEPENDS ${XDG_SHELL_HEADER} ${XDG_SHELL_CODE})

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
    src/widget/*.cpp
    src/widget/widgets/*.cpp
    src/layout/*.cpp
    src/render/*.cpp
    src/resource/*.cpp
    src/ipc/*.cpp
    src/utils/*.cpp
)

# 创建静态库
add_library(RenderUICore STATIC ${CORE_SOURCES} ${XDG_SHELL_CODE})
add_dependencies(RenderUICore wayland_protocols)

target_include_directories(RenderUICore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/core>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/widget>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/widget/widgets>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/layout>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/render>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/resource>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/ipc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/utils>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
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
# 主程序
# ============================================================================

# 添加主程序可执行文件
add_executable(renderui_demo src/main.cpp)
target_link_libraries(renderui_demo PRIVATE RenderUICore)

# ============================================================================
# 示例程序
# ============================================================================

# 阶段五：启用示例构建
set(BUILD_EXAMPLES ON CACHE BOOL "Build examples" FORCE)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# ============================================================================
# 测试
# ============================================================================

# 阶段五：启用测试构建
set(BUILD_TESTS ON CACHE BOOL "Build tests" FORCE)

# if(BUILD_TESTS)
#     # GoogleTest - 单元测试（阶段五）
#     # 禁用 gtest 的测试和示例，避免冲突
#     set(INSTALL_GTEST OFF CACHE BOOL "Disable installation of googletest")
#     set(gtest_build_samples OFF CACHE BOOL "Disable building gtest samples")
#     set(gtest_build_tests OFF CACHE BOOL "Disable building gtest tests")
#     
#     FetchContent_Declare(
#         googletest
#         GIT_REPOSITORY https://github.com/google/googletest.git
#         GIT_TAG        v1.14.0
#     )
#     # For Windows: Prevent overriding the parent project's compiler/linker settings
#     set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#     FetchContent_MakeAvailable(googletest)
#     
#     enable_testing()
#     
#     # 检查 tests 目录是否存在，如果存在则添加
#     if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
#         add_subdirectory(tests)
#     else()
#         message(STATUS "Tests directory not found, skipping test build")
#     endif()
# endif()

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
