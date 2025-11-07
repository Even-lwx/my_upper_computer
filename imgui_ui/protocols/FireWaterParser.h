/**
 * @file FireWaterParser.h
 * @brief FireWater协议解析器（标准VOFA+格式）
 * @author AI Assistant
 * @date 2025
 *
 * 标准VOFA+ FireWater协议格式：
 * - 数据：N个float（4字节，IEEE 754小端序）
 * - 帧尾：0x00 0x00 0x80 0x7F（4字节固定标识）
 *
 * 示例（2通道）：
 * 00 00 80 3F 00 00 00 40 00 00 80 7F
 * [float1=1.0] [float2=2.0] [帧尾标识]
 *
 * 说明：
 * - 无帧头，直接发送float数据流
 * - 帧尾0x0000807F是一个特殊的float值（NaN），用于帧同步
 * - 兼容标准VOFA+上位机软件
 */

#ifndef FIREWATER_PARSER_H
#define FIREWATER_PARSER_H

#include "ProtocolParser.h"
#include <cstring>

/**
 * @brief FireWater协议解析器
 */
class FireWaterParser : public ProtocolParser {
public:
    FireWaterParser(size_t channel_count = 4)
        : channel_count_(channel_count)
        , state_(State::READ_DATA)
        , data_index_(0)
        , tail_index_(0)
    {
        data_buffer_.resize(channel_count * 4);  // 每个通道4字节
    }

    ParseResult Parse(const unsigned char* buffer, size_t length) override {
        ParseResult result;
        result.values.reserve(channel_count_);

        size_t consumed = 0;

        for (size_t i = 0; i < length; i++) {
            unsigned char byte = buffer[i];
            consumed++;

            switch (state_) {
                case State::READ_DATA:
                    data_buffer_[data_index_++] = byte;

                    if (data_index_ >= data_buffer_.size()) {
                        // 数据读取完成，开始验证帧尾
                        state_ = State::VERIFY_TAIL;
                        tail_index_ = 0;
                    }
                    break;

                case State::VERIFY_TAIL:
                    if (byte == FRAME_TAIL[tail_index_]) {
                        tail_index_++;

                        if (tail_index_ >= 4) {
                            // 帧尾完全匹配，解析float数据
                            for (size_t ch = 0; ch < channel_count_; ch++) {
                                float value;
                                std::memcpy(&value, &data_buffer_[ch * 4], 4);
                                result.values.push_back(value);
                            }
                            result.success = true;
                            result.bytes_consumed = consumed;

                            // 准备接收下一帧
                            state_ = State::READ_DATA;
                            data_index_ = 0;
                            tail_index_ = 0;
                            return result;
                        }
                    } else {
                        // 帧尾不匹配，执行数据滑动搜索
                        // 将缓冲区数据左移1字节，尝试重新对齐
                        for (size_t j = 0; j < data_buffer_.size() - 1; j++) {
                            data_buffer_[j] = data_buffer_[j + 1];
                        }

                        // 如果已经读取了部分帧尾，也需要处理
                        if (tail_index_ > 0) {
                            // 将已读取的帧尾字节放入数据缓冲区
                            for (size_t j = 0; j < tail_index_; j++) {
                                if (data_buffer_.size() > 0) {
                                    data_buffer_[data_buffer_.size() - tail_index_ + j] = FRAME_TAIL[j];
                                }
                            }
                        }

                        // 将当前字节放入缓冲区末尾
                        data_buffer_[data_buffer_.size() - 1] = byte;

                        // 重新尝试验证帧尾
                        tail_index_ = 0;
                        if (byte == FRAME_TAIL[0]) {
                            tail_index_ = 1;
                        }
                    }
                    break;
            }
        }

        // 数据不完整，等待更多数据
        result.bytes_consumed = consumed;
        return result;
    }

    void Reset() override {
        state_ = State::READ_DATA;
        data_index_ = 0;
        tail_index_ = 0;
    }

    ProtocolType GetType() const override {
        return ProtocolType::FIREWATER;
    }

    std::string GetName() const override {
        return "FireWater";
    }

    size_t GetExpectedChannelCount() const override {
        return channel_count_;
    }

    void SetExpectedChannelCount(size_t count) override {
        if (count > 0 && count <= 16) {
            channel_count_ = count;
            data_buffer_.resize(channel_count_ * 4);
            Reset();
        }
    }

private:
    enum class State {
        READ_DATA,      // 读取float数据
        VERIFY_TAIL     // 验证4字节帧尾
    };

    // 标准VOFA+ FireWater帧尾（4字节）
    static constexpr unsigned char FRAME_TAIL[4] = {0x00, 0x00, 0x80, 0x7F};

    size_t channel_count_;              // 通道数量
    State state_;                       // 当前状态
    std::vector<unsigned char> data_buffer_;  // 数据缓冲区
    size_t data_index_;                 // 当前数据索引
    size_t tail_index_;                 // 帧尾验证索引（0-3）
};

#endif // FIREWATER_PARSER_H
