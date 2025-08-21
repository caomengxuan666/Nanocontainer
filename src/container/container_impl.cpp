#include "container_impl.h"
#include "container.h"
#include "mount_guard.h"
#include "network/bridge_network_strategy.h"
#include "network/none_network_strategy.h"
#include "overlayfs_strategy.h"
#include "runtime/cgroup_v2_manager.h"
#include <csignal>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
namespace fs = std::filesystem;

void ContainerImpl::setup_rootfs() {
    auto lowerdir = config.image;
    auto upperdir =
            (Container::data_dir() / "containers" / config.id / "diff").string();
    auto workdir =
            (Container::data_dir() / "containers" / config.id / "work").string();
    auto merged =
            (Container::data_dir() / "containers" / config.id / "merged").string();

    fs::create_directories(upperdir);
    fs::create_directories(workdir);
    fs::create_directories(merged);

    rootfs_manager = std::make_unique<OverlayfsStrategy>(workdir);
    rootfs_manager->setup(lowerdir, upperdir, merged);
    mounts.push_back(std::make_unique<MountGuard>(merged));

    config.rootfs_path = merged;
    // 新增：打印merged目录的实际路径
    std::cout << "merged目录路径: " << merged << std::endl;
}

void ContainerImpl::setup_cgroups() {
    std::string cgroup_path = "/sys/fs/cgroup/nanocontainer/" + config.id;
    cgroup_manager = std::make_unique<CgroupV2Manager>(cgroup_path);

    if (config.memory_limit > 0) {
        cgroup_manager->set_memory_limit(config.memory_limit);
    }
    if (config.cpu_shares > 0) {
        cgroup_manager->set_cpu_limit(config.cpu_shares);
    }
}

