// src/utils/env_loader.cpp
#include "env_loader.h"
#include <cstdlib>
#include <cstring>
#include <fstream>

EnvLoader &EnvLoader::instance() {
    static EnvLoader instance;
    return instance;
}

void EnvLoader::load(const std::string &path) {
    std::ifstream file(path);
    if (!file)
        return;

    std::string line;
    while (std::getline(file, line)) {
        auto comment = line.find('#');
        if (comment != std::string::npos) {
            line = line.substr(0, comment);
        }
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            values_[key] = value;
        }
    }
}

std::string EnvLoader::get(const std::string &key,
                           const std::string &default_value) const {
    auto it = values_.find(key);
    return it != values_.end() ? it->second : default_value;
}

uint64_t EnvLoader::get_memory(const std::string &key,
                               uint64_t default_value) const {
    std::string val = get(key);
    if (val.empty())
        return default_value;
    char *end;
    double n = std::strtod(val.c_str(), &end);
    if (n <= 0)
        return default_value;
    if (std::strcmp(end, "G") == 0)
        return n * 1024 * 1024 * 1024;
    if (std::strcmp(end, "M") == 0)
        return n * 1024 * 1024;
    return n;
}