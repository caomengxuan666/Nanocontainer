// include/network/bridge_network_strategy.h
#pragma once
#include "network_strategy.h"
#include <string>

// --network=bridge：使用 veth + bridge
class BridgeNetworkStrategy : public NetworkStrategy {
    std::string bridge_name_ = "nanobridge";
    std::string host_if_ = "veth-host";
    std::string container_if_ = "eth0";
    std::string bridge_ip_ = "172.18.0.1";
    std::string netmask_ = "16";
    std::string container_ip_;

public:
    BridgeNetworkStrategy();

    void setup_in_container() override;
    void setup_on_host(pid_t container_pid) override;
    void teardown() override;

    // 获取容器 IP（基于 PID）
    std::string get_container_ip(pid_t pid) const;
};