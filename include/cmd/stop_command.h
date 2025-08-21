// include/cmd/stop_command.h
#pragma once
#include <string>

class StopCommand {
    std::string container_id_;

public:
    explicit StopCommand(std::string container_id);
    void execute() const;
};