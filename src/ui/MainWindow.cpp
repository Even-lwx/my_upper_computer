/**
 * @file MainWindow.cpp
 * @brief 主窗口类实现
 * @author AI Assistant
 * @date 2025
 */

#include "MainWindow.h"
#include "core/SerialManager.h"
#include "core/DataLogger.h"
#include "ui/widgets/SerialConfigWidget.h"
#include "ui/widgets/DataDisplayWidget.h"
#include "ui/widgets/SendWidget.h"
#include "ui/widgets/ChartWidget.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QSplitter>
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_serialManager(nullptr)
    , m_dataLogger(nullptr)
    , m_totalRxBytes(0)
    , m_totalTxBytes(0)
{
    // 初始化核心管理器
    m_serialManager = new SerialManager(this);
    m_dataLogger = new DataLogger(this);

    // 初始化UI
    initUI();

    // 连接信号槽
    connectSignals();

    // 加载配置
    loadSettings();

    qInfo() << "MainWindow initialized";
}

MainWindow::~MainWindow() {
    // 保存配置
    saveSettings();

    // 关闭串口
    if (m_serialManager && m_serialManager->isOpen()) {
        m_serialManager->closePort();
    }

    qInfo() << "MainWindow destroyed";
}

void MainWindow::initUI() {
    // 设置窗口属性
    setWindowTitle("串口调试助手 v1.0.0");
    resize(1280, 800);

    // 创建菜单栏
    createMenuBar();

    // 创建工具栏
    createToolBar();

    // 创建中央窗口
    createCentralWidget();

    // 创建停靠窗口
    createDockWidgets();

    // 创建状态栏
    createStatusBar();
}

void MainWindow::createMenuBar() {
    // ========== 文件菜单 ==========
    m_fileMenu = menuBar()->addMenu("文件(&F)");

    QAction* newAction = m_fileMenu->addAction("新建连接(&N)");
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewConnection);

    QAction* saveAction = m_fileMenu->addAction("保存日志(&S)");
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveLog);

    m_fileMenu->addSeparator();

    QAction* exitAction = m_fileMenu->addAction("退出(&X)");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);

    // ========== 编辑菜单 ==========
    m_editMenu = menuBar()->addMenu("编辑(&E)");

    QAction* clearAction = m_editMenu->addAction("清空显示(&C)");
    clearAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::onClearDisplay);

    QAction* settingsAction = m_editMenu->addAction("设置(&S)");
    settingsAction->setShortcut(QKeySequence::Preferences);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);

    // ========== 视图菜单 ==========
    m_viewMenu = menuBar()->addMenu("视图(&V)");
    // 停靠窗口可见性将在createDockWidgets后添加

    // ========== 工具菜单 ==========
    m_toolsMenu = menuBar()->addMenu("工具(&T)");

    // ========== 帮助菜单 ==========
    m_helpMenu = menuBar()->addMenu("帮助(&H)");

    QAction* aboutAction = m_helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::createToolBar() {
    m_mainToolBar = addToolBar("主工具栏");
    m_mainToolBar->setObjectName("MainToolBar");
    m_mainToolBar->setMovable(false);

    // 连接按钮
    m_connectAction = m_mainToolBar->addAction("连接");
    m_connectAction->setObjectName("connectButton");
    m_connectAction->setToolTip("连接串口");
    connect(m_connectAction, &QAction::triggered, this, &MainWindow::onToggleConnection);

    // 断开按钮
    m_disconnectAction = m_mainToolBar->addAction("断开");
    m_disconnectAction->setObjectName("disconnectButton");
    m_disconnectAction->setEnabled(false);
    m_disconnectAction->setToolTip("断开串口");
    connect(m_disconnectAction, &QAction::triggered, this, &MainWindow::onToggleConnection);

    m_mainToolBar->addSeparator();

    // 刷新端口
    m_refreshAction = m_mainToolBar->addAction("刷新");
    m_refreshAction->setToolTip("刷新串口列表");
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::onRefreshPorts);

    // 清空显示
    m_clearAction = m_mainToolBar->addAction("清空");
    m_clearAction->setToolTip("清空接收显示");
    connect(m_clearAction, &QAction::triggered, this, &MainWindow::onClearDisplay);
}

