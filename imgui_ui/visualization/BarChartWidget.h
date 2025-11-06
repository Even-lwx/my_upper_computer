/**
 * @file BarChartWidget.h
 * @brief 柱状图组件 - 多通道实时对比
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 多通道柱状图对比
 * - 实时更新显示当前值
 * - 支持水平/垂直方向
 * - 自动Y轴缩放
 * - 显示数值标签
 */

#ifndef BAR_CHART_WIDGET_H
#define BAR_CHART_WIDGET_H

#include "Widget.h"
#include <implot.h>

/**
 * @brief 柱状图组件
 */
class BarChartWidget : public Widget {
public:
    BarChartWidget(const std::string& name = "Bar Chart")
        : Widget(WidgetType::BAR_CHART, name)
        , horizontal_(false)
        , show_values_(true)
        , bar_width_(0.67)
        , auto_fit_y_(true)
    {
        // 默认显示前4个通道
        channels_ = {0, 1, 2, 3};
    }

    void Render(DataChannelManager& channel_manager) override {
        if (!visible_) return;

        ImGui::SetNextWindowSize(GetSize(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(GetPosition(), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(GetImGuiID().c_str(), &visible_)) {
            RenderBarChart(channel_manager);
        }
        ImGui::End();
    }

    void RenderConfig() override {
        ImGui::Text("柱状图配置");
        ImGui::Separator();

        ImGui::Checkbox("水平方向", &horizontal_);
        ImGui::Checkbox("显示数值", &show_values_);
        ImGui::Checkbox("自动缩放Y轴", &auto_fit_y_);
        ImGui::SliderFloat("柱宽度", &bar_width_, 0.1f, 1.0f);

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
     * @brief 渲染柱状图
     */
    void RenderBarChart(DataChannelManager& channel_manager) {
        // 收集当前值
        std::vector<double> values;
        std::vector<std::string> labels;
        std::vector<ImVec4> colors;

        for (size_t channel_index : channels_) {
            if (channel_index >= DataChannelManager::MAX_CHANNELS) {
                continue;
            }

            // 检查通道是否启用
            if (!channel_manager.IsChannelEnabled(channel_index)) {
                continue;
            }

            // 获取通道配置和统计
            ChannelConfig config = channel_manager.GetChannelConfig(channel_index);
            ChannelStats stats = channel_manager.GetChannelStats(channel_index);

            values.push_back(stats.last_value);
            labels.push_back(config.name);
            colors.push_back(ImVec4(config.color[0], config.color[1], config.color[2], config.color[3]));
        }

        if (values.empty()) {
            ImGui::Text("没有启用的通道");
            return;
        }

        // 设置图表标志
        ImPlotFlags plot_flags = ImPlotFlags_NoLegend;

        // 设置轴标志
        ImPlotAxisFlags x_flags = ImPlotAxisFlags_None;
        ImPlotAxisFlags y_flags = ImPlotAxisFlags_None;
        if (auto_fit_y_) {
            y_flags |= ImPlotAxisFlags_AutoFit;
        }

        // 开始绘制
        if (ImPlot::BeginPlot(GetImGuiID().c_str(), ImVec2(-1, -1), plot_flags)) {
            if (horizontal_) {
                // 水平柱状图
                ImPlot::SetupAxis(ImAxis_X1, "Value", y_flags);
                ImPlot::SetupAxis(ImAxis_Y1, nullptr, x_flags);

                // 设置Y轴标签
                std::vector<const char*> label_ptrs;
                for (const auto& label : labels) {
                    label_ptrs.push_back(label.c_str());
                }
                ImPlot::SetupAxisTicks(ImAxis_Y1, 0, static_cast<double>(values.size() - 1),
                                      static_cast<int>(values.size()), label_ptrs.data());

                // 绘制柱状图
                for (size_t i = 0; i < values.size(); i++) {
                    ImPlot::PushStyleColor(ImPlotCol_Fill, colors[i]);
                    double x_vals[1] = {values[i]};
                    double y_vals[1] = {static_cast<double>(i)};
                    ImPlot::PlotBars(labels[i].c_str(), x_vals, y_vals, 1, bar_width_, ImPlotBarsFlags_Horizontal);
                    ImPlot::PopStyleColor();
                }
            } else {
                // 垂直柱状图
                ImPlot::SetupAxis(ImAxis_X1, nullptr, x_flags);
                ImPlot::SetupAxis(ImAxis_Y1, "Value", y_flags);

                // 设置X轴标签
                std::vector<const char*> label_ptrs;
                for (const auto& label : labels) {
                    label_ptrs.push_back(label.c_str());
                }
                ImPlot::SetupAxisTicks(ImAxis_X1, 0, static_cast<double>(values.size() - 1),
                                      static_cast<int>(values.size()), label_ptrs.data());

                // 绘制柱状图
                for (size_t i = 0; i < values.size(); i++) {
                    ImPlot::PushStyleColor(ImPlotCol_Fill, colors[i]);
                    double x_pos[1] = {static_cast<double>(i)};
                    double y_val[1] = {values[i]};
                    ImPlot::PlotBars(labels[i].c_str(), x_pos, y_val, 1, bar_width_);
                    ImPlot::PopStyleColor();
                }
            }

            ImPlot::EndPlot();
        }

        // 显示数值
        if (show_values_) {
            ImGui::Separator();
            ImGui::Columns(static_cast<int>(values.size()), nullptr, false);

            for (size_t i = 0; i < values.size(); i++) {
                ImGui::TextColored(colors[i], "%s", labels[i].c_str());
                ImGui::Text("%.3f", values[i]);
                ImGui::NextColumn();
            }

            ImGui::Columns(1);
        }
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
    bool horizontal_;           // 水平方向
    bool show_values_;          // 显示数值
    float bar_width_;           // 柱宽度
    bool auto_fit_y_;           // 自动缩放Y轴
};

#endif // BAR_CHART_WIDGET_H
