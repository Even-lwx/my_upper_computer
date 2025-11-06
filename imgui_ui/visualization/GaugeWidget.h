/**
 * @file GaugeWidget.h
 * @brief 仪表盘组件 - 圆形指针式显示
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 圆形仪表盘显示
 * - 动态指针指示当前值
 * - 可配置范围和颜色区间
 * - 刻度线和数值标签
 * - 支持多通道网格布局
 */

#ifndef GAUGE_WIDGET_H
#define GAUGE_WIDGET_H

#include "Widget.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief 仪表盘组件
 */
class GaugeWidget : public Widget {
public:
    GaugeWidget(const std::string& name = "Gauge")
        : Widget(WidgetType::GAUGE, name)
        , min_value_(0.0f)
        , max_value_(100.0f)
        , grid_columns_(2)
        , show_value_text_(true)
        , show_ticks_(true)
    {
        // 默认显示通道0
        channels_ = {0};
    }

    void Render(DataChannelManager& channel_manager) override {
        if (!visible_) return;

        ImGui::SetNextWindowSize(GetSize(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(GetPosition(), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(GetImGuiID().c_str(), &visible_)) {
            RenderGauges(channel_manager);
        }
        ImGui::End();
    }

    void RenderConfig() override {
        ImGui::Text("仪表盘配置");
        ImGui::Separator();

        ImGui::SliderFloat("最小值", &min_value_, -1000.0f, 1000.0f);
        ImGui::SliderFloat("最大值", &max_value_, -1000.0f, 1000.0f);
        ImGui::SliderInt("网格列数", &grid_columns_, 1, 4);
        ImGui::Checkbox("显示数值", &show_value_text_);
        ImGui::Checkbox("显示刻度", &show_ticks_);

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
     * @brief 渲染所有仪表盘
     */
    void RenderGauges(DataChannelManager& channel_manager) {
        // 计算网格布局
        int cols = grid_columns_;
        int rows = static_cast<int>(std::ceil(static_cast<float>(channels_.size()) / cols));

        ImVec2 available_size = ImGui::GetContentRegionAvail();
        float cell_width = available_size.x / cols;
        float cell_height = available_size.y / rows;

        ImGui::BeginChild("GaugeGrid", available_size, false, ImGuiWindowFlags_NoScrollbar);

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

            // 绘制单个仪表盘
            ImGui::BeginChild(
                ("GaugeCell_" + std::to_string(channel_index)).c_str(),
                ImVec2(cell_width - 10, cell_height - 10),
                true
            );

            // 通道名称
            ImGui::TextColored(
                ImVec4(config.color[0], config.color[1], config.color[2], 1.0f),
                "%s", config.name.c_str()
            );

            // 绘制仪表盘
            DrawGauge(config, stats.last_value);

            ImGui::EndChild();
        }

        ImGui::EndChild();
    }

    /**
     * @brief 绘制单个仪表盘
     */
    void DrawGauge(const ChannelConfig& config, float value) {
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

        // 计算中心和半径
        float gauge_size = std::min(canvas_size.x, canvas_size.y) - 20.0f;
        ImVec2 center(canvas_pos.x + canvas_size.x * 0.5f, canvas_pos.y + canvas_size.y * 0.5f);
        float radius = gauge_size * 0.5f;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // 仪表盘范围：-135度到+135度（270度）
        float start_angle = 0.75f * static_cast<float>(M_PI);  // -135度
        float end_angle = 2.25f * static_cast<float>(M_PI);     // +135度

        // 绘制外圈背景
        draw_list->AddCircle(center, radius, IM_COL32(100, 100, 100, 255), 64, 3.0f);

        // 绘制刻度线
        if (show_ticks_) {
            int num_ticks = 10;
            for (int i = 0; i <= num_ticks; i++) {
                float angle = start_angle + (end_angle - start_angle) * i / num_ticks;
                ImVec2 tick_start(center.x + std::cos(angle) * (radius - 10),
                                 center.y + std::sin(angle) * (radius - 10));
                ImVec2 tick_end(center.x + std::cos(angle) * radius,
                               center.y + std::sin(angle) * radius);
                draw_list->AddLine(tick_start, tick_end, IM_COL32(150, 150, 150, 255), 2.0f);
            }
        }

        // 计算当前值对应的角度
        float normalized = (value - min_value_) / (max_value_ - min_value_);
        normalized = std::max(0.0f, std::min(1.0f, normalized));  // 限制在[0,1]
        float value_angle = start_angle + (end_angle - start_angle) * normalized;

        // 绘制彩色弧线（根据值显示不同颜色）
        ImU32 arc_color;
        if (normalized < 0.33f) {
            arc_color = IM_COL32(0, 255, 0, 255);  // 绿色
        } else if (normalized < 0.66f) {
            arc_color = IM_COL32(255, 255, 0, 255);  // 黄色
        } else {
            arc_color = IM_COL32(255, 0, 0, 255);  // 红色
        }

        // 绘制从起始到当前值的弧线
        int num_segments = static_cast<int>(64 * normalized);
        if (num_segments > 1) {
            for (int i = 0; i < num_segments; i++) {
                float a1 = start_angle + (value_angle - start_angle) * i / num_segments;
                float a2 = start_angle + (value_angle - start_angle) * (i + 1) / num_segments;
                ImVec2 p1(center.x + std::cos(a1) * (radius - 5),
                         center.y + std::sin(a1) * (radius - 5));
                ImVec2 p2(center.x + std::cos(a2) * (radius - 5),
                         center.y + std::sin(a2) * (radius - 5));
                draw_list->AddLine(p1, p2, arc_color, 4.0f);
            }
        }

        // 绘制指针
        float needle_length = radius * 0.7f;
        ImVec2 needle_end(center.x + std::cos(value_angle) * needle_length,
                         center.y + std::sin(value_angle) * needle_length);
        draw_list->AddLine(center, needle_end, IM_COL32(255, 255, 255, 255), 3.0f);

        // 绘制中心圆点
        draw_list->AddCircleFilled(center, 5.0f, IM_COL32(200, 200, 200, 255));

        // 显示数值文本
        if (show_value_text_) {
            char value_text[32];
            snprintf(value_text, sizeof(value_text), "%.2f", value);
            ImVec2 text_size = ImGui::CalcTextSize(value_text);
            ImVec2 text_pos(center.x - text_size.x * 0.5f, center.y + radius * 0.4f);
            draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), value_text);
        }

        // 显示范围文本
        char min_text[32], max_text[32];
        snprintf(min_text, sizeof(min_text), "%.0f", min_value_);
        snprintf(max_text, sizeof(max_text), "%.0f", max_value_);

        ImVec2 min_text_size = ImGui::CalcTextSize(min_text);
        ImVec2 max_text_size = ImGui::CalcTextSize(max_text);

        ImVec2 min_text_pos(center.x - radius * 0.7f - min_text_size.x * 0.5f,
                           center.y + radius * 0.7f);
        ImVec2 max_text_pos(center.x + radius * 0.7f - max_text_size.x * 0.5f,
                           center.y + radius * 0.7f);

        draw_list->AddText(min_text_pos, IM_COL32(150, 150, 150, 255), min_text);
        draw_list->AddText(max_text_pos, IM_COL32(150, 150, 150, 255), max_text);
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
    float min_value_;           // 最小值
    float max_value_;           // 最大值
    int grid_columns_;          // 网格列数
    bool show_value_text_;      // 显示数值
    bool show_ticks_;           // 显示刻度
};

#endif // GAUGE_WIDGET_H
