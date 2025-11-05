/**
 * @file ChartWidget.cpp
 * @brief 实时数据曲线图表实现
 * @author AI Assistant
 * @date 2025
 */

#include "ChartWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_isRunning(false)
{
    initUI();
}

ChartWidget::~ChartWidget() {
}

void ChartWidget::initUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ========== 工具栏 ==========
    QHBoxLayout* toolbarLayout = new QHBoxLayout();

    m_startButton = new QPushButton("开始", this);
    connect(m_startButton, &QPushButton::clicked, this, &ChartWidget::onStartButtonClicked);
    toolbarLayout->addWidget(m_startButton);

    m_stopButton = new QPushButton("停止", this);
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &ChartWidget::onStopButtonClicked);
    toolbarLayout->addWidget(m_stopButton);

    m_clearButton = new QPushButton("清空", this);
    connect(m_clearButton, &QPushButton::clicked, this, &ChartWidget::onClearButtonClicked);
    toolbarLayout->addWidget(m_clearButton);

    m_exportButton = new QPushButton("导出", this);
    connect(m_exportButton, &QPushButton::clicked, this, &ChartWidget::onExportButtonClicked);
    toolbarLayout->addWidget(m_exportButton);

    toolbarLayout->addStretch();

    // 通道选择
    m_channel1CheckBox = new QCheckBox("通道1", this);
    m_channel1CheckBox->setChecked(true);
    toolbarLayout->addWidget(m_channel1CheckBox);

    m_channel2CheckBox = new QCheckBox("通道2", this);
    m_channel2CheckBox->setChecked(true);
    toolbarLayout->addWidget(m_channel2CheckBox);

    mainLayout->addLayout(toolbarLayout);

    // ========== 图表显示区（占位） ==========
    m_placeholderLabel = new QLabel(this);
    m_placeholderLabel->setAlignment(Qt::AlignCenter);
    m_placeholderLabel->setStyleSheet("QLabel { background-color: #252526; border: 1px solid #3e3e42; border-radius: 5px; color: #6d6d6d; font-size: 14pt; }");
    m_placeholderLabel->setText(
        "📊 实时图表区域\n\n"
        "暂未集成图表库\n"
        "可集成 QCustomPlot 或 QtCharts\n\n"
        "功能预览：\n"
        "• 多通道实时曲线\n"
        "• 波形缩放、拖拽\n"
        "• 数据导出 CSV"
    );
    mainLayout->addWidget(m_placeholderLabel);

    setLayout(mainLayout);
}

void ChartWidget::addDataPoint(int channel, double value) {
    // TODO: 实现数据点添加（集成图表库后实现）
    Q_UNUSED(channel);
    Q_UNUSED(value);
}

void ChartWidget::clearChart() {
    // TODO: 清空图表（集成图表库后实现）
    qDebug() << "图表已清空";
}

void ChartWidget::onStartButtonClicked() {
    m_isRunning = true;
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    qInfo() << "图表开始记录";
}

void ChartWidget::onStopButtonClicked() {
    m_isRunning = false;
    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    qInfo() << "图表停止记录";
}

void ChartWidget::onClearButtonClicked() {
    clearChart();
    QMessageBox::information(this, "提示", "图表已清空");
}

void ChartWidget::onExportButtonClicked() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "导出数据",
        QString(),
        "CSV文件 (*.csv);;所有文件 (*.*)"
    );

    if (!fileName.isEmpty()) {
        // TODO: 实现数据导出（集成图表库后实现）
        QMessageBox::information(this, "提示", "数据导出功能待实现\n将保存到: " + fileName);
    }
}
