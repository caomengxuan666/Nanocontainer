// include/daemon/daemon.h
#pragma once
#include <string>

class Daemon {
public:
    void start();
    void stop();

private:
    void handle_command(const std::string &cmd);
};