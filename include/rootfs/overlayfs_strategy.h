#pragma once
#include "rootfs_manager.h"
#include <string>

class OverlayfsStrategy : public RootfsManager {
    std::string workdir_;

public:
    explicit OverlayfsStrategy(std::string workdir);
    void setup(const std::string &lowerdir, const std::string &upperdir,
               const std::string &merged) override;
    void teardown(const std::string &merged) override;

    OverlayfsStrategy(const OverlayfsStrategy &) = delete;
    OverlayfsStrategy &operator=(const OverlayfsStrategy &) = delete;
    OverlayfsStrategy(OverlayfsStrategy &&) noexcept = default;
    OverlayfsStrategy &operator=(OverlayfsStrategy &&) noexcept = default;
};