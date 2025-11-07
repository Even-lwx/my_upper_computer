@echo off
chcp 65001 >nul
echo ========================================
echo   串口调试助手 - 统一编译脚本
echo ========================================
echo.

REM 设置编译类型（默认Release）
set BUILD_TYPE=Release
if not "%1"=="" set BUILD_TYPE=%1

echo 编译类型: %BUILD_TYPE%
echo.

REM 创建build目录
if not exist build mkdir build

REM 进入build目录
cd build

echo [1/3] 配置CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %errorlevel% neq 0 (
    echo CMake配置失败！
    cd ..
    pause
    exit /b 1
)

echo.
echo [2/3] 编译项目...
mingw32-make -j%NUMBER_OF_PROCESSORS%
if %errorlevel% neq 0 (
    echo 编译失败！
    cd ..
    pause
    exit /b 1
)

echo.
echo [3/3] 复制运行时库...
cd ..

REM 复制MinGW运行时库到Release目录（如果不存在）
if not exist Release\libgcc_s_seh-1.dll (
    echo 复制libgcc_s_seh-1.dll...
    copy /Y "%MINGW_PATH%\bin\libgcc_s_seh-1.dll" Release\ >nul
)
if not exist Release\libstdc++-6.dll (
    echo 复制libstdc++-6.dll...
    copy /Y "%MINGW_PATH%\bin\libstdc++-6.dll" Release\ >nul
)
if not exist Release\libwinpthread-1.dll (
    echo 复制libwinpthread-1.dll...
    copy /Y "%MINGW_PATH%\bin\libwinpthread-1.dll" Release\ >nul
)

echo.
echo ========================================
echo   编译完成！
echo   可执行文件: Release\SerialDebugger_ImGui.exe
echo ========================================
echo.
pause
