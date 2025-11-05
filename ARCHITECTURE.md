# 架构设计文档 Architecture Design

## 📐 整体架构

本项目采用经典的 **MVC（Model-View-Controller）架构模式**，结合 **多线程设计** 和 **信号槽机制**，实现高性能串口通信。

```
┌─────────────────────────────────────────────────────────┐
│                      表示层 (UI Layer)                    │
│  ┌─────────────┐  ┌──────────────┐  ┌───────────────┐  │
│  │ MainWindow  │  │  Widgets     │  │   Styles      │  │
│  │  (主窗口)   │  │ (UI组件集)    │  │  (QSS主题)    │  │
│  └──────┬──────┘  └──────┬───────┘  └───────────────┘  │
└─────────┼─────────────────┼──────────────────────────────┘
          │                 │
          │  Qt信号/槽      │
          ▼                 ▼
┌─────────────────────────────────────────────────────────┐
│                   业务逻辑层 (Core Layer)                 │
│  ┌──────────────────┐     ┌──────────────────┐         │
│  │ SerialManager    │     │   DataLogger     │         │
│  │  (串口管理器)    │────▶│   (日志管理器)   │         │
│  └────────┬─────────┘     └──────────────────┘         │
│           │                                              │
│           │  独立线程                                    │
│           ▼                                              │
│  ┌──────────────────┐                                   │
│  │  QSerialPort     │                                   │
│  │  (Qt串口库)      │                                   │
│  └──────────────────┘                                   │
└─────────────────────────────────────────────────────────┘
          │
          │  系统调用
          ▼
┌─────────────────────────────────────────────────────────┐
│                   硬件抽象层 (Hardware)                   │
│                     串口驱动 (OS)                         │
└─────────────────────────────────────────────────────────┘
```

---

## 🏗️ 模块设计

### 1. 表示层（UI Layer）

#### 1.1 MainWindow (主窗口)

**职责：**
- 管理整个应用程序的主界面
- 协调各子组件之间的通信
- 管理菜单栏、工具栏、状态栏
- 处理窗口布局和停靠面板

**关键方法：**
```cpp
void createMenuBar();      // 创建菜单栏
void createToolBar();      // 创建工具栏
void createStatusBar();    // 创建状态栏
void createDockWidgets();  // 创建停靠窗口
void connectSignals();     // 连接信号槽
```

**信号/槽：**
```cpp
// 槽函数
void onSerialConnected();    // 处理串口连接
void onSerialDisconnected(); // 处理串口断开
void onDataReceived(qint64); // 更新接收统计
void onDataSent(qint64);     // 更新发送统计
```

---

#### 1.2 SerialConfigWidget (串口配置面板)

**职责：**
- 显示可用串口列表
- 配置串口参数（波特率、数据位等）
- 触发连接/断开操作

**关键属性：**
```cpp
QComboBox* m_portComboBox;      // 端口选择
QComboBox* m_baudRateComboBox;  // 波特率
QComboBox* m_dataBitsComboBox;  // 数据位
QComboBox* m_stopBitsComboBox;  // 停止位
QComboBox* m_parityComboBox;    // 校验位
```

**信号：**
```cpp
void connectRequest();  // 连接请求信号
```

---

#### 1.3 DataDisplayWidget (数据显示面板)

**职责：**
- 显示接收和发送的数据
- 支持多种显示模式（HEX/ASCII/混合）
- 提供编码格式选择
- 批量刷新优化

**核心设计：**

**批量刷新机制：**
```cpp
// 避免频繁重绘，定时批量更新
QTimer* m_updateTimer;      // 50ms定时器
QByteArray m_pendingData;   // 缓冲待显示数据

void onUpdateDisplay() {
    // 批量追加数据
    m_textEdit->appendHtml(m_pendingData);
    m_pendingData.clear();
}
```

**数据格式化：**
```cpp
QString formatData(const QByteArray& data, bool isReceived) {
    // 1. 添加时间戳
    // 2. 添加方向标识 [RX]/[TX]
    // 3. 根据模式格式化（HEX/ASCII）
    // 4. 返回HTML格式字符串
}
```

