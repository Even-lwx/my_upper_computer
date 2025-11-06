/**
 * @file ConfigManager.h
 * @brief 配置管理器 - 实现配置的持久化存储
 * @author AI Assistant
 * @date 2025
 *
 * 功能：
 * - 将AppState配置保存为JSON文件
 * - 从JSON文件加载配置到AppState
 * - 支持增量更新和默认值回退
 */

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

// 前向声明
struct AppState;
class VisualizationUI;

using json = nlohmann::json;

/**
 * @brief 配置管理器类（静态工具类）
 */
class ConfigManager {
public:
    /**
     * @brief 保存配置到JSON文件
     * @param state 应用程序状态
     * @return 成功返回true，失败返回false
     */
    static bool SaveConfig(const AppState& state);

    /**
     * @brief 从JSON文件加载配置
     * @param state 应用程序状态（输出参数）
     * @return 成功返回true，失败返回false
     */
    static bool LoadConfig(AppState& state);

    /**
     * @brief 获取配置文件路径
     * @return 配置文件完整路径
     */
    static std::string GetConfigPath();

private:
    /**
     * @brief 将AppState序列化为JSON对象
     */
    static json SerializeState(const AppState& state);

    /**
     * @brief 从JSON对象反序列化到AppState
     */
    static void DeserializeState(AppState& state, const json& j);

    // ===== 分模块序列化方法 =====

    /**
     * @brief 序列化串口配置
     */
    static json SerializeSerial(const AppState& state);

    /**
     * @brief 序列化UI设置
     */
    static json SerializeUI(const AppState& state);

    /**
     * @brief 序列化可视化配置
     */
    static json SerializeVisualization(const AppState& state);

    /**
     * @brief 序列化线程配置
     */
    static json SerializeThreading(const AppState& state);

    // ===== 分模块反序列化方法 =====

    /**
     * @brief 反序列化串口配置
     */
    static void DeserializeSerial(AppState& state, const json& j);

    /**
     * @brief 反序列化UI设置
     */
    static void DeserializeUI(AppState& state, const json& j);

    /**
     * @brief 反序列化可视化配置
     */
    static void DeserializeVisualization(AppState& state, const json& j);

    /**
     * @brief 反序列化线程配置
     */
    static void DeserializeThreading(AppState& state, const json& j);

    // ===== 工具方法 =====

    /**
     * @brief 安全获取JSON值（带默认值）
     */
    template<typename T>
    static T SafeGet(const json& j, const std::string& key, const T& default_value) {
        if (j.contains(key) && !j[key].is_null()) {
            try {
                return j[key].get<T>();
            } catch (...) {
                return default_value;
            }
        }
        return default_value;
    }
};

#endif // CONFIG_MANAGER_H
