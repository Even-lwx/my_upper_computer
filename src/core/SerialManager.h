/**
 * @file SerialManager.h
 * @brief 串口管理器 - 核心通信模块
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 多线程异步收发
 * - 高频数据处理
 * - 自动端口扫描
 * - 错误处理和重连
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QThread>
#include <QMutex>
#include <QQueue>

/**
 * @class SerialManager
 * @brief 串口管理器类
 *
 * 负责串口的打开、关闭、数据收发
 * 使用独立线程处理串口通信，保证UI流畅
 */
class SerialManager : public QObject {
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    /**
     * @brief 获取可用串口列表
     * @return 串口名称列表
     */
    static QStringList availablePorts();

    /**
     * @brief 打开串口
     * @param portName 串口名称
     * @param baudRate 波特率
     * @param dataBits 数据位（默认8位）
     * @param stopBits 停止位（默认1位）
     * @param parity 校验位（默认无校验）
     * @return 是否成功打开
     */
    bool openPort(const QString& portName,
                  int baudRate,
                  QSerialPort::DataBits dataBits = QSerialPort::Data8,
                  QSerialPort::StopBits stopBits = QSerialPort::OneStop,
                  QSerialPort::Parity parity = QSerialPort::NoParity);

    /**
     * @brief 关闭串口
     */
    void closePort();

    /**
     * @brief 判断串口是否打开
     * @return true表示已打开
     */
    bool isOpen() const;

    /**
     * @brief 获取当前端口名称
     * @return 端口名称
     */
    QString portName() const;

    /**
     * @brief 获取当前波特率
     * @return 波特率
     */
    int baudRate() const;

public slots:
    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @return 实际发送的字节数
     */
    qint64 sendData(const QByteArray& data);

signals:
    /**
     * @brief 串口已连接信号
     */
    void connected();

    /**
     * @brief 串口已断开信号
     */
    void disconnected();

    /**
     * @brief 数据接收信号
     * @param data 接收到的数据
     */
    void dataReceived(const QByteArray& data);

    /**
     * @brief 数据发送信号
     * @param data 发送的数据
     */
    void dataSent(const QByteArray& data);

    /**
     * @brief 接收字节数统计
     * @param bytes 字节数
     */
    void bytesReceived(qint64 bytes);

    /**
     * @brief 发送字节数统计
     * @param bytes 字节数
     */
    void bytesSent(qint64 bytes);

    /**
     * @brief 错误信号
     * @param error 错误描述
     */
    void errorOccurred(const QString& error);

private slots:
    /**
     * @brief 处理串口接收数据
     */
    void onReadyRead();

    /**
     * @brief 处理串口错误
     */
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort* m_serial;         // 串口对象
    QThread* m_thread;             // 独立线程
    QMutex m_mutex;                // 线程锁
    QString m_portName;            // 当前端口名
    int m_baudRate;                // 当前波特率
    bool m_isOpen;                 // 是否已打开

    // 接收缓冲区
    QByteArray m_receiveBuffer;
};

#endif // SERIALMANAGER_H
