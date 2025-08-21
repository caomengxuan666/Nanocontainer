// src/daemon/daemon.cpp
#include "daemon.h"
#include "cmd/exec_command.h"
#include "cmd/ps_command.h"
#include "cmd/stop_command.h"
#include "container/container.h"
#include "container_message.h"
#include "image/image_manager.h"
#include "socket_server.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

void Daemon::start() {
    // 创建运行目录
    fs::create_directories("/tmp/nanocontainer/run");

    SocketServer server("/tmp/nanocontainer/run/daemon.sock");
    server.start([this](const std::string &message) {
        std::cout << "Received: " << message << "\n";
        handle_command(message);
    });
}

void Daemon::handle_command(const std::string &json_str) {
    try {
        ContainerMessage msg = ContainerMessage::from_json(json_str);

        if (msg.cmd == "run") {
            Container container(std::move(msg.config));
            container.start();
            std::cout << "RUN_OK " << container.pid() << "\n";

        } else if (msg.cmd == "ps") {
            PsCommand{}.execute();

        } else if (msg.cmd == "exec") {
            ExecCommand{msg.container_id, msg.exec_args}.execute();
            std::cout << "EXEC_OK\n";

        } else if (msg.cmd == "stop") {
            StopCommand{msg.container_id}.execute();
            std::cout << "STOP_OK\n";

        } else {
            std::cout << "ERROR unknown command\n";
        }

    } catch (const std::exception &e) {
        std::cout << "ERROR " << e.what() << "\n";
    }
}

void Daemon::stop() {
    // 实现优雅关闭
}