@echo off
chcp 65001 >nul
REM =====================================================
REM 串口调试助手 - 简化编译脚本
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

echo [提示] 请确保：
echo 1. 已安装 Qt 6.x 或 Qt 5.15+
echo 2. 已将 Qt 的 bin 目录添加到 PATH
echo 3. 正在使用 "Qt 命令提示符" 运行此脚本
echo.
pause

REM 创建构建目录
if not exist "build_release" (
    echo [步骤 1/3] 创建构建目录...
    mkdir build_release
) else (
    echo [步骤 1/3] 清理旧的构建目录...
    rmdir /s /q build_release
    mkdir build_release
)

cd build_release

REM 配置CMake
echo.
echo [步骤 2/3] 配置CMake...
echo ----------------------------------------
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo.
    echo [错误] CMake配置失败！
    echo.
    echo 可能的原因：
    echo 1. 未安装Qt或未添加到PATH
    echo 2. 未安装MinGW
    echo.
    echo 解决方法：
    echo - 使用 "Qt 命令提示符" 而不是普通cmd
    echo - 或手动设置Qt路径：
    echo   set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\mingw_64
    echo.
    cd ..
    pause
    exit /b 1
)

REM 编译项目
echo.
echo [步骤 3/3] 编译项目...
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
if not exist "bin\SerialDebugger.exe" (
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
echo %CD%\bin\SerialDebugger.exe
echo.
echo ----------------------------------------
echo 下一步：收集Qt依赖
echo ----------------------------------------
echo.
echo 请执行以下命令：
echo.
echo   cd bin
echo   windeployqt SerialDebugger.exe --release --no-translations
echo.
echo 然后就可以分发 bin 文件夹中的所有文件了！
echo.

REM 询问是否自动运行windeployqt
set /p DEPLOY="是否现在自动收集Qt依赖？(Y/N): "
if /i "%DEPLOY%"=="Y" (
    echo.
    echo 正在收集Qt依赖...
    cd bin
    windeployqt SerialDebugger.exe --release --no-translations
    if errorlevel 1 (
        echo.
        echo [警告] windeployqt执行失败
        echo 请确保Qt的bin目录在PATH中
        echo 或使用 "Qt 命令提示符" 运行此脚本
        echo.
    ) else (
        echo.
        echo ========================================
        echo   打包完成！
        echo ========================================
        echo.
        echo 完整发布包位置：
        echo %CD%
        echo.
        echo 您可以将此文件夹的所有文件打包成zip分发
        echo.

        REM 询问是否测试运行
        set /p RUN="是否测试运行？(Y/N): "
        if /i "!RUN!"=="Y" (
            start "" SerialDebugger.exe
        )
    )
    cd ..
)

cd ..

echo.
pause
