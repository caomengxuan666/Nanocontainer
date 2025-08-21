// cgroup.h
#pragma once
#include <string>
void setup_cgroups_v2();
void create_cgroup_v2(int pid, const std::string &path);
void remove_cgroup_v2(const std::string &path);