// src/cmd/exec_command.cpp
#include "exec_command.h"
#include "container/container.h"
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sched.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

ExecCommand::ExecCommand(std::string container_id, std::vector<std::string> cmd)
    : container_id_(std::move(container_id)), cmd_(std::move(cmd)) {}

void ExecCommand::execute() const {
    Container container = Container::load(container_id_);

    if (container.state() != "running") {
        throw std::runtime_error("container is not running");
    }

    pid_t pid = container.pid();

    // 打开所有 namespace fd
    auto ns_path = [&](const std::string &ns) {
        return "/proc/" + std::to_string(pid) + "/ns/" + ns;
    };

    int mount_fd = open(ns_path("mnt").c_str(), O_RDONLY);
    int pid_fd = open(ns_path("pid").c_str(), O_RDONLY);
    int net_fd = open(ns_path("net").c_str(), O_RDONLY);// 网络 namespace

    if (mount_fd == -1 || pid_fd == -1 || net_fd == -1) {
        if (mount_fd != -1)
            close(mount_fd);
        if (pid_fd != -1)
            close(pid_fd);
        if (net_fd != -1)
            close(net_fd);
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "open ns failed");
    }

    pid_t child = fork();
    if (child == 0) {
        // 子进程：进入 namespace（顺序很重要）
        if (setns(mount_fd, CLONE_NEWNS) != 0) {
            std::cerr << "setns mount failed\n";
            std::exit(1);
        }
        if (setns(net_fd, CLONE_NEWNET) != 0) {
            std::cerr << "setns net failed\n";
            std::exit(1);
        }
        if (setns(pid_fd, CLONE_NEWPID) != 0) {
            std::cerr << "setns pid failed\n";
            std::exit(1);
        }

        // chroot 到容器根目录
        auto container_dir = container.container_dir();
        std::string rootfs = (container_dir / "merged").string();
        if (chdir(rootfs.c_str()) != 0 || chroot(".") != 0) {
            std::cerr << "chroot failed\n";
            std::exit(1);
        }

        // 挂载 /proc
        if (mount("proc", "/proc", "proc", 0, nullptr) != 0) {
            std::cerr << "mount /proc failed\n";
        }

        // 执行命令
        std::vector<char *> c_args;
        for (auto &arg: cmd_) {
            c_args.push_back(const_cast<char *>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        execvp(c_args[0], c_args.data());
        std::cerr << "execvp failed: " << strerror(errno) << "\n";
        std::exit(1);
    }

    close(mount_fd);
    close(pid_fd);
    close(net_fd);

    int status;
    waitpid(child, &status, 0);
}