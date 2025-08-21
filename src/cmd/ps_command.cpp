// src/cmd/ps_command.cpp
#include "ps_command.h"
#include "container/container.h"
#include <filesystem>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;
static std::string join(const std::vector<std::string> &vec,
                        const std::string &delim) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0)
            oss << delim;
        oss << vec[i];
    }
    return oss.str();
}

void PsCommand::execute() const {
    auto data_dir = Container::data_dir();
    auto containers_dir = data_dir / "containers";

    if (!fs::exists(containers_dir)) {
        std::cout << "No containers found.\n";
        return;
    }

    std::cout << std::left << std::setw(14) << "CONTAINER ID" << std::setw(8)
              << "NAME" << std::setw(8) << "PID" << std::setw(10) << "STATUS"
              << "COMMAND\n";

    for (const auto &entry: fs::directory_iterator(containers_dir)) {
        if (!entry.is_directory())
            continue;

        std::string id = entry.path().filename();
        try {
            Container container = Container::load(id);
            std::cout << std::left << std::setw(14) << id.substr(0, 13)
                      << std::setw(8) << container.name().substr(0, 7) << std::setw(8)
                      << container.pid() << std::setw(10) << container.state()
                      << join(container.cmd(), " ")
                      << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Failed to load container " << id << ": " << e.what()
                      << "\n";
        }
    }
}
