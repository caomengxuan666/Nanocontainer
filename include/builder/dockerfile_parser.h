#pragma once
#include <string>
#include <vector>

struct BuildInstruction {
    std::string type;  // FROM, RUN, COPY, CMD
    std::string arg;
};

class DockerfileParser {
public:
    static std::vector<BuildInstruction> parse(const std::string& dockerfile_path);
};