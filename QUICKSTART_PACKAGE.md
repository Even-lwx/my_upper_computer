# 🎯 最简单的打包方法 - Qt Creator

## ⏱️ 预计时间：5分钟

---

## 第一步：打开项目（30秒）

1. 启动 **Qt Creator**
2. 点击 **"打开项目"** 或 `文件` → `打开文件或项目`
3. 选择 `E:\Desktop\my_upper_computer\CMakeLists.txt`
4. 选择一个 **Kit**（构建套件），推荐选择：
   - `Desktop Qt 6.x.x MinGW 64-bit`

---

## 第二步：切换到Release模式（10秒）

1. 在Qt Creator左下角找到 **构建模式选择器**
2. 从 `Debug` 切换到 **`Release`**

```
Debug ▼  →  Release ▼
```

---

## 第三步：编译项目（2-5分钟）

1. 点击左下角的 **"构建"** 按钮（锤子图标 🔨）
2. 或者使用快捷键 `Ctrl + B`
3. 等待编译完成

**编译输出窗口会显示：**
```
[100%] Built target SerialDebugger
编译成功
```

---

## 第四步：找到可执行文件（10秒）

编译成功后，可执行文件在：

```
build-SerialDebugger-<配置名称>-Release/bin/SerialDebugger.exe
```

例如：
```
E:\Desktop\build-SerialDebugger-Desktop_Qt_6_5_0_MinGW_64_bit-Release\bin\SerialDebugger.exe
```

**快速找到方法：**
1. 在Qt Creator中，点击 **"项目"** 视图
2. 查看 **"常规"** 下的 **"构建目录"**
3. 打开该目录下的 `bin` 文件夹

---

## 第五步：收集Qt依赖（1分钟）

### 方法A：使用Qt命令提示符（推荐）

1. 在Windows开始菜单搜索：`Qt 6.x.x for Desktop`
2. 打开 **Qt命令提示符**
3. 执行以下命令：

```cmd
cd E:\Desktop\build-SerialDebugger-Desktop_Qt_6_5_0_MinGW_64_bit-Release\bin
windeployqt SerialDebugger.exe --release --no-translations
```

### 方法B：在Qt Creator中设置部署

1. 在Qt Creator顶部菜单：`工具` → `外部` → `Qt`
2. 选择 `Deploy Application (windeployqt)`
3. 选择 `SerialDebugger.exe`

---

## 第六步：测试运行（10秒）

直接双击：
```
bin\SerialDebugger.exe
```

**预期效果：**
- ✅ 程序正常启动
- ✅ 显示深色主题界面
- ✅ 左侧显示串口配置面板
- ✅ 可以看到可用串口列表

---

## 第七步：打包分发

将 `bin` 文件夹中的 **所有文件** 打包成zip：

```
SerialDebugger_v1.0.0/
├── SerialDebugger.exe          ← 主程序
├── Qt6Core.dll                 ← Qt库
├── Qt6Gui.dll
├── Qt6Widgets.dll
├── Qt6SerialPort.dll
├── libgcc_s_seh-1.dll          ← 编译器库
├── libstdc++-6.dll
├── libwinpthread-1.dll
├── platforms/                  ← Qt插件
│   └── qwindows.dll
└── styles/
    └── qwindowsvistastyle.dll
```

**文件大小：** 约 20-30 MB

---

## ✅ 完成清单

- [ ] Qt Creator中编译成功（Release模式）
- [ ] 找到了 SerialDebugger.exe
- [ ] 运行了 windeployqt
- [ ] 双击exe能正常启动
- [ ] 界面是深色主题
- [ ] 准备好分发的zip包

---

## 🎯 一键式命令（高级）

如果您想用命令行完成所有步骤：

```cmd
REM 1. 打开Qt命令提示符

REM 2. 进入项目目录
cd E:\Desktop\my_upper_computer

REM 3. 运行简化脚本
build_simple.bat
```

脚本会自动完成所有步骤！

---

## ⚠️ 常见问题

### 问题：编译时报错 "找不到Qt"

**解决：**
- 在Qt Creator中重新配置Kit
- 确保选择的是完整的Qt安装（不是在线安装器）

### 问题：windeployqt命令找不到

**解决：**
- 必须使用 **Qt命令提示符**（不是普通cmd）
- 或添加Qt的bin到PATH：
  ```cmd
  set PATH=C:\Qt\6.5.0\mingw_64\bin;%PATH%
  ```

### 问题：运行exe时报缺少DLL

**解决：**
- 重新运行 windeployqt
- 确保将 bin 文件夹中的所有文件都打包

### 问题：程序启动后是灰白色，不是深色主题

**原因：** 资源文件没有编译进exe

**解决：**
1. 在Qt Creator中点击 **"清理"** (Clean)
2. 再点击 **"重新构建"** (Rebuild)

---

## 🚀 开始吧！

**立即操作：**

1. 打开 **Qt Creator**
2. 打开 `CMakeLists.txt`
3. 选择 **Release** 模式
4. 点击 **构建** 按钮
5. 等待完成！

---

<div align="center">

**5分钟后，您就有一个可以分发的专业串口工具了！** 🎉

</div>
