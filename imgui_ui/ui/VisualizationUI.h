/**
 * @file VisualizationUI.h
 * @brief VOFA+风格极简可视化UI
 * @author AI Assistant
 * @date 2025
 *
 * 布局：
 * - 左侧：极窄工具栏
 * - 中间：大波形显示区
 * - 右侧：紧凑通道列表
 * - 底部：协议参数栏
 */

#ifndef VISUALIZATION_UI_H
#define VISUALIZATION_UI_H

#include "../core/DataChannelManager.h"
#include "../protocols/ProtocolParser.h"
#include "../protocols/FireWaterParser.h"
#include "../protocols/JustFloatParser.h"
#include "../protocols/RawDataParser.h"
#include "../protocols/CustomParser.h"
#include <imgui.h>
#include <implot.h>
#include <memory>

/**
 * @brief VOFA+风格可视化UI管理器
 */
class VisualizationUI {
public:
    VisualizationUI()
        : current_protocol_type_(ProtocolType::FIREWATER)
        , auto_scale_y_(true)
    {
        protocol_parser_ = std::make_unique<FireWaterParser>();
    }

    /**
     * @brief 渲染VOFA+风格UI（极简，单一窗口）
     */
    void Render() {
        ImVec2 window_size = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(window_size);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_MenuBar;

        if (ImGui::Begin("##MainWindow", nullptr, flags)) {
            // 顶部菜单栏
            if (ImGui::BeginMenuBar()) {
                ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "串口调试助手 - 实时可视化");
                ImGui::EndMenuBar();
            }

            ImVec2 content_size = ImGui::GetContentRegionAvail();

            // === 左侧工具栏（20px） ===
            ImGui::BeginChild("##Toolbar", ImVec2(20, content_size.y - 35), true, ImGuiWindowFlags_NoScrollbar);
            RenderToolbar();
            ImGui::EndChild();

            ImGui::SameLine();

            // === 中间波形区（主要区域） ===
            float waveform_width = content_size.x - 220; // 减去左栏20px和右栏200px
            ImGui::BeginChild("##Waveform", ImVec2(waveform_width, content_size.y - 35), true);
            RenderWaveform();
            ImGui::EndChild();

            ImGui::SameLine();

            // === 右侧通道列表（200px） ===
            ImGui::BeginChild("##ChannelList", ImVec2(200, content_size.y - 35), true);
            RenderChannelList();
            ImGui::EndChild();

            // === 底部参数栏（30px） ===
            ImGui::BeginChild("##StatusBar", ImVec2(content_size.x, 30), true, ImGuiWindowFlags_NoScrollbar);
            RenderStatusBar();
            ImGui::EndChild();
        }
        ImGui::End();
    }

    DataChannelManager& GetChannelManager() { return channel_manager_; }
    ProtocolParser* GetProtocolParser() { return protocol_parser_.get(); }

    void SetProtocolType(ProtocolType type) {
        if (type == current_protocol_type_) return;
        current_protocol_type_ = type;

        switch (type) {
            case ProtocolType::FIREWATER:
                protocol_parser_ = std::make_unique<FireWaterParser>();
                break;
            case ProtocolType::JUSTFLOAT:
                protocol_parser_ = std::make_unique<JustFloatParser>();
                break;
            case ProtocolType::RAWDATA:
                protocol_parser_ = std::make_unique<RawDataParser>();
                break;
            case ProtocolType::CUSTOM:
                protocol_parser_ = std::make_unique<CustomParser>();
                break;
        }
    }

    void ProcessReceivedData(const unsigned char* data, size_t length) {
        if (!protocol_parser_) return;

        ParseResult result = protocol_parser_->Parse(data, length);
        if (result.success && !result.values.empty()) {
            channel_manager_.PushMultiChannelData(result.values.data(), result.values.size());
        }
    }

private:
    /**
     * @brief 渲染左侧工具栏（极简图标）
     */
    void RenderToolbar() {
        // 暂时只显示一个标识
        ImGui::SetCursorPosY(10);
        ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "V");
    }

    /**
     * @brief 渲染中间波形显示区
     */
    void RenderWaveform() {
        ImVec2 plot_size = ImGui::GetContentRegionAvail();

        if (ImPlot::BeginPlot("##MainPlot", plot_size, ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxes("时间 (s)", "数值", ImPlotAxisFlags_None, ImPlotAxisFlags_None);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10, ImGuiCond_Always);

            if (auto_scale_y_) {
                ImPlot::SetupAxisLimits(ImAxis_Y1, -5, 5, ImGuiCond_Once);
            }

            // 绘制所有启用的通道
            for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
                if (!channel_manager_.IsChannelEnabled(i)) continue;

                ChannelConfig config = channel_manager_.GetChannelConfig(i);
                std::vector<double> timestamps;
                std::vector<float> y_values_float;
                size_t point_count = channel_manager_.GetChannelData(i, timestamps, y_values_float, 2000);

                if (point_count > 0) {
                    std::vector<double> y_values(y_values_float.begin(), y_values_float.end());
                    ImVec4 color(config.color[0], config.color[1], config.color[2], config.color[3]);
                    ImPlot::SetNextLineStyle(color, 2.0f); // 线条稍粗
                    ImPlot::PlotLine(config.name.c_str(), timestamps.data(), y_values.data(),
                                    static_cast<int>(point_count));
                }
            }

            ImPlot::EndPlot();
        }
    }

    /**
     * @brief 渲染右侧通道列表（紧凑）
     */
    void RenderChannelList() {
        ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "数据");
        ImGui::Separator();

        // 表头
        ImGui::Columns(3, "channelcols", false);
        ImGui::SetColumnWidth(0, 30);
        ImGui::SetColumnWidth(1, 50);
        ImGui::SetColumnWidth(2, 120);

        ImGui::Text(""); ImGui::NextColumn();
        ImGui::Text("通道"); ImGui::NextColumn();
        ImGui::Text("数值"); ImGui::NextColumn();
        ImGui::Separator();

        // 通道列表
        for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
            ImGui::PushID(static_cast<int>(i));

            ChannelConfig config = channel_manager_.GetChannelConfig(i);
            ChannelStats stats = channel_manager_.GetChannelStats(i);

            // 复选框
            bool enabled = config.enabled;
            if (ImGui::Checkbox("##en", &enabled)) {
                channel_manager_.SetChannelEnabled(i, enabled);
            }
            ImGui::NextColumn();

            // 通道名（带颜色指示器）
            ImGui::ColorButton("##colorind", ImVec4(config.color[0], config.color[1],
                                                     config.color[2], config.color[3]),
                              ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker,
                              ImVec2(10, 10));
            ImGui::SameLine();
            ImGui::Text("I%zu", i);
            ImGui::NextColumn();

            // 当前值（带颜色高亮）
            ImVec4 value_color = enabled ? ImVec4(1, 1, 1, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1);
            ImGui::TextColored(value_color, "%.3f", stats.last_value);
            ImGui::NextColumn();

            ImGui::PopID();
        }

        ImGui::Columns(1);
    }

    /**
     * @brief 渲染底部状态栏（紧凑参数）
     */
    void RenderStatusBar() {
        // 协议选择
        const char* protocols[] = {"FireWater", "JustFloat", "RawData", "Custom"};
        int current = static_cast<int>(current_protocol_type_);

        ImGui::Text("Δt:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::InputInt("##dt", &sample_interval_ms_, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::SameLine();
        ImGui::Text("ms   缓冲区上限:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        int buffer_limit = 50000;
        ImGui::InputInt("##buffer", &buffer_limit, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::SameLine();
        ImGui::Text("/ch   Auto点数对齐:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80);
        int auto_align = 101;
        ImGui::InputInt("##align", &auto_align, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        ImGui::SetNextItemWidth(100);
        if (ImGui::Combo("##protocol", &current, protocols, IM_ARRAYSIZE(protocols))) {
            SetProtocolType(static_cast<ProtocolType>(current));
        }
    }

    DataChannelManager channel_manager_;
    std::unique_ptr<ProtocolParser> protocol_parser_;
    ProtocolType current_protocol_type_;

    bool auto_scale_y_;
    int sample_interval_ms_ = 1;
};

#endif // VISUALIZATION_UI_H
