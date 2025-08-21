// include/cmd/rm_command.h
#pragma once
#include <string>

class RmCommand {
    std::string container_id_;

public:
    explicit RmCommand(std::string container_id);
    void execute() const;
};