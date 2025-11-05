@echo off
REM =====================================================
REM 串口调试助手 - Windows 打包脚本
REM =====================================================

echo.
echo ========================================
echo   串口调试助手 - 自动编译打包
echo ========================================
echo.

REM 检查是否在项目根目录
if not exist "CMakeLists.txt" (
    echo [错误] 请在项目根目录运行此脚本！
    pause
    exit /b 1
)

REM 设置编译配置
set BUILD_TYPE=Release
set BUILD_DIR=build_release

REM 创建构建目录
if not exist "%BUILD_DIR%" (
    echo [步骤 1/5] 创建构建目录...
    mkdir "%BUILD_DIR%"
) else (
    echo [步骤 1/5] 构建目录已存在
)

cd "%BUILD_DIR%"

REM 配置CMake
echo.
echo [步骤 2/5] 配置CMake (Release模式)...
echo ----------------------------------------
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
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
echo [步骤 3/5] 编译项目 (Release)...
echo ----------------------------------------
cmake --build . --config Release -j 4
if errorlevel 1 (
    echo [错误] 编译失败！
    cd ..
    pause
    exit /b 1
)

REM 检查可执行文件
if not exist "bin\SerialDebugger.exe" (
    echo [错误] 未找到可执行文件！
    cd ..
    pause
    exit /b 1
)

echo.
echo [步骤 4/5] 打包Qt依赖...
echo ----------------------------------------

REM 创建发布目录
set DEPLOY_DIR=SerialDebugger_v1.0.0
if not exist "%DEPLOY_DIR%" (
    mkdir "%DEPLOY_DIR%"
) else (
    echo 清理旧的发布目录...
    rmdir /s /q "%DEPLOY_DIR%"
    mkdir "%DEPLOY_DIR%"
)

REM 复制可执行文件
echo 复制可执行文件...
copy "bin\SerialDebugger.exe" "%DEPLOY_DIR%\"

REM 使用windeployqt打包Qt依赖
echo 收集Qt依赖库...
cd "%DEPLOY_DIR%"
windeployqt SerialDebugger.exe --release --no-translations
if errorlevel 1 (
    echo [警告] windeployqt执行失败，可能需要手动添加Qt库
    echo 请确保Qt的bin目录在PATH中
)
cd ..

REM 复制其他必要文件
echo 复制文档文件...
if exist "..\README.md" copy "..\README.md" "%DEPLOY_DIR%\"
if exist "..\QUICKSTART.md" copy "..\QUICKSTART.md" "%DEPLOY_DIR%\"

REM 创建快捷方式说明
echo 创建使用说明...
(
echo ====================================
echo   串口调试助手 v1.0.0
echo ====================================
echo.
echo 快速开始：
echo 1. 双击运行 SerialDebugger.exe
echo 2. 选择串口和波特率
echo 3. 点击"连接串口"按钮
echo 4. 开始发送和接收数据
echo.
echo 详细说明请查看 README.md
echo.
echo 系统要求：
echo - Windows 10 或更高版本
echo - 已安装串口驱动
echo.
echo 技术支持：
echo https://github.com/your-repo
echo ====================================
) > "%DEPLOY_DIR%\使用说明.txt"

echo.
echo [步骤 5/5] 创建压缩包...
echo ----------------------------------------

REM 压缩发布包（如果有7zip或WinRAR）
where 7z >nul 2>nul
if %errorlevel% equ 0 (
    echo 使用7-Zip压缩...
    7z a -tzip "%DEPLOY_DIR%.zip" "%DEPLOY_DIR%"
    echo 压缩包已创建: %DEPLOY_DIR%.zip
) else (
    echo [提示] 未找到7-Zip，请手动压缩 %DEPLOY_DIR% 文件夹
)

cd ..

REM 完成
echo.
echo ========================================
echo   打包完成！
echo ========================================
echo.
echo 可执行文件位置：
echo %BUILD_DIR%\%DEPLOY_DIR%\SerialDebugger.exe
echo.
echo 完整发布包：
echo %BUILD_DIR%\%DEPLOY_DIR%
if exist "%BUILD_DIR%\%DEPLOY_DIR%.zip" (
    echo.
    echo 压缩包：
    echo %BUILD_DIR%\%DEPLOY_DIR%.zip
)
echo.
echo 文件大小估算：15-25 MB
echo.

REM 询问是否打开文件夹
set /p OPEN="是否打开发布文件夹？(Y/N): "
if /i "%OPEN%"=="Y" (
    start "" "%BUILD_DIR%\%DEPLOY_DIR%"
)

pause
