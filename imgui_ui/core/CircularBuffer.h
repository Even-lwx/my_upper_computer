/**
 * @file CircularBuffer.h
 * @brief 高性能循环缓冲区 - 用于存储时序数据
 * @author AI Assistant
 * @date 2025
 *
 * 循环缓冲区特性：
 * - 固定大小，避免动态内存分配
 * - O(1)时间复杂度的读写操作
 * - 自动覆盖最旧数据
 * - 线程安全（通过外部互斥锁）
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <vector>
#include <algorithm>
#include "DataTypes.h"

/**
 * @brief 循环缓冲区模板类
 * @tparam T 数据类型
 * @tparam Capacity 缓冲区容量（默认2000个点）
 */
template<typename T, size_t Capacity = 2000>
class CircularBuffer {
public:
    CircularBuffer() : head_(0), count_(0) {
        data_.resize(Capacity);
    }

    /**
     * @brief 添加数据点
     * @param value 数据值
     */
    void Push(const T& value) {
        data_[head_] = value;
        head_ = (head_ + 1) % Capacity;
        if (count_ < Capacity) {
            count_++;
        }
    }

    /**
     * @brief 批量添加数据点
     * @param values 数据数组
     * @param length 数据长度
     */
    void PushBatch(const T* values, size_t length) {
        for (size_t i = 0; i < length; i++) {
            Push(values[i]);
        }
    }

    /**
     * @brief 清空缓冲区
     */
    void Clear() {
        head_ = 0;
        count_ = 0;
    }

    /**
     * @brief 获取当前数据点数量
     */
    size_t Size() const {
        return count_;
    }

    /**
     * @brief 判断缓冲区是否为空
     */
    bool Empty() const {
        return count_ == 0;
    }

    /**
     * @brief 判断缓冲区是否已满
     */
    bool Full() const {
        return count_ == Capacity;
    }

    /**
     * @brief 获取最新的数据点
     */
    const T& GetLatest() const {
        if (count_ == 0) {
            static T dummy;
            return dummy;
        }
        size_t latest_index = (head_ + Capacity - 1) % Capacity;
        return data_[latest_index];
    }

    /**
     * @brief 获取索引位置的数据（0是最旧，count-1是最新）
     * @param index 索引位置
     */
    const T& Get(size_t index) const {
        if (index >= count_) {
            static T dummy;
            return dummy;
        }
        // 计算实际位置
        size_t oldest_index = (head_ + Capacity - count_) % Capacity;
        size_t actual_index = (oldest_index + index) % Capacity;
        return data_[actual_index];
    }

    /**
     * @brief 获取连续数据用于绘图（ImPlot要求）
     * @param output 输出缓冲区
     * @param max_points 最大点数（用于降采样）
     * @return 实际返回的点数
     */
    size_t GetContinuousData(std::vector<T>& output, size_t max_points = 0) const {
        if (count_ == 0) {
            output.clear();
            return 0;
        }

        // 确定返回的点数
        size_t num_points = count_;
        if (max_points > 0 && count_ > max_points) {
            // 降采样
            num_points = max_points;
        }

        output.resize(num_points);

        if (num_points == count_) {
            // 不需要降采样，直接拷贝
            size_t oldest_index = (head_ + Capacity - count_) % Capacity;
            if (oldest_index + count_ <= Capacity) {
                // 数据连续，一次拷贝
                std::copy(data_.begin() + oldest_index,
                         data_.begin() + oldest_index + count_,
                         output.begin());
            } else {
                // 数据跨越边界，两次拷贝
                size_t first_part = Capacity - oldest_index;
                std::copy(data_.begin() + oldest_index, data_.end(), output.begin());
                std::copy(data_.begin(), data_.begin() + count_ - first_part,
                         output.begin() + first_part);
            }
        } else {
            // 需要降采样
            size_t step = count_ / num_points;
            for (size_t i = 0; i < num_points; i++) {
                output[i] = Get(i * step);
            }
        }

        return num_points;
    }

    /**
     * @brief 获取仅Y值数组用于ImPlot（假设X是等间隔的）
     * @param y_values 输出Y值数组
     * @param max_points 最大点数
     * @return 实际点数
     */
    size_t GetYValues(std::vector<float>& y_values, size_t max_points = 0) const {
        std::vector<T> data;
        size_t n = GetContinuousData(data, max_points);

        y_values.resize(n);
        for (size_t i = 0; i < n; i++) {
            // 如果T是DataPoint，提取value
            if constexpr (std::is_same_v<T, DataPoint>) {
                y_values[i] = data[i].value;
            } else {
                // 否则直接转换为float
                y_values[i] = static_cast<float>(data[i]);
            }
        }

        return n;
    }

    /**
     * @brief 获取时间戳和Y值数组用于ImPlot
     * @param timestamps 输出时间戳数组
     * @param y_values 输出Y值数组
     * @param max_points 最大点数
     * @return 实际点数
     */
    size_t GetXYValues(std::vector<double>& timestamps, std::vector<float>& y_values, size_t max_points = 0) const {
        std::vector<T> data;
        size_t n = GetContinuousData(data, max_points);

        timestamps.resize(n);
        y_values.resize(n);

        for (size_t i = 0; i < n; i++) {
            if constexpr (std::is_same_v<T, DataPoint>) {
                timestamps[i] = data[i].timestamp;
                y_values[i] = data[i].value;
            } else {
                timestamps[i] = static_cast<double>(i);
                y_values[i] = static_cast<float>(data[i]);
            }
        }

        return n;
    }

    /**
     * @brief 获取缓冲区容量
     */
    constexpr size_t GetCapacity() const {
        return Capacity;
    }

private:
    std::vector<T> data_;   // 数据存储
    size_t head_;           // 写入位置（指向下一个写入位置）
    size_t count_;          // 当前数据点数量
};

#endif // CIRCULAR_BUFFER_H
