/**
 * @file ChartWidget.h
 * @brief 实时数据曲线图表
 * @author AI Assistant
 * @date 2025
 */

#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

/**
 * @class ChartWidget
 * @brief 实时数据曲线图表组件
 *
 * 功能：
 * - 多通道实时曲线显示
 * - 波形缩放、拖拽
 * - 数据导出
 *
 * 注意：这是一个基础框架，后续可集成QCustomPlot或QtCharts
 */
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();

public slots:
    /**
     * @brief 添加数据点
     * @param channel 通道编号（0-7）
     * @param value 数据值
     */
    void addDataPoint(int channel, double value);

    /**
     * @brief 清空图表
     */
    void clearChart();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onClearButtonClicked();
    void onExportButtonClicked();

private:
    void initUI();

    // UI组件
    QLabel* m_placeholderLabel;  // 占位标签（未集成图表库时显示）
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_clearButton;
    QPushButton* m_exportButton;
    QCheckBox* m_channel1CheckBox;
    QCheckBox* m_channel2CheckBox;

    bool m_isRunning;
};

#endif // CHARTWIDGET_H
