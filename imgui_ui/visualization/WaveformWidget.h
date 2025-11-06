/**
 * @file WaveformWidget.h
 * @brief 波形图组件 - 实时显示多通道时序数据
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 支持多通道叠加显示
 * - 实时滚动显示
 * - 自动Y轴缩放
 * - 图例显示
 * - 支持缩放和平移
 */

#ifndef WAVEFORM_WIDGET_H
#define WAVEFORM_WIDGET_H

#include "Widget.h"
#include <implot.h>
#include <vector>

/**
 * @brief 波形图组件
 */
class WaveformWidget : public Widget {
public:
    WaveformWidget(const std::string& name = "Waveform")
        : Widget(WidgetType::WAVEFORM, name)
        , auto_fit_y_(true)
        , show_legend_(true)
        , history_seconds_(10.0)
        , max_points_(1000)
    {
        // 默认显示通道0
        channels_ = {0};
    }

    void Render(DataChannelManager& channel_manager) override {
        if (!visible_) return;

        ImGui::SetNextWindowSize(GetSize(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(GetPosition(), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(GetImGuiID().c_str(), &visible_)) {
            // 渲染波形图
            RenderWaveform(channel_manager);
        }
        ImGui::End();
    }

    void RenderConfig() override {
        ImGui::Text("波形图配置");
        ImGui::Separator();

        ImGui::Checkbox("自动缩放Y轴", &auto_fit_y_);
        ImGui::Checkbox("显示图例", &show_legend_);

        ImGui::SliderFloat("历史时长(秒)", &history_seconds_, 1.0f, 60.0f);
        ImGui::SliderInt("最大点数", &max_points_, 100, 2000);

        ImGui::Separator();
        ImGui::Text("通道选择：");

        // 通道选择复选框
        for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
            bool selected = IsChannelSelected(i);
            ImGui::PushID(static_cast<int>(i));

            if (ImGui::Checkbox(("CH" + std::to_string(i + 1)).c_str(), &selected)) {
                if (selected) {
                    AddChannel(i);
                } else {
                    RemoveChannel(i);
                }
            }

            ImGui::PopID();

            // 每4个换行
            if ((i + 1) % 4 != 0) {
                ImGui::SameLine();
            }
        }
    }

private:
    /**
     * @brief 渲染波形图
     */
    void RenderWaveform(DataChannelManager& channel_manager) {
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);

        // 设置图表标志
        ImPlotFlags plot_flags = ImPlotFlags_None;
        if (!show_legend_) {
            plot_flags |= ImPlotFlags_NoLegend;
        }

        // 设置X轴（时间）标志
        ImPlotAxisFlags x_flags = ImPlotAxisFlags_None;

        // 设置Y轴标志
        ImPlotAxisFlags y_flags = ImPlotAxisFlags_None;
        if (auto_fit_y_) {
            y_flags |= ImPlotAxisFlags_AutoFit;
        }

        // 开始绘制
        if (ImPlot::BeginPlot(GetImGuiID().c_str(), ImVec2(-1, -1), plot_flags)) {
            ImPlot::SetupAxis(ImAxis_X1, "Time (s)", x_flags);
            ImPlot::SetupAxis(ImAxis_Y1, "Value", y_flags);

            // 绘制每个通道
            for (size_t channel_index : channels_) {
                if (channel_index >= DataChannelManager::MAX_CHANNELS) {
                    continue;
                }

                // 检查通道是否启用
                if (!channel_manager.IsChannelEnabled(channel_index)) {
                    continue;
                }

                // 获取通道配置
                ChannelConfig config = channel_manager.GetChannelConfig(channel_index);

                // 获取数据
                std::vector<double> timestamps;
                std::vector<float> y_values_float;
                size_t point_count = channel_manager.GetChannelData(
                    channel_index, timestamps, y_values_float, max_points_);

                if (point_count > 0) {
                    // 转换float到double以匹配ImPlot类型要求
                    std::vector<double> y_values(y_values_float.begin(), y_values_float.end());

                    // 设置线条颜色
                    ImVec4 color(config.color[0], config.color[1], config.color[2], config.color[3]);
                    ImPlot::SetNextLineStyle(color);

                    // 绘制折线图
                    ImPlot::PlotLine(config.name.c_str(),
                                    timestamps.data(),
                                    y_values.data(),
                                    static_cast<int>(point_count));
                }
            }

            ImPlot::EndPlot();
        }

        ImPlot::PopStyleVar();
    }

    /**
     * @brief 判断通道是否已选择
     */
    bool IsChannelSelected(size_t channel_index) const {
        return std::find(channels_.begin(), channels_.end(), channel_index) != channels_.end();
    }

    /**
     * @brief 添加通道
     */
    void AddChannel(size_t channel_index) {
        if (!IsChannelSelected(channel_index)) {
            channels_.push_back(channel_index);
        }
    }

    /**
     * @brief 移除通道
     */
    void RemoveChannel(size_t channel_index) {
        channels_.erase(
            std::remove(channels_.begin(), channels_.end(), channel_index),
            channels_.end());
    }

    // 配置选项
    bool auto_fit_y_;           // 自动缩放Y轴
    bool show_legend_;          // 显示图例
    float history_seconds_;     // 历史时长（秒）
    int max_points_;            // 最大显示点数
};

#endif // WAVEFORM_WIDGET_H
