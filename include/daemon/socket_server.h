// include/daemon/socket_server.h
#pragma once
#include <functional>
#include <string>

class SocketServer {
    std::string socket_path_;
    int server_fd_ = -1;

public:
    explicit SocketServer(std::string socket_path);
    ~SocketServer();

    SocketServer(const SocketServer &) = delete;
    SocketServer &operator=(const SocketServer &) = delete;

    void start(std::function<void(const std::string &)> on_message);
    void stop();
};