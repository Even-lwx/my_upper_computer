/**
 * @file Widget.h
 * @brief 可视化组件基类
 * @author AI Assistant
 * @date 2025
 *
 * Widget是所有可视化组件的基类，定义统一接口
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <string>
#include <vector>
#include <memory>
#include <imgui.h>
#include "../core/DataChannelManager.h"

/**
 * @brief 组件类型枚举
 */
enum class WidgetType {
    WAVEFORM,        // 波形图
    DIGITAL_DISPLAY, // 数字表盘
    BAR_CHART,       // 柱状图
    GAUGE,           // 仪表盘
    DATA_TABLE       // 数据表格
};

/**
 * @brief 获取组件类型名称
 */
inline std::string GetWidgetTypeName(WidgetType type) {
    switch (type) {
        case WidgetType::WAVEFORM:        return "Waveform";
        case WidgetType::DIGITAL_DISPLAY: return "Digital Display";
        case WidgetType::BAR_CHART:       return "Bar Chart";
        case WidgetType::GAUGE:           return "Gauge";
        case WidgetType::DATA_TABLE:      return "Data Table";
        default: return "Unknown";
    }
}

/**
 * @brief Widget基类（抽象类）
 */
class Widget {
public:
    Widget(WidgetType type, const std::string& name = "")
        : type_(type)
        , name_(name.empty() ? GetWidgetTypeName(type) : name)
        , id_(next_id_++)
        , visible_(true)
    {}

    virtual ~Widget() = default;

    /**
     * @brief 渲染组件
     * @param channel_manager 数据通道管理器
     */
    virtual void Render(DataChannelManager& channel_manager) = 0;

    /**
     * @brief 渲染配置面板
     */
    virtual void RenderConfig() {}

    /**
     * @brief 设置组件使用的通道
     */
    virtual void SetChannels(const std::vector<size_t>& channels) {
        channels_ = channels;
    }

    /**
     * @brief 获取组件使用的通道
     */
    const std::vector<size_t>& GetChannels() const {
        return channels_;
    }

    /**
     * @brief 获取组件类型
     */
    WidgetType GetType() const {
        return type_;
    }

    /**
     * @brief 获取组件名称
     */
    const std::string& GetName() const {
        return name_;
    }

    /**
     * @brief 设置组件名称
     */
    void SetName(const std::string& name) {
        name_ = name;
    }

    /**
     * @brief 获取组件唯一ID
     */
    int GetID() const {
        return id_;
    }

    /**
     * @brief 设置组件可见性
     */
    void SetVisible(bool visible) {
        visible_ = visible;
    }

    /**
     * @brief 判断组件是否可见
     */
    bool IsVisible() const {
        return visible_;
    }

    /**
     * @brief 设置组件位置
     */
    void SetPosition(float x, float y) {
        position_x_ = x;
        position_y_ = y;
    }

    /**
     * @brief 设置组件大小
     */
    void SetSize(float width, float height) {
        width_ = width;
        height_ = height;
    }

    /**
     * @brief 获取组件位置
     */
    ImVec2 GetPosition() const {
        return ImVec2(position_x_, position_y_);
    }

    /**
     * @brief 获取组件大小
     */
    ImVec2 GetSize() const {
        return ImVec2(width_, height_);
    }

protected:
    /**
     * @brief 生成ImGui窗口ID
     */
    std::string GetImGuiID() const {
        return name_ + "##" + std::to_string(id_);
    }

    WidgetType type_;                   // 组件类型
    std::string name_;                  // 组件名称
    int id_;                            // 唯一ID
    bool visible_;                      // 是否可见
    std::vector<size_t> channels_;      // 使用的通道列表

    // 位置和大小
    float position_x_ = 0.0f;
    float position_y_ = 0.0f;
    float width_ = 400.0f;
    float height_ = 300.0f;

private:
    static int next_id_;  // 全局ID计数器
};

// 初始化静态成员
inline int Widget::next_id_ = 0;

#endif // WIDGET_H
