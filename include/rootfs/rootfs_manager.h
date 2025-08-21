#pragma once
#include <string>

class RootfsManager {
public:
    virtual ~RootfsManager() = default;
    virtual void setup(const std::string &lowerdir, const std::string &upperdir,
                       const std::string &merged) = 0;
    virtual void teardown(const std::string &merged) = 0;
};