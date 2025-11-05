/**
 * @file main.cpp
 * @brief 串口调试助手 - 主程序入口
 * @author AI Assistant
 * @date 2025
 *
 * 高性能工业级串口上位机
 * 特性：多线程异步通信、实时数据图表、协议解析
 */

#include <QApplication>
#include <QFile>
#include <QTextStream>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QTextCodec>
#else
#include <QStringConverter>
#endif
#include <QDebug>
#include "ui/MainWindow.h"

/**
 * @brief 加载QSS样式表
 * @param filePath 样式表文件路径
 * @return 是否加载成功
 */
bool loadStyleSheet(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "无法打开样式表文件:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");  // Qt 5: 设置UTF-8编码
    #else
    stream.setEncoding(QStringConverter::Utf8);  // Qt 6: 使用新API
    #endif
    QString styleSheet = stream.readAll();
    qApp->setStyleSheet(styleSheet);

    file.close();
    qDebug() << "样式表加载成功:" << filePath;
    return true;
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[]) {
    // 启用高DPI支持（Qt 5.6+）
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("SerialDebugger");
    app.setApplicationDisplayName("串口调试助手");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("OpenSource");

    // 设置默认编码为UTF-8（Qt 5）
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    #endif

    // 加载专业深色主题样式表（从Qt资源系统）
    if (!loadStyleSheet(":/themes/professional_dark.qss")) {
        qWarning() << "主题加载失败，使用默认样式";
    }

    // 创建并显示主窗口
    MainWindow mainWindow;
    mainWindow.setWindowTitle("串口调试助手 v1.0.0");
    mainWindow.resize(1280, 800);  // 默认窗口大小
    mainWindow.show();

    qInfo() << "========================================";
    qInfo() << "应用程序启动成功";
    qInfo() << "版本:" << app.applicationVersion();
    qInfo() << "Qt版本:" << qVersion();
    qInfo() << "========================================";

    return app.exec();
}
