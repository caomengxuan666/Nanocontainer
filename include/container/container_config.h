#pragma once
#include "mount_pair.h"
#include <cstdint>
#include <string>
#include <vector>

struct ContainerConfig {
    std::string id;
    std::string name;
    std::string image;
    std::vector<std::string> cmd;
    uint64_t memory_limit = 1024 * 1024 * 1024;
    double cpu_shares = 0.0;
    std::string log_path;
    std::string rootfs_path;
    std::vector<MountPair> mounts;
    std::string network_mode = "bridge";
    void validate() const;
};