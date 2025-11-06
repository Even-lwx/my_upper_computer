/**
 * @file AppState.h
 * @brief 应用程序状态结构定义
 * @author AI Assistant
 * @date 2025
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <queue>
#include <chrono>
#include <future>
#include <imgui.h>
#include "ThreadPool.h"
#include "../ui/VisualizationUI.h"
#include "../SerialPort_Win.h"

// 视图类型枚举
enum class ViewType {
    SERIAL_TERMINAL,  // 串口收发（默认）
    WAVEFORM,         // 波形显示
};

// 应用程序状态
struct AppState {
    // 当前视图
    ViewType current_view = ViewType::SERIAL_TERMINAL;

    bool show_demo_window = false;  // ImGui演示窗口
    ImVec4 clear_color = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);  // 背景色

    // 串口配置
    std::vector<std::string> available_ports;  // 可用串口列表（向后兼容）
    std::vector<SerialPortInfo> available_ports_info;  // 串口详细信息列表
    std::future<std::vector<SerialPortInfo>> port_enum_future;  // 异步枚举任务
    bool ports_enumerating = false;  // 是否正在枚举
    bool ports_enumerated = false;   // 是否已完成枚举
    int selected_port_index = 0;
    int selected_baudrate_index = 7;  // 默认115200
    int selected_databits_index = 3;  // 默认8位
    int selected_stopbits_index = 0;  // 默认1位
    int selected_parity_index = 0;    // 默认无校验
    bool is_connected = false;

    // 串口管理器
    SerialPort_Win serial_port;
    std::mutex receive_mutex;  // 接收数据互斥锁

    // 数据显示
    char receive_buffer[65536] = "";
    size_t receive_buffer_pos = 0;  // 跟踪缓冲区当前位置
    char send_buffer[1024] = "";
    bool hex_display = false;
    bool hex_send = false;
    bool auto_scroll = true;
    bool scroll_to_bottom = false;  // 标记需要滚动到底部

    // 统计信息
    int bytes_received = 0;
    int bytes_sent = 0;

    // 日志功能
    bool enable_logging = false;
    std::string log_filename;

    // 定时发送
    bool enable_auto_send = false;
    int auto_send_interval_ms = 1000;  // 默认1秒
    std::chrono::steady_clock::time_point last_send_time;

    // 可视化系统
    VisualizationUI visualization_ui;

    // 线程池配置
    struct ThreadConfig {
        int num_worker_threads = 2;  // 默认2个工作线程
        bool enable_multithreading = true;  // 默认启用多线程
    };
    ThreadConfig thread_config;
    std::unique_ptr<ThreadPool> thread_pool;

    // 数据处理队列
    struct DataPacket {
        std::vector<unsigned char> data;
        std::chrono::steady_clock::time_point timestamp;
    };
    std::queue<DataPacket> data_queue;
    std::mutex queue_mutex;

    // UI状态
    bool show_settings_dialog = false;  // 显示设置对话框
};

#endif // APP_STATE_H