---

#### 1.4 SendWidget (发送控制面板)

**职责：**
- 提供数据发送输入框
- 支持HEX/ASCII发送模式
- 定时自动发送
- 历史命令管理

**定时发送实现：**
```cpp
QTimer* m_sendTimer;

void onTimerSendToggled(bool checked) {
    if (checked) {
        int interval = m_timerIntervalSpinBox->value();
        m_sendTimer->start(interval);
    } else {
        m_sendTimer->stop();
    }
}

void onTimerTimeout() {
    // 定时自动发送
    onSendButtonClicked();
}
```

---

#### 1.5 ChartWidget (图表面板)

**职责：**
- 实时显示数据曲线（预留接口）
- 多通道支持
- 波形缩放、拖拽
- 数据导出

**扩展方向：**
- 集成 QCustomPlot 库
- 或使用 QtCharts 模块

---

### 2. 业务逻辑层（Core Layer）

#### 2.1 SerialManager (串口管理器)

**核心职责：**
- 串口的打开、关闭
- 异步数据收发
- 错误处理
- 线程安全

**关键设计：**

**线程安全：**
```cpp
QMutex m_mutex;  // 线程锁

qint64 sendData(const QByteArray& data) {
    QMutexLocker locker(&m_mutex);  // 自动加锁
    // ... 发送数据
}
```

**异步接收：**
```cpp
void onReadyRead() {
    QByteArray data = m_serial->readAll();
    emit dataReceived(data);  // 发送信号
}
```

**信号定义：**
```cpp
signals:
    void connected();                    // 已连接
    void disconnected();                 // 已断开
    void dataReceived(const QByteArray&); // 数据接收
    void dataSent(const QByteArray&);     // 数据发送
    void bytesReceived(qint64);          // 字节统计
    void bytesSent(qint64);              // 字节统计
    void errorOccurred(const QString&);  // 错误
```

---

#### 2.2 DataLogger (数据日志管理器)

**职责：**
- 实时记录收发数据到文件
- 自动添加时间戳
- 日志文件管理

**文件格式：**
```
========================================
串口数据日志
开始时间: 2025-01-05 14:30:00
========================================

2025-01-05 14:30:01.123 [RX] AA 55 01 02
2025-01-05 14:30:01.456 [TX] FF FE FD
...

========================================
结束时间: 2025-01-05 14:35:00
========================================
```

**实现：**
```cpp
void logData(const QByteArray& data, bool isReceived) {
    QString timestamp = QDateTime::currentDateTime()
        .toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString direction = isReceived ? "[RX]" : "[TX]";

    *m_stream << timestamp << " "
              << direction << " "
              << data.toHex(' ').toUpper() << "\n";
    m_stream->flush();
}
```

---

## 🔄 数据流设计

### 数据接收流程

```
[硬件串口]
    │
    ▼
[QSerialPort::readyRead() 信号]
    │
    ▼
[SerialManager::onReadyRead()]
    │  读取数据
    │
    ├──▶ emit dataReceived(data)  ────▶ [DataDisplayWidget]
    │                                        │ 格式化显示
    │                                        ▼
    └──▶ emit bytesReceived(size) ────▶ [MainWindow]
                                             │ 更新统计
```

### 数据发送流程

```
[SendWidget]
    │ 用户输入
    │
    ▼
emit sendData(data)
    │
    ▼
[SerialManager::sendData()]
    │  QMutex加锁
    │  serial->write(data)
    │
    ├──▶ emit dataSent(data)  ────▶ [DataDisplayWidget]
    │                                     │ 显示发送内容
    │                                     ▼
    └──▶ emit bytesSent(size) ────▶ [MainWindow]
                                          │ 更新统计
```

---

## 🧵 多线程设计

### 线程模型

