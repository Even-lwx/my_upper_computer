/**
 * @file DigitalDisplayWidget.h
 * @brief 数字表盘组件 - 大号数字显示当前值
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 大号字体显示当前值
 * - 支持多通道网格布局
 * - 可配置小数位数和单位
 * - 显示统计信息（最小/最大/平均）
 * - 自动颜色映射
 */

#ifndef DIGITAL_DISPLAY_WIDGET_H
#define DIGITAL_DISPLAY_WIDGET_H

#include "Widget.h"
#include <implot.h>
#include <cmath>

/**
 * @brief 数字表盘组件
 */
class DigitalDisplayWidget : public Widget {
public:
    DigitalDisplayWidget(const std::string& name = "Digital Display")
        : Widget(WidgetType::DIGITAL_DISPLAY, name)
        , decimal_places_(2)
        , show_stats_(true)
        , grid_columns_(2)
        , font_scale_(3.0f)
    {
        // 默认显示通道0
        channels_ = {0};
        unit_ = "";
    }

    void Render(DataChannelManager& channel_manager) override {
        if (!visible_) return;

        ImGui::SetNextWindowSize(GetSize(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(GetPosition(), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(GetImGuiID().c_str(), &visible_)) {
            RenderDigitalDisplay(channel_manager);
        }
        ImGui::End();
    }

    void RenderConfig() override {
        ImGui::Text("数字表盘配置");
        ImGui::Separator();

        ImGui::SliderInt("小数位数", &decimal_places_, 0, 6);
        ImGui::SliderFloat("字体缩放", &font_scale_, 1.0f, 5.0f);
        ImGui::SliderInt("网格列数", &grid_columns_, 1, 4);
        ImGui::Checkbox("显示统计信息", &show_stats_);

        // 单位输入
        char unit_buffer[32];
        strncpy(unit_buffer, unit_.c_str(), sizeof(unit_buffer) - 1);
        unit_buffer[sizeof(unit_buffer) - 1] = '\0';
        if (ImGui::InputText("单位", unit_buffer, sizeof(unit_buffer))) {
            unit_ = unit_buffer;
        }

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
     * @brief 渲染数字显示
     */
    void RenderDigitalDisplay(DataChannelManager& channel_manager) {
        // 计算网格布局
        int cols = grid_columns_;
        int rows = static_cast<int>(std::ceil(static_cast<float>(channels_.size()) / cols));

        ImVec2 available_size = ImGui::GetContentRegionAvail();
        float cell_width = available_size.x / cols;
        float cell_height = available_size.y / rows;

        ImGui::BeginChild("DigitalDisplayGrid", available_size, false, ImGuiWindowFlags_NoScrollbar);

        for (size_t idx = 0; idx < channels_.size(); idx++) {
            size_t channel_index = channels_[idx];

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

            // 计算网格位置
            int col = static_cast<int>(idx % cols);
            int row = static_cast<int>(idx / cols);

            ImVec2 cell_pos(col * cell_width, row * cell_height);
            ImGui::SetCursorPos(cell_pos);

            // 绘制单元格
            ImGui::BeginChild(
                ("DigitalCell_" + std::to_string(channel_index)).c_str(),
                ImVec2(cell_width - 10, cell_height - 10),
                true
            );

            // 通道名称
            ImGui::TextColored(
                ImVec4(config.color[0], config.color[1], config.color[2], 1.0f),
                "%s", config.name.c_str()
            );
            ImGui::Separator();

            // 当前值（大号字体）
            ImGui::SetWindowFontScale(font_scale_);

            // 格式化数值
            char value_str[64];
            snprintf(value_str, sizeof(value_str), "%.*f", decimal_places_, stats.last_value);

            // 居中显示
            float text_width = ImGui::CalcTextSize(value_str).x;
            float window_width = ImGui::GetWindowWidth();
            ImGui::SetCursorPosX((window_width - text_width) * 0.5f);
            ImGui::Text("%s", value_str);

            ImGui::SetWindowFontScale(1.0f);

            // 单位
            if (!unit_.empty()) {
                ImGui::SameLine();
                ImGui::Text("%s", unit_.c_str());
            }

            // 统计信息
            if (show_stats_ && stats.sample_count > 0) {
                ImGui::Separator();
                ImGui::Text("Min: %.*f", decimal_places_, stats.min_value);
                ImGui::Text("Max: %.*f", decimal_places_, stats.max_value);
                ImGui::Text("Avg: %.*f", decimal_places_, stats.avg_value);
                ImGui::Text("Samples: %zu", stats.sample_count);
            }

            ImGui::EndChild();
        }

        ImGui::EndChild();
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
    int decimal_places_;        // 小数位数
    bool show_stats_;           // 显示统计信息
    int grid_columns_;          // 网格列数
    float font_scale_;          // 字体缩放
    std::string unit_;          // 单位
};

#endif // DIGITAL_DISPLAY_WIDGET_H
