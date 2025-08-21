//todo 目前的deamon实现有问题，会导致各种各样的奇怪BUG，后续会修复并且开放
// src/main.cpp
#include "CLI11.hpp"
#include "rootfs/mount_pair.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <vector>

// 命令头文件
#include "builder/image_builder.h"
#include "cmd/exec_command.h"
#include "cmd/logs_command.h"
#include "cmd/ps_command.h"
#include "cmd/rm_command.h"
#include "cmd/stop_command.h"
#include "container/container.h"
#include "image/image_manager.h"

static std::vector<MountPair> parse_mounts(const std::vector<std::string> &mount_args) {
    std::vector<MountPair> mounts;
    for (const auto &m: mount_args) {
        // 找到最后一个 ':'，防止路径中有 ':'
        auto pos = m.find_last_of(':');
        if (pos == std::string::npos) {
            throw std::invalid_argument("invalid mount format: " + m + ", expected: host:target");
        }
        std::string host = m.substr(0, pos);
        std::string target = m.substr(pos + 1);

        // 检查路径是否为空
        if (host.empty() || target.empty()) {
            throw std::invalid_argument("mount path cannot be empty: " + m);
        }

        mounts.push_back({host, target});
    }
    return mounts;
}


int main(int argc, char *argv[]) {
    CLI::App app{"nanocontainer - A lightweight container implementation"};

    try {
        // run 子命令
        auto run_app = app.add_subcommand("run", "Run a command in a new container");
        std::string image_name;
        std::vector<std::string> cmd_args;
        std::string network_mode = "bridge";
        bool rm = false;
        std::vector<std::string> mount_args;

        run_app->add_option("image", image_name, "Image to run")->required();
        run_app->add_option("cmd", cmd_args, "Command and arguments to execute")->take_all();
        run_app->add_option("--network", network_mode, "Network mode (none, bridge)");
        run_app->add_flag("--rm", rm, "Remove container after exit");
        run_app->add_option("--mount", mount_args, "Bind mount: host:target")->take_all();

        // build 子命令
        auto build_app = app.add_subcommand("build", "Build an image from Dockerfile");
        std::string tag;
        std::string context_path = ".";
        build_app->add_option("-t", tag, "Tag")->required();
        build_app->add_option("path", context_path, "Build context");

        // ps 子命令
        auto ps_app = app.add_subcommand("ps", "List containers");

        // exec 子命令
        auto exec_app = app.add_subcommand("exec", "Execute a command in a running container");
        std::string id;
        std::vector<std::string> exec_args;
        exec_app->add_option("id", id, "Container ID")->required();
        exec_app->add_option("cmd", exec_args, "Command and arguments to execute")->required()->take_all();

        // pull 子命令
        auto pull_app = app.add_subcommand("pull", "Pull an image");
        std::string pull_image_name;
        pull_app->add_option("image", pull_image_name, "Image to pull")->required();

        // log 子命令
        auto log_app = app.add_subcommand("log", "Show container logs");
        std::string log_id;
        log_app->add_option("id", log_id, "Container ID")->required();

        // stop 子命令
        auto stop_app = app.add_subcommand("stop", "Stop a running container");
        std::string stop_id;
        stop_app->add_option("id", stop_id, "Container ID")->required();

        // rm 子命令
        auto rm_app = app.add_subcommand("rm", "Remove a container");
        std::string rm_id;
        rm_app->add_option("id", rm_id, "Container ID")->required();


        // network 子命令
        auto network_app = app.add_subcommand("network", "Manage container network");

        // deamon 子命令
        auto daemon_app = app.add_subcommand("daemon", "Run the nanocontainer daemon");

        CLI11_PARSE(app, argc, argv);

        if (run_app->parsed()) {
            ContainerConfig cfg;
            cfg.id = "test1";
            cfg.name = "mybox";
            cfg.image = ImageManager::instance().get_rootfs_path(image_name);
            cfg.memory_limit = 1024 * 1024 * 1024;
            cfg.cpu_shares = 0.5;
            cfg.network_mode = network_mode;
            cfg.mounts = parse_mounts(mount_args);

            // ✅ 如果用户没有输入 cmd，尝试从镜像读取 CMD
            if (cmd_args.empty()) {
                std::string cmd_file = cfg.image + "/image_cmd.txt";
                std::ifstream ifs(cmd_file);
                if (ifs.is_open()) {
                    std::string cmd;
                    std::getline(ifs, cmd);
                    ifs.close();

                    if (!cmd.empty()) {
                        std::istringstream iss(cmd);
                        std::string arg;
                        while (iss >> arg) {
                            cfg.cmd.push_back(arg);
                        }
                    }
                }

                if (cfg.cmd.empty()) {
                    std::cerr << "Error: no command specified and image has no CMD\n";
                    return 1;
                }
            } else {
                cfg.cmd = cmd_args;
            }

            Container container(std::move(cfg));
            container.start();

            std::cout << "Started " << container.id() << " PID=" << container.pid() << std::endl;
            int status;
            waitpid(container.pid(), &status, 0);
            container.stop();


            //if (rm) {
            //    std::system(("sudo rm -rf " + Container::data_dir().string() + "/containers/" + cfg.id).c_str());
            //    std::system(("sudo rmdir /sys/fs/cgroup/nanocontainer/" + cfg.id).c_str());
            //}

        } else if (build_app->parsed()) {
            ImageBuilder builder(context_path);
            builder.build(tag);

        } else if (ps_app->parsed()) {
            PsCommand{}.execute();

        } else if (exec_app->parsed()) {
            ExecCommand{id, exec_args}.execute();

        } else if (pull_app->parsed()) {
            ImageManager::instance().pull(pull_image_name);

        } else if (log_app->parsed()) {
            LogsCommand{log_id}.execute();

        } else if (stop_app->parsed()) {
            StopCommand{stop_id}.execute();

        } else if (rm_app->parsed()) {
            RmCommand{rm_id}.execute();
        }

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}