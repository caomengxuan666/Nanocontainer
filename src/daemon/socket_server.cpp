// src/daemon/socket_server.cpp
#include "socket_server.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

SocketServer::SocketServer(std::string socket_path)
    : socket_path_(std::move(socket_path)) {}

SocketServer::~SocketServer() {
    stop();
    unlink(socket_path_.c_str());
}

void SocketServer::start(std::function<void(const std::string &)> on_message) {
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "socket failed");
    }

    struct sockaddr_un addr {};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

    unlink(socket_path_.c_str());

    if (bind(server_fd_, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "bind failed");
    }

    if (listen(server_fd_, 5) == -1) {
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "listen failed");
    }

    std::cout << "Daemon listening on " << socket_path_ << "\n";

    while (true) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd == -1) {
            if (errno == EBADF)
                break;
            continue;
        }

        std::thread([client_fd, on_message]() {
            char buffer[1024];
            ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0';
                on_message(std::string(buffer));
            }
            close(client_fd);
        }).detach();
    }
}

void SocketServer::stop() {
    if (server_fd_ != -1) {
        close(server_fd_);
        server_fd_ = -1;
    }
}