```
主线程 (GUI Thread)
│
├─ MainWindow
├─ 所有 Widget 组件
├─ 事件循环
│
└─ SerialManager (运行在主线程)
    │
    └─ QSerialPort (内部使用异步I/O)
```

**说明：**
- QSerialPort 本身是异步的，不需要额外线程
- 使用 Qt 的信号槽机制天然线程安全
- 如需要，可将 SerialManager 移至独立QThread

### 性能优化点

1. **批量刷新UI**
   ```cpp
   // 50ms定时器批量更新，避免每条数据都刷新
   m_updateTimer->setInterval(50);
   ```

2. **限制显示行数**
   ```cpp
   m_textEdit->setMaximumBlockCount(10000);
   ```

3. **使用 QMutex 保护共享资源**
   ```cpp
   QMutexLocker locker(&m_mutex);
   ```

---

## 🎨 UI设计模式

### 1. 停靠窗口模式

使用 `QDockWidget` 实现灵活布局：

```cpp
QDockWidget* dock = new QDockWidget("标题", this);
dock->setWidget(widget);
dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
addDockWidget(Qt::LeftDockWidgetArea, dock);
```

**优点：**
- 用户可自由拖拽调整布局
- 可隐藏/显示面板
- 自动保存布局状态

### 2. 信号槽解耦

组件之间通过信号槽通信，避免直接依赖：

```cpp
// ❌ 不推荐：直接调用
sendWidget->sendDataToSerial(data);

// ✅ 推荐：信号槽
connect(sendWidget, &SendWidget::sendData,
        serialManager, &SerialManager::sendData);
```

---

## 📦 扩展性设计

### 1. 插件接口（预留）

```cpp
class IPlugin {
public:
    virtual QString name() const = 0;
    virtual void onDataReceived(const QByteArray& data) = 0;
    virtual QWidget* createWidget() = 0;
};
```

### 2. 协议引擎（预留）

```cpp
class ProtocolEngine {
public:
    void loadProtocol(const QString& jsonFile);
    QVariantMap parseData(const QByteArray& data);
};
```

### 3. 数据转换工具

```cpp
class DataConverter {
public:
    static QByteArray hexToBytes(const QString& hex);
    static QString bytesToHex(const QByteArray& bytes);
    static QString bytesToString(const QByteArray& bytes, const QString& encoding);
};
```

---

## 🔐 错误处理策略

### 1. 串口错误

```cpp
void onErrorOccurred(QSerialPort::SerialPortError error) {
    // 忽略无错误和资源错误
    if (error == QSerialPort::NoError ||
        error == QSerialPort::ResourceError) {
        return;
    }

    // 发出错误信号
    emit errorOccurred(m_serial->errorString());

    // 严重错误自动关闭
    if (error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError) {
        closePort();
    }
}
```

### 2. 日志记录

```cpp
qDebug()    << "调试信息";   // 开发模式
qInfo()     << "普通信息";   // 重要操作
qWarning()  << "警告信息";   // 潜在问题
qCritical() << "严重错误";   // 致命错误
```

---

## 📊 性能考量

### 1. 内存管理

- 限制显示行数：`setMaximumBlockCount(10000)`
- 使用对象池（可选）
- 及时释放资源

### 2. CPU优化

- 批量更新UI（50ms定时器）
- 避免频繁字符串拼接
- 使用 `QByteArray` 而非 `QString`（二进制数据）

### 3. 编译优化

```cmake
# Release模式启用O3优化
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O3>
)
```

---

## 🔮 未来规划

1. **协议解析引擎**
   - JSON配置协议
   - 可视化协议编辑器
   - CRC/校验和计算

2. **数据分析**
   - 实时曲线图表（QCustomPlot）
   - FFT频谱分析
   - 数据统计

3. **自动化测试**
   - 脚本发送
   - 数据比对
   - 压力测试

4. **跨平台优化**
   - macOS适配
   - Linux权限处理
   - 触摸屏支持

---

<div align="center">

**架构遵循原则：高内聚、低耦合、可扩展、易维护**

</div>
