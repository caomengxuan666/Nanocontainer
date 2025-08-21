#include "builder/dockerfile_parser.h"
#include <fstream>
#include <sstream>
#include <vector>


std::vector<BuildInstruction> DockerfileParser::parse(const std::string& dockerfile_path) {
    std::ifstream file(dockerfile_path);
    if (!file.is_open()) {
        throw std::invalid_argument("cannot open Dockerfile: " + dockerfile_path);
    }

    std::vector<BuildInstruction> instructions;
    std::string line;

    while (std::getline(file, line)) {
        // 去除空白
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        std::string arg;
        std::getline(iss, arg);
        arg.erase(0, arg.find_first_not_of(" \t"));
        arg.erase(arg.find_last_not_of(" \t") + 1);

        instructions.push_back({cmd, arg});
    }

    if (instructions.empty() || instructions[0].type != "FROM") {
        throw std::invalid_argument("Dockerfile must start with FROM");
    }

    return instructions;
}