/**
 * @file DataDisplayWidget.h
 * @brief 数据显示界面
 * @author AI Assistant
 * @date 2025
 */

#ifndef DATADISPLAYWIDGET_H
#define DATADISPLAYWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

/**
 * @class DataDisplayWidget
 * @brief 数据显示组件
 *
 * 功能：
 * - HEX/ASCII显示切换
 * - 编码格式选择（UTF-8/GBK/ASCII）
 * - 时间戳显示
 * - 自动滚动
 * - 数据保存
 */
class DataDisplayWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        ASCII,      ///< ASCII文本模式
        HEX,        ///< 十六进制模式
        MIX         ///< 混合模式
    };

    explicit DataDisplayWidget(QWidget *parent = nullptr);
    ~DataDisplayWidget();

public slots:
    /**
     * @brief 追加接收数据
     * @param data 接收的数据
     */
    void appendReceivedData(const QByteArray& data);

    /**
     * @brief 追加发送数据
     * @param data 发送的数据
     */
    void appendSentData(const QByteArray& data);

    /**
     * @brief 清空显示
     */
    void clear();

    /**
     * @brief 保存到文件
     * @param filePath 文件路径
     * @return 是否成功
     */
    bool saveToFile(const QString& filePath);

private slots:
    void onDisplayModeChanged(int index);
    void onEncodingChanged(int index);
    void onTimestampToggled(bool checked);
    void onClearButtonClicked();
    void onSaveButtonClicked();
    void onUpdateDisplay();  // 定时批量更新

private:
    void initUI();
    QString formatData(const QByteArray& data, bool isReceived);

    // UI组件
    QPlainTextEdit* m_textEdit;
    QComboBox* m_displayModeComboBox;
    QComboBox* m_encodingComboBox;
    QCheckBox* m_timestampCheckBox;
    QCheckBox* m_autoScrollCheckBox;
    QPushButton* m_clearButton;
    QPushButton* m_saveButton;
    QLabel* m_countLabel;

    // 数据缓冲（用于批量刷新）
    QByteArray m_pendingData;
    QTimer* m_updateTimer;

    // 设置
    DisplayMode m_displayMode;
    QString m_encoding;
    bool m_showTimestamp;
    bool m_autoScroll;
    int m_dataCount;  // 数据条数统计
};

#endif // DATADISPLAYWIDGET_H
