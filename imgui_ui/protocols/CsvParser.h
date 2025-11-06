/**
 * @file CsvParser.h
 * @brief CSV文本协议解析器
 * @author AI Assistant
 * @date 2025
 *
 * CSV协议格式：
 * - 文本格式：逗号分隔的数值
 * - 行尾符：\r\n 或 \n
 * - 支持整数和浮点数混合
 *
 * 示例（9通道）：
 * 89870, -46.73, -8.88, 33.12, 35.50, 0.02, -1.46, -5.71, -41.22\r\n
 * 通道1  通道2   通道3   通道4   通道5   通道6  通道7   通道8   通道9
 */

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include "ProtocolParser.h"
#include <string>
#include <sstream>
#include <vector>

/**
 * @brief CSV文本协议解析器
 */
class CsvParser : public ProtocolParser {
public:
    CsvParser(size_t channel_count = 9)  // 默认9通道（常见场景）
        : channel_count_(channel_count)
    {
    }

    ParseResult Parse(const unsigned char* buffer, size_t length) override {
        ParseResult result;
        result.values.reserve(channel_count_);

        size_t consumed = 0;

        // 逐字节处理
        for (size_t i = 0; i < length; i++) {
            unsigned char byte = buffer[i];
            consumed++;

            // 累积字节到行缓冲区
            if (byte == '\n') {
                // 检测到换行符，解析当前行
                if (!line_buffer_.empty()) {
                    // 移除可能的 \r（如果是 \r\n）
                    if (line_buffer_.back() == '\r') {
                        line_buffer_.pop_back();
                    }

                    // 解析CSV行
                    std::vector<float> values = ParseCsvLine(line_buffer_);

                    if (!values.empty()) {
                        result.values = values;
                        result.success = true;
                        result.bytes_consumed = consumed;

                        // 清空行缓冲区
                        line_buffer_.clear();

                        return result;
                    }

                    // 解析失败，清空缓冲区继续
                    line_buffer_.clear();
                }
            } else {
                // 累积字符到行缓冲区
                line_buffer_ += static_cast<char>(byte);

                // 防止缓冲区溢出（最大4KB）
                if (line_buffer_.size() > 4096) {
                    line_buffer_.clear();
                }
            }
        }

        // 数据不完整，等待更多数据
        result.bytes_consumed = consumed;
        return result;
    }

    void Reset() override {
        line_buffer_.clear();
    }

    ProtocolType GetType() const override {
        return ProtocolType::CSV;
    }

    std::string GetName() const override {
        return "CSV";
    }

    size_t GetExpectedChannelCount() const override {
        return channel_count_;
    }

    void SetExpectedChannelCount(size_t count) override {
        if (count > 0 && count <= 16) {
            channel_count_ = count;
            Reset();
        }
    }

private:
    /**
     * @brief 解析CSV行为浮点数数组
     * @param line CSV行字符串（例如："1.23, 4.56, -7.89"）
     * @return 浮点数数组
     */
    std::vector<float> ParseCsvLine(const std::string& line) {
        std::vector<float> values;
        std::stringstream ss(line);
        std::string token;

        // 按逗号分割
        while (std::getline(ss, token, ',')) {
            // 尝试转换为浮点数
            try {
                // 移除前后空格
                token = Trim(token);

                if (!token.empty()) {
                    float value = std::stof(token);
                    values.push_back(value);
                }
            } catch (...) {
                // 转换失败，跳过此字段
                continue;
            }
        }

        return values;
    }

    /**
     * @brief 去除字符串前后空格
     * @param str 输入字符串
     * @return 去除空格后的字符串
     */
    std::string Trim(const std::string& str) {
        size_t start = 0;
        size_t end = str.size();

        // 找到第一个非空格字符
        while (start < end && std::isspace(static_cast<unsigned char>(str[start]))) {
            start++;
        }

        // 找到最后一个非空格字符
        while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
            end--;
        }

        return str.substr(start, end - start);
    }

    size_t channel_count_;      // 期望通道数
    std::string line_buffer_;   // 行缓冲区
};

#endif // CSV_PARSER_H
