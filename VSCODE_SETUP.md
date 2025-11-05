# 🎯 VSCode完整配置指南

## ✅ 已完成的配置

我已经为您创建了完整的VSCode配置文件：

- ✅ `.vscode/c_cpp_properties.json` - C++智能感知配置
- ✅ `.vscode/settings.json` - 工作区设置
- ✅ `.vscode/tasks.json` - 编译任务
- ✅ `.vscode/launch.json` - 运行和调试配置

---

## 📦 第一步：安装必要插件

在VSCode中按 `Ctrl+Shift+X` 打开扩展面板，安装以下插件：

### 必装插件（3个）

1. **C/C++** (Microsoft)
   - ID: `ms-vscode.cpptools`
   - 提供C++智能感知、调试等功能

2. **CMake Tools** (Microsoft)
   - ID: `ms-vscode.cmake-tools`
   - 提供CMake集成和可视化配置

3. **CMake** (twxs)
   - ID: `twxs.cmake`
   - 提供CMakeLists.txt语法高亮

### 推荐插件（可选）

4. **Qt tools** (Tonye Mu)
   - ID: `tonka3000.qtvsctools`
   - 提供.qss/.ui文件支持

5. **Chinese (Simplified)** Language Pack
   - ID: `MS-CEINTL.vscode-language-pack-zh-hans`
   - 中文界面

---

## ⚙️ 第二步：修改Qt路径

**重要！** 默认配置使用的是 `C:/Qt/6.5.0/mingw_64`，您需要根据实际安装路径修改。

### 找到您的Qt安装路径

通常Qt安装在：
- `C:/Qt/6.5.0/mingw_64`
- `C:/Qt/6.6.0/mingw_64`
- `D:/Qt/6.5.0/mingw_64`

### 需要修改的文件（3个）

#### 1. `.vscode/c_cpp_properties.json`

找到第7-13行，修改Qt路径：

```json
"includePath": [
    "${workspaceFolder}/**",
    "${workspaceFolder}/src",
    "${workspaceFolder}/include",
    "C:/Qt/6.5.0/mingw_64/include/**",           // ← 改这里
    "C:/Qt/6.5.0/mingw_64/include/QtCore/**",    // ← 改这里
    "C:/Qt/6.5.0/mingw_64/include/QtWidgets/**", // ← 改这里
    // ... 其他路径
],
```

第25行，修改编译器路径：
```json
"compilerPath": "C:/Qt/Tools/mingw1120_64/bin/g++.exe",  // ← 改这里
```

#### 2. `.vscode/settings.json`

找到第3行，修改Qt路径：

```json
"cmake.configureSettings": {
    "CMAKE_PREFIX_PATH": "C:/Qt/6.5.0/mingw_64",  // ← 改这里
    "CMAKE_BUILD_TYPE": "Release"
},
```

#### 3. `.vscode/tasks.json`

找到第15行，修改Qt路径：

```json
"-DCMAKE_PREFIX_PATH=C:/Qt/6.5.0/mingw_64"  // ← 改这里
```

#### 4. `.vscode/launch.json`

找到第14行和第24行，修改PATH：

```json
"environment": [
    {
        "name": "PATH",
        "value": "C:/Qt/6.5.0/mingw_64/bin;C:/Qt/Tools/mingw1120_64/bin;${env:PATH}"
        //       ^^^^^^^^^^^^^^^^^^^^^^^^  ^^^^^^^^^^^^^^^^^^^^^^^^^  ← 改这两处
    }
],
```

找到第21行和第51行，修改gdb路径：
```json
"miDebuggerPath": "C:/Qt/Tools/mingw1120_64/bin/gdb.exe",  // ← 改这里
```

---

## 🚀 第三步：使用VSCode编译运行

### 方法A：使用CMake Tools插件（推荐⭐）

1. **配置CMake**
   - 打开VSCode
   - 底部状态栏会出现CMake工具栏
   - 点击 **"Configure"** 或按 `Ctrl+Shift+P` 输入 `CMake: Configure`

2. **选择构建类型**
   - 底部状态栏点击 `[Debug]`
   - 切换到 **`Release`**

3. **编译项目**
   - 点击底部状态栏的 **"Build"** 按钮
   - 或按 `F7`
   - 或 `Ctrl+Shift+P` → `CMake: Build`

4. **运行项目**
   - 点击底部状态栏的 **"Run"** 按钮
   - 或按 `Shift+F5`
   - 或 `Ctrl+Shift+P` → `CMake: Run Without Debugging`

### 方法B：使用Tasks（手动控制）

1. **完整编译**
   - 按 `Ctrl+Shift+B`（默认构建任务）
   - 或 `Ctrl+Shift+P` → `Tasks: Run Build Task`
   - 选择 **"Build Full"**（会自动Configure + Build）

2. **仅编译**
   - `Ctrl+Shift+P` → `Tasks: Run Task`
   - 选择 **"CMake: Build"**

3. **清理构建**
   - `Ctrl+Shift+P` → `Tasks: Run Task`
   - 选择 **"CMake: Clean"**

4. **部署Qt依赖**
   - `Ctrl+Shift+P` → `Tasks: Run Task`
   - 选择 **"Deploy Qt Dependencies"**

### 方法C：使用调试（F5）

1. **直接运行**
   - 按 `F5` 启动调试
   - 会自动先编译，再运行

2. **不调试运行**
   - 按 `Ctrl+F5`
   - 更快，但不能设断点

