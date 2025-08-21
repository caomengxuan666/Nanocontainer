// include/network/network_strategy.h
#pragma once
#include <string>

// 网络策略抽象接口
class NetworkStrategy {
public:
    virtual ~NetworkStrategy() = default;

    // 在容器内设置网络（子进程调用）
    virtual void setup_in_container() = 0;

    // 在宿主机上设置网络（父进程调用）
    virtual void setup_on_host(pid_t container_pid) {
        // 默认无需操作
    }

    // 清理网络资源
    virtual void teardown() {
        // 默认无需操作
    }
};