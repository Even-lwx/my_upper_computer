/**
 * @file DataLogger.cpp
 * @brief 数据日志管理器实现
 * @author AI Assistant
 * @date 2025
 */

#include "DataLogger.h"
#include <QDebug>

DataLogger::DataLogger(QObject *parent)
    : QObject(parent)
    , m_logFile(nullptr)
    , m_stream(nullptr)
    , m_isLogging(false)
{
    qDebug() << "DataLogger initialized";
}

DataLogger::~DataLogger() {
    stopLogging();
    qDebug() << "DataLogger destroyed";
}

bool DataLogger::startLogging(const QString& filePath) {
    // 如果已在记录，先停止
    if (m_isLogging) {
        stopLogging();
    }

    // 创建日志文件
    m_logFile = new QFile(filePath, this);

    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        qWarning() << "无法打开日志文件:" << filePath;
        delete m_logFile;
        m_logFile = nullptr;
        return false;
    }

    // 创建文本流
    m_stream = new QTextStream(m_logFile);
    m_stream->setCodec("UTF-8");

    // 写入文件头
    *m_stream << "========================================\n";
    *m_stream << "串口数据日志\n";
    *m_stream << "开始时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
    *m_stream << "========================================\n\n";
    m_stream->flush();

    m_isLogging = true;
    qInfo() << "开始记录日志:" << filePath;

    return true;
}

void DataLogger::stopLogging() {
    if (!m_isLogging) {
        return;
    }

    // 写入文件尾
    if (m_stream) {
        *m_stream << "\n========================================\n";
        *m_stream << "结束时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "\n";
        *m_stream << "========================================\n";
        m_stream->flush();

        delete m_stream;
        m_stream = nullptr;
    }

    // 关闭文件
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
        m_logFile = nullptr;
    }

    m_isLogging = false;
    qInfo() << "停止记录日志";
}

bool DataLogger::isLogging() const {
    return m_isLogging;
}

void DataLogger::logData(const QByteArray& data, bool isReceived) {
    if (!m_isLogging || !m_stream) {
        return;
    }

    // 时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");

    // 方向标识
    QString direction = isReceived ? "[RX]" : "[TX]";

    // 写入日志
    *m_stream << timestamp << " " << direction << " " << data.toHex(' ').toUpper() << "\n";
    m_stream->flush();
}
