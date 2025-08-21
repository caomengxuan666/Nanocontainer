// src/network/bridge_network_strategy.cpp
#include "bridge_network_strategy.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/mount.h>
#include <unistd.h>

BridgeNetworkStrategy::BridgeNetworkStrategy() {
    // 生成容器 IP：172.18.PID.2
    std::ostringstream oss;
    oss << "172.18." << getpid() % 254 << ".2";
    container_ip_ = oss.str();
}

std::string BridgeNetworkStrategy::get_container_ip(pid_t pid) const {
    std::ostringstream oss;
    oss << "172.18." << (pid % 254) << ".2";
    return oss.str();
}

void BridgeNetworkStrategy::setup_on_host(pid_t container_pid) {
    try {
        // 1. 创建 bridge（如果不存在）
        if (system(("ip link show " + bridge_name_ + " >/dev/null 2>&1").c_str()) !=
            0) {
            std::system(
                    ("ip link add " + bridge_name_ + " type bridge >/dev/null 2>&1")
                            .c_str());
            std::system(("ip addr add " + bridge_ip_ + "/" + netmask_ + " dev " +
                         bridge_name_ + " >/dev/null 2>&1")
                                .c_str());
            //  加上这一行：启用 bridge
            std::system(
                    ("ip link set " + bridge_name_ + " up >/dev/null 2>&1").c_str());

            // 添加iptables规则以允许转发
            std::system(("iptables -I FORWARD -i " + bridge_name_ +
                         " -j ACCEPT 2>/dev/null || true")
                                .c_str());
            std::system(("iptables -I FORWARD -o " + bridge_name_ +
                         " -j ACCEPT 2>/dev/null || true")
                                .c_str());

            std::cout << "Created and brought up bridge " << bridge_name_ << "\r\n"
                      << std::flush;
        }

        // 2. 生成唯一的 veth 名称
        std::ostringstream oss;
        oss << "veth-" << container_pid;
        host_if_ = oss.str();
        container_if_ = "eth0";

        // 3. 先尝试删除可能存在的同名接口，避免"RTNETLINK answers: File exists"错误
        std::string delete_cmd =
                "ip link delete " + host_if_ + " 2>/dev/null || true";
        system(delete_cmd.c_str());

        // 同时删除可能存在的容器端接口
        delete_cmd = "ip netns exec " + std::to_string(container_pid) +
                     " ip link delete " + container_if_ + " 2>/dev/null || true";
        system(delete_cmd.c_str());

        // 4. 创建 veth pair
        std::string cmd = "ip link add " + host_if_ + " type veth peer name " +
                          container_if_ + " netns " + std::to_string(container_pid);
        if (system(cmd.c_str()) != 0) {
            // 如果上面的命令失败，尝试另一种方式创建
            std::string cmd1 = "ip link add " + host_if_ +
                               " type veth peer name temp_veth 2>/dev/null";
            std::string cmd2 = "ip link set temp_veth netns " +
                               std::to_string(container_pid) + " name " +
                               container_if_ + " 2>/dev/null";

            if (system(cmd1.c_str()) != 0 || system(cmd2.c_str()) != 0) {
                throw std::runtime_error("create veth pair failed");
            }
        }

        // 5. 将 host 端加入 bridge
        cmd =
                "ip link set " + host_if_ + " master " + bridge_name_ + " 2>/dev/null";
        if (system(cmd.c_str()) != 0) {
            throw std::runtime_error("add veth to bridge failed");
        }

        // 6. 启动 host 端
        cmd = "ip link set " + host_if_ + " up 2>/dev/null";
        if (system(cmd.c_str()) != 0) {
            throw std::runtime_error("bring up host veth failed");
        }

        // 输出网络连接信息并强制刷新
        std::cout << "Attached " << host_if_ << " to " << bridge_name_ << "\r\n"
                  << std::flush;

    } catch (const std::exception &e) {
        std::cerr << "Bridge setup failed: " << e.what() << "\r\n";
        teardown();
        throw;
    }
}

void BridgeNetworkStrategy::setup_in_container() {
    try {
        // 2. 检查接口是否已经正确命名
        // 先检查eth0是否存在
        std::string check_cmd = "ip link show eth0 >/dev/null 2>&1";
        if (system(check_cmd.c_str()) != 0) {
            // 检查容器接口是否存在
            check_cmd = "ip link show " + container_if_ + " >/dev/null 2>&1";
            if (system(check_cmd.c_str()) == 0) {
                // 只有当容器接口存在且不叫eth0时才尝试重命名
                if (container_if_ != "eth0") {
                    std::string cmd =
                            "ip link set " + container_if_ + " name eth0 2>/dev/null";
                    system(cmd.c_str());
                }
            }
        }


        // 3. 设置 IP
        std::string ip_cmd = "ip addr add " + container_ip_ + "/" + netmask_ +
                             " dev eth0 2>/dev/null";
        if (system(ip_cmd.c_str()) != 0) {
            throw std::runtime_error("set container IP failed");
        }

        // 4. 启动接口
        if (system("ip link set eth0 up 2>/dev/null") != 0) {
            throw std::runtime_error("bring up eth0 failed");
        }

        // 5. 设置默认路由
        std::string route_cmd =
                "ip route add default via " + bridge_ip_ + " 2>/dev/null";
        if (system(route_cmd.c_str()) != 0) {
            throw std::runtime_error("set default route failed");
        }

        // 关键：写入 DNS 配置
        std::ofstream resolv("/etc/resolv.conf");
        if (resolv.is_open()) {
            resolv << "nameserver 8.8.8.8\n";        // Google 公共 DNS
            resolv << "nameserver 114.114.114.114\n";// 国内备用 DNS
            resolv.close();
            std::cout << "容器内 DNS 配置完成" << std::endl;
        } else {
            std::cerr << "警告：无法写入 /etc/resolv.conf可能影响域名解析" << std::endl;
        }

        // 输出容器IP并强制刷新输出缓冲区，使用\r\n确保回车换行
        std::cout << "Container IP: " << container_ip_ << "\r\n\r\n"
                  << std::flush;

    } catch (const std::exception &e) {
        std::cerr << "Container network setup failed: " << e.what() << "\r\n"
                  << std::flush;
        throw;
    }
}

void BridgeNetworkStrategy::teardown() {
    // 删除 veth 接口（host 端）
    if (!host_if_.empty()) {
        std::string cmd = "ip link delete " + host_if_ + " 2>/dev/null || true";
        system(cmd.c_str());
    }

    // 删除iptables规则
    if (!bridge_name_.empty()) {
        std::string cmd1 = "iptables -D FORWARD -i " + bridge_name_ +
                           " -j ACCEPT 2>/dev/null || true";
        std::string cmd2 = "iptables -D FORWARD -o " + bridge_name_ +
                           " -j ACCEPT 2>/dev/null || true";
        system(cmd1.c_str());
        system(cmd2.c_str());
    }
}