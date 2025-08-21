// include/utils/env_loader.h
#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
class EnvLoader {
    std::unordered_map<std::string, std::string> values_;

public:
    static EnvLoader &instance();

    void load(const std::string &path = ".env");
    std::string get(const std::string &key,
                    const std::string &default_value = "") const;
    uint64_t get_memory(const std::string &key, uint64_t default_value = 0) const;
};