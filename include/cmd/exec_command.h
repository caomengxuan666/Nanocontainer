// include/cmd/exec_command.h
#pragma once
#include <string>
#include <vector>

class ExecCommand {
    std::string container_id_;
    std::vector<std::string> cmd_;

public:
    ExecCommand(std::string container_id, std::vector<std::string> cmd);

    void execute() const;
};