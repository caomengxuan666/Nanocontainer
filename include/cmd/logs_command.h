// include/cmd/logs_command.h
#pragma once
#include <string>

class LogsCommand {
    std::string container_id_;

public:
    explicit LogsCommand(std::string container_id);
    void execute() const;
};