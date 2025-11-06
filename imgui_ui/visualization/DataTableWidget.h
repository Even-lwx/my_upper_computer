/**
 * @file DataTableWidget.h
 * @brief 数据表格组件 - 显示历史数据记录
 * @author AI Assistant
 * @date 2025
 *
 * 特性：
 * - 表格形式显示多通道历史数据
 * - 支持滚动查看
 * - 时间戳显示
 * - 可配置显示行数
 * - 支持CSV导出（预留接口）
 */

#ifndef DATA_TABLE_WIDGET_H
#define DATA_TABLE_WIDGET_H

#include "Widget.h"
#include <vector>
#include <iomanip>
#include <sstream>

/**
 * @brief 数据表格组件
 */
class DataTableWidget : public Widget {
public:
    DataTableWidget(const std::string& name = "Data Table")
        : Widget(WidgetType::DATA_TABLE, name)
        , max_rows_(100)
        , show_timestamp_(true)
        , decimal_places_(3)
        , auto_scroll_(true)
    {
        // 默认显示前4个通道
        channels_ = {0, 1, 2, 3};
    }

    void Render(DataChannelManager& channel_manager) override {
        if (!visible_) return;

        ImGui::SetNextWindowSize(GetSize(), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(GetPosition(), ImGuiCond_FirstUseEver);

        if (ImGui::Begin(GetImGuiID().c_str(), &visible_)) {
            RenderDataTable(channel_manager);
        }
        ImGui::End();
    }

    void RenderConfig() override {
        ImGui::Text("数据表格配置");
        ImGui::Separator();

        ImGui::SliderInt("最大行数", &max_rows_, 10, 1000);
        ImGui::SliderInt("小数位数", &decimal_places_, 0, 6);
        ImGui::Checkbox("显示时间戳", &show_timestamp_);
        ImGui::Checkbox("自动滚动", &auto_scroll_);

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
     * @brief 渲染数据表格
     */
    void RenderDataTable(DataChannelManager& channel_manager) {
        // 收集所有通道的数据
        std::vector<std::vector<double>> timestamps_list;
        std::vector<std::vector<float>> values_list;
        std::vector<std::string> channel_names;

        size_t min_data_points = SIZE_MAX;

        for (size_t channel_index : channels_) {
            if (channel_index >= DataChannelManager::MAX_CHANNELS) {
                continue;
            }

            if (!channel_manager.IsChannelEnabled(channel_index)) {
                continue;
            }

            ChannelConfig config = channel_manager.GetChannelConfig(channel_index);
            channel_names.push_back(config.name);

            std::vector<double> timestamps;
            std::vector<float> values;
            size_t count = channel_manager.GetChannelData(
                channel_index, timestamps, values, max_rows_);

            timestamps_list.push_back(timestamps);
            values_list.push_back(values);

            if (count < min_data_points) {
                min_data_points = count;
            }
        }

        if (channel_names.empty() || min_data_points == 0) {
            ImGui::Text("没有可用数据");
            return;
        }

        // 创建表格
        int num_columns = static_cast<int>(channel_names.size()) + (show_timestamp_ ? 1 : 0);

        ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                               ImGuiTableFlags_RowBg |
                               ImGuiTableFlags_ScrollY |
                               ImGuiTableFlags_Resizable |
                               ImGuiTableFlags_Reorderable;

        if (ImGui::BeginTable("DataTable", num_columns, flags)) {
            // 表头
            if (show_timestamp_) {
                ImGui::TableSetupColumn("Time (s)", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            }
            for (const auto& name : channel_names) {
                ImGui::TableSetupColumn(name.c_str(), ImGuiTableColumnFlags_WidthStretch);
            }
            ImGui::TableHeadersRow();

            // 数据行
            for (size_t row = 0; row < min_data_points; row++) {
                ImGui::TableNextRow();

                // 时间戳列
                if (show_timestamp_) {
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%.3f", timestamps_list[0][row]);
                }

                // 各通道数据列
                for (size_t col = 0; col < channel_names.size(); col++) {
                    int table_col = static_cast<int>(col) + (show_timestamp_ ? 1 : 0);
                    ImGui::TableSetColumnIndex(table_col);

                    // 格式化数值
                    char value_str[32];
                    snprintf(value_str, sizeof(value_str), "%.*f",
                            decimal_places_, values_list[col][row]);
                    ImGui::Text("%s", value_str);
                }
            }

            // 自动滚动到底部
            if (auto_scroll_ && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndTable();
        }

        // 统计信息
        ImGui::Separator();
        ImGui::Text("总行数: %zu", min_data_points);

        // 导出按钮（预留功能）
        ImGui::SameLine();
        if (ImGui::Button("导出CSV")) {
            ExportToCSV(timestamps_list, values_list, channel_names);
        }
    }

    /**
     * @brief 导出数据到CSV文件
     */
    void ExportToCSV(const std::vector<std::vector<double>>& timestamps_list,
                     const std::vector<std::vector<float>>& values_list,
                     const std::vector<std::string>& channel_names) {
        // 生成文件名
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << "data_export_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S") << ".csv";
        std::string filename = ss.str();

        // 打开文件
        std::ofstream file(filename);
        if (!file.is_open()) {
            // TODO: 显示错误消息
            return;
        }

        // 写入表头
        if (show_timestamp_) {
            file << "Time(s)";
            for (const auto& name : channel_names) {
                file << "," << name;
            }
        } else {
            for (size_t i = 0; i < channel_names.size(); i++) {
                if (i > 0) file << ",";
                file << channel_names[i];
            }
        }
        file << "\n";

        // 写入数据
        size_t row_count = timestamps_list.empty() ? 0 : timestamps_list[0].size();
        for (size_t row = 0; row < row_count; row++) {
            if (show_timestamp_) {
                file << std::fixed << std::setprecision(3) << timestamps_list[0][row];
                for (size_t col = 0; col < values_list.size(); col++) {
                    file << "," << std::fixed << std::setprecision(decimal_places_)
                         << values_list[col][row];
                }
            } else {
                for (size_t col = 0; col < values_list.size(); col++) {
                    if (col > 0) file << ",";
                    file << std::fixed << std::setprecision(decimal_places_)
                         << values_list[col][row];
                }
            }
            file << "\n";
        }

        file.close();

        // TODO: 显示成功消息，可以使用ImGui::OpenPopup
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
    int max_rows_;              // 最大显示行数
    bool show_timestamp_;       // 显示时间戳
    int decimal_places_;        // 小数位数
    bool auto_scroll_;          // 自动滚动
};

#endif // DATA_TABLE_WIDGET_H
