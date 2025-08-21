#include "overlayfs_strategy.h"
#include <cstring>
#include <sstream>
#include <sys/mount.h>
#include <sys/stat.h>
#include <system_error>
#include <iostream>
#include <cstdlib>
#include <unistd.h>

namespace {
    void create_dir(const std::string &path) {
        if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
            std::cerr << "创建目录失败: " << path << ", 错误: " << strerror(errno) << std::endl;
            throw std::system_error(std::error_code(errno, std::generic_category()),
                                    "mkdir failed for " + path);
        }
    }

    bool is_mounted(const std::string &mount_point) {
        std::string cmd = "mount | grep ' " + mount_point + " '";
        return system(cmd.c_str()) == 0;
    }

    void ensure_directory_writable(const std::string &path) {
        if (access(path.c_str(), W_OK) != 0) {
            std::cerr << "目录不可写: " << path << ", 权限: " << strerror(errno) << std::endl;
            throw std::runtime_error("directory not writable: " + path);
        }
    }
}// namespace

OverlayfsStrategy::OverlayfsStrategy(std::string workdir)
    : workdir_(std::move(workdir)) {
    std::cout << "初始化OverlayFS工作目录: " << workdir_ << std::endl;
    create_dir(workdir_);
}

void OverlayfsStrategy::setup(const std::string &lowerdir,
                              const std::string &upperdir,
                              const std::string &merged) {
    // 1. 验证输入目录
    if (access(lowerdir.c_str(), F_OK) != 0) {
        throw std::invalid_argument("lowerdir不存在: " + lowerdir);
    }
    if (access(upperdir.c_str(), F_OK) != 0) {
        throw std::invalid_argument("upperdir不存在: " + upperdir);
    }

    // 2. 创建 merged 目录
    create_dir(merged);
    std::cout << "准备挂载OverlayFS到: " << merged << std::endl;

    // 3. 检查是否已挂载
    if (is_mounted(merged)) {
        //std::cout << "警告: " << merged << " 已挂载，先卸载" << std::endl;
        umount2(merged.c_str(), MNT_DETACH);
    }

    // 4. 构造挂载参数（必须 rw）
    std::ostringstream opts;
    opts << "lowerdir=" << lowerdir
         << ",upperdir=" << upperdir
         << ",workdir=" << workdir_
         << ",rw";
    std::string opts_str = opts.str();
    std::cout << "OverlayFS挂载参数: " << opts_str << std::endl;

    // 5. 执行挂载
    if (mount("overlay", merged.c_str(), "overlay", 0, opts_str.c_str()) != 0) {
        std::cerr << "OverlayFS挂载失败! 路径: " << merged
                  << ", 错误: " << strerror(errno) << std::endl;
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "mount overlay failed with opts: " + opts_str);
    }

    // 6. 验证挂载是否被内核记录
    std::string verify_cmd = "mount | grep 'overlay on " + merged + "'";
    int verify_ret = system(verify_cmd.c_str());
    if (verify_ret != 0) {
        std::cerr << "严重错误: 内核未记录OverlayFS挂载! 路径: " << merged << std::endl;
        umount2(merged.c_str(), MNT_DETACH);
        throw std::runtime_error("OverlayFS mount not recognized by kernel");
    }

    // 7. 确保 merged 目录可写
    ensure_directory_writable(merged);

    std::cout << "OverlayFS挂载成功: " << merged << std::endl;
}

void OverlayfsStrategy::teardown(const std::string &merged) {
    std::cout << "卸载OverlayFS: " << merged << std::endl;
    if (umount2(merged.c_str(), MNT_DETACH) != 0) {
        std::cerr << "卸载OverlayFS失败: " << merged << ", 错误: " << strerror(errno) << std::endl;
    }
}