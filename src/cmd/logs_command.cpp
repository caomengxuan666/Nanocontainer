// src/cmd/logs_command.cpp
#include "logs_command.h"
#include "container/container.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

LogsCommand::LogsCommand(std::string container_id)
    : container_id_(std::move(container_id)) {}

void LogsCommand::execute() const {
    Container container = Container::load(container_id_);

    std::string log_path = container.log_path();
    std::ifstream file(log_path);
    if (!file) {
        throw std::runtime_error("cannot open log file: " + log_path);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << '\n';
    }
}