// src/cmd/rm_command.cpp
#include "rm_command.h"
#include "container/container.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

RmCommand::RmCommand(std::string container_id)
    : container_id_(std::move(container_id)) {}

void RmCommand::execute() const {
    Container container = Container::load(container_id_);

    if (container.state() == "running") {
        std::cerr << "Error: container is running. Use stop first.\n";
        return;
    }

    auto container_dir = Container::data_dir() / "containers" / container_id_;
    if (fs::exists(container_dir)) {
        fs::remove_all(container_dir);
    }

    auto cgroup_path = "/sys/fs/cgroup/nanocontainer/" + container_id_;
    if (fs::exists(cgroup_path)) {
        fs::remove_all(cgroup_path);
    }

    std::cout << "Container " << container_id_ << " removed.\n";
}