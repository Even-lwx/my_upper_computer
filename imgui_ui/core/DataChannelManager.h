/**
 * @file DataChannelManager.h
 * @brief 16通道数据管理器
 * @author AI Assistant
 * @date 2025
 *
 * 管理16个独立数据通道，每个通道包含：
 * - 循环缓冲区（存储历史数据）
 * - 通道配置（名称、颜色、启用状态等）
 * - 统计信息（最大值、最小值、平均值）
 */

#ifndef DATA_CHANNEL_MANAGER_H
#define DATA_CHANNEL_MANAGER_H

#include <array>
#include <mutex>
#include <chrono>
#include <string>
#include "DataTypes.h"
#include "CircularBuffer.h"

/**
 * @brief 通道统计信息
 */
struct ChannelStats {
    float min_value;
    float max_value;
    float avg_value;
    float last_value;     // 最后一个值
    size_t sample_count;

    ChannelStats()
        : min_value(0.0f)
        , max_value(0.0f)
        , avg_value(0.0f)
        , last_value(0.0f)
        , sample_count(0)
    {}

    void Reset() {
        min_value = max_value = avg_value = last_value = 0.0f;
        sample_count = 0;
    }
};

/**
 * @brief 16通道数据管理器
 */
class DataChannelManager {
public:
    static constexpr size_t MAX_CHANNELS = 16;
    static constexpr size_t BUFFER_SIZE = 2000;  // 每个通道2000个点

    DataChannelManager() {
        InitializeChannels();
        start_time_ = std::chrono::steady_clock::now();
    }

    /**
     * @brief 添加数据到指定通道
     * @param channel_index 通道索引（0-15）
     * @param value 数据值
     */
    void PushData(size_t channel_index, float value) {
        if (channel_index >= MAX_CHANNELS) return;

        std::lock_guard<std::mutex> lock(mutex_);

        // 计算时间戳
        auto now = std::chrono::steady_clock::now();
        double timestamp = std::chrono::duration<double>(now - start_time_).count();

        // 添加到缓冲区
        buffers_[channel_index].Push(DataPoint(timestamp, value));

        // 更新统计信息
        UpdateStats(channel_index, value);
    }

    /**
     * @brief 批量添加数据到多个通道
     * @param values 数据数组（按通道顺序）
     * @param num_channels 通道数量
     */
    void PushMultiChannelData(const float* values, size_t num_channels) {
        if (num_channels > MAX_CHANNELS) {
            num_channels = MAX_CHANNELS;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        auto now = std::chrono::steady_clock::now();
        double timestamp = std::chrono::duration<double>(now - start_time_).count();

        for (size_t i = 0; i < num_channels; i++) {
            buffers_[i].Push(DataPoint(timestamp, values[i]));
            UpdateStats(i, values[i]);
        }
    }

    /**
     * @brief 获取通道数据用于绘图
     * @param channel_index 通道索引
     * @param timestamps 输出时间戳数组
     * @param y_values 输出Y值数组
     * @param max_points 最大点数（0表示全部）
     * @return 实际点数
     */
    size_t GetChannelData(size_t channel_index,
                         std::vector<double>& timestamps,
                         std::vector<float>& y_values,
                         size_t max_points = 0) {
        if (channel_index >= MAX_CHANNELS) return 0;

        std::lock_guard<std::mutex> lock(mutex_);
        return buffers_[channel_index].GetXYValues(timestamps, y_values, max_points);
    }

    /**
     * @brief 获取通道Y值数组
     * @param channel_index 通道索引
     * @param y_values 输出Y值数组
     * @param max_points 最大点数
     * @return 实际点数
     */
    size_t GetChannelYValues(size_t channel_index,
                            std::vector<float>& y_values,
                            size_t max_points = 0) {
        if (channel_index >= MAX_CHANNELS) return 0;

        std::lock_guard<std::mutex> lock(mutex_);
        return buffers_[channel_index].GetYValues(y_values, max_points);
    }

    /**
     * @brief 获取通道最新值
     */
    float GetLatestValue(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return 0.0f;

        std::lock_guard<std::mutex> lock(mutex_);
        if (buffers_[channel_index].Empty()) {
            return 0.0f;
        }
        return buffers_[channel_index].GetLatest().value;
    }

    /**
     * @brief 清空指定通道
     */
    void ClearChannel(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return;

        std::lock_guard<std::mutex> lock(mutex_);
        buffers_[channel_index].Clear();
        stats_[channel_index].Reset();
    }

    /**
     * @brief 清空所有通道
     */
    void ClearAll() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (size_t i = 0; i < MAX_CHANNELS; i++) {
            buffers_[i].Clear();
            stats_[i].Reset();
        }
        start_time_ = std::chrono::steady_clock::now();
    }

    /**
     * @brief 设置通道配置
     */
    void SetChannelConfig(size_t channel_index, const ChannelConfig& config) {
        if (channel_index >= MAX_CHANNELS) return;

        std::lock_guard<std::mutex> lock(mutex_);
        configs_[channel_index] = config;
    }

    /**
     * @brief 获取通道配置
     */
    ChannelConfig GetChannelConfig(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return ChannelConfig();

        std::lock_guard<std::mutex> lock(mutex_);
        return configs_[channel_index];
    }

    /**
     * @brief 启用/禁用通道
     */
    void SetChannelEnabled(size_t channel_index, bool enabled) {
        if (channel_index >= MAX_CHANNELS) return;

        std::lock_guard<std::mutex> lock(mutex_);
        configs_[channel_index].enabled = enabled;
    }

    /**
     * @brief 判断通道是否启用
     */
    bool IsChannelEnabled(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        return configs_[channel_index].enabled;
    }

    /**
     * @brief 获取通道统计信息
     */
    ChannelStats GetChannelStats(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return ChannelStats();

        std::lock_guard<std::mutex> lock(mutex_);
        return stats_[channel_index];
    }

    /**
     * @brief 获取通道数据点数量
     */
    size_t GetChannelSize(size_t channel_index) {
        if (channel_index >= MAX_CHANNELS) return 0;

        std::lock_guard<std::mutex> lock(mutex_);
        return buffers_[channel_index].Size();
    }

    /**
     * @brief 获取所有启用的通道索引
     */
    std::vector<size_t> GetEnabledChannels() {
        std::lock_guard<std::mutex> lock(mutex_);

        std::vector<size_t> enabled;
        for (size_t i = 0; i < MAX_CHANNELS; i++) {
            if (configs_[i].enabled) {
                enabled.push_back(i);
            }
        }
        return enabled;
    }

private:
    /**
     * @brief 初始化所有通道
     */
    void InitializeChannels() {
        // 预定义颜色（16种不同颜色）
        const float colors[MAX_CHANNELS][4] = {
            {1.0f, 0.0f, 0.0f, 1.0f},   // 红
            {0.0f, 1.0f, 0.0f, 1.0f},   // 绿
            {0.0f, 0.0f, 1.0f, 1.0f},   // 蓝
            {1.0f, 1.0f, 0.0f, 1.0f},   // 黄
            {1.0f, 0.0f, 1.0f, 1.0f},   // 品红
            {0.0f, 1.0f, 1.0f, 1.0f},   // 青
            {1.0f, 0.5f, 0.0f, 1.0f},   // 橙
            {0.5f, 0.0f, 1.0f, 1.0f},   // 紫
            {0.0f, 0.5f, 1.0f, 1.0f},   // 天蓝
            {1.0f, 0.0f, 0.5f, 1.0f},   // 玫红
            {0.5f, 1.0f, 0.0f, 1.0f},   // 黄绿
            {0.0f, 1.0f, 0.5f, 1.0f},   // 春绿
            {1.0f, 0.75f, 0.0f, 1.0f},  // 金色
            {0.75f, 0.75f, 0.75f, 1.0f}, // 灰
            {1.0f, 0.5f, 0.5f, 1.0f},   // 粉红
            {0.5f, 0.5f, 1.0f, 1.0f}    // 淡蓝
        };

        for (size_t i = 0; i < MAX_CHANNELS; i++) {
            configs_[i].enabled = false;
            configs_[i].name = "CH" + std::to_string(i + 1);
            configs_[i].dataType = DataType::FLOAT;
            configs_[i].scale = 1.0f;
            configs_[i].offset = 0.0f;

            for (int j = 0; j < 4; j++) {
                configs_[i].color[j] = colors[i][j];
            }
        }
    }

    /**
     * @brief 更新通道统计信息
     */
    void UpdateStats(size_t channel_index, float value) {
        ChannelStats& stats = stats_[channel_index];

        if (stats.sample_count == 0) {
            stats.min_value = stats.max_value = stats.avg_value = value;
            stats.sample_count = 1;
        } else {
            // 更新最小/最大值
            if (value < stats.min_value) stats.min_value = value;
            if (value > stats.max_value) stats.max_value = value;

            // 更新平均值（增量式）
            stats.avg_value = (stats.avg_value * stats.sample_count + value) / (stats.sample_count + 1);
            stats.sample_count++;
        }

        // 更新最后一个值
        stats.last_value = value;
    }

    std::array<CircularBuffer<DataPoint, BUFFER_SIZE>, MAX_CHANNELS> buffers_;  // 16个缓冲区
    std::array<ChannelConfig, MAX_CHANNELS> configs_;                           // 16个通道配置
    std::array<ChannelStats, MAX_CHANNELS> stats_;                              // 16个统计信息
    mutable std::mutex mutex_;                                                   // 互斥锁
    std::chrono::steady_clock::time_point start_time_;                          // 起始时间
};

#endif // DATA_CHANNEL_MANAGER_H
