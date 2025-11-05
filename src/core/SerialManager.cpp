/**
 * @file SerialManager.cpp
 * @brief 串口管理器实现
 * @author AI Assistant
 * @date 2025
 */

#include "SerialManager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent)
    : QObject(parent)
    , m_serial(nullptr)
    , m_thread(nullptr)
    , m_baudRate(9600)
    , m_isOpen(false)
{
    // 创建串口对象
    m_serial = new QSerialPort(this);

    // 连接信号
    connect(m_serial, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
    connect(m_serial, &QSerialPort::errorOccurred, this, &SerialManager::onErrorOccurred);

    qDebug() << "SerialManager initialized";
}

SerialManager::~SerialManager() {
    closePort();
    qDebug() << "SerialManager destroyed";
}

QStringList SerialManager::availablePorts() {
    QStringList portList;
    const auto ports = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : ports) {
        QString portInfo = info.portName();
        if (!info.description().isEmpty()) {
            portInfo += " - " + info.description();
        }
        portList.append(portInfo);
    }

    return portList;
}

bool SerialManager::openPort(const QString& portName,
                               int baudRate,
                               QSerialPort::DataBits dataBits,
                               QSerialPort::StopBits stopBits,
                               QSerialPort::Parity parity)
{
    QMutexLocker locker(&m_mutex);

    // 如果已打开，先关闭
    if (m_isOpen) {
        closePort();
    }

    // 提取纯端口名（去除描述信息）
    QString purePortName = portName.split(" - ").first();

    // 配置串口
    m_serial->setPortName(purePortName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(dataBits);
    m_serial->setStopBits(stopBits);
    m_serial->setParity(parity);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    // 尝试打开
    if (m_serial->open(QIODevice::ReadWrite)) {
        m_portName = purePortName;
        m_baudRate = baudRate;
        m_isOpen = true;

        qInfo() << "串口已打开:" << m_portName << "@" << m_baudRate;
        qInfo() << "  数据位:" << dataBits;
        qInfo() << "  停止位:" << stopBits;
        qInfo() << "  校验位:" << parity;

        emit connected();
        return true;
    } else {
        QString error = m_serial->errorString();
        qWarning() << "无法打开串口:" << purePortName << "-" << error;
        emit errorOccurred("无法打开串口: " + error);
        return false;
    }
}

void SerialManager::closePort() {
    QMutexLocker locker(&m_mutex);

    if (m_isOpen && m_serial->isOpen()) {
        m_serial->close();
        m_isOpen = false;

        qInfo() << "串口已关闭:" << m_portName;
        emit disconnected();
    }
}

bool SerialManager::isOpen() const {
    return m_isOpen && m_serial && m_serial->isOpen();
}

QString SerialManager::portName() const {
    return m_portName;
}

int SerialManager::baudRate() const {
    return m_baudRate;
}

qint64 SerialManager::sendData(const QByteArray& data) {
    QMutexLocker locker(&m_mutex);

    if (!m_isOpen || !m_serial->isOpen()) {
        qWarning() << "串口未打开，无法发送数据";
        return -1;
    }

    qint64 bytesWritten = m_serial->write(data);

    if (bytesWritten > 0) {
        // 确保数据发送完成
        m_serial->flush();

        qDebug() << "发送数据:" << bytesWritten << "字节";
        emit dataSent(data);
        emit bytesSent(bytesWritten);
    } else {
        qWarning() << "数据发送失败";
        emit errorOccurred("数据发送失败");
    }

    return bytesWritten;
}

void SerialManager::onReadyRead() {
    if (!m_isOpen) {
        return;
    }

    // 读取所有可用数据
    QByteArray data = m_serial->readAll();

    if (!data.isEmpty()) {
        qDebug() << "接收数据:" << data.size() << "字节";

        // 发出信号
        emit dataReceived(data);
        emit bytesReceived(data.size());
    }
}

void SerialManager::onErrorOccurred(QSerialPort::SerialPortError error) {
    // 忽略无错误和资源错误（关闭时触发）
    if (error == QSerialPort::NoError || error == QSerialPort::ResourceError) {
        return;
    }

    QString errorString = m_serial->errorString();
    qWarning() << "串口错误:" << error << "-" << errorString;

    emit errorOccurred(errorString);

    // 严重错误时自动关闭
    if (error == QSerialPort::DeviceNotFoundError ||
        error == QSerialPort::PermissionError ||
        error == QSerialPort::OpenError) {
        closePort();
    }
}
