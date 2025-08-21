// src/cmd/stop_command.cpp
#include "stop_command.h"
#include "container/container.h"
#include <iostream>

StopCommand::StopCommand(std::string container_id)
    : container_id_(std::move(container_id)) {}

void StopCommand::execute() const {
    Container container = Container::load(container_id_);
    if (container.state() != "running") {
        std::cout << "Container " << container_id_ << " is not running.\n";
        return;
    }

    container.stop();
    std::cout << "Container " << container_id_ << " stopped.\n";
}