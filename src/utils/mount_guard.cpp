#include "mount_guard.h"
#include <sys/mount.h>

MountGuard::MountGuard(std::string mount_point)
    : mount_point_(std::move(mount_point)) {}
MountGuard::~MountGuard() {
    if (!mount_point_.empty())
        umount2(mount_point_.c_str(), MNT_DETACH);
}

MountGuard::MountGuard(MountGuard &&other) noexcept
    : mount_point_(std::move(other.mount_point_)) {
    other.mount_point_.clear();
}

MountGuard &MountGuard::operator=(MountGuard &&rhs) noexcept {
    if (this != &rhs) {
        if (!mount_point_.empty())
            umount2(mount_point_.c_str(), MNT_DETACH);
        mount_point_ = std::move(rhs.mount_point_);
        rhs.mount_point_.clear();
    }
    return *this;
}