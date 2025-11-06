/**
 * @file ComponentLibrary.h
 * @brief 组件库 - 侧边栏显示所有可用的可视化组件
 * @author AI Assistant
 * @date 2025
 *
 * 功能：
 * - 显示所有可用组件类型
 * - 支持拖拽组件到工作区
 * - 显示组件说明和图标
 * - 统计当前工作区组件数量
 */

#ifndef COMPONENT_LIBRARY_H
#define COMPONENT_LIBRARY_H

#include "WorkspaceManager.h"
#include <imgui.h>

/**
 * @brief 组件库类
 */
class ComponentLibrary {
public:
    /**
     * @brief 组件信息
     */
    struct ComponentInfo {
        WidgetType type;
        std::string name;
        std::string description;
        const char* icon;  // Unicode图标
    };

    ComponentLibrary() {
        // 初始化组件信息
        components_ = {
            {WidgetType::WAVEFORM, "波形图", "实时显示多通道时序数据", "📈"},
            {WidgetType::DIGITAL_DISPLAY, "数字表盘", "大号数字显示当前值", "🔢"},
            {WidgetType::BAR_CHART, "柱状图", "多通道对比显示", "📊"},
            {WidgetType::GAUGE, "仪表盘", "圆形指针式显示", "⏲️"},
            {WidgetType::DATA_TABLE, "数据表格", "历史数据表格显示", "📋"}
        };
    }

    /**
     * @brief 渲染组件库侧边栏
     * @param workspace_manager 工作区管理器
     * @param is_open 侧边栏是否打开
     */
    void Render(WorkspaceManager& workspace_manager, bool* is_open = nullptr) {
        ImGui::SetNextWindowSize(ImVec2(250, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(10, 50), ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

        if (ImGui::Begin("组件库", is_open, flags)) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "拖拽组件到工作区");
            ImGui::Separator();

            // 显示所有可用组件
            for (const auto& comp : components_) {
                RenderComponentItem(comp, workspace_manager);
                ImGui::Separator();
            }

            // 统计信息
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("当前组件数: %zu", workspace_manager.GetWidgetCount());

            // 清空按钮
            if (ImGui::Button("清空所有组件", ImVec2(-1, 0))) {
                workspace_manager.Clear();
            }
        }
        ImGui::End();
    }

private:
    /**
     * @brief 渲染单个组件项
     */
    void RenderComponentItem(const ComponentInfo& comp, WorkspaceManager& workspace_manager) {
        ImGui::PushID(&comp);

        // 组件图标和名称
        ImGui::Text("%s %s", comp.icon, comp.name.c_str());
        ImGui::TextWrapped("%s", comp.description.c_str());

        // 创建按钮
        if (ImGui::Button("添加", ImVec2(-1, 0))) {
            workspace_manager.CreateWidget(comp.type);
        }

        // 拖拽源
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // 设置拖拽数据（组件类型）
            ImGui::SetDragDropPayload("WIDGET_TYPE", &comp.type, sizeof(WidgetType));

            // 拖拽时显示的内容
            ImGui::Text("%s %s", comp.icon, comp.name.c_str());

            ImGui::EndDragDropSource();
        }

        // 鼠标悬停提示
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("点击或拖拽添加 %s", comp.name.c_str());
            ImGui::EndTooltip();
        }

        ImGui::PopID();
    }

    std::vector<ComponentInfo> components_;  // 所有可用组件信息
};

#endif // COMPONENT_LIBRARY_H
