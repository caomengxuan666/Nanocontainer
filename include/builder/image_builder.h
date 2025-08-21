#pragma once
#include "dockerfile_parser.h"
#include <string>

class ImageBuilder {
    std::string context_dir_;
    std::string dockerfile_path_;

public:
    ImageBuilder(std::string context_dir, std::string dockerfile_path = "Dockerfile");

    void build(const std::string& tag);
};