@echo off
REM =====================================================
REM 串口调试助手 - Windows 快速编译脚本
REM =====================================================

echo.
echo ========================================
echo   串口调试助手 - 快速编译
echo ========================================
echo.

REM 检查是否在项目根目录
if not exist "CMakeLists.txt" (
    echo [错误] 请在项目根目录运行此脚本！
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "build" (
    echo [信息] 创建构建目录...
    mkdir build
)

cd build

REM 配置CMake
echo.
echo [步骤 1/3] 配置CMake...
echo ----------------------------------------
cmake .. -G "MinGW Makefiles"
if errorlevel 1 (
    echo [错误] CMake配置失败！
    echo 提示：请确保已安装Qt并设置CMAKE_PREFIX_PATH
    echo 示例：set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\mingw_64
    cd ..
    pause
    exit /b 1
)

REM 编译项目
echo.
echo [步骤 2/3] 编译项目...
echo ----------------------------------------
cmake --build . --config Release
if errorlevel 1 (
    echo [错误] 编译失败！
    cd ..
    pause
    exit /b 1
)

REM 完成
echo.
echo [步骤 3/3] 编译完成！
echo ----------------------------------------
echo.
echo ✅ 编译成功！
echo.
echo 可执行文件位置：
echo %cd%\bin\SerialDebugger.exe
echo.

REM 询问是否运行
set /p RUN="是否立即运行程序？(Y/N): "
if /i "%RUN%"=="Y" (
    echo.
    echo [信息] 启动程序...
    start "" "bin\SerialDebugger.exe"
)

cd ..
pause
