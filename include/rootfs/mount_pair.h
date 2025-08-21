#pragma once
#include <nlohmann/json.hpp>
#include <string>

struct MountPair {
    std::string host_path;
    std::string container_path;
};

// 为MountPair添加JSON序列化支持
void to_json(nlohmann::json &j, const MountPair &m);
void from_json(const nlohmann::json &j, MountPair &m);