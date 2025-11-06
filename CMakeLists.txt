cmake_minimum_required(VERSION 3.20)
project(SerialDebugger_ImGui VERSION 1.0.0 LANGUAGES CXX)

# C++标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录设置（统一输出到项目根目录的Release文件夹）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Release/lib)

# ========================================
# 下载ImGui和GLFW (使用FetchContent)
# ========================================
include(FetchContent)

# GLFW - 窗口管理库
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Dear ImGui - UI库
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)

# ImPlot - 图表库
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)
FetchContent_MakeAvailable(implot)

# nlohmann/json - JSON序列化库（用于配置持久化）
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# ========================================
# 创建ImGui静态库
# ========================================
add_library(imgui_lib STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_lib PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui_lib PUBLIC
    glfw
)

# ========================================
# 创建ImPlot静态库
# ========================================
add_library(implot_lib STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
)

target_include_directories(implot_lib PUBLIC
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot_lib PUBLIC
    imgui_lib
)

# ========================================
# 包含目录
# ========================================
include_directories(
    ${CMAKE_SOURCE_DIR}/../src
    ${CMAKE_SOURCE_DIR}/../include
)

# ========================================
# 收集源文件（只包含ImGui UI）
# ========================================
# 注意：暂不包含Qt依赖的core文件
# file(GLOB CORE_SOURCES
#     "${CMAKE_SOURCE_DIR}/../src/core/*.cpp"
# )

# file(GLOB CORE_HEADERS
#     "${CMAKE_SOURCE_DIR}/../src/core/*.h"
# )

# ImGui UI源文件
file(GLOB IMGUI_UI_SOURCES
    "${CMAKE_SOURCE_DIR}/imgui_ui/*.cpp"
    "${CMAKE_SOURCE_DIR}/imgui_ui/core/*.cpp"
)

file(GLOB IMGUI_UI_HEADERS
    "${CMAKE_SOURCE_DIR}/imgui_ui/*.h"
    "${CMAKE_SOURCE_DIR}/imgui_ui/core/*.h"
    "${CMAKE_SOURCE_DIR}/imgui_ui/ui/*.h"
)

# ========================================
# 创建可执行文件
# ========================================
add_executable(${PROJECT_NAME}
    # ${CORE_SOURCES}
    # ${CORE_HEADERS}
    ${IMGUI_UI_SOURCES}
    ${IMGUI_UI_HEADERS}
)

# ========================================
# 链接库
# ========================================
target_link_libraries(${PROJECT_NAME}
    imgui_lib
    implot_lib
    glfw
    opengl32  # Windows OpenGL库
    nlohmann_json::nlohmann_json  # JSON序列化库
)

# Windows串口API
if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        setupapi  # 串口枚举需要
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON  # 隐藏控制台窗口
    )

    # MinGW/GCC需要额外的链接选项来隐藏终端
    if(MINGW OR CMAKE_COMPILER_IS_GNUCXX)
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()

    # 如果使用MSVC，需要设置子系统为Windows
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
    endif()
endif()

# ========================================
# 编译选项
# ========================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8 /execution-charset:utf-8)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# ========================================
# 安装规则
# ========================================
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# ========================================
# 打印配置信息
# ========================================
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "UI Framework: Dear ImGui ${imgui_VERSION}")
message(STATUS "Window Library: GLFW")
message(STATUS "Graphics API: OpenGL 3")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "========================================")
