@echo off
chcp 65001 >nul
REM =====================================================
REM ImGui版本串口调试助手 - 快速编译脚本
REM =====================================================

echo.
echo ========================================
echo   ImGui 串口调试助手 - 快速编译
echo ========================================
echo.

REM 检查是否在项目根目录
if not exist "CMakeLists_imgui.txt" (
    echo [错误] 请在项目根目录运行此脚本！
    pause
    exit /b 1
)

echo [提示] ImGui版本特点：
echo - 游戏级美观UI
echo - 60fps流畅渲染
echo - 文件大小仅5-10MB
echo - 极快的启动速度
echo.
echo [提示] 首次编译会自动下载依赖：
echo - Dear ImGui 1.90.1
echo - GLFW 3.3.8
echo - ImPlot 0.16
echo.
echo 这需要稳定的网络连接和5-10分钟时间
echo.
pause

REM 创建构建目录
if not exist "build_imgui" (
    echo [步骤 1/3] 创建构建目录...
    mkdir build_imgui
) else (
    echo [步骤 1/3] 清理旧的构建目录...
    rmdir /s /q build_imgui
    mkdir build_imgui
)

cd build_imgui

REM 配置CMake
echo.
echo [步骤 2/3] 配置CMake（下载ImGui和GLFW）...
echo ----------------------------------------
echo 这可能需要几分钟时间，请耐心等待...
echo.

REM 复制ImGui的CMakeLists
copy ..\CMakeLists_imgui.txt CMakeLists.txt >nul

cmake . -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo.
    echo [错误] CMake配置失败！
    echo.
    echo 可能的原因：
    echo 1. 未安装CMake
    echo 2. 未安装MinGW
    echo 3. 网络连接问题（无法下载依赖）
    echo.
    echo 解决方法：
    echo - 确保CMake和MinGW已安装
    echo - 检查网络连接
    echo - 或使用代理：set http_proxy=...
    echo.
    cd ..
    pause
    exit /b 1
)

REM 编译项目
echo.
echo [步骤 3/3] 编译项目（Release）...
echo ----------------------------------------
cmake --build . --config Release -j 4
if errorlevel 1 (
    echo.
    echo [错误] 编译失败！
    echo 请检查上面的错误信息
    echo.
    cd ..
    pause
    exit /b 1
)

REM 检查可执行文件
if not exist "bin\SerialDebugger_ImGui.exe" (
    echo.
    echo [错误] 未找到可执行文件！
    echo.
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo   编译成功！
echo ========================================
echo.
echo 可执行文件位置：
echo %CD%\bin\SerialDebugger_ImGui.exe
echo.
echo 文件大小：约 5-10 MB
echo.
echo ========================================
echo.

REM 询问是否运行
set /p RUN="是否立即运行并查看效果？(Y/N): "
if /i "%RUN%"=="Y" (
    echo.
    echo 正在启动 ImGui 串口调试助手...
    echo 您将看到游戏级的美观界面！
    echo.
    start "" "bin\SerialDebugger_ImGui.exe"
)

cd ..

echo.
echo 提示：
echo - 程序启动速度极快（< 0.3秒）
echo - 流畅的60fps渲染
echo - 可以在菜单栏选择"帮助" → "显示演示窗口"查看ImGui所有功能
echo.
pause
