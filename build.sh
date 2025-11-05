#!/bin/bash

# =====================================================
# 串口调试助手 - Linux/macOS 快速编译脚本
# =====================================================

set -e  # 遇到错误立即退出

echo ""
echo "========================================"
echo "  串口调试助手 - 快速编译"
echo "========================================"
echo ""

# 检查是否在项目根目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "[错误] 请在项目根目录运行此脚本！"
    exit 1
fi

# 创建构建目录
if [ ! -d "build" ]; then
    echo "[信息] 创建构建目录..."
    mkdir build
fi

cd build

# 配置CMake
echo ""
echo "[步骤 1/3] 配置CMake..."
echo "----------------------------------------"
cmake .. || {
    echo "[错误] CMake配置失败！"
    echo "提示：请确保已安装Qt并设置CMAKE_PREFIX_PATH"
    echo "示例：export CMAKE_PREFIX_PATH=/opt/Qt/6.5.0/gcc_64"
    exit 1
}

# 编译项目
echo ""
echo "[步骤 2/3] 编译项目..."
echo "----------------------------------------"
make -j$(nproc) || {
    echo "[错误] 编译失败！"
    exit 1
}

# 完成
echo ""
echo "[步骤 3/3] 编译完成！"
echo "----------------------------------------"
echo ""
echo "✅ 编译成功！"
echo ""
echo "可执行文件位置："
echo "$(pwd)/bin/SerialDebugger"
echo ""

# 询问是否运行
read -p "是否立即运行程序？(y/n): " RUN
if [[ "$RUN" =~ ^[Yy]$ ]]; then
    echo ""
    echo "[信息] 启动程序..."
    ./bin/SerialDebugger &
fi

cd ..