void MainWindow::createCentralWidget() {
    // 创建数据显示组件（中央窗口）
    m_displayWidget = new DataDisplayWidget(this);
    setCentralWidget(m_displayWidget);
}

void MainWindow::createDockWidgets() {
    // ========== 串口配置面板 ==========
    m_configWidget = new SerialConfigWidget(this);
    m_configDock = new QDockWidget("串口配置", this);
    m_configDock->setObjectName("ConfigDock");
    m_configDock->setWidget(m_configWidget);
    m_configDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_configDock);

    // ========== 发送控制面板 ==========
    m_sendWidget = new SendWidget(this);
    m_sendDock = new QDockWidget("发送控制", this);
    m_sendDock->setObjectName("SendDock");
    m_sendDock->setWidget(m_sendWidget);
    m_sendDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_sendDock);

    // ========== 图表面板 ==========
    m_chartWidget = new ChartWidget(this);
    m_chartDock = new QDockWidget("实时图表", this);
    m_chartDock->setObjectName("ChartDock");
    m_chartDock->setWidget(m_chartWidget);
    m_chartDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_chartDock);

    // 添加到视图菜单
    m_viewMenu->addAction(m_configDock->toggleViewAction());
    m_viewMenu->addAction(m_sendDock->toggleViewAction());
    m_viewMenu->addAction(m_chartDock->toggleViewAction());
}

void MainWindow::createStatusBar() {
    // 连接状态
    m_statusLabel = new QLabel("未连接", this);
    m_statusLabel->setObjectName("statusLabel");
    statusBar()->addWidget(m_statusLabel);

    statusBar()->addWidget(new QLabel(" | ", this));

    // 当前端口
    m_portLabel = new QLabel("端口: --", this);
    statusBar()->addWidget(m_portLabel);

    statusBar()->addWidget(new QLabel(" | ", this));

    // 接收统计
    m_rxLabel = new QLabel("RX: 0 字节", this);
    statusBar()->addWidget(m_rxLabel);

    statusBar()->addWidget(new QLabel(" | ", this));

    // 发送统计
    m_txLabel = new QLabel("TX: 0 字节", this);
    statusBar()->addWidget(m_txLabel);

    // 右侧弹簧
    statusBar()->addPermanentWidget(new QLabel("就绪", this));
}

void MainWindow::connectSignals() {
    // 串口管理器信号
    connect(m_serialManager, &SerialManager::connected, this, &MainWindow::onSerialConnected);
    connect(m_serialManager, &SerialManager::disconnected, this, &MainWindow::onSerialDisconnected);
    connect(m_serialManager, &SerialManager::dataReceived, m_displayWidget, &DataDisplayWidget::appendReceivedData);
    connect(m_serialManager, &SerialManager::dataSent, m_displayWidget, &DataDisplayWidget::appendSentData);
    connect(m_serialManager, &SerialManager::bytesReceived, this, &MainWindow::onDataReceived);
    connect(m_serialManager, &SerialManager::bytesSent, this, &MainWindow::onDataSent);

    // 发送控件信号
    connect(m_sendWidget, &SendWidget::sendData, m_serialManager, &SerialManager::sendData);

    // 配置控件信号
    connect(m_configWidget, &SerialConfigWidget::connectRequest, this, &MainWindow::onToggleConnection);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 询问是否退出
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认退出",
        "确定要退出串口调试助手吗？",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        saveSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::loadSettings() {
    QSettings settings("OpenSource", "SerialDebugger");

    // 恢复窗口几何
    if (settings.contains("MainWindow/Geometry")) {
        restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
    }

    // 恢复窗口状态（停靠窗口布局）
    if (settings.contains("MainWindow/State")) {
        restoreState(settings.value("MainWindow/State").toByteArray());
    }

    qDebug() << "Settings loaded";
}

void MainWindow::saveSettings() {
    QSettings settings("OpenSource", "SerialDebugger");

    // 保存窗口几何
    settings.setValue("MainWindow/Geometry", saveGeometry());

    // 保存窗口状态
    settings.setValue("MainWindow/State", saveState());

    qDebug() << "Settings saved";
}

// ========== 槽函数实现 ==========

void MainWindow::onNewConnection() {
    // 清空数据显示
    m_displayWidget->clear();
    m_totalRxBytes = 0;
    m_totalTxBytes = 0;
    m_rxLabel->setText("RX: 0 字节");
    m_txLabel->setText("TX: 0 字节");

    QMessageBox::information(this, "新建连接", "已清空数据，准备新连接");
}

void MainWindow::onSaveLog() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存日志",
        QString(),
        "文本文件 (*.txt);;所有文件 (*.*)"
    );

    if (!fileName.isEmpty()) {
        m_displayWidget->saveToFile(fileName);
        QMessageBox::information(this, "保存成功", "日志已保存到:\n" + fileName);
    }
}

