/**
 * @file main.cpp (ImGui Version)
 * @brief 串口调试助手 - ImGui版本主程序
 * @author AI Assistant
 * @date 2025
 *
 * 使用Dear ImGui + GLFW + OpenGL3
 * 特性：游戏级美观UI、60fps流畅渲染、现代化设计
 */

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <implot_internal.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <mutex>
#include <fstream>
#include <chrono>

// Windows OpenGL
#if defined(_WIN32)
#include <windows.h>
#endif

// 串口和数据转换
#include "SerialPort_Win.h"
#include "DataConverter.h"

// 可视化系统
#include "ui/VisualizationUI.h"

// 线程池
#include "core/ThreadPool.h"
#include <queue>
#include <memory>

// 应用程序状态和配置管理
#include "core/AppState.h"
#include "core/ConfigManager.h"

// 视图类型枚举已移至AppState.h
// 应用程序状态结构已移至AppState.h

// GLFW错误回调
static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// 设置ImGui样式 - 现代深色主题
void SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // 主色调配置
    const ImVec4 bg_color = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);           // 深蓝灰背景
    const ImVec4 widget_bg = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);          // 控件背景
    const ImVec4 widget_bg_hover = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);   // 悬停
    const ImVec4 widget_bg_active = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);  // 激活
    const ImVec4 accent_color = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);      // 强调色（蓝色）
    const ImVec4 text_color = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);        // 文字颜色

    // 圆角设置
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 6.0f;

    // 间距设置
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;

    // 边框
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    // 颜色主题
    colors[ImGuiCol_Text] = text_color;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = bg_color;
    colors[ImGuiCol_ChildBg] = widget_bg;
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.14f, 0.17f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.27f, 0.30f, 0.80f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = widget_bg;
    colors[ImGuiCol_FrameBgHovered] = widget_bg_hover;
    colors[ImGuiCol_FrameBgActive] = widget_bg_active;
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.10f, 0.13f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.27f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = accent_color;
    colors[ImGuiCol_CheckMark] = accent_color;
    colors[ImGuiCol_SliderGrab] = accent_color;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.36f, 0.69f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = widget_bg;
    colors[ImGuiCol_ButtonHovered] = widget_bg_hover;
    colors[ImGuiCol_ButtonActive] = widget_bg_active;
    colors[ImGuiCol_Header] = widget_bg_hover;
    colors[ImGuiCol_HeaderHovered] = widget_bg_active;
    colors[ImGuiCol_HeaderActive] = accent_color;
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.27f, 0.30f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.37f, 0.40f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = accent_color;
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.27f, 0.30f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.37f, 0.40f, 0.75f);
    colors[ImGuiCol_ResizeGripActive] = accent_color;
    colors[ImGuiCol_Tab] = widget_bg;
    colors[ImGuiCol_TabHovered] = widget_bg_hover;
    colors[ImGuiCol_TabActive] = widget_bg_active;
    colors[ImGuiCol_TabUnfocused] = widget_bg;
    colors[ImGuiCol_TabUnfocusedActive] = widget_bg_hover;
    colors[ImGuiCol_PlotLines] = accent_color;
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = accent_color;
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = accent_color;
    colors[ImGuiCol_NavHighlight] = accent_color;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.70f);
}

// 渲染侧边栏（左侧功能按钮栏）
void RenderSidebar(AppState& state) {
    ImGui::BeginChild("##Sidebar", ImVec2(80, 0), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.16f, 0.20f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.25f, 0.30f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.59f, 0.98f, 1.00f));

    // 串口收发按钮
    bool is_terminal_selected = (state.current_view == ViewType::SERIAL_TERMINAL);
    if (is_terminal_selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.6f));
    }
    if (ImGui::Button("串口\n收发", ImVec2(70, 70))) {
        state.current_view = ViewType::SERIAL_TERMINAL;
    }
    if (is_terminal_selected) {
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // 波形图按钮
    bool is_waveform_selected = (state.current_view == ViewType::WAVEFORM);
    if (is_waveform_selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.6f));
    }
    if (ImGui::Button("波形\n显示", ImVec2(70, 70))) {
        state.current_view = ViewType::WAVEFORM;
    }
    if (is_waveform_selected) {
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();

    // 系统设置按钮
    if (ImGui::Button("系统\n设置", ImVec2(70, 70))) {
        state.show_settings_dialog = true;
    }

    ImGui::PopStyleColor(3);
    ImGui::EndChild();
}

