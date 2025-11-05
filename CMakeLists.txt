cmake_minimum_required(VERSION 3.20)
project(SerialDebugger VERSION 1.0.0 LANGUAGES CXX)

# C++标准设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt自动化工具
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 输出目录设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找Qt6组件
find_package(Qt6 COMPONENTS
    Core
    Widgets
    SerialPort
    Network
    REQUIRED
)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

# 收集源文件
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS
    "src/*.h"
)

# 资源文件
file(GLOB_RECURSE RESOURCES
    "resources/*.qrc"
)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::SerialPort
    Qt6::Network
)

# Windows特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# 编译选项
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
    # Release模式优化
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/O2>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    # Release模式优化
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-O3>)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# 打印配置信息
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "========================================")
