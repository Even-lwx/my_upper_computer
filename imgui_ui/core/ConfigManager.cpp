/**
 * @file ConfigManager.cpp
 * @brief 配置管理器实现
 * @author AI Assistant
 * @date 2025
 */

#include "ConfigManager.h"
#include "AppState.h"
#include "../ui/VisualizationUI.h"
#include "../core/DataChannelManager.h"
#include <fstream>
#include <iostream>

/**
 * @brief 获取配置文件路径（与可执行文件同目录）
 */
std::string ConfigManager::GetConfigPath() {
    return "app_config.json";
}

/**
 * @brief 保存配置到JSON文件
 */
bool ConfigManager::SaveConfig(const AppState& state) {
    try {
        json j = SerializeState(state);

        std::ofstream file(GetConfigPath());
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << GetConfigPath() << std::endl;
            return false;
        }

        file << j.dump(4);  // 4空格缩进，便于阅读
        file.close();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "SaveConfig exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 从JSON文件加载配置
 */
bool ConfigManager::LoadConfig(AppState& state) {
    try {
        std::ifstream file(GetConfigPath());
        if (!file.is_open()) {
            // 配置文件不存在是正常情况（首次运行）
            return false;
        }

        json j;
        file >> j;
        file.close();

        DeserializeState(state, j);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "LoadConfig exception: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 序列化完整的AppState
 */
json ConfigManager::SerializeState(const AppState& state) {
    json j;

    // 版本信息
    j["version"] = "1.0.0";

    // 分模块序列化
    j["serial"] = SerializeSerial(state);
    j["ui"] = SerializeUI(state);
    j["visualization"] = SerializeVisualization(state);
    j["threading"] = SerializeThreading(state);

    return j;
}

/**
 * @brief 反序列化到AppState
 */
void ConfigManager::DeserializeState(AppState& state, const json& j) {
    try {
        // 检查版本兼容性
        std::string version = SafeGet<std::string>(j, "version", "1.0.0");

        // 分模块反序列化
        if (j.contains("serial")) {
            DeserializeSerial(state, j["serial"]);
        }
        if (j.contains("ui")) {
            DeserializeUI(state, j["ui"]);
        }
        if (j.contains("visualization")) {
            DeserializeVisualization(state, j["visualization"]);
        }
        if (j.contains("threading")) {
            DeserializeThreading(state, j["threading"]);
        }
    } catch (const std::exception& e) {
        std::cerr << "DeserializeState exception: " << e.what() << std::endl;
    }
}

// ========================================
// 串口配置序列化
// ========================================

json ConfigManager::SerializeSerial(const AppState& state) {
    json j;

    j["selected_port_index"] = state.selected_port_index;
    j["selected_baudrate_index"] = state.selected_baudrate_index;
    j["selected_databits_index"] = state.selected_databits_index;
    j["selected_stopbits_index"] = state.selected_stopbits_index;
    j["selected_parity_index"] = state.selected_parity_index;

    return j;
}

void ConfigManager::DeserializeSerial(AppState& state, const json& j) {
    state.selected_port_index = SafeGet<int>(j, "selected_port_index", 0);
    state.selected_baudrate_index = SafeGet<int>(j, "selected_baudrate_index", 7);
    state.selected_databits_index = SafeGet<int>(j, "selected_databits_index", 3);
    state.selected_stopbits_index = SafeGet<int>(j, "selected_stopbits_index", 0);
    state.selected_parity_index = SafeGet<int>(j, "selected_parity_index", 0);
}

// ========================================
// UI设置序列化
// ========================================

json ConfigManager::SerializeUI(const AppState& state) {
    json j;

    // 当前视图
    j["current_view"] = static_cast<int>(state.current_view);

    // 显示选项
    j["hex_display"] = state.hex_display;
    j["hex_send"] = state.hex_send;
    j["auto_scroll"] = state.auto_scroll;

    // 编码和时间戳设置
    j["encoding_type"] = static_cast<int>(state.encoding_type);
    j["show_timestamp"] = state.show_timestamp;

    // 日志设置
    j["enable_logging"] = state.enable_logging;
    j["log_filename"] = state.log_filename;

    // 定时发送
    j["enable_auto_send"] = state.enable_auto_send;
    j["auto_send_interval_ms"] = state.auto_send_interval_ms;
    j["send_buffer"] = std::string(state.send_buffer);

    // 发送后缀配置
    j["send_line_ending"] = static_cast<int>(state.send_line_ending);
    j["enable_custom_prefix"] = state.enable_custom_prefix;
    j["enable_custom_suffix"] = state.enable_custom_suffix;
    j["custom_prefix"] = std::string(state.custom_prefix);
    j["custom_suffix"] = std::string(state.custom_suffix);

    // 发送历史
    j["send_history"] = state.send_history;

    return j;
}

void ConfigManager::DeserializeUI(AppState& state, const json& j) {
    // 当前视图
    int view_type = SafeGet<int>(j, "current_view", 0);
    state.current_view = static_cast<ViewType>(view_type);

    // 显示选项
    state.hex_display = SafeGet<bool>(j, "hex_display", false);
    state.hex_send = SafeGet<bool>(j, "hex_send", false);
    state.auto_scroll = SafeGet<bool>(j, "auto_scroll", true);

    // 编码和时间戳设置
    int encoding_type = SafeGet<int>(j, "encoding_type", 0);
    state.encoding_type = static_cast<EncodingType>(encoding_type);
    state.show_timestamp = SafeGet<bool>(j, "show_timestamp", true);

    // 日志设置
    state.enable_logging = SafeGet<bool>(j, "enable_logging", false);
    state.log_filename = SafeGet<std::string>(j, "log_filename", "");

    // 定时发送
    state.enable_auto_send = SafeGet<bool>(j, "enable_auto_send", false);
    state.auto_send_interval_ms = SafeGet<int>(j, "auto_send_interval_ms", 1000);

    // 发送缓冲区
    std::string send_buffer_str = SafeGet<std::string>(j, "send_buffer", "");
    if (!send_buffer_str.empty() && send_buffer_str.length() < sizeof(state.send_buffer)) {
        strcpy_s(state.send_buffer, sizeof(state.send_buffer), send_buffer_str.c_str());
    }

    // 发送后缀配置
    int line_ending = SafeGet<int>(j, "send_line_ending", 0);
    state.send_line_ending = static_cast<LineEnding>(line_ending);
    state.enable_custom_prefix = SafeGet<bool>(j, "enable_custom_prefix", false);
    state.enable_custom_suffix = SafeGet<bool>(j, "enable_custom_suffix", false);

    std::string custom_prefix_str = SafeGet<std::string>(j, "custom_prefix", "");
    if (custom_prefix_str.length() < sizeof(state.custom_prefix)) {
        strcpy_s(state.custom_prefix, sizeof(state.custom_prefix), custom_prefix_str.c_str());
    }

    std::string custom_suffix_str = SafeGet<std::string>(j, "custom_suffix", "");
    if (custom_suffix_str.length() < sizeof(state.custom_suffix)) {
        strcpy_s(state.custom_suffix, sizeof(state.custom_suffix), custom_suffix_str.c_str());
    }

    // 发送历史
    if (j.contains("send_history") && j["send_history"].is_array()) {
        state.send_history.clear();
        for (const auto& item : j["send_history"]) {
            if (item.is_string()) {
                std::string history_item = item.get<std::string>();
                if (!history_item.empty()) {
                    state.send_history.push_back(history_item);
                }
            }
        }
    }
}

// ========================================
// 可视化配置序列化
// ========================================

json ConfigManager::SerializeVisualization(const AppState& state) {
    json j;

    // 协议类型
    j["protocol_type"] = static_cast<int>(state.visualization_ui.GetProtocolParser()->GetType());

    // 通道配置
    json channels = json::array();
    DataChannelManager& channel_mgr = const_cast<VisualizationUI&>(state.visualization_ui).GetChannelManager();

    for (size_t i = 0; i < DataChannelManager::MAX_CHANNELS; i++) {
        ChannelConfig config = channel_mgr.GetChannelConfig(i);

        json channel;
        channel["enabled"] = config.enabled;
        channel["name"] = config.name;
        channel["color"] = json::array({config.color[0], config.color[1], config.color[2], config.color[3]});

        channels.push_back(channel);
    }

    j["channels"] = channels;

    return j;
}

void ConfigManager::DeserializeVisualization(AppState& state, const json& j) {
    // 协议类型
    int protocol_type = SafeGet<int>(j, "protocol_type", 0);
    state.visualization_ui.SetProtocolType(static_cast<ProtocolType>(protocol_type));

    // 通道配置
    if (j.contains("channels") && j["channels"].is_array()) {
        DataChannelManager& channel_mgr = state.visualization_ui.GetChannelManager();
        json channels = j["channels"];

        for (size_t i = 0; i < channels.size() && i < DataChannelManager::MAX_CHANNELS; i++) {
            json channel = channels[i];

            // 获取当前配置
            ChannelConfig config = channel_mgr.GetChannelConfig(i);

            // 启用状态
            config.enabled = SafeGet<bool>(channel, "enabled", false);

            // 通道名称
            std::string name = SafeGet<std::string>(channel, "name", "");
            if (!name.empty()) {
                config.name = name;
            }

            // 通道颜色
            if (channel.contains("color") && channel["color"].is_array() && channel["color"].size() == 4) {
                config.color[0] = channel["color"][0].get<float>();
                config.color[1] = channel["color"][1].get<float>();
                config.color[2] = channel["color"][2].get<float>();
                config.color[3] = channel["color"][3].get<float>();
            }

            // 应用配置
            channel_mgr.SetChannelConfig(i, config);
        }
    }
}

// ========================================
// 线程配置序列化
// ========================================

json ConfigManager::SerializeThreading(const AppState& state) {
    json j;

    j["enable_multithreading"] = state.thread_config.enable_multithreading;
    j["num_worker_threads"] = state.thread_config.num_worker_threads;

    return j;
}

void ConfigManager::DeserializeThreading(AppState& state, const json& j) {
    state.thread_config.enable_multithreading = SafeGet<bool>(j, "enable_multithreading", true);
    state.thread_config.num_worker_threads = SafeGet<int>(j, "num_worker_threads", 2);

    // 限制线程数范围
    if (state.thread_config.num_worker_threads < 1) {
        state.thread_config.num_worker_threads = 1;
    }
    if (state.thread_config.num_worker_threads > 8) {
        state.thread_config.num_worker_threads = 8;
    }
}