// 数据处理函数（在后台线程中执行）
void ProcessDataPacket(AppState* state, const std::vector<unsigned char>& data) {
    size_t length = data.size();

    // 传递原始数据给可视化系统
    state->visualization_ui.ProcessReceivedData(data.data(), length);

    // 格式转换
    std::string dataStr;
    if (state->hex_display) {
        dataStr = DataConverter::BytesToHexString(data.data(), length, true);
    } else {
        dataStr = DataConverter::BytesToAsciiString(data.data(), length, true);
    }

    // 准备日志数据
    bool should_log = false;
    std::string log_filename_copy;
    if (state->enable_logging && !state->log_filename.empty()) {
        should_log = true;
        log_filename_copy = state->log_filename;
    }

    // 缩小锁范围：只在必要时持有锁
    {
        std::lock_guard<std::mutex> lock(state->receive_mutex);

        // 追加到接收缓冲区
        size_t available_space = sizeof(state->receive_buffer) - state->receive_buffer_pos - 1;
        size_t data_len = dataStr.length();

        if (data_len + 1 < available_space) {
            memcpy(state->receive_buffer + state->receive_buffer_pos, dataStr.c_str(), data_len);
            state->receive_buffer_pos += data_len;
            state->receive_buffer[state->receive_buffer_pos++] = '\n';
            state->receive_buffer[state->receive_buffer_pos] = '\0';
        }

        state->bytes_received += length;
        state->scroll_to_bottom = true;
    }

    // 文件写入在锁外执行（避免阻塞）
    if (should_log) {
        std::ofstream logFile(log_filename_copy, std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now_c);
            char timeStr[64];
            strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S] ", &timeinfo);

            logFile << timeStr << dataStr << std::endl;
            logFile.close();
        }
    }
}

// 渲染串口配置面板
void RenderSerialConfigPanel(AppState& state) {
    // 面板标题
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "串口配置");
    ImGui::Separator();
    ImGui::Spacing();

    // 串口选择
    ImGui::Text("串口端口");

    // 创建串口列表的char*数组供ImGui使用
    std::vector<const char*> port_cstrs;
    for (const auto& port : state.available_ports) {
        port_cstrs.push_back(port.c_str());
    }

    ImGui::PushItemWidth(300);
    if (!state.available_ports.empty()) {
        ImGui::Combo("##port", &state.selected_port_index, port_cstrs.data(), port_cstrs.size());
    } else {
        ImGui::Text("未找到串口");
    }
    ImGui::PopItemWidth();

    if (ImGui::Button("刷新端口", ImVec2(300, 40))) {
        // 刷新串口列表
        state.available_ports = SerialPort_Win::EnumeratePorts();
        if (state.selected_port_index >= (int)state.available_ports.size()) {
            state.selected_port_index = 0;
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    // 通信参数
    ImGui::Text("通信参数");

    ImGui::Text("波特率:");
    const char* baudrates[] = { "1200", "2400", "4800", "9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600" };
    ImGui::PushItemWidth(300);
    ImGui::Combo("##baudrate", &state.selected_baudrate_index, baudrates, IM_ARRAYSIZE(baudrates));

    ImGui::Text("数据位:");
    const char* databits[] = { "5", "6", "7", "8" };
    ImGui::Combo("##databits", &state.selected_databits_index, databits, IM_ARRAYSIZE(databits));

    ImGui::Text("停止位:");
    const char* stopbits[] = { "1", "1.5", "2" };
    ImGui::Combo("##stopbits", &state.selected_stopbits_index, stopbits, IM_ARRAYSIZE(stopbits));

    ImGui::Text("校验位:");
    const char* parity[] = { "无校验", "奇校验", "偶校验" };
    ImGui::Combo("##parity", &state.selected_parity_index, parity, IM_ARRAYSIZE(parity));
    ImGui::PopItemWidth();

    ImGui::Separator();
    ImGui::Spacing();

    // 连接按钮
    ImVec4 button_color = state.is_connected ? ImVec4(0.8f, 0.2f, 0.2f, 1.0f) : ImVec4(0.2f, 0.7f, 0.3f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_color.x * 1.2f, button_color.y * 1.2f, button_color.z * 1.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_color.x * 0.8f, button_color.y * 0.8f, button_color.z * 0.8f, 1.0f));

    if (ImGui::Button(state.is_connected ? "断开串口" : "连接串口", ImVec2(300, 50))) {
        if (!state.is_connected) {
            // 连接串口
            if (!state.available_ports.empty() && state.selected_port_index < (int)state.available_ports.size()) {
                SerialConfig config;
                config.portName = state.available_ports[state.selected_port_index];

                // 波特率映射
                const int baudrate_values[] = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
                config.baudRate = baudrate_values[state.selected_baudrate_index];

                // 数据位映射
                config.dataBits = 5 + state.selected_databits_index;

                // 停止位映射
                config.stopBits = 1 + state.selected_stopbits_index;

                // 校验位映射
                config.parity = state.selected_parity_index;

                // 打开串口
                if (state.serial_port.Open(config)) {
                    state.is_connected = true;

                    // 初始化定时发送计时器
                    state.last_send_time = std::chrono::steady_clock::now();

                    // 设置接收回调（异步处理模式）
                    state.serial_port.SetReceiveCallback([&state](const unsigned char* data, int length) {
                        // 复制数据到队列（快速操作，不阻塞）
                        std::vector<unsigned char> data_copy(data, data + length);

                        if (state.thread_config.enable_multithreading && state.thread_pool) {
                            // 多线程模式：提交到线程池异步处理
                            state.thread_pool->Enqueue(ProcessDataPacket, &state, data_copy);
                        } else {
                            // 单线程模式：直接同步处理
                            ProcessDataPacket(&state, data_copy);
                        }
                    });
                }
            }
        } else {
            // 断开串口
            state.serial_port.Close();
            state.is_connected = false;
        }
    }

    ImGui::PopStyleColor(3);
}

// 渲染数据显示面板
void RenderDataDisplayPanel(AppState& state) {
    // 面板标题
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "数据显示");
    ImGui::Separator();
    ImGui::Spacing();

    // 显示选项
    ImGui::Checkbox("HEX显示", &state.hex_display);
    ImGui::SameLine();
    ImGui::Checkbox("自动滚动", &state.auto_scroll);
    ImGui::SameLine();
    if (ImGui::Button("清空")) {
        state.receive_buffer[0] = '\0';
        state.receive_buffer_pos = 0;
        state.bytes_received = 0;
    }

    // 日志控制
    ImGui::Checkbox("保存日志", &state.enable_logging);
    if (state.enable_logging && state.log_filename.empty()) {
        // 自动生成日志文件名
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        struct tm timeinfo;
        localtime_s(&timeinfo, &now_c);
        char filename[128];
        strftime(filename, sizeof(filename), "serial_log_%Y%m%d_%H%M%S.txt", &timeinfo);
        state.log_filename = filename;
    }
    if (state.enable_logging) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "Logging: %s", state.log_filename.c_str());
    }

    ImGui::Separator();

    // 数据显示区（多行文本框）
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.10f, 0.13f, 1.00f));
    ImGui::InputTextMultiline("##receive", state.receive_buffer, sizeof(state.receive_buffer),
                               ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - 30),
                               ImGuiInputTextFlags_ReadOnly);

    // 自动滚动到底部
    if (state.auto_scroll && state.scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
        state.scroll_to_bottom = false;
    }

    ImGui::PopStyleColor();

    // 统计信息
    ImGui::Text("已接收: %d 字节", state.bytes_received);
}

// 渲染发送控制面板
void RenderSendPanel(AppState& state) {
    // 面板标题
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "发送控制");
    ImGui::Separator();
    ImGui::Spacing();

    // 发送选项
    ImGui::Checkbox("HEX发送", &state.hex_send);

    ImGui::Separator();
    ImGui::Spacing();

    // 定时发送
    ImGui::Text("定时发送:");
    ImGui::Checkbox("启用定时发送", &state.enable_auto_send);

    if (state.enable_auto_send) {
        ImGui::Text("发送间隔(毫秒):");
        ImGui::PushItemWidth(200);
        ImGui::InputInt("##interval", &state.auto_send_interval_ms, 100, 1000);
        ImGui::PopItemWidth();

        // 限制间隔范围
        if (state.auto_send_interval_ms < 100) state.auto_send_interval_ms = 100;
        if (state.auto_send_interval_ms > 60000) state.auto_send_interval_ms = 60000;

        // 显示下次发送倒计时
        if (state.is_connected) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.last_send_time).count();
            int remaining = state.auto_send_interval_ms - elapsed;
            if (remaining < 0) remaining = 0;
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "下次发送: %d ms", remaining);
        } else {
            ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), "未连接");
        }
    }

    ImGui::Separator();

    // 发送输入框
    ImGui::Text("发送数据:");
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::InputTextMultiline("##send", state.send_buffer, sizeof(state.send_buffer), ImVec2(-FLT_MIN, 150));
    ImGui::PopItemWidth();

    // 发送按钮
    ImVec4 send_button_color = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, send_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.00f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.49f, 0.88f, 1.0f));

    if (ImGui::Button("发送数据", ImVec2(-FLT_MIN, 45))) {
        if (state.is_connected && state.send_buffer[0] != '\0') {
            if (state.hex_send) {
                // HEX发送
                std::vector<unsigned char> hexData;
                if (DataConverter::HexStringToBytes(state.send_buffer, hexData)) {
                    int sent = state.serial_port.Write(hexData.data(), hexData.size());
                    if (sent > 0) {
                        state.bytes_sent += sent;
                    }
                }
            } else {
                // ASCII发送
                int sent = state.serial_port.Write(state.send_buffer);
                if (sent > 0) {
                    state.bytes_sent += sent;
                }
            }
        }
    }

    ImGui::PopStyleColor(3);

    // 统计信息
    ImGui::Text("已发送: %d 字节", state.bytes_sent);
}

// 渲染设置对话框
void RenderSettingsDialog(AppState& state) {
    if (!state.show_settings_dialog) return;

    ImGui::SetNextWindowSize(ImVec2(500, 350), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("系统设置", &state.show_settings_dialog, ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "多线程配置");
        ImGui::Separator();
        ImGui::Spacing();

        // 启用多线程
        bool enable_mt = state.thread_config.enable_multithreading;
        if (ImGui::Checkbox("启用多线程处理", &enable_mt)) {
            state.thread_config.enable_multithreading = enable_mt;

            if (enable_mt && !state.thread_pool) {
                // 创建线程池
                state.thread_pool = std::make_unique<ThreadPool>(state.thread_config.num_worker_threads);
            } else if (!enable_mt && state.thread_pool) {
                // 销毁线程池
                state.thread_pool.reset();
            }
        }

        if (state.thread_config.enable_multithreading) {
            ImGui::Spacing();
            ImGui::Text("工作线程数量:");
            ImGui::PushItemWidth(400);

            int thread_count = state.thread_config.num_worker_threads;
            if (ImGui::SliderInt("##thread_count", &thread_count, 1, 8)) {
                state.thread_config.num_worker_threads = thread_count;

                // 重启线程池以应用新的线程数
                if (state.thread_pool) {
                    state.thread_pool->Restart(thread_count);
                }
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                              "建议值: CPU核心数 = %d, 推荐设置 2-4 个线程",
                              std::thread::hardware_concurrency());

            // 显示当前状态
            if (state.thread_pool) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("当前状态:");
                ImGui::BulletText("活跃线程: %zu", state.thread_pool->GetThreadCount());
                ImGui::BulletText("待处理任务: %zu", state.thread_pool->GetTaskCount());
            }
        } else {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.3f, 1.0f),
                              "多线程已禁用，所有处理将在主线程中执行");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 关闭按钮
        if (ImGui::Button("确定", ImVec2(120, 40))) {
            state.show_settings_dialog = false;
        }
    }
    ImGui::End();
}

// 渲染串口收发视图（三列布局）
void RenderSerialTerminalView(AppState& state) {
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    // 三列宽度
    float left_width = 340.0f;
    float right_width = 390.0f;
    float spacing = 10.0f;

    // 左侧：串口配置
    ImGui::BeginChild("LeftPanel", ImVec2(left_width, 0), true);
    RenderSerialConfigPanel(state);
    ImGui::EndChild();

    ImGui::SameLine();

    // 中央：数据显示
    float middle_width = content_size.x - left_width - right_width - spacing * 2;
    ImGui::BeginChild("MiddlePanel", ImVec2(middle_width, 0), true);
    RenderDataDisplayPanel(state);
    ImGui::EndChild();

    ImGui::SameLine();

    // 右侧：发送控制
    ImGui::BeginChild("RightPanel", ImVec2(right_width, 0), true);
    RenderSendPanel(state);
    ImGui::EndChild();
}

// 主函数（Windows GUI应用入口）
#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nCmdShow;
#else
int main(int, char**) {
#endif
    // 初始化GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // OpenGL 3.3 + GLSL 330
    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1400, 900, "串口调试助手 v1.0.0 (ImGui)", NULL, NULL);
    if (window == NULL)
        return 1;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // 启用垂直同步

    // 初始化ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();  // 初始化ImPlot上下文
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 启用键盘导航
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // 停靠功能（需要docking分支）

    // 设置ImGui样式
    SetupImGuiStyle();

    // 设置ImGui后端
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // 加载中文字体（优化：仅加载常用简体中文2500字，大幅提升启动速度）
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

    // 应用程序状态
    AppState app_state;

    // 加载保存的配置
    ConfigManager::LoadConfig(app_state);

    // 初始化线程池（如果启用了多线程）
    if (app_state.thread_config.enable_multithreading) {
        app_state.thread_pool = std::make_unique<ThreadPool>(app_state.thread_config.num_worker_threads);
    }

    // 延迟串口枚举标志（启动时不枚举，首次渲染时再枚举，加快启动速度）
    bool ports_enumerated = false;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 处理事件
        glfwPollEvents();

        // 延迟串口枚举（仅首次执行）
        if (!ports_enumerated) {
            app_state.available_ports = SerialPort_Win::EnumeratePorts();
            ports_enumerated = true;
        }

        // 定时发送逻辑
        if (app_state.enable_auto_send && app_state.is_connected && app_state.send_buffer[0] != '\0') {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - app_state.last_send_time).count();

            if (elapsed >= app_state.auto_send_interval_ms) {
                // 执行发送
                if (app_state.hex_send) {
                    // HEX发送
                    std::vector<unsigned char> hexData;
                    if (DataConverter::HexStringToBytes(app_state.send_buffer, hexData)) {
                        int sent = app_state.serial_port.Write(hexData.data(), hexData.size());
                        if (sent > 0) {
                            app_state.bytes_sent += sent;
                        }
                    }
                } else {
                    // ASCII发送
                    int sent = app_state.serial_port.Write(app_state.send_buffer);
                    if (sent > 0) {
                        app_state.bytes_sent += sent;
                    }
                }
                // 更新最后发送时间
                app_state.last_send_time = now;
            }
        }

        // 开始ImGui帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 创建全屏主窗口
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                             ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;

        ImGui::Begin("MainWindow", nullptr, main_window_flags);

        // 顶部菜单栏
        if (ImGui::BeginMenuBar()) {
            ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "串口调试助手 v2.0");

            ImGui::Dummy(ImVec2(20, 0));

            if (ImGui::BeginMenu("文件")) {
                if (ImGui::MenuItem("退出", "Alt+F4")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("帮助")) {
                ImGui::MenuItem("ImGui演示", NULL, &app_state.show_demo_window);
                ImGui::EndMenu();
            }

            // 右侧显示连接状态
            float status_x = ImGui::GetIO().DisplaySize.x - 150;
            ImGui::SameLine(status_x);
            if (app_state.is_connected) {
                ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "● 已连接");
            } else {
                ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), "○ 未连接");
            }

            ImGui::EndMenuBar();
        }

        // 主布局：左侧侧边栏 + 右侧内容区
        ImVec2 content_size = ImGui::GetContentRegionAvail();

        // 左侧侧边栏（80px）
        RenderSidebar(app_state);

        ImGui::SameLine();

        // 右侧内容区（根据当前视图切换显示）
        ImGui::BeginChild("##ContentArea", ImVec2(content_size.x - 80, 0), false);

        switch (app_state.current_view) {
            case ViewType::SERIAL_TERMINAL:
                RenderSerialTerminalView(app_state);
                break;

            case ViewType::WAVEFORM:
                app_state.visualization_ui.Render();
                break;
        }

        ImGui::EndChild();

        ImGui::End();

        // ImGui演示窗口（可选，用于学习）
        if (app_state.show_demo_window)
            ImGui::ShowDemoWindow(&app_state.show_demo_window);

        // 设置对话框
        RenderSettingsDialog(app_state);

        // 渲染
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(app_state.clear_color.x, app_state.clear_color.y,
                     app_state.clear_color.z, app_state.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // 保存配置
    ConfigManager::SaveConfig(app_state);

    // 清理
    ImPlot::DestroyContext();  // 销毁ImPlot上下文
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
