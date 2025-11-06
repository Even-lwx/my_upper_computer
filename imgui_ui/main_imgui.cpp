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

// 设置ImGui样式 - 现代深色主题（优化版）
void SetupImGuiStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // 主色调配置（优化版）
    const ImVec4 bg_color = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);           // 深蓝灰背景
    const ImVec4 widget_bg = ImVec4(0.14f, 0.16f, 0.20f, 1.00f);          // 控件背景
    const ImVec4 widget_bg_hover = ImVec4(0.18f, 0.20f, 0.25f, 1.00f);   // 悬停
    const ImVec4 widget_bg_active = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);  // 激活
    const ImVec4 accent_color = ImVec4(0.30f, 0.70f, 1.00f, 1.00f);      // 强调色（更亮的蓝色）
    const ImVec4 text_color = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);        // 文字颜色

    // 圆角设置（统一为 6px）
    style.WindowRounding = 6.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;

    // 间距设置（优化版）
    style.WindowPadding = ImVec2(14, 14);      // 增加内边距
    style.FramePadding = ImVec2(10, 7);        // 控件更舒适
    style.ItemSpacing = ImVec2(12, 10);        // 控件间距更宽松
    style.ItemInnerSpacing = ImVec2(8, 6);     // 内部间距
    style.ScrollbarSize = 16.0f;               // 滚动条更宽（高DPI友好）
    style.GrabMinSize = 12.0f;                 // 滑块更大

    // 边框（添加微妙阴影）
    style.WindowBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    // 颜色主题（更新强调色）
    colors[ImGuiCol_Text] = text_color;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = bg_color;
    colors[ImGuiCol_ChildBg] = widget_bg;
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.14f, 0.17f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.28f, 0.30f, 0.33f, 0.80f);         // 更明显的边框
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);   // 添加阴影
    colors[ImGuiCol_FrameBg] = widget_bg;
    colors[ImGuiCol_FrameBgHovered] = widget_bg_hover;
    colors[ImGuiCol_FrameBgActive] = widget_bg_active;
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.10f, 0.13f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.10f, 0.13f, 0.75f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.30f, 0.33f, 1.00f);  // 更明显
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.40f, 0.43f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = accent_color;
    colors[ImGuiCol_CheckMark] = accent_color;
    colors[ImGuiCol_SliderGrab] = accent_color;
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.80f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = widget_bg;
    colors[ImGuiCol_ButtonHovered] = widget_bg_hover;
    colors[ImGuiCol_ButtonActive] = widget_bg_active;
    colors[ImGuiCol_Header] = widget_bg_hover;
    colors[ImGuiCol_HeaderHovered] = widget_bg_active;
    colors[ImGuiCol_HeaderActive] = accent_color;
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.30f, 0.33f, 1.00f);      // 更明显
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.38f, 0.40f, 0.43f, 1.00f);
    colors[ImGuiCol_SeparatorActive] = accent_color;
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.30f, 0.33f, 0.50f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.38f, 0.40f, 0.43f, 0.75f);
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
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30f, 0.70f, 1.00f, 0.35f);
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

// 添加数据到日志（支持时间戳和方向）
void AddDataLog(AppState* state, const std::string& content, DataDirection direction) {
    std::lock_guard<std::mutex> lock(state->log_mutex);

    DataLogEntry entry;
    entry.direction = direction;
    entry.content = content;

    // 生成时间戳
    if (state->show_timestamp) {
        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        struct tm timeinfo;
        localtime_s(&timeinfo, &now_c);
        char buf[32];
        sprintf(buf, "[%02d:%02d:%02d.%03d]",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, (int)ms.count());
        entry.timestamp = buf;
    }

    // 添加到日志
    state->data_log.push_back(entry);

    // 限制日志条数（避免内存溢出）
    if (state->data_log.size() > state->max_log_entries) {
        state->data_log.erase(state->data_log.begin(),
                              state->data_log.begin() + 1000);
    }

    state->scroll_to_bottom = true;
}

// 数据处理函数（在后台线程中执行）
void ProcessDataPacket(AppState* state, const std::vector<unsigned char>& data) {
    size_t length = data.size();

    // 传递原始数据给可视化系统
    state->visualization_ui.ProcessReceivedData(data.data(), length);

    // 编码转换
    std::string dataStr;
    if (state->hex_display) {
        dataStr = DataConverter::BytesToHexString(data.data(), length, true);
    } else {
        // 使用指定编码转换为UTF-8
        dataStr = DataConverter::ConvertToUTF8(data.data(), length, state->encoding_type);
    }

    // 添加到数据日志（分色显示：RX为蓝色）
    AddDataLog(state, dataStr, DataDirection::RX);

    // 更新统计信息
    {
        std::lock_guard<std::mutex> lock(state->receive_mutex);
        state->bytes_received += length;
    }

    // 文件写入（锁外执行，避免阻塞）
    if (state->enable_logging && !state->log_filename.empty()) {
        std::ofstream logFile(state->log_filename, std::ios::app);
        if (logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            struct tm timeinfo;
            localtime_s(&timeinfo, &now_c);
            char timeStr[64];
            strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S] ", &timeinfo);

            logFile << timeStr << "RX: " << dataStr << std::endl;
            logFile.close();
        }
    }

    // 同时保持旧的receive_buffer兼容性（可选）
    {
        std::lock_guard<std::mutex> lock(state->receive_mutex);
        size_t available_space = sizeof(state->receive_buffer) - state->receive_buffer_pos - 1;
        size_t data_len = dataStr.length();

        if (data_len + 5 < available_space) {  // 5 = "RX: " + '\n'
            const char* prefix = "RX: ";
            memcpy(state->receive_buffer + state->receive_buffer_pos, prefix, 4);
            state->receive_buffer_pos += 4;
            memcpy(state->receive_buffer + state->receive_buffer_pos, dataStr.c_str(), data_len);
            state->receive_buffer_pos += data_len;
            state->receive_buffer[state->receive_buffer_pos++] = '\n';
            state->receive_buffer[state->receive_buffer_pos] = '\0';
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

    ImGui::PushItemWidth(300);

    // 显示加载状态或端口选择
    if (state.ports_enumerating) {
        // 正在枚举中，显示加载提示
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "正在扫描串口设备...");
    } else if (!state.available_ports_info.empty()) {
        // 枚举完成，显示端口下拉框
        // 使用Lambda为Combo提供友好名称显示
        auto getter = [](void* data, int idx, const char** out_text) -> bool {
            auto& ports_info = *static_cast<std::vector<SerialPortInfo>*>(data);
            if (idx < 0 || idx >= static_cast<int>(ports_info.size())) return false;

            // 直接使用成员字符串的指针（而非临时对象），避免悬空指针
            if (!ports_info[idx].friendlyName.empty()) {
                *out_text = ports_info[idx].friendlyName.c_str();
            } else {
                *out_text = ports_info[idx].portName.c_str();
            }
            return true;
        };

        if (ImGui::Combo("##port", &state.selected_port_index, getter,
                        &state.available_ports_info,
                        static_cast<int>(state.available_ports_info.size()))) {
            // 端口选择变更处理（如需要）
        }

        // 添加tooltip显示完整设备信息
        if (ImGui::IsItemHovered() && state.selected_port_index >= 0 &&
            state.selected_port_index < static_cast<int>(state.available_ports_info.size())) {
            const SerialPortInfo& info = state.available_ports_info[state.selected_port_index];
            ImGui::BeginTooltip();
            ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "设备详细信息");
            ImGui::Separator();
            ImGui::Text("端口: %s", info.portName.c_str());
            if (!info.friendlyName.empty()) {
                ImGui::Text("友好名称: %s", info.friendlyName.c_str());
            }
            if (!info.description.empty()) {
                ImGui::Text("描述: %s", info.description.c_str());
            }
            if (!info.manufacturer.empty()) {
                ImGui::Text("制造商: %s", info.manufacturer.c_str());
            }
            if (!info.hardwareId.empty()) {
                ImGui::Text("硬件ID: %s", info.hardwareId.c_str());
            }
            ImGui::EndTooltip();
        }
    } else {
        // 没有找到任何端口
        ImGui::Text("未找到串口");
    }

    ImGui::PopItemWidth();

    // 异步刷新按钮
    bool can_refresh = !state.ports_enumerating && !state.is_connected;
    if (!can_refresh) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
    }

    if (ImGui::Button(state.ports_enumerating ? "扫描中..." : "刷新端口", ImVec2(300, 40))) {
        if (!state.ports_enumerating && !state.is_connected) {
            // 启动异步枚举
            state.port_enum_future = SerialPort_Win::EnumeratePortsAsync();
            state.ports_enumerating = true;
        }
    }

    if (!can_refresh) {
        ImGui::PopStyleVar();
    }

    if (state.is_connected) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.3f, 1.0f), "(断开后可刷新)");
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

    // 第一行：HEX显示、时间戳、自动滚动、清空
    ImGui::Checkbox("HEX显示", &state.hex_display);
    ImGui::SameLine();
    ImGui::Checkbox("显示时间戳", &state.show_timestamp);
    ImGui::SameLine();
    ImGui::Checkbox("自动滚动", &state.auto_scroll);
    ImGui::SameLine();
    if (ImGui::Button("清空")) {
        std::lock_guard<std::mutex> lock(state.log_mutex);
        state.data_log.clear();
        state.receive_buffer[0] = '\0';
        state.receive_buffer_pos = 0;
        state.bytes_received = 0;
    }

    // 第二行：编码选择、日志控制
    ImGui::PushItemWidth(120);
    const char* encodings[] = { "UTF-8", "GBK", "ASCII" };
    int encoding_index = static_cast<int>(state.encoding_type);
    if (ImGui::Combo("编码", &encoding_index, encodings, 3)) {
        state.encoding_type = static_cast<EncodingType>(encoding_index);
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
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
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "%s", state.log_filename.c_str());
    }

    ImGui::Separator();

    // 数据显示区（分色显示：RX蓝色，TX绿色）
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.10f, 0.13f, 1.00f));
    ImGui::BeginChild("##DataLogWindow", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - 30),
                      true, ImGuiWindowFlags_HorizontalScrollbar);

    // 渲染数据日志（带颜色区分 + 虚拟化滚动）
    {
        std::lock_guard<std::mutex> lock(state.log_mutex);

        // 使用 ImGuiListClipper 进行虚拟化渲染（性能优化）
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(state.data_log.size()));

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                const auto& entry = state.data_log[i];

                // 设置颜色：RX蓝色，TX绿色
                ImVec4 color;
                const char* prefix;
                if (entry.direction == DataDirection::RX) {
                    color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);  // 亮蓝色
                    prefix = "RX";
                } else {
                    color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);  // 亮绿色
                    prefix = "TX";
                }

                // 显示格式：[时间戳] RX/TX: 数据内容
                if (state.show_timestamp && !entry.timestamp.empty()) {
                    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", entry.timestamp.c_str());
                    ImGui::SameLine();
                }
                ImGui::TextColored(color, "%s: %s", prefix, entry.content.c_str());
            }
        }
    }

    // 自动滚动到底部
    if (state.auto_scroll && state.scroll_to_bottom) {
        ImGui::SetScrollHereY(1.0f);
        state.scroll_to_bottom = false;
    }

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // 统计信息
    ImGui::Text("已接收: %d 字节  已发送: %d 字节", state.bytes_received, state.bytes_sent);
}

// 添加发送历史（去重）
void AddSendHistory(AppState* state, const std::string& data) {
    if (data.empty()) return;

    // 检查是否已存在
    auto it = std::find(state->send_history.begin(), state->send_history.end(), data);
    if (it != state->send_history.end()) {
        // 已存在，移到最前面
        state->send_history.erase(it);
    }

    // 插入到最前面
    state->send_history.insert(state->send_history.begin(), data);

    // 限制历史记录数量
    if (state->send_history.size() > 20) {
        state->send_history.resize(20);
    }
}

// 渲染发送控制面板
void RenderSendPanel(AppState& state) {
    // 面板标题
    ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "发送控制");
    ImGui::Separator();
    ImGui::Spacing();

    // 第一行：HEX发送
    ImGui::Checkbox("HEX发送", &state.hex_send);

    // 第二行：行尾符选择
    ImGui::Text("行尾符:");
    ImGui::PushItemWidth(120);
    const char* line_endings[] = { "无", "\\r", "\\n", "\\r\\n" };
    int ending_index = static_cast<int>(state.send_line_ending);
    if (ImGui::Combo("##line_ending", &ending_index, line_endings, 4)) {
        state.send_line_ending = static_cast<LineEnding>(ending_index);
    }
    ImGui::PopItemWidth();

    // 第三行：自定义前缀
    ImGui::Checkbox("自定义前缀", &state.enable_custom_prefix);
    if (state.enable_custom_prefix) {
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputText("##custom_prefix", state.custom_prefix, sizeof(state.custom_prefix));
        ImGui::PopItemWidth();
    }

    // 第四行：自定义后缀
    ImGui::Checkbox("自定义后缀", &state.enable_custom_suffix);
    if (state.enable_custom_suffix) {
        ImGui::SameLine();
        ImGui::PushItemWidth(200);
        ImGui::InputText("##custom_suffix", state.custom_suffix, sizeof(state.custom_suffix));
        ImGui::PopItemWidth();
    }

    // 发送历史下拉框
    if (!state.send_history.empty()) {
        ImGui::Text("发送历史:");
        ImGui::PushItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##send_history", "选择历史记录")) {
            for (size_t i = 0; i < state.send_history.size(); i++) {
                const std::string& history = state.send_history[i];
                // 限制显示长度
                std::string display = history.length() > 50 ?
                                     (history.substr(0, 47) + "...") : history;

                if (ImGui::Selectable(display.c_str())) {
                    strcpy_s(state.send_buffer, sizeof(state.send_buffer), history.c_str());
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
    }

    ImGui::Separator();
    ImGui::Spacing();

    // 定时发送
    ImGui::Checkbox("启用定时发送", &state.enable_auto_send);
    if (state.enable_auto_send) {
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::InputInt("##interval", &state.auto_send_interval_ms, 100, 500);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::Text("ms");

        // 限制间隔范围
        if (state.auto_send_interval_ms < 100) state.auto_send_interval_ms = 100;
        if (state.auto_send_interval_ms > 60000) state.auto_send_interval_ms = 60000;

        // 显示倒计时
        if (state.is_connected) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - state.last_send_time).count();
            int remaining = state.auto_send_interval_ms - elapsed;
            if (remaining < 0) remaining = 0;
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "(%d ms)", remaining);
        }
    }

    ImGui::Separator();

    // 发送输入框
    ImGui::Text("发送数据:");
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::InputTextMultiline("##send", state.send_buffer, sizeof(state.send_buffer), ImVec2(-FLT_MIN, 120));
    ImGui::PopItemWidth();

    // 发送按钮
    ImVec4 send_button_color = ImVec4(0.26f, 0.59f, 0.98f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, send_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.36f, 0.69f, 1.00f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.16f, 0.49f, 0.88f, 1.0f));

    if (ImGui::Button("发送数据", ImVec2(-FLT_MIN, 40))) {
        if (state.is_connected && state.send_buffer[0] != '\0') {
            // 构建最终发送数据
            std::string final_data;

            // 添加自定义前缀
            if (state.enable_custom_prefix && state.custom_prefix[0] != '\0') {
                final_data += state.custom_prefix;
            }

            // 添加主体数据
            final_data += state.send_buffer;

            // 添加自定义后缀
            if (state.enable_custom_suffix && state.custom_suffix[0] != '\0') {
                final_data += state.custom_suffix;
            }

            // 添加行尾符
            switch (state.send_line_ending) {
                case LineEnding::CR:   final_data += "\r"; break;
                case LineEnding::LF:   final_data += "\n"; break;
                case LineEnding::CRLF: final_data += "\r\n"; break;
                case LineEnding::NONE: break;
            }

            // 发送数据
            int sent = 0;
            if (state.hex_send) {
                // HEX发送
                std::vector<unsigned char> hexData;
                if (DataConverter::HexStringToBytes(final_data, hexData)) {
                    sent = state.serial_port.Write(hexData.data(), hexData.size());
                }
            } else {
                // 编码转换后发送
                std::vector<unsigned char> encoded_data;
                if (DataConverter::ConvertFromUTF8(final_data, state.encoding_type, encoded_data)) {
                    sent = state.serial_port.Write(encoded_data.data(), encoded_data.size());
                } else {
                    sent = state.serial_port.Write(final_data);
                }
            }

            // 更新统计和日志
            if (sent > 0) {
                state.bytes_sent += sent;

                // 添加到数据日志（TX绿色显示）
                std::string display_data;
                if (state.hex_send) {
                    std::vector<unsigned char> hexData;
                    DataConverter::HexStringToBytes(final_data, hexData);
                    display_data = DataConverter::BytesToHexString(hexData.data(), hexData.size(), true);
                } else {
                    display_data = state.send_buffer;  // 只显示用户输入的部分
                }
                AddDataLog(&state, display_data, DataDirection::TX);

                // 添加到发送历史
                AddSendHistory(&state, state.send_buffer);
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

// 渲染串口收发视图（三列布局 - 响应式设计）
void RenderSerialTerminalView(AppState& state) {
    ImVec2 content_size = ImGui::GetContentRegionAvail();

    // 响应式布局：使用百分比分配（可轻松调整比例）
    float left_ratio = 0.24f;    // 24% - 串口配置
    float right_ratio = 0.28f;   // 28% - 发送控制
    float middle_ratio = 0.48f;  // 48% - 数据显示

    float left_width = content_size.x * left_ratio;
    float right_width = content_size.x * right_ratio;
    float middle_width = content_size.x * middle_ratio;

    // 最小宽度保护（防止窗口过小时布局错乱）
    const float min_left_width = 300.0f;
    const float min_middle_width = 400.0f;
    const float min_right_width = 350.0f;

    if (left_width < min_left_width) left_width = min_left_width;
    if (middle_width < min_middle_width) middle_width = min_middle_width;
    if (right_width < min_right_width) right_width = min_right_width;

    // 左侧：串口配置
    ImGui::BeginChild("LeftPanel", ImVec2(left_width, 0), true);
    RenderSerialConfigPanel(state);
    ImGui::EndChild();

    ImGui::SameLine();

    // 中央：数据显示
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

    // 设置Windows控制台为UTF-8编码
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
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
    GLFWwindow* window = glfwCreateWindow(1400, 900, "串口调试助手 v2.0", NULL, NULL);
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

    // 加载中文字体（完整字符集，解决乱码问题）
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 20.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());

    // 应用程序状态
    AppState app_state;

    // 加载保存的配置
    ConfigManager::LoadConfig(app_state);

    // 初始化线程池（如果启用了多线程）
    if (app_state.thread_config.enable_multithreading) {
        app_state.thread_pool = std::make_unique<ThreadPool>(app_state.thread_config.num_worker_threads);
    }

    // 启动异步串口枚举（不阻塞UI）
    app_state.port_enum_future = SerialPort_Win::EnumeratePortsAsync();
    app_state.ports_enumerating = true;

    // 主循环
    while (!glfwWindowShouldClose(window)) {
        // 处理事件
        glfwPollEvents();

        // 检查异步串口枚举是否完成
        if (app_state.ports_enumerating) {
            if (app_state.port_enum_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                app_state.available_ports_info = app_state.port_enum_future.get();
                app_state.ports_enumerating = false;
                app_state.ports_enumerated = true;

                // 向后兼容：同时更新available_ports
                app_state.available_ports.clear();
                for (const auto& info : app_state.available_ports_info) {
                    app_state.available_ports.push_back(info.portName);
                }
            }
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
