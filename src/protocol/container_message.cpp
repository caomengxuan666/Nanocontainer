// src/protocol/container_message.cpp
#include "container_message.h"
#include <iostream>

std::string ContainerMessage::to_json() const {
    nl::json j;
    j["cmd"] = cmd;

    if (cmd == "run") {
        nl::json config_json;
        config_json["id"] = config.id;
        config_json["name"] = config.name;
        config_json["image"] = config.image;
        config_json["cmd"] = config.cmd;
        config_json["memory_limit"] = config.memory_limit;
        config_json["cpu_shares"] = config.cpu_shares;
        config_json["network_mode"] = config.network_mode;
        config_json["mounts"] = config.mounts;
        config_json["log_path"] = config.log_path;
        j["config"] = config_json;
    } else if (cmd == "exec" || cmd == "stop") {
        j["container_id"] = container_id;
        if (cmd == "exec") {
            j["exec_args"] = exec_args;
        }
    }

    return j.dump();
}

ContainerMessage ContainerMessage::from_json(const std::string &json_str) {
    ContainerMessage msg;
    try {
        nl::json j = nl::json::parse(json_str);

        msg.cmd = j.at("cmd").get<std::string>();

        if (msg.cmd == "run" && j.contains("config")) {
            auto c = j["config"];
            msg.config.id = c.value("id", "");
            msg.config.name = c.value("name", "");
            msg.config.image = c.value("image", "");
            msg.config.cmd = c.value("cmd", std::vector<std::string>{});
            msg.config.memory_limit = c.value("memory_limit", 0);
            msg.config.cpu_shares = c.value("cpu_shares", 0.0);
            msg.config.network_mode = c.value("network_mode", "none");
            msg.config.mounts = c.value("mounts", std::vector<MountPair>{});
            msg.config.log_path = c.value("log_path", "");
        } else if (msg.cmd == "exec" || msg.cmd == "stop") {
            msg.container_id = j.at("container_id").get<std::string>();
            if (msg.cmd == "exec") {
                msg.exec_args = j.value("exec_args", std::vector<std::string>{});
            }
        }

    } catch (const std::exception &e) {
        std::cerr << "JSON parse error: " << e.what() << "\n";
        throw;
    }
    return msg;
}