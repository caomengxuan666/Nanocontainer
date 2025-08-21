// src/network/none_network_strategy.cpp
#include "none_network_strategy.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sys/mount.h>
#include <system_error>
void NoneNetworkStrategy::setup_in_container() {
    // 1. 挂载 /sys（如果还没挂）
    std::cout << "1111111" << std::endl;
    if (mount("sysfs", "/sys", "sysfs", 0, nullptr) != 0) {
        std::cerr << "WARNING: mount sysfs failed\n";
    }

    // 2. 启用 loopback 接口
    if (system("ip link set lo up") != 0) {
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "bring up lo failed");
    }

    // 3. 可选：写入 resolv.conf
    std::ofstream resolv("/etc/resolv.conf");
    if (resolv) {
        resolv << "nameserver 8.8.8.8\n";
        resolv.close();
    }
}