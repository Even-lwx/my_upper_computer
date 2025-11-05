/**
 * @file SerialConfigWidget.cpp
 * @brief 串口配置界面实现
 * @author AI Assistant
 * @date 2025
 */

#include "SerialConfigWidget.h"
#include "core/SerialManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>

SerialConfigWidget::SerialConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    refreshPortList();
}

SerialConfigWidget::~SerialConfigWidget() {
}

void SerialConfigWidget::initUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);  // 增大间距
    mainLayout->setContentsMargins(16, 16, 16, 16);  // 增大边距

    // ========== 串口选择组 ==========
    m_portGroup = new QGroupBox("串口选择", this);
    QVBoxLayout* portLayout = new QVBoxLayout(m_portGroup);
    portLayout->setSpacing(12);  // 增大间距

    // 端口下拉框
    m_portComboBox = new QComboBox(this);
    m_portComboBox->setToolTip("选择串口");
    m_portComboBox->setMinimumHeight(36);  // 增大高度
    portLayout->addWidget(m_portComboBox);

    // 刷新按钮
    m_refreshButton = new QPushButton("刷新端口", this);
    m_refreshButton->setToolTip("刷新可用串口列表");
    m_refreshButton->setMinimumHeight(36);  // 增大高度
    connect(m_refreshButton, &QPushButton::clicked, this, &SerialConfigWidget::refreshPortList);
    portLayout->addWidget(m_refreshButton);

    mainLayout->addWidget(m_portGroup);

    // ========== 参数配置组 ==========
    m_paramsGroup = new QGroupBox("通信参数", this);
    QFormLayout* paramsLayout = new QFormLayout(m_paramsGroup);
    paramsLayout->setSpacing(12);  // 增大间距
    paramsLayout->setVerticalSpacing(12);  // 垂直间距
    paramsLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);  // 标签右对齐

    // 波特率
    m_baudRateComboBox = new QComboBox(this);
    m_baudRateComboBox->addItems({
        "1200", "2400", "4800", "9600", "19200", "38400",
        "57600", "115200", "230400", "460800", "921600"
    });
    m_baudRateComboBox->setCurrentText("115200");  // 默认115200
    m_baudRateComboBox->setEditable(true);  // 允许自定义
    m_baudRateComboBox->setMinimumHeight(36);  // 增大高度
    paramsLayout->addRow("波特率:", m_baudRateComboBox);

    // 数据位
    m_dataBitsComboBox = new QComboBox(this);
    m_dataBitsComboBox->addItems({"5", "6", "7", "8"});
    m_dataBitsComboBox->setCurrentText("8");
    m_dataBitsComboBox->setMinimumHeight(36);  // 增大高度
    paramsLayout->addRow("数据位:", m_dataBitsComboBox);

    // 停止位
    m_stopBitsComboBox = new QComboBox(this);
    m_stopBitsComboBox->addItem("1", QSerialPort::OneStop);
    m_stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
    m_stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
    m_stopBitsComboBox->setCurrentIndex(0);
    m_stopBitsComboBox->setMinimumHeight(36);  // 增大高度
    paramsLayout->addRow("停止位:", m_stopBitsComboBox);

    // 校验位
    m_parityComboBox = new QComboBox(this);
    m_parityComboBox->addItem("无校验", QSerialPort::NoParity);
    m_parityComboBox->addItem("奇校验", QSerialPort::OddParity);
    m_parityComboBox->addItem("偶校验", QSerialPort::EvenParity);
    m_parityComboBox->setCurrentIndex(0);
    m_parityComboBox->setMinimumHeight(36);  // 增大高度
    paramsLayout->addRow("校验位:", m_parityComboBox);

    mainLayout->addWidget(m_paramsGroup);

    // ========== 连接按钮 ==========
    m_connectButton = new QPushButton("连接串口", this);  // 改进按钮文字
    m_connectButton->setObjectName("connectButton");
    m_connectButton->setMinimumHeight(44);  // 更大的按钮
    m_connectButton->setStyleSheet("font-size: 11pt; font-weight: bold;");  // 更大更粗的字体
    connect(m_connectButton, &QPushButton::clicked, this, &SerialConfigWidget::onConnectButtonClicked);
    mainLayout->addWidget(m_connectButton);

    // 弹簧，推向顶部
    mainLayout->addStretch();

    setLayout(mainLayout);
}

void SerialConfigWidget::refreshPortList() {
    m_portComboBox->clear();
    QStringList ports = SerialManager::availablePorts();

    if (ports.isEmpty()) {
        m_portComboBox->addItem("无可用串口");
        m_portComboBox->setEnabled(false);
        m_connectButton->setEnabled(false);
    } else {
        m_portComboBox->addItems(ports);
        m_portComboBox->setEnabled(true);
        m_connectButton->setEnabled(true);
    }
}

QString SerialConfigWidget::getSelectedPort() const {
    return m_portComboBox->currentText();
}

int SerialConfigWidget::getBaudRate() const {
    return m_baudRateComboBox->currentText().toInt();
}

QSerialPort::DataBits SerialConfigWidget::getDataBits() const {
    int bits = m_dataBitsComboBox->currentText().toInt();
    return static_cast<QSerialPort::DataBits>(bits);
}

QSerialPort::StopBits SerialConfigWidget::getStopBits() const {
    return static_cast<QSerialPort::StopBits>(m_stopBitsComboBox->currentData().toInt());
}

QSerialPort::Parity SerialConfigWidget::getParity() const {
    return static_cast<QSerialPort::Parity>(m_parityComboBox->currentData().toInt());
}

void SerialConfigWidget::onConnectButtonClicked() {
    emit connectRequest();
}
