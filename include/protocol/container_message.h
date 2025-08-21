// include/protocol/container_message.h
#pragma once
#include "container_config.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace nl = nlohmann;

// JSON 通信协议
struct ContainerMessage {
    std::string cmd;                   // "run", "ps", "exec", "stop"
    ContainerConfig config;            // run 命令参数
    std::string container_id;          // exec, stop, logs
    std::vector<std::string> exec_args;// exec 命令参数

    // 序列化为 JSON 字符串
    std::string to_json() const;

    // 从 JSON 字符串解析
    static ContainerMessage from_json(const std::string &json_str);
};