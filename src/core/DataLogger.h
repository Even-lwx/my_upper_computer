/**
 * @file DataLogger.h
 * @brief 数据日志管理器
 * @author AI Assistant
 * @date 2025
 */

#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QDateTime>

/**
 * @class DataLogger
 * @brief 数据日志记录器
 *
 * 负责将串口收发数据记录到文件
 */
class DataLogger : public QObject {
    Q_OBJECT

public:
    explicit DataLogger(QObject *parent = nullptr);
    ~DataLogger();

    /**
     * @brief 开始记录日志
     * @param filePath 日志文件路径
     * @return 是否成功开始
     */
    bool startLogging(const QString& filePath);

    /**
     * @brief 停止记录日志
     */
    void stopLogging();

    /**
     * @brief 是否正在记录
     * @return true表示正在记录
     */
    bool isLogging() const;

public slots:
    /**
     * @brief 记录数据
     * @param data 数据内容
     * @param isReceived true表示接收，false表示发送
     */
    void logData(const QByteArray& data, bool isReceived);

private:
    QFile* m_logFile;          // 日志文件
    QTextStream* m_stream;     // 文本流
    bool m_isLogging;          // 是否正在记录
};

#endif // DATALOGGER_H
