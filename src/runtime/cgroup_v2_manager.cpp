// src/runtime/cgroup_v2_manager.cpp
#include "cgroup_v2_manager.h"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace {

    // 安全写文件，失败抛异常
    void write_file(const std::string &path, const std::string &value) {
        std::ofstream file(path);
        if (!file) {
            int err = errno;
            throw std::system_error(std::error_code(err, std::generic_category()),
                                    "open failed: " + path);
        }
        file << value;
        if (!file) {
            int err = errno;
            throw std::system_error(std::error_code(err, std::generic_category()),
                                    "write failed: " + path);
        }
    }

    // 创建目录
    void create_directory(const std::string &path) {
        if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
            int err = errno;
            throw std::system_error(std::error_code(err, std::generic_category()),
                                    "mkdir failed: " + path);
        }
    }

    // 删除目录（仅当为空时）
    void remove_directory(const std::string &path) noexcept {
        if (rmdir(path.c_str()) != 0) {
            // 忽略 ENOENT 和 ENOTEMPTY
            if (errno != ENOENT && errno != ENOTEMPTY) {
                // 可选：记录日志
            }
        }
    }
}// namespace

CgroupV2Manager::CgroupV2Manager(std::string path) : path_(std::move(path)) {
    // 让程序自己创建目录
    if (mkdir(path_.c_str(), 0755) != 0) {
        if (errno != EEXIST) {
            std::cerr << "mkdir " << path_ << " failed: " << strerror(errno) << "\n";
            throw std::system_error(std::error_code(errno, std::generic_category()),
                                    "mkdir failed");
        }
    }

    // 确保能写 memory.max
    std::string max_path = path_ + "/memory.max";
    std::ofstream file(max_path);
    if (!file) {
        std::cerr << "Cannot write to " << max_path << " (errno=" << errno << ")\n";
        std::cerr << "Try: echo 100000000 | sudo tee " << max_path << "\n";
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "write memory.max failed");
    }
}

CgroupV2Manager::~CgroupV2Manager() {
    // 尝试清理：先移出所有进程
    try {
        write_file(path_ + "/cgroup.procs", "");
        remove_directory(path_);
    } catch (...) {
        // 析构中不抛异常
    }
}

void CgroupV2Manager::set_memory_limit(uint64_t bytes) {
    if (bytes == 0)
        return;

    std::string path = path_ + "/memory.max";
    std::ofstream file(path);
    if (!file) {
        std::cerr << "ERROR: cannot open " << path << " (errno=" << errno << ")\n";
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "open failed");
    }

    file << bytes;
    if (!file) {
        std::cerr << "ERROR: write failed to " << path << " (errno=" << errno
                  << ")\n";
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "write failed");
    }
}

void CgroupV2Manager::set_cpu_limit(double shares) {
    if (shares <= 0.0)
        return;

    // cgroups v2 使用 cpu.max: "quota period"
    // 默认 period = 100000 us (100ms)
    // quota = shares * 100000
    uint64_t period = 100000;
    uint64_t quota = static_cast<uint64_t>(shares * period);

    std::ostringstream oss;
    if (shares >= 1.0) {
        // 不限（最大）
        oss << "max " << period;
    } else {
        oss << quota << " " << period;
    }

    write_file(path_ + "/cpu.max", oss.str());
}

void CgroupV2Manager::set_memory_high(uint64_t bytes) {
    if (bytes == 0)
        return;
    write_file(path_ + "/memory.high", std::to_string(bytes));
}

void CgroupV2Manager::add_process(pid_t pid) {
    write_file(path_ + "/cgroup.procs", std::to_string(pid));
}

const std::string &CgroupV2Manager::cgroup_path() const noexcept {
    return path_;
}