/**
 * @file MainWindow.h
 * @brief 主窗口类定义
 * @author AI Assistant
 * @date 2025
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QSplitter>

// 前向声明（避免循环依赖）
class SerialManager;
class SerialConfigWidget;
class DataDisplayWidget;
class SendWidget;
class ChartWidget;
class DataLogger;

/**
 * @class MainWindow
 * @brief 主窗口类 - 应用程序的核心界面
 *
 * 主要功能：
 * - 管理所有子窗口和面板
 * - 提供菜单栏、工具栏、状态栏
 * - 协调各模块之间的通信
 * - 支持停靠窗口自由布局
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /**
     * @brief 窗口关闭事件（保存配置）
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 菜单栏槽函数
    void onNewConnection();      // 新建连接
    void onSaveLog();            // 保存日志
    void onExit();               // 退出程序
    void onClearDisplay();       // 清空显示
    void onSettings();           // 设置
    void onAbout();              // 关于

    // 工具栏槽函数
    void onToggleConnection();   // 切换连接状态
    void onRefreshPorts();       // 刷新端口

    // 状态更新槽函数
    void onSerialConnected();    // 串口已连接
    void onSerialDisconnected(); // 串口已断开
    void onDataReceived(qint64 bytes);  // 数据接收
    void onDataSent(qint64 bytes);      // 数据发送

private:
    /**
     * @brief 初始化UI组件
     */
    void initUI();

    /**
     * @brief 创建菜单栏
     */
    void createMenuBar();

    /**
     * @brief 创建工具栏
     */
    void createToolBar();

    /**
     * @brief 创建状态栏
     */
    void createStatusBar();

    /**
     * @brief 创建中央窗口
     */
    void createCentralWidget();

    /**
     * @brief 创建停靠窗口
     */
    void createDockWidgets();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 加载配置
     */
    void loadSettings();

    /**
     * @brief 保存配置
     */
    void saveSettings();

private:
    // ========== 核心管理器 ==========
    SerialManager* m_serialManager;      // 串口管理器
    DataLogger* m_dataLogger;            // 数据日志

    // ========== UI组件 ==========
    // 停靠窗口
    QDockWidget* m_configDock;           // 串口配置面板
    QDockWidget* m_sendDock;             // 发送控制面板
    QDockWidget* m_chartDock;            // 图表面板

    // 子窗口组件
    SerialConfigWidget* m_configWidget;  // 串口配置
    DataDisplayWidget* m_displayWidget;  // 数据显示（中央）
    SendWidget* m_sendWidget;            // 发送控制
    ChartWidget* m_chartWidget;          // 实时图表

    // ========== 菜单 ==========
    QMenu* m_fileMenu;
    QMenu* m_editMenu;
    QMenu* m_viewMenu;
    QMenu* m_toolsMenu;
    QMenu* m_helpMenu;

    // ========== 工具栏 ==========
    QToolBar* m_mainToolBar;
    QAction* m_connectAction;
    QAction* m_disconnectAction;
    QAction* m_refreshAction;
    QAction* m_clearAction;

    // ========== 状态栏 ==========
    QLabel* m_statusLabel;        // 连接状态
    QLabel* m_rxLabel;            // 接收字节数
    QLabel* m_txLabel;            // 发送字节数
    QLabel* m_portLabel;          // 当前端口

    // ========== 统计数据 ==========
    qint64 m_totalRxBytes;        // 总接收字节
    qint64 m_totalTxBytes;        // 总发送字节
};

#endif // MAINWINDOW_H
