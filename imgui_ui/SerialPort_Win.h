/**
 * @file SerialPort_Win.h
 * @brief Windows串口通信管理器
 * @author AI Assistant
 * @date 2025
 * 
 * 使用Windows API实现串口通信功能
 */

#ifndef SERIALPORT_WIN_H
#define SERIALPORT_WIN_H

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <windows.h>

/**
 * @brief 串口配置参数
 */
struct SerialConfig {
    std::string portName = "COM1";       // 串口名称
    int baudRate = 115200;                // 波特率
    int dataBits = 8;                     // 数据位 (5, 6, 7, 8)
    int stopBits = 1;                     // 停止位 (1=ONESTOPBIT, 2=TWOSTOPBITS)
    int parity = 0;                       // 校验位 (0=NOPARITY, 1=ODDPARITY, 2=EVENPARITY)
};

/**
 * @brief Windows串口管理器
 */
class SerialPort_Win {
public:
    SerialPort_Win();
    ~SerialPort_Win();

    /**
     * @brief 枚举所有可用串口
     * @return 串口名称列表
     */
    static std::vector<std::string> EnumeratePorts();

    /**
     * @brief 打开串口
     * @param config 串口配置参数
     * @return 成功返回true，失败返回false
     */
    bool Open(const SerialConfig& config);

    /**
     * @brief 关闭串口
     */
    void Close();

    /**
     * @brief 判断串口是否已打开
     * @return 已打开返回true，否则返回false
     */
    bool IsOpen() const;

    /**
     * @brief 发送数据
     * @param data 要发送的数据
     * @param length 数据长度
     * @return 实际发送的字节数，失败返回-1
     */
    int Write(const unsigned char* data, int length);

    /**
     * @brief 发送字符串
     * @param str 要发送的字符串
     * @return 实际发送的字节数，失败返回-1
     */
    int Write(const std::string& str);

    /**
     * @brief 设置数据接收回调函数
     * @param callback 回调函数，参数为接收到的数据和长度
     */
    void SetReceiveCallback(std::function<void(const unsigned char*, int)> callback);

    /**
     * @brief 获取最后一次错误信息
     * @return 错误信息字符串
     */
    std::string GetLastError() const;

    /**
     * @brief 清空接收缓冲区
     */
    void ClearReceiveBuffer();

    /**
     * @brief 清空发送缓冲区
     */
    void ClearTransmitBuffer();

private:
    /**
     * @brief 配置串口参数
     * @param config 串口配置参数
     * @return 成功返回true，失败返回false
     */
    bool ConfigurePort(const SerialConfig& config);

    /**
     * @brief 接收线程函数
     */
    void ReceiveThread();

    HANDLE hComm_;                                              // 串口句柄
    std::atomic<bool> isOpen_;                                  // 串口是否打开
    std::atomic<bool> isReceiving_;                             // 是否正在接收数据
    std::thread receiveThread_;                                 // 接收线程
    std::function<void(const unsigned char*, int)> receiveCallback_;  // 接收回调函数
    mutable std::mutex mutex_;                                  // 互斥锁
    std::string lastError_;                                     // 最后一次错误信息
    SerialConfig currentConfig_;                                // 当前配置
};

#endif // SERIALPORT_WIN_H
