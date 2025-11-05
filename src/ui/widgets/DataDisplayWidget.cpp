/**
 * @file DataDisplayWidget.cpp
 * @brief 数据显示界面实现
 * @author AI Assistant
 * @date 2025
 */

#include "DataDisplayWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDebug>

DataDisplayWidget::DataDisplayWidget(QWidget *parent)
    : QWidget(parent)
    , m_displayMode(ASCII)
    , m_encoding("UTF-8")
    , m_showTimestamp(true)
    , m_autoScroll(true)
    , m_dataCount(0)
{
    initUI();

    // 批量刷新定时器（20fps）
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(50);  // 50ms
    connect(m_updateTimer, &QTimer::timeout, this, &DataDisplayWidget::onUpdateDisplay);
    m_updateTimer->start();
}

DataDisplayWidget::~DataDisplayWidget() {
}

void DataDisplayWidget::initUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    // ========== 工具栏 ==========
    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);
    toolbarLayout->setContentsMargins(5, 5, 5, 5);

    // 显示模式
    toolbarLayout->addWidget(new QLabel("显示模式:", this));
    m_displayModeComboBox = new QComboBox(this);
    m_displayModeComboBox->addItem("ASCII");
    m_displayModeComboBox->addItem("HEX");
    m_displayModeComboBox->addItem("混合");
    connect(m_displayModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataDisplayWidget::onDisplayModeChanged);
    toolbarLayout->addWidget(m_displayModeComboBox);

    // 编码选择
    toolbarLayout->addWidget(new QLabel("编码:", this));
    m_encodingComboBox = new QComboBox(this);
    m_encodingComboBox->addItems({"UTF-8", "GBK", "ASCII", "GB2312"});
    connect(m_encodingComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataDisplayWidget::onEncodingChanged);
    toolbarLayout->addWidget(m_encodingComboBox);

    // 时间戳
    m_timestampCheckBox = new QCheckBox("时间戳", this);
    m_timestampCheckBox->setChecked(true);
    connect(m_timestampCheckBox, &QCheckBox::toggled, this, &DataDisplayWidget::onTimestampToggled);
    toolbarLayout->addWidget(m_timestampCheckBox);

    // 自动滚动
    m_autoScrollCheckBox = new QCheckBox("自动滚动", this);
    m_autoScrollCheckBox->setChecked(true);
    connect(m_autoScrollCheckBox, &QCheckBox::toggled, [this](bool checked) {
        m_autoScroll = checked;
    });
    toolbarLayout->addWidget(m_autoScrollCheckBox);

    toolbarLayout->addStretch();

    // 统计标签
    m_countLabel = new QLabel("数据: 0 条", this);
    toolbarLayout->addWidget(m_countLabel);

    // 清空按钮
    m_clearButton = new QPushButton("清空", this);
    connect(m_clearButton, &QPushButton::clicked, this, &DataDisplayWidget::onClearButtonClicked);
    toolbarLayout->addWidget(m_clearButton);

    // 保存按钮
    m_saveButton = new QPushButton("保存", this);
    connect(m_saveButton, &QPushButton::clicked, this, &DataDisplayWidget::onSaveButtonClicked);
    toolbarLayout->addWidget(m_saveButton);

    mainLayout->addLayout(toolbarLayout);

    // ========== 文本显示区 ==========
    m_textEdit = new QPlainTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    m_textEdit->setMaximumBlockCount(10000);  // 限制最大行数，避免内存溢出
    mainLayout->addWidget(m_textEdit);

    setLayout(mainLayout);
}

void DataDisplayWidget::appendReceivedData(const QByteArray& data) {
    m_pendingData.append(formatData(data, true));
    m_dataCount++;
}

void DataDisplayWidget::appendSentData(const QByteArray& data) {
    m_pendingData.append(formatData(data, false));
    m_dataCount++;
}

QString DataDisplayWidget::formatData(const QByteArray& data, bool isReceived) {
    QString result;

    // 时间戳
    if (m_showTimestamp) {
        result += QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz] ");
    }

    // 方向标识
    if (isReceived) {
        result += "<span style='color: #4ec9b0;'>[RX]</span> ";
    } else {
        result += "<span style='color: #ce9178;'>[TX]</span> ";
    }

    // 数据格式化
    switch (m_displayMode) {
    case ASCII: {
        // ASCII模式
        QTextCodec* codec = QTextCodec::codecForName(m_encoding.toUtf8());
        if (codec) {
            result += codec->toUnicode(data);
        } else {
            result += QString::fromUtf8(data);
        }
        break;
    }
    case HEX: {
        // HEX模式（空格分隔）
        QString hexStr = data.toHex(' ').toUpper();
        result += hexStr;
        break;
    }
    case MIX: {
        // 混合模式
        QString hexStr = data.toHex(' ').toUpper();
        QTextCodec* codec = QTextCodec::codecForName(m_encoding.toUtf8());
        QString asciiStr = codec ? codec->toUnicode(data) : QString::fromUtf8(data);
        result += hexStr + " | " + asciiStr;
        break;
    }
    }

    result += "\n";
    return result;
}

void DataDisplayWidget::clear() {
    m_textEdit->clear();
    m_pendingData.clear();
    m_dataCount = 0;
    m_countLabel->setText("数据: 0 条");
}

bool DataDisplayWidget::saveToFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件:" << filePath;
        return false;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << m_textEdit->toPlainText();
    file.close();

    qInfo() << "数据已保存到:" << filePath;
    return true;
}

void DataDisplayWidget::onDisplayModeChanged(int index) {
    m_displayMode = static_cast<DisplayMode>(index);
    // 清空重新显示（可选）
}

void DataDisplayWidget::onEncodingChanged(int index) {
    m_encoding = m_encodingComboBox->itemText(index);
}

void DataDisplayWidget::onTimestampToggled(bool checked) {
    m_showTimestamp = checked;
}

void DataDisplayWidget::onClearButtonClicked() {
    clear();
}

void DataDisplayWidget::onSaveButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "保存数据",
        QString(),
        "文本文件 (*.txt);;所有文件 (*.*)"
    );

    if (!fileName.isEmpty()) {
        if (saveToFile(fileName)) {
            QMessageBox::information(this, "成功", "数据已保存");
        } else {
            QMessageBox::critical(this, "错误", "保存失败");
        }
    }
}

void DataDisplayWidget::onUpdateDisplay() {
    if (m_pendingData.isEmpty()) {
        return;
    }

    // 批量追加数据
    m_textEdit->appendHtml(QString::fromUtf8(m_pendingData));
    m_pendingData.clear();

    // 更新统计
    m_countLabel->setText(QString("数据: %1 条").arg(m_dataCount));

    // 自动滚动到底部
    if (m_autoScroll) {
        QTextCursor cursor = m_textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_textEdit->setTextCursor(cursor);
    }
}
