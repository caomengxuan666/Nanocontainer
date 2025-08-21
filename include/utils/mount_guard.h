#pragma once
#include <string>

class MountGuard {
    std::string mount_point_;

public:
    explicit MountGuard(std::string mount_point);
    ~MountGuard();

    MountGuard(const MountGuard &) = delete;
    MountGuard &operator=(const MountGuard &) = delete;

    MountGuard(MountGuard &&other) noexcept;
    MountGuard &operator=(MountGuard &&) noexcept;
};