/**
 * @file SendWidget.h
 * @brief 发送控制面板
 * @author AI Assistant
 * @date 2025
 */

#ifndef SENDWIDGET_H
#define SENDWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTimer>

/**
 * @class SendWidget
 * @brief 发送控制面板
 *
 * 功能：
 * - HEX/ASCII发送模式
 * - 定时发送
 * - 历史命令记录
 * - 发送计数统计
 */
class SendWidget : public QWidget {
    Q_OBJECT

public:
    explicit SendWidget(QWidget *parent = nullptr);
    ~SendWidget();

signals:
    /**
     * @brief 发送数据信号
     * @param data 要发送的数据
     */
    void sendData(const QByteArray& data);

private slots:
    void onSendButtonClicked();
    void onSendModeChanged(int index);
    void onTimerSendToggled(bool checked);
    void onTimerTimeout();
    void onClearInputClicked();

private:
    void initUI();
    QByteArray getInputData();

    // UI组件
    QTextEdit* m_sendTextEdit;
    QComboBox* m_sendModeComboBox;
    QComboBox* m_historyComboBox;
    QPushButton* m_sendButton;
    QPushButton* m_clearButton;

    // 定时发送
    QCheckBox* m_timerSendCheckBox;
    QSpinBox* m_timerIntervalSpinBox;
    QTimer* m_sendTimer;

    // 统计
    int m_sendCount;
    QLabel* m_sendCountLabel;

    // 设置
    enum SendMode { ASCII_MODE, HEX_MODE };
    SendMode m_sendMode;
};

#endif // SENDWIDGET_H
