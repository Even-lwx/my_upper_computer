/**
 * @file SerialConfigWidget.h
 * @brief 串口配置界面
 * @author AI Assistant
 * @date 2025
 */

#ifndef SERIALCONFIGWIDGET_H
#define SERIALCONFIGWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QSerialPort>

/**
 * @class SerialConfigWidget
 * @brief 串口配置面板
 *
 * 提供串口参数配置界面：
 * - 端口选择
 * - 波特率选择
 * - 数据位、停止位、校验位
 * - 连接/断开按钮
 */
class SerialConfigWidget : public QWidget {
    Q_OBJECT

public:
    explicit SerialConfigWidget(QWidget *parent = nullptr);
    ~SerialConfigWidget();

    /**
     * @brief 获取选中的端口名
     */
    QString getSelectedPort() const;

    /**
     * @brief 获取波特率
     */
    int getBaudRate() const;

    /**
     * @brief 获取数据位
     */
    QSerialPort::DataBits getDataBits() const;

    /**
     * @brief 获取停止位
     */
    QSerialPort::StopBits getStopBits() const;

    /**
     * @brief 获取校验位
     */
    QSerialPort::Parity getParity() const;

public slots:
    /**
     * @brief 刷新串口列表
     */
    void refreshPortList();

signals:
    /**
     * @brief 连接请求信号
     */
    void connectRequest();

private slots:
    void onConnectButtonClicked();

private:
    void initUI();

    // UI组件
    QGroupBox* m_portGroup;
    QComboBox* m_portComboBox;
    QPushButton* m_refreshButton;

    QGroupBox* m_paramsGroup;
    QComboBox* m_baudRateComboBox;
    QComboBox* m_dataBitsComboBox;
    QComboBox* m_stopBitsComboBox;
    QComboBox* m_parityComboBox;

    QPushButton* m_connectButton;
};

#endif // SERIALCONFIGWIDGET_H
