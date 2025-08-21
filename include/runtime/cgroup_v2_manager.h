#pragma once
#include <cstdint>
#include <string>

// 管理 cgroups v2 子系统，自动创建/销毁 cgroup
class CgroupV2Manager {
    std::string path_;// cgroup 目录路径，如 /sys/fs/cgroup/mydocker/test1

public:
    // 构造：传入 cgroup 路径（通常为 /sys/fs/cgroup/<name>）
    explicit CgroupV2Manager(std::string path);

    // 禁止拷贝
    CgroupV2Manager(const CgroupV2Manager &) = delete;
    CgroupV2Manager &operator=(const CgroupV2Manager &) = delete;

    // 允许移动
    CgroupV2Manager(CgroupV2Manager &&) noexcept = default;
    CgroupV2Manager &operator=(CgroupV2Manager &&) noexcept = default;

    // 析构：自动清理 cgroup（如果为空）
    ~CgroupV2Manager();

    // 设置内存限制（字节）
    void set_memory_limit(uint64_t bytes);

    // 设置 CPU 限制（如 0.5 表示 50% 一个核心）
    void set_cpu_limit(double shares);

    void set_memory_high(uint64_t bytes);

    // 将进程加入 cgroup
    void add_process(pid_t pid);

    // 获取 cgroup 路径
    [[nodiscard]] const std::string &cgroup_path() const noexcept;
};