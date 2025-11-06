/**
 * @file VisualizationUI.h
 * @brief 可视化UI管理器 - 整合所有可视化功能
 * @author AI Assistant
 * @date 2025
 *
 * 功能：
 * - 整合组件库和工作区
 * - 实现拖拽创建组件
 * - 管理协议选择和通道配置
 * - 提供统一的UI入口
 */

#ifndef VISUALIZATION_UI_H
#define VISUALIZATION_UI_H

#include "ComponentLibrary.h"
#include "WorkspaceManager.h"
#include "../core/DataChannelManager.h"
#include "../protocols/ProtocolParser.h"
#include "../protocols/FireWaterParser.h"
#include "../protocols/JustFloatParser.h"
#include "../protocols/RawDataParser.h"
#include "../protocols/CustomParser.h"
#include <memory>

/**
 * @brief 可视化UI管理器
 */
class VisualizationUI {
public:
    VisualizationUI()
        : show_component_library_(true)
        , show_channel_config_(true)
        , show_protocol_config_(true)
        , current_protocol_type_(ProtocolType::FIREWATER)
    {
        // 初始化默认协议解析器
        protocol_parser_ = std::make_unique<FireWaterParser>();
    }

    /**
     * @brief 渲染所有UI
     */
    void Render() {
        // 渲染组件库侧边栏
        if (show_component_library_) {
            component_library_.Render(workspace_manager_, &show_component_library_);
        }

        // 渲染工作区（拖拽目标）
        RenderWorkspace();

        // 渲染通道配置窗口
        if (show_channel_config_) {
            RenderChannelConfig();
        }

        // 渲染协议配置窗口
        if (show_protocol_config_) {
            RenderProtocolConfig();
        }

        // 渲染组件配置窗口
        RenderWidgetConfigs();
    }

    /**
     * @brief 获取数据通道管理器
     */
    DataChannelManager& GetChannelManager() {
        return channel_manager_;
    }

    /**
     * @brief 获取协议解析器
     */
    ProtocolParser* GetProtocolParser() {
        return protocol_parser_.get();
    }

    /**
     * @brief 设置协议类型
     */
    void SetProtocolType(ProtocolType type) {
        if (type == current_protocol_type_) {
            return;
        }

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

    /**
     * @brief 处理接收到的数据
     * @param data 数据缓冲区
     * @param length 数据长度
     */
    void ProcessReceivedData(const unsigned char* data, size_t length) {
        if (!protocol_parser_) {
            return;
        }

        // 解析数据
        ParseResult result = protocol_parser_->Parse(data, length);

        if (result.success && !result.values.empty()) {
            // 将数据推送到通道管理器
            channel_manager_.PushMultiChannelData(result.values.data(), result.values.size());
        }
    }

private:
    /**
     * @brief 渲染工作区（拖拽目标）
     */
    void RenderWorkspace() {
        ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(270, 50), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("工作区", nullptr, ImGuiWindowFlags_NoCollapse)) {
            // 拖拽目标区域
            ImVec2 workspace_size = ImGui::GetContentRegionAvail();
            ImGui::InvisibleButton("workspace_area", workspace_size);

            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET_TYPE")) {
                    WidgetType type = *static_cast<const WidgetType*>(payload->Data);

                    // 在鼠标位置创建组件
                    Widget* new_widget = workspace_manager_.CreateWidget(type);
                    if (new_widget) {
                        ImVec2 mouse_pos = ImGui::GetMousePos();
                        ImVec2 window_pos = ImGui::GetWindowPos();
                        new_widget->SetPosition(mouse_pos.x - window_pos.x, mouse_pos.y - window_pos.y);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // 显示提示信息
            if (workspace_manager_.GetWidgetCount() == 0) {
                ImVec2 text_size = ImGui::CalcTextSize("拖拽组件到此处创建可视化");
                ImGui::SetCursorPos(ImVec2(
                    (workspace_size.x - text_size.x) * 0.5f,
                    (workspace_size.y - text_size.y) * 0.5f
                ));
                ImGui::TextDisabled("拖拽组件到此处创建可视化");
            }
        }
        ImGui::End();

        // 渲染所有组件
        workspace_manager_.RenderAll(channel_manager_);
    }

    /**
     * @brief 渲染通道配置窗口
     */
    void RenderChannelConfig() {
        ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(1080, 50), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("通道配置", &show_channel_config_)) {
            ImGui::Text("16通道配置");
            ImGui::Separator();

            for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
                ImGui::PushID(static_cast<int>(i));

                ChannelConfig config = channel_manager_.GetChannelConfig(i);
                ChannelStats stats = channel_manager_.GetChannelStats(i);

                // 启用/禁用复选框
                bool enabled = config.enabled;
                if (ImGui::Checkbox("##enabled", &enabled)) {
                    channel_manager_.SetChannelEnabled(i, enabled);
                }

                ImGui::SameLine();

                // 通道名称
                char name_buffer[64];
                strncpy(name_buffer, config.name.c_str(), sizeof(name_buffer) - 1);
                name_buffer[sizeof(name_buffer) - 1] = '\0';

                ImGui::SetNextItemWidth(150);
                if (ImGui::InputText("##name", name_buffer, sizeof(name_buffer))) {
                    config.name = name_buffer;
                    channel_manager_.SetChannelConfig(i, config);
                }

                ImGui::SameLine();

                // 颜色选择
                if (ImGui::ColorEdit4("##color", config.color,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
                    channel_manager_.SetChannelConfig(i, config);
                }

                ImGui::SameLine();

                // 当前值
                ImGui::Text("%.3f", stats.last_value);

                ImGui::PopID();
            }
        }
        ImGui::End();
    }

    /**
     * @brief 渲染协议配置窗口
     */
    void RenderProtocolConfig() {
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(270, 660), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("协议配置", &show_protocol_config_)) {
            ImGui::Text("选择协议类型");
            ImGui::Separator();

            const char* protocol_names[] = {"FireWater", "JustFloat", "RawData", "Custom"};
            int current_protocol = static_cast<int>(current_protocol_type_);

            if (ImGui::Combo("协议", &current_protocol, protocol_names, IM_ARRAYSIZE(protocol_names))) {
                SetProtocolType(static_cast<ProtocolType>(current_protocol));
            }

            ImGui::Separator();
            ImGui::Text("协议信息：");
            if (protocol_parser_) {
                ImGui::Text("名称: %s", protocol_parser_->GetName().c_str());
                ImGui::Text("通道数: %zu", protocol_parser_->GetExpectedChannelCount());
            }
        }
        ImGui::End();
    }

    /**
     * @brief 渲染组件配置窗口
     */
    void RenderWidgetConfigs() {
        if (workspace_manager_.GetWidgetCount() == 0) {
            return;
        }

        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(580, 660), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("组件配置")) {
            workspace_manager_.RenderAllConfigs();
        }
        ImGui::End();
    }

    ComponentLibrary component_library_;            // 组件库
    WorkspaceManager workspace_manager_;            // 工作区管理器
    DataChannelManager channel_manager_;            // 数据通道管理器
    std::unique_ptr<ProtocolParser> protocol_parser_;  // 协议解析器
    ProtocolType current_protocol_type_;            // 当前协议类型

    // UI显示控制
    bool show_component_library_;
    bool show_channel_config_;
    bool show_protocol_config_;
};

#endif // VISUALIZATION_UI_H
