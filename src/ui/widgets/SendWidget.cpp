/**
 * @file SendWidget.cpp
 * @brief 发送控制面板实现
 * @author AI Assistant
 * @date 2025
 */

#include "SendWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QDebug>

SendWidget::SendWidget(QWidget *parent)
    : QWidget(parent)
    , m_sendCount(0)
    , m_sendMode(ASCII_MODE)
{
    initUI();

    // 定时发送定时器
    m_sendTimer = new QTimer(this);
    connect(m_sendTimer, &QTimer::timeout, this, &SendWidget::onTimerTimeout);
}

SendWidget::~SendWidget() {
}

void SendWidget::initUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ========== 发送模式选择 ==========
    QHBoxLayout* modeLayout = new QHBoxLayout();
    modeLayout->addWidget(new QLabel("发送模式:", this));

    m_sendModeComboBox = new QComboBox(this);
    m_sendModeComboBox->addItem("ASCII");
    m_sendModeComboBox->addItem("HEX");
    connect(m_sendModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SendWidget::onSendModeChanged);
    modeLayout->addWidget(m_sendModeComboBox);

    modeLayout->addStretch();
    mainLayout->addLayout(modeLayout);

    // ========== 发送文本框 ==========
    m_sendTextEdit = new QTextEdit(this);
    m_sendTextEdit->setPlaceholderText("输入要发送的数据...");
    m_sendTextEdit->setMaximumHeight(120);
    mainLayout->addWidget(m_sendTextEdit);

    // ========== 历史命令 ==========
    QHBoxLayout* historyLayout = new QHBoxLayout();
    historyLayout->addWidget(new QLabel("历史命令:", this));

    m_historyComboBox = new QComboBox(this);
    m_historyComboBox->setEditable(false);
    connect(m_historyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this](int index) {
                if (index >= 0) {
                    m_sendTextEdit->setText(m_historyComboBox->itemText(index));
                }
            });
    historyLayout->addWidget(m_historyComboBox);

    mainLayout->addLayout(historyLayout);

    // ========== 定时发送 ==========
    QGroupBox* timerGroup = new QGroupBox("定时发送", this);
    QHBoxLayout* timerLayout = new QHBoxLayout(timerGroup);

    m_timerSendCheckBox = new QCheckBox("启用", this);
    connect(m_timerSendCheckBox, &QCheckBox::toggled, this, &SendWidget::onTimerSendToggled);
    timerLayout->addWidget(m_timerSendCheckBox);

    timerLayout->addWidget(new QLabel("间隔:", this));
    m_timerIntervalSpinBox = new QSpinBox(this);
    m_timerIntervalSpinBox->setRange(10, 10000);
    m_timerIntervalSpinBox->setValue(1000);
    m_timerIntervalSpinBox->setSuffix(" ms");
    timerLayout->addWidget(m_timerIntervalSpinBox);

    timerLayout->addStretch();
    mainLayout->addWidget(timerGroup);

    // ========== 发送按钮 ==========
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    m_clearButton = new QPushButton("清空", this);
    connect(m_clearButton, &QPushButton::clicked, this, &SendWidget::onClearInputClicked);
    buttonLayout->addWidget(m_clearButton);

    buttonLayout->addStretch();

    m_sendCountLabel = new QLabel("已发送: 0 次", this);
    buttonLayout->addWidget(m_sendCountLabel);

    m_sendButton = new QPushButton("发送", this);
    m_sendButton->setMinimumHeight(35);
    connect(m_sendButton, &QPushButton::clicked, this, &SendWidget::onSendButtonClicked);
    buttonLayout->addWidget(m_sendButton);

    mainLayout->addLayout(buttonLayout);

    // 弹簧
    mainLayout->addStretch();

    setLayout(mainLayout);
}

QByteArray SendWidget::getInputData() {
    QString text = m_sendTextEdit->toPlainText();

    if (text.isEmpty()) {
        return QByteArray();
    }

    QByteArray data;

    if (m_sendMode == HEX_MODE) {
        // HEX模式：解析十六进制字符串
        // 移除空格和非法字符
        text.remove(QRegExp("[^0-9A-Fa-f]"));

        // 转换为字节数组
        data = QByteArray::fromHex(text.toUtf8());

        if (data.isEmpty()) {
            QMessageBox::warning(this, "警告", "HEX数据格式错误");
        }
    } else {
        // ASCII模式：直接转换
        data = text.toUtf8();
    }

    return data;
}

void SendWidget::onSendButtonClicked() {
    QByteArray data = getInputData();

    if (data.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送数据为空");
        return;
    }

    // 发送数据
    emit sendData(data);

    // 更新统计
    m_sendCount++;
    m_sendCountLabel->setText(QString("已发送: %1 次").arg(m_sendCount));

    // 添加到历史
    QString text = m_sendTextEdit->toPlainText();
    if (m_historyComboBox->findText(text) == -1) {
        m_historyComboBox->addItem(text);

        // 限制历史记录数量
        if (m_historyComboBox->count() > 20) {
            m_historyComboBox->removeItem(0);
        }
    }

    qDebug() << "发送数据:" << data.size() << "字节";
}

void SendWidget::onSendModeChanged(int index) {
    m_sendMode = static_cast<SendMode>(index);

    if (m_sendMode == HEX_MODE) {
        m_sendTextEdit->setPlaceholderText("输入十六进制数据 (如: AA 55 01 02)");
    } else {
        m_sendTextEdit->setPlaceholderText("输入ASCII文本数据");
    }
}

void SendWidget::onTimerSendToggled(bool checked) {
    if (checked) {
        int interval = m_timerIntervalSpinBox->value();
        m_sendTimer->start(interval);
        m_sendButton->setEnabled(false);
        qInfo() << "定时发送已启用，间隔:" << interval << "ms";
    } else {
        m_sendTimer->stop();
        m_sendButton->setEnabled(true);
        qInfo() << "定时发送已停止";
    }
}

void SendWidget::onTimerTimeout() {
    // 定时发送
    onSendButtonClicked();
}

void SendWidget::onClearInputClicked() {
    m_sendTextEdit->clear();
    m_sendCount = 0;
    m_sendCountLabel->setText("已发送: 0 次");
}