void ContainerImpl::launch() {
    if (state != "created") {
        throw std::runtime_error("container already started");
    }

    // 1. 先初始化根文件系统、cgroup和网络（保持原有逻辑）
    setup_rootfs();
    setup_cgroups();
    setup_network();

    // 2. 处理挂载逻辑（核心修改部分）
    for (const auto &m : config.mounts) {
        // 打印当前挂载的基本参数（第一步：确认输入正确）
        std::cout << "\n===== 开始处理挂载 =====" << std::endl;
        std::cout << "宿主机路径: " << m.host_path << std::endl;
        std::cout << "容器内路径: " << m.container_path << std::endl;

        // 计算容器内挂载点的实际路径（rootfs + 容器内目录）
        std::string container_mount_point = config.rootfs_path + m.container_path;
        std::cout << "实际挂载点路径（宿主机视角）: " << container_mount_point << std::endl;

        // 验证宿主机路径是否存在（已有逻辑，保留）
        if (access(m.host_path.c_str(), F_OK) != 0) {
            std::cerr << "错误：宿主机路径不存在！" << m.host_path << std::endl;
            throw std::invalid_argument("Host path not found: " + m.host_path);
        }

        // 3. 创建挂载点目录（确保目录存在，添加详细日志）
        int mkdir_ret = mkdir(container_mount_point.c_str(), 0755);
        if (mkdir_ret != 0) {
            if (errno == EEXIST) {
                std::cout << "挂载点目录已存在（无需创建）: " << container_mount_point << std::endl;
            } else {
                std::cerr << "错误：创建挂载点目录失败！" << std::endl;
                std::cerr << "路径: " << container_mount_point << ", 原因: " << strerror(errno) << std::endl;
                throw std::system_error(std::error_code(errno, std::generic_category()), "mkdir failed");
            }
        } else {
            std::cout << "成功创建挂载点目录: " << container_mount_point << std::endl;
        }

        // 强制验证：挂载点目录必须存在（避免内核忽略无效路径）
        if (access(container_mount_point.c_str(), F_OK) != 0) {
            std::cerr << "严重错误：挂载点目录创建后仍不存在！" << std::endl;
            std::cerr << "路径: " << container_mount_point << "（可能是权限不足或路径无效）" << std::endl;
            throw std::runtime_error("Mount point directory not found after creation");
        }

        // 4. 执行bind mount（核心步骤，添加系统调用级日志）
        std::cout << "开始执行bind mount: " << m.host_path << " -> " << container_mount_point << std::endl;
        int mount_ret = mount(
            m.host_path.c_str(),
            container_mount_point.c_str(),
            nullptr,
            MS_BIND | MS_REC,  // 递归绑定，确保子目录也能挂载
            nullptr
        );

        if (mount_ret != 0) {
            std::cerr << "错误：mount系统调用失败！" << std::endl;
            std::cerr << "参数: " << m.host_path << " -> " << container_mount_point << std::endl;
            std::cerr << "内核错误: " << strerror(errno) << "（错误码: " << errno << "）" << std::endl;
            throw std::system_error(std::error_code(errno, std::generic_category()), 
                                  "bind mount failed: " + m.host_path + " -> " + container_mount_point);
        }

        // 5. 立即验证：检查内核是否记录了该挂载（关键！）
        std::cout << "验证挂载是否生效..." << std::endl;
        std::string verify_cmd = "mount | grep '" + container_mount_point + "'";
        int verify_ret = system(verify_cmd.c_str());
        if (verify_ret != 0) {
            std::cerr << "警告：内核未记录该挂载！可能被OverlayFS覆盖或参数无效。" << std::endl;
            std::cerr << "建议检查：OverlayFS是否为可写模式（需在setup_rootfs中确保rw参数）" << std::endl;
            // 此处不抛异常，继续执行但提示风险
        } else {
            std::cout << "挂载验证成功：内核已记录该bind mount" << std::endl;
        }

        // 6. 记录挂载并打印成功日志
        mounts.push_back(std::make_unique<MountGuard>(container_mount_point));
        std::cout << "===== 挂载完成：" << m.host_path << " -> " << m.container_path << " =====" << std::endl;
    }

    // 7. 后续fork和容器启动逻辑（保持原有，但添加关键日志）
    pid = fork();
    if (pid == 0) {
        try {
            // 加入cgroup
            cgroup_manager->add_process(getpid());
            std::cout << "子进程：已加入cgroup（PID=" << getpid() << "）" << std::endl;

            // 网络隔离
            if (unshare(CLONE_NEWNET) != 0) {
                throw std::system_error(std::error_code(errno, std::generic_category()), "unshare net failed");
            }

            // chroot到rootfs
            if (chdir(config.rootfs_path.c_str()) != 0 || chroot(".") != 0) {
                throw std::system_error(std::error_code(errno, std::generic_category()), "chroot failed");
            }
            std::cout << "子进程：已切换根目录到 " << config.rootfs_path << std::endl;

            // 挂载proc和sys（保持原有，添加详细日志）
            if (mkdir("/proc", 0755) != 0 && errno != EEXIST) {
                std::cerr << "WARNING: mkdir /proc failed: " << strerror(errno) << "\n";
            }
            if (mount("proc", "/proc", "proc", 0, nullptr) != 0) {
                std::cerr << "WARNING: mount proc failed: " << strerror(errno) << "\n";
            } else {
                std::cout << "子进程：/proc挂载成功" << std::endl;
            }

            if (mkdir("/sys", 0755) != 0 && errno != EEXIST) {
                std::cerr << "WARNING: mkdir /sys failed: " << strerror(errno) << "\n";
            }
            if (mount("sysfs", "/sys", "sysfs", 0, nullptr) != 0) {
                std::cerr << "WARNING: mount sysfs failed: " << strerror(errno) << "\n";
            } else {
                std::cout << "子进程：/sys挂载成功" << std::endl;
            }

            // 网络设置
            network_strategy->setup_in_container();
            std::cout << "子进程：网络设置完成" << std::endl;

            // 执行命令
            std::vector<char *> c_args;
            for (auto &arg : config.cmd) {
                c_args.push_back(const_cast<char *>(arg.c_str()));
            }
            c_args.push_back(nullptr);

            setenv("PATH", "/bin:/usr/bin:/sbin:/usr/sbin", 1);
            std::cout << "子进程：准备执行命令: " << config.cmd[0] << std::endl;

            execvp(c_args[0], c_args.data());
            std::cerr << "exec failed: " << strerror(errno) << "\n";
            std::exit(1);

        } catch (const std::exception &e) {
            std::cerr << "子进程初始化失败: " << e.what() << "\n";
            std::exit(1);
        }
    }

    if (pid > 0) {
        // 宿主机网络设置
        if (network_strategy) {
            network_strategy->setup_on_host(pid);
        }

        std::cout << std::flush;
        state = "running";
        std::cout << "容器启动成功：" << config.name << "（PID=" << pid << "）" << std::endl;
    } else {
        throw std::system_error(std::error_code(errno, std::generic_category()), "fork failed");
    }
}

void ContainerImpl::stop() {
    if (state != "running")
        return;

    if (kill(pid, SIGTERM) == 0) {
        int status;
        waitpid(pid, &status, 0);
    }
    cleanup();
    state = "stopped";
}

void ContainerImpl::cleanup() {
    mounts.clear();
    rootfs_manager.reset();
    cgroup_manager.reset();
}

// 构造函数
ContainerImpl::ContainerImpl(ContainerConfig config)
    : config(std::move(config)), state("created") {
    if (this->config.id.empty()) {
        this->config.id =
                "container_" +
                std::to_string(std::hash<std::string>{}(std::to_string(time(nullptr))));
    }
    if (this->config.name.empty()) {
        this->config.name = this->config.id;
    }
    if (this->config.log_path.empty()) {
        this->config.log_path =
                (Container::data_dir() / "containers" / this->config.id / "log.txt")
                        .string();
    }
}

void ContainerImpl::setup_network() {
    if (config.network_mode == "none") {
        network_strategy = std::make_unique<NoneNetworkStrategy>();
    } else if (config.network_mode == "bridge") {
        network_strategy = std::make_unique<BridgeNetworkStrategy>();
    } else {
        throw std::invalid_argument("unsupported network mode: " +
                                    config.network_mode);
    }
}
