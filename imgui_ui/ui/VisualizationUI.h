/**
 * @file VisualizationUI.h
 * @brief VOFA+风格极简可视化UI
 * @author AI Assistant
 * @date 2025
 *
 * 布局：
 * - 左侧：配置面板（协议、通道、参数）
 * - 中间：大波形显示区
 * - 右侧：紧凑通道列表
 * - 底部：简化状态栏
 */

#ifndef VISUALIZATION_UI_H
#define VISUALIZATION_UI_H

#include "../core/DataChannelManager.h"
#include "../protocols/ProtocolParser.h"
#include "../protocols/FireWaterParser.h"
#include "../protocols/JustFloatParser.h"
#include "../protocols/RawDataParser.h"
#include "../protocols/CustomParser.h"
#include "../protocols/CsvParser.h"
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
     * @brief 渲染VOFA+风格UI（在当前内容区域内渲染，不创建新窗口）
     */
    void Render() {
        ImVec2 content_size = ImGui::GetContentRegionAvail();

        // === 左侧配置面板（130px） ===
        ImGui::BeginChild("##ConfigPanel", ImVec2(130, content_size.y - 35), true, ImGuiWindowFlags_NoScrollbar);
        RenderConfigPanel();
        ImGui::EndChild();

        ImGui::SameLine();

        // === 中间波形区（主要区域） ===
        float waveform_width = content_size.x - 340; // 减去左栏130px和右栏210px
        ImGui::BeginChild("##Waveform", ImVec2(waveform_width, content_size.y - 35), true);
        RenderWaveform();
        ImGui::EndChild();

        ImGui::SameLine();

        // === 右侧通道列表（210px） ===
        ImGui::BeginChild("##ChannelList", ImVec2(210, content_size.y - 35), true);
        RenderChannelList();
        ImGui::EndChild();

        // === 底部状态栏（30px） ===
        ImGui::BeginChild("##StatusBar", ImVec2(content_size.x, 30), true, ImGuiWindowFlags_NoScrollbar);
        RenderStatusBar();
        ImGui::EndChild();
    }

    DataChannelManager& GetChannelManager() { return channel_manager_; }
    ProtocolParser* GetProtocolParser() { return protocol_parser_.get(); }
    const ProtocolParser* GetProtocolParser() const { return protocol_parser_.get(); }

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
            case ProtocolType::CSV:
                protocol_parser_ = std::make_unique<CsvParser>();
                break;
            case ProtocolType::CUSTOM:
                protocol_parser_ = std::make_unique<CustomParser>();
                break;
        }

        // 同步通道数配置到新协议
        if (protocol_parser_) {
            protocol_parser_->SetExpectedChannelCount(channel_count_);
        }

        // 自动调整启用的通道数量
        for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
            channel_manager_.SetChannelEnabled(i, static_cast<int>(i) < channel_count_);
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
     * @brief 渲染左侧配置面板（VOFA+风格）
     */
    void RenderConfigPanel() {
        // 面板标题
        ImGui::TextColored(ImVec4(0.26f, 0.59f, 0.98f, 1.0f), "协议配置");
        ImGui::Separator();
        ImGui::Spacing();

        // === 数据引擎选择 ===
        ImGui::AlignTextToFramePadding();
        ImGui::Text("数据引擎:");
        ImGui::SetNextItemWidth(-FLT_MIN);  // 填满剩余宽度
        const char* protocols[] = {"FireWater", "JustFloat", "RawData", "CSV", "Custom"};
        int current = static_cast<int>(current_protocol_type_);
        if (ImGui::Combo("##protocol", &current, protocols, IM_ARRAYSIZE(protocols))) {
            SetProtocolType(static_cast<ProtocolType>(current));
        }

        ImGui::Spacing();

        // === 通道数配置 ===
        ImGui::AlignTextToFramePadding();
        ImGui::Text("通道数:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        int temp_channel_count = channel_count_;
        if (ImGui::InputInt("##channels", &temp_channel_count, 1, 1)) {
            // 限制范围：1-16通道
            if (temp_channel_count < 1) temp_channel_count = 1;
            if (temp_channel_count > 16) temp_channel_count = 16;

            if (temp_channel_count != channel_count_) {
                channel_count_ = temp_channel_count;

                // 更新协议解析器的通道数
                if (protocol_parser_) {
                    protocol_parser_->SetExpectedChannelCount(channel_count_);
                }

                // 自动启用相应数量的通道
                for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
                    channel_manager_.SetChannelEnabled(i, static_cast<int>(i) < channel_count_);
                }
            }
        }

        ImGui::Spacing();

        // === 采样间隔显示 ===
        ImGui::AlignTextToFramePadding();
        ImGui::Text("采样间隔:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputInt("##dt", &sample_interval_ms_, 0, 0, ImGuiInputTextFlags_ReadOnly);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // === X轴范围滑块 ===
        ImGui::AlignTextToFramePadding();
        ImGui::Text("X轴范围:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::SliderFloat("##xrange", &x_axis_range_, 1.0f, 60.0f, "%.1fs");

        ImGui::Spacing();

        // === Y轴自动缩放 ===
        ImGui::Checkbox("Y轴自动缩放", &auto_scale_y_);
    }

    /**
     * @brief 渲染中间波形显示区
     */
    void RenderWaveform() {
        ImVec2 plot_size = ImGui::GetContentRegionAvail();

        if (ImPlot::BeginPlot("##MainPlot", plot_size, ImPlotFlags_NoTitle)) {
            ImPlot::SetupAxes("时间 (s)", "数值", ImPlotAxisFlags_None, ImPlotAxisFlags_None);
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, x_axis_range_, ImGuiCond_Always);

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

        ImGui::TextUnformatted(""); ImGui::NextColumn();
        ImGui::Text("通道"); ImGui::NextColumn();
        ImGui::Text("数值"); ImGui::NextColumn();
        ImGui::Separator();

        // 通道列表
        for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
            ImGui::PushID(static_cast<int>(i));

            ChannelConfig config = channel_manager_.GetChannelConfig(i);
            ChannelStats stats = channel_manager_.GetChannelStats(i);

            // 眼睛图标按钮（可见性开关）
            bool enabled = config.enabled;
            const char* icon = enabled ? "●" : "○";  // 实心圆=可见，空心圆=隐藏
            ImVec4 button_color = enabled
                ? ImVec4(0.30f, 0.70f, 1.00f, 1.00f)   // 蓝色（开启）
                : ImVec4(0.50f, 0.50f, 0.50f, 0.50f);  // 灰色（关闭）

            ImGui::PushStyleColor(ImGuiCol_Button, button_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(button_color.x * 1.2f, button_color.y * 1.2f, button_color.z * 1.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                ImVec4(button_color.x * 0.8f, button_color.y * 0.8f, button_color.z * 0.8f, 1.0f));

            if (ImGui::Button(icon, ImVec2(20, 20))) {
                channel_manager_.SetChannelEnabled(i, !enabled);
            }

            ImGui::PopStyleColor(3);
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
     * @brief 渲染底部状态栏（简化版本）
     */
    void RenderStatusBar() {
        // 计算总数据点数（所有启用通道）
        size_t total_points = 0;
        for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
            ChannelConfig config = channel_manager_.GetChannelConfig(i);
            if (config.enabled) {
                ChannelStats stats = channel_manager_.GetChannelStats(i);
                total_points += stats.sample_count;
            }
        }

        // 显示统计信息
        ImGui::Text("总点数:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.30f, 0.70f, 1.00f, 1.0f), "%zu", total_points);

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        // 显示当前协议
        const char* protocol_name = GetProtocolName(current_protocol_type_).c_str();
        ImGui::Text("协议:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "%s", protocol_name);

        ImGui::SameLine();
        ImGui::Dummy(ImVec2(20, 0));
        ImGui::SameLine();

        // 显示通道数
        ImGui::Text("通道数:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.3f, 1.0f), "%d", channel_count_);
    }

    DataChannelManager channel_manager_;
    std::unique_ptr<ProtocolParser> protocol_parser_;
    ProtocolType current_protocol_type_;

    bool auto_scale_y_;
    int sample_interval_ms_ = 1;
    int channel_count_ = 4;  // 默认4通道
    float x_axis_range_ = 10.0f;  // X轴显示范围（秒）
};

#endif // VISUALIZATION_UI_H
