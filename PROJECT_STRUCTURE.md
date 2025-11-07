# 项目结构说明

## 目录结构

```
my_upper_computer/
├── build/              # CMake临时编译目录（不提交到Git）
├── Release/            # 最终发布目录
│   ├── SerialDebugger_ImGui.exe    # 可执行文件
│   ├── *.dll                       # 运行时库
│   ├── app_config.json             # 用户配置
│   ├── imgui.ini                   # ImGui配置
│   └── *.txt                       # 文档文件
├── imgui_ui/           # 源代码目录
│   ├── core/           # 核心功能
│   ├── ui/             # UI组件
│   ├── protocols/      # 通信协议
│   └── *.cpp/h         # 源文件
├── CMakeLists.txt      # CMake配置文件
├── build.bat           # 统一编译脚本
└── README.md           # 项目说明
```

## 编译流程

### 方式1: 使用统一编译脚本（推荐）

```bash
# Release版本（默认）
build.bat

# Debug版本
build.bat Debug
```

### 方式2: 手动CMake编译

```bash
# 1. 清理并创建build目录
rm -rf build && mkdir build

# 2. 配置CMake
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

# 3. 编译
mingw32-make -j8

# 4. 可执行文件自动输出到：../Release/SerialDebugger_ImGui.exe
```

## 输出目录

所有编译产物统一输出到 `Release/` 目录：

- **可执行文件**: `Release/SerialDebugger_ImGui.exe`
- **静态库**: `Release/lib/` (如果有)
- **配置文件**: 程序运行时自动生成在 `Release/`

## Git版本控制

**提交的文件:**
- 源代码 (`imgui_ui/`)
- 配置文件 (`CMakeLists.txt`, `build.bat`)
- 文档 (`README.md`, `Release/*.txt`)

**忽略的文件:**
- 编译临时目录 (`build/`)
- 可执行文件 (`Release/*.exe`, `Release/*.dll`)
- IDE配置 (`.vscode/`, `.idea/`)
- 临时文件 (`nul`, `imgui.ini`)

## 注意事项

1. **不要直接在根目录编译**：始终使用 `build/` 目录
2. **发布版本在 `Release/` 目录**：包含exe和所需的dll
3. **清理项目**：删除 `build/` 目录，保留 `Release/`
4. **添加新源文件**：自动包含（使用 `file(GLOB ...)`）

## 依赖库（自动下载）

编译时会自动下载以下依赖：
- GLFW (窗口管理)
- Dear ImGui (UI框架)
- ImPlot (图表库)
- nlohmann/json (JSON解析)

首次编译需要较长时间下载依赖，后续编译会使用缓存。

## 运行时库

MinGW运行时库需要手动复制到 `Release/` 目录：
- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`

这些文件通常位于 MinGW 的 `bin/` 目录下。
