# 编译指南 Build Guide

## 📋 目录
- [环境准备](#环境准备)
- [Windows编译](#windows编译)
- [Linux编译](#linux编译)
- [常见问题](#常见问题)

---

## 环境准备

### 必需软件

| 软件 | 版本要求 | 下载地址 |
|------|---------|---------|
| Qt | 6.2+ | https://www.qt.io/download |
| CMake | 3.20+ | https://cmake.org/download/ |
| 编译器 | MSVC 2019+ / GCC 9+ | - |

### 检查环境

```bash
# 检查Qt版本
qmake --version

# 检查CMake版本
cmake --version

# 检查编译器
# Windows (MSVC)
cl

# Linux (GCC)
g++ --version
```

---

## Windows编译

### 方法1：使用Qt Creator（推荐）

1. **打开Qt Creator**
   - 启动Qt Creator

2. **打开项目**
   - 文件 → 打开文件或项目
   - 选择 `CMakeLists.txt`

3. **配置构建套件**
   - 选择合适的Kit（例如：Desktop Qt 6.x MSVC 2019 64bit）
   - 点击"Configure Project"

4. **编译项目**
   - 点击左下角"构建"按钮（锤子图标）
   - 或按快捷键 `Ctrl+B`

5. **运行程序**
   - 点击"运行"按钮（绿色三角）
   - 或按快捷键 `Ctrl+R`

### 方法2：命令行编译

```powershell
# 1. 打开Qt命令提示符
# 开始菜单 → Qt → Qt 6.x.x (MSVC 2019 64-bit) → Qt 6.x.x (MSVC 2019 64-bit) Command Prompt

# 2. 进入项目目录
cd E:\Desktop\my_upper_computer

# 3. 创建构建目录
mkdir build
cd build

# 4. 配置CMake
cmake .. -G "Visual Studio 16 2019" -A x64

# 5. 编译
cmake --build . --config Release

# 6. 运行
.\bin\Release\SerialDebugger.exe
```

### 方法3：使用Visual Studio

```powershell
# 1. 生成Visual Studio解决方案
cd E:\Desktop\my_upper_computer
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"

# 2. 打开生成的 .sln 文件
start SerialDebugger.sln

# 3. 在Visual Studio中按F5编译运行
```

---

## Linux编译

### Ubuntu/Debian

```bash
# 1. 安装依赖
sudo apt update
sudo apt install qt6-base-dev qt6-serialport-dev cmake g++ build-essential

# 2. 进入项目目录
cd ~/my_upper_computer

# 3. 创建构建目录
mkdir build
cd build

# 4. 配置CMake
cmake ..

# 5. 编译（使用多核加速）
make -j$(nproc)

# 6. 运行
./bin/SerialDebugger
```

### Fedora/CentOS

```bash
# 1. 安装依赖
sudo dnf install qt6-qtbase-devel qt6-qtserialport-devel cmake gcc-c++

# 2. 编译（同上）
cd ~/my_upper_computer
mkdir build && cd build
cmake ..
make -j$(nproc)
./bin/SerialDebugger
```

### Arch Linux

```bash
# 1. 安装依赖
sudo pacman -S qt6-base qt6-serialport cmake gcc

# 2. 编译运行
cd ~/my_upper_computer
mkdir build && cd build
cmake ..
make -j$(nproc)
./bin/SerialDebugger
```

---

## 常见问题

### ❌ 问题1：找不到Qt

**错误信息：**
```
CMake Error: Could not find Qt6
```

**解决方案：**

Windows:
```powershell
# 设置Qt路径
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
cmake ..
```

Linux:
```bash
# 设置Qt路径
export CMAKE_PREFIX_PATH=/opt/Qt/6.5.0/gcc_64
cmake ..
```

---

### ❌ 问题2：QSerialPort找不到

**错误信息：**
```
fatal error: QSerialPort: No such file or directory
```

**解决方案：**

确保安装了Qt SerialPort模块：

Windows (Qt维护工具):
- 打开Qt Maintenance Tool
- 选择"添加或移除组件"
- 勾选 `Qt SerialPort`

Linux:
```bash
sudo apt install qt6-serialport-dev
```

---

### ❌ 问题3：编译错误 C++17

**错误信息：**
```
error: 'xxx' requires C++17
```

**解决方案：**

确保编译器支持C++17：
- MSVC 2017 15.7+
- GCC 7+
- Clang 5+

更新CMakeLists.txt（已设置）：
```cmake
set(CMAKE_CXX_STANDARD 17)
```

---

### ❌ 问题4：乱码问题

**问题描述：**
界面或日志出现中文乱码

**解决方案：**

1. 确保源文件保存为UTF-8编码
2. MSVC编译器添加 `/utf-8` 选项（CMakeLists.txt已添加）
3. 运行时设置编码：
```cpp
QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
```

---

### ❌ 问题5：样式表不生效

**问题描述：**
深色主题没有应用

**解决方案：**

1. 确保 `resources/styles/dark_theme.qss` 存在
2. 检查main.cpp中的路径
3. 使用绝对路径测试：
```cpp
loadStyleSheet("E:/Desktop/my_upper_computer/resources/styles/dark_theme.qss");
```

---

### ❌ 问题6：串口无权限（Linux）

**错误信息：**
```
Permission denied: /dev/ttyUSB0
```

**解决方案：**

```bash
# 方法1：添加用户到dialout组
sudo usermod -a -G dialout $USER
# 注销后重新登录

# 方法2：临时修改权限
sudo chmod 666 /dev/ttyUSB0
```

---

## 📦 打包发布

### Windows

```powershell
# 1. 编译Release版本
cd build
cmake --build . --config Release

# 2. 使用windeployqt打包
cd bin\Release
windeployqt SerialDebugger.exe

# 3. 打包成zip
# 将整个Release文件夹压缩
```

### Linux

```bash
# 1. 编译
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# 2. 创建AppImage（可选）
# 使用linuxdeployqt工具

# 3. 或创建deb包
cpack -G DEB
```

---

## 🔍 调试技巧

### 启用详细日志

在main.cpp中添加：
```cpp
qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
```

### CMake调试

```bash
# 查看详细编译信息
cmake --build . --verbose

# 查看CMake变量
cmake .. -LAH
```

### Qt Creator调试

1. 左侧选择"调试"模式
2. 设置断点（点击行号）
3. 按F5开始调试
4. 使用F10单步执行

---

## ✅ 验证编译成功

运行程序后应该看到：

1. ✅ 主窗口正常显示
2. ✅ 深色主题生效
3. ✅ 串口配置面板显示
4. ✅ 能扫描到串口设备
5. ✅ 日志输出正常

控制台输出示例：
```
========================================
应用程序启动成功
版本: 1.0.0
Qt版本: 6.5.0
========================================
SerialManager initialized
DataLogger initialized
MainWindow initialized
样式表加载成功: resources/styles/dark_theme.qss
```

---

## 📞 获取帮助

如果遇到其他问题：

1. 查看 [README.md](README.md)
2. 检查Qt文档：https://doc.qt.io/
3. 提交Issue到项目仓库

---

<div align="center">

**祝编译顺利！🎉**

</div>
