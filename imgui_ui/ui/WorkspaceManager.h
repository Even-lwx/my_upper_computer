/**
 * @file WorkspaceManager.h
 * @brief 工作区管理器 - 管理所有可视化组件实例
 * @author AI Assistant
 * @date 2025
 *
 * 功能：
 * - 管理组件实例的创建和销毁
 * - 渲染所有活动组件
 * - 支持组件拖拽和定位
 * - 组件配置持久化（预留）
 */

#ifndef WORKSPACE_MANAGER_H
#define WORKSPACE_MANAGER_H

#include "../visualization/Widget.h"
#include "../visualization/WaveformWidget.h"
#include "../visualization/DigitalDisplayWidget.h"
#include "../visualization/BarChartWidget.h"
#include "../visualization/GaugeWidget.h"
#include "../visualization/DataTableWidget.h"
#include <vector>
#include <memory>
#include <string>

/**
 * @brief 工作区管理器
 */
class WorkspaceManager {
public:
    WorkspaceManager() = default;
    ~WorkspaceManager() = default;

    /**
     * @brief 创建新组件
     * @param type 组件类型
     * @return 组件指针（不转移所有权）
     */
    Widget* CreateWidget(WidgetType type) {
        std::unique_ptr<Widget> widget;

        switch (type) {
            case WidgetType::WAVEFORM:
                widget = std::make_unique<WaveformWidget>("Waveform " + std::to_string(next_widget_id_++));
                break;

            case WidgetType::DIGITAL_DISPLAY:
                widget = std::make_unique<DigitalDisplayWidget>("Digital Display " + std::to_string(next_widget_id_++));
                break;

            case WidgetType::BAR_CHART:
                widget = std::make_unique<BarChartWidget>("Bar Chart " + std::to_string(next_widget_id_++));
                break;

            case WidgetType::GAUGE:
                widget = std::make_unique<GaugeWidget>("Gauge " + std::to_string(next_widget_id_++));
                break;

            case WidgetType::DATA_TABLE:
                widget = std::make_unique<DataTableWidget>("Data Table " + std::to_string(next_widget_id_++));
                break;

            default:
                return nullptr;
        }

        // 设置默认位置（阶梯式）
        float offset = static_cast<float>(widgets_.size() * 30);
        widget->SetPosition(200.0f + offset, 100.0f + offset);

        Widget* ptr = widget.get();
        widgets_.push_back(std::move(widget));
        return ptr;
    }

    /**
     * @brief 删除组件
     * @param widget_id 组件ID
     */
    void DeleteWidget(int widget_id) {
        widgets_.erase(
            std::remove_if(widgets_.begin(), widgets_.end(),
                [widget_id](const std::unique_ptr<Widget>& w) {
                    return w->GetID() == widget_id;
                }),
            widgets_.end());
    }

    /**
     * @brief 渲染所有组件
     * @param channel_manager 数据通道管理器
     */
    void RenderAll(DataChannelManager& channel_manager) {
        // 先删除标记为不可见的组件（用户关闭窗口）
        widgets_.erase(
            std::remove_if(widgets_.begin(), widgets_.end(),
                [](const std::unique_ptr<Widget>& w) {
                    return !w->IsVisible();
                }),
            widgets_.end());

        // 渲染所有可见组件
        for (auto& widget : widgets_) {
            if (widget->IsVisible()) {
                widget->Render(channel_manager);
            }
        }
    }

    /**
     * @brief 渲染所有组件的配置窗口
     */
    void RenderAllConfigs() {
        for (auto& widget : widgets_) {
            ImGui::PushID(widget->GetID());

            if (ImGui::TreeNode(widget->GetName().c_str())) {
                widget->RenderConfig();

                // 删除按钮
                ImGui::Separator();
                if (ImGui::Button("删除此组件")) {
                    DeleteWidget(widget->GetID());
                    ImGui::TreePop();
                    ImGui::PopID();
                    break;  // 退出循环，因为vector已修改
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    /**
     * @brief 获取所有组件
     */
    const std::vector<std::unique_ptr<Widget>>& GetWidgets() const {
        return widgets_;
    }

    /**
     * @brief 清空所有组件
     */
    void Clear() {
        widgets_.clear();
    }

    /**
     * @brief 获取组件数量
     */
    size_t GetWidgetCount() const {
        return widgets_.size();
    }

    /**
     * @brief 根据ID查找组件
     */
    Widget* FindWidget(int widget_id) {
        for (auto& widget : widgets_) {
            if (widget->GetID() == widget_id) {
                return widget.get();
            }
        }
        return nullptr;
    }

private:
    std::vector<std::unique_ptr<Widget>> widgets_;  // 所有组件实例
    int next_widget_id_ = 1;                        // 下一个组件ID
};

#endif // WORKSPACE_MANAGER_H