void MainWindow::onExit() {
    close();
}

void MainWindow::onClearDisplay() {
    m_displayWidget->clear();
    m_totalRxBytes = 0;
    m_totalTxBytes = 0;
    m_rxLabel->setText("RX: 0 字节");
    m_txLabel->setText("TX: 0 字节");
}

void MainWindow::onSettings() {
    QMessageBox::information(this, "设置", "设置功能即将实现");
}

void MainWindow::onAbout() {
    QMessageBox::about(
        this,
        "关于",
        "<h2>串口调试助手 v1.0.0</h2>"
        "<p>高性能工业级串口上位机</p>"
        "<p>特性：</p>"
        "<ul>"
        "<li>多线程异步通信</li>"
        "<li>HEX/ASCII显示切换</li>"
        "<li>实时数据图表</li>"
        "<li>协议解析引擎</li>"
        "<li>数据日志记录</li>"
        "</ul>"
        "<p>技术栈：C++17 + Qt6</p>"
        "<p>开源项目 © 2025</p>"
    );
}

void MainWindow::onToggleConnection() {
    if (m_serialManager->isOpen()) {
        // 断开连接
        m_serialManager->closePort();
    } else {
        // 连接
        QString portName = m_configWidget->getSelectedPort();
        int baudRate = m_configWidget->getBaudRate();

        if (portName.isEmpty()) {
            QMessageBox::warning(this, "警告", "请先选择串口");
            return;
        }

        if (m_serialManager->openPort(portName, baudRate)) {
            qInfo() << "串口已连接:" << portName << "@" << baudRate;
        } else {
            QMessageBox::critical(this, "错误", "无法打开串口:\n" + portName);
        }
    }
}

void MainWindow::onRefreshPorts() {
    m_configWidget->refreshPortList();
    QMessageBox::information(this, "刷新", "串口列表已刷新");
}

void MainWindow::onSerialConnected() {
    m_statusLabel->setText("已连接");
    m_portLabel->setText("端口: " + m_serialManager->portName());
    m_connectAction->setEnabled(false);
    m_disconnectAction->setEnabled(true);
    m_configWidget->setEnabled(false);

    statusBar()->showMessage("串口已连接", 3000);
}

void MainWindow::onSerialDisconnected() {
    m_statusLabel->setText("未连接");
    m_portLabel->setText("端口: --");
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
    m_configWidget->setEnabled(true);

    statusBar()->showMessage("串口已断开", 3000);
}

void MainWindow::onDataReceived(qint64 bytes) {
    m_totalRxBytes += bytes;
    m_rxLabel->setText(QString("RX: %1 字节").arg(m_totalRxBytes));
}

void MainWindow::onDataSent(qint64 bytes) {
    m_totalTxBytes += bytes;
    m_txLabel->setText(QString("TX: %1 字节").arg(m_totalTxBytes));
}