---

## 🎯 常用快捷键

| 功能 | 快捷键 |
|------|--------|
| 配置CMake | `Ctrl+Shift+P` → `CMake: Configure` |
| 编译项目 | `Ctrl+Shift+B` 或 `F7` |
| 运行（调试） | `F5` |
| 运行（不调试） | `Ctrl+F5` |
| 清理构建 | `Ctrl+Shift+P` → `Tasks: Run Task` → `CMake: Clean` |
| 打开命令面板 | `Ctrl+Shift+P` |
| 打开终端 | ``Ctrl+` `` |

---

## 📊 VSCode底部状态栏

配置完成后，VSCode底部会显示CMake工具栏：

```
[Kit: GCC x.x.x] [Release] [Build] [Run] [Debug]
```

- **Kit**: 选择编译器（选择MinGW）
- **Release**: 构建类型（Debug/Release）
- **Build**: 点击编译
- **Run**: 点击运行
- **Debug**: 点击调试

---

## ⚠️ 常见问题

### 问题1：找不到Qt头文件（红色波浪线）

**原因：** Qt路径配置错误

**解决：**
1. 检查 `.vscode/c_cpp_properties.json` 中的Qt路径
2. 确保路径使用正斜杠 `/` 而不是反斜杠 `\`
3. 修改后按 `Ctrl+Shift+P` → `C/C++: Reload IntelliSense Database`

### 问题2：CMake配置失败

**原因：** 找不到Qt或MinGW

**解决：**
1. 检查 `.vscode/settings.json` 中的 `CMAKE_PREFIX_PATH`
2. 确保Qt和MinGW已正确安装
3. 在VSCode终端执行：
   ```cmd
   set PATH=C:\Qt\6.5.0\mingw_64\bin;%PATH%
   set PATH=C:\Qt\Tools\mingw1120_64\bin;%PATH%
   ```

### 问题3：编译成功但运行失败

**原因：** 缺少Qt的DLL

**解决：**
1. 运行 **"Deploy Qt Dependencies"** 任务
2. 或手动在终端执行：
   ```cmd
   cd build_release\bin
   windeployqt SerialDebugger.exe --release --no-translations
   ```

### 问题4：调试器启动失败

**原因：** gdb路径错误

**解决：**
1. 检查 `.vscode/launch.json` 中的 `miDebuggerPath`
2. 确保路径指向正确的 `gdb.exe`

### 问题5：中文乱码

**原因：** 编码设置问题

**解决：**
已在 `settings.json` 中配置：
```json
"files.encoding": "utf8",
"files.autoGuessEncoding": true
```

如果仍有问题，在文件右下角点击编码，选择 **UTF-8**。

---

## 🎨 推荐的VSCode设置

在 **文件 → 首选项 → 设置** 中搜索并设置：

```json
{
    // 自动保存
    "files.autoSave": "onFocusChange",
    
    // 括号对高亮
    "editor.bracketPairColorization.enabled": true,
    
    // 代码补全
    "editor.suggest.snippetsPreventQuickSuggestions": false,
    
    // CMake
    "cmake.buildBeforeRun": true,
    "cmake.configureOnOpen": true
}
```

---

## ✅ 验证配置

完成配置后，测试以下功能：

- [ ] Qt头文件没有红色波浪线
- [ ] 按 `F7` 可以编译项目
- [ ] 按 `F5` 可以运行程序
- [ ] 底部状态栏显示CMake工具
- [ ] 智能感知可以自动补全Qt类和函数
- [ ] 可以在代码中设置断点并调试

---

## 🔥 完整工作流程

### 日常开发流程

1. **打开项目**
   ```
   VSCode → 打开文件夹 → 选择 my_upper_computer
   ```

2. **首次配置**（仅第一次）
   ```
   Ctrl+Shift+P → CMake: Configure
   ```

3. **编码**
   ```
   编写代码，智能感知会自动提示
   ```

4. **编译**
   ```
   按 F7 或 Ctrl+Shift+B
   ```

5. **运行**
   ```
   按 F5（调试）或 Ctrl+F5（不调试）
   ```

6. **修改后重新编译**
   ```
   按 F7，然后按 F5
   ```

### 打包发布流程

1. **切换到Release模式**
   ```
   底部状态栏 [Debug] → 点击 → 选择 Release
   ```

2. **重新配置和编译**
   ```
   Ctrl+Shift+P → CMake: Clean Rebuild
   ```

3. **部署Qt依赖**
   ```
   Ctrl+Shift+P → Tasks: Run Task → Deploy Qt Dependencies
   ```

4. **打包分发**
   ```
   将 build_release/bin/ 文件夹打包成zip
   ```

---

## 🎯 总结

**VSCode配置完成后的优势：**

✅ **一键编译** - 按 `F7` 即可  
✅ **一键运行** - 按 `F5` 即可  
✅ **智能感知** - Qt类和函数自动补全  
✅ **调试支持** - 设断点、查看变量  
✅ **终端集成** - 内置终端执行命令  
✅ **Git集成** - 版本控制可视化  

---

## 📞 需要帮助？

如果遇到问题：
1. 检查所有配置文件中的Qt路径
2. 确保Qt和MinGW已正确安装
3. 在VSCode终端运行 `cmake --version` 和 `g++ --version` 验证环境
4. 查看VSCode输出面板的错误信息

---

<div align="center">

**现在您可以像专业开发者一样在VSCode中开发Qt项目了！** 🎉

</div>
