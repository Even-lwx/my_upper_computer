/**
 * @file FireWaterParser.h
 * @brief FireWater协议解析器
 * @author AI Assistant
 * @date 2025
 *
 * FireWater协议格式：
 * - 帧头：0xAA
 * - 数据：N个float（4字节，小端序）
 * - 帧尾：0x7F
 *
 * 示例（2通道）：
 * AA 00 00 80 3F 00 00 00 40 7F
 *  ^  [float1=1.0]  [float2=2.0]  ^
 * 帧头                             帧尾
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
        , state_(State::SEARCH_HEADER)
        , data_index_(0)
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
                case State::SEARCH_HEADER:
                    if (byte == FRAME_HEADER) {
                        state_ = State::READ_DATA;
                        data_index_ = 0;
                    }
                    break;

                case State::READ_DATA:
                    data_buffer_[data_index_++] = byte;

                    if (data_index_ >= data_buffer_.size()) {
                        // 数据读取完成，等待帧尾
                        state_ = State::VERIFY_TAIL;
                    }
                    break;

                case State::VERIFY_TAIL:
                    if (byte == FRAME_TAIL) {
                        // 帧完整，解析float数据
                        for (size_t ch = 0; ch < channel_count_; ch++) {
                            float value;
                            std::memcpy(&value, &data_buffer_[ch * 4], 4);
                            result.values.push_back(value);
                        }
                        result.success = true;
                        result.bytes_consumed = consumed;
                        state_ = State::SEARCH_HEADER;  // 准备接收下一帧
                        return result;
                    } else {
                        // 帧尾错误，重新搜索帧头
                        result.error_message = "Frame tail mismatch";
                        state_ = State::SEARCH_HEADER;
                        // 如果当前字节是帧头，开始新帧
                        if (byte == FRAME_HEADER) {
                            state_ = State::READ_DATA;
                            data_index_ = 0;
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
        state_ = State::SEARCH_HEADER;
        data_index_ = 0;
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
        SEARCH_HEADER,  // 搜索帧头0xAA
        READ_DATA,      // 读取float数据
        VERIFY_TAIL     // 验证帧尾0x7F
    };

    static constexpr unsigned char FRAME_HEADER = 0xAA;
    static constexpr unsigned char FRAME_TAIL = 0x7F;

    size_t channel_count_;              // 通道数量
    State state_;                       // 当前状态
    std::vector<unsigned char> data_buffer_;  // 数据缓冲区
    size_t data_index_;                 // 当前数据索引
};

#endif // FIREWATER_PARSER_H
