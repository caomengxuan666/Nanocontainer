#pragma once
#include "container_config.h"
#include "network/network_strategy.h"
#include "rootfs/rootfs_manager.h"
#include "utils/mount_guard.h"
#include <memory>

class CgroupV2Manager;// forward

class ContainerImpl {
public:
    ContainerConfig config;
    std::unique_ptr<RootfsManager> rootfs_manager;
    std::unique_ptr<CgroupV2Manager> cgroup_manager;
    std::vector<std::unique_ptr<MountGuard>> mounts;

    pid_t pid = -1;
    std::string state;

    explicit ContainerImpl(ContainerConfig config);
    void setup_rootfs();
    void setup_cgroups();
    void launch();
    void stop();
    void cleanup();
    void setup_network();

private:
    std::unique_ptr<NetworkStrategy> network_strategy;
};