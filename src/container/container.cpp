#include "container.h"
#include "cgroup_v2_manager.h"
#include "container_config.h"
#include "container_impl.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

using json = nlohmann::json;
namespace fs = std::filesystem;

fs::path Container::data_dir() { return "/tmp/nanocontainer"; }

Container::Container(ContainerConfig config)
    : pimpl_(std::make_unique<ContainerImpl>(std::move(config))) {}

Container::~Container() = default;
Container::Container(Container &&) noexcept = default;
Container &Container::operator=(Container &&) noexcept = default;

void Container::start() {
    pimpl_->launch();
    save();
}
void Container::stop() {
    pimpl_->stop();
    save();
}

std::string Container::state() const { return pimpl_->state; }
pid_t Container::pid() const { return pimpl_->pid; }
const std::string &Container::id() const { return pimpl_->config.id; }
const std::string &Container::name() const { return pimpl_->config.name; }

void Container::save() const {
    auto dir = container_dir();
    fs::create_directories(dir);
    auto path = dir / "config.json";
    json j;
    j["id"] = pimpl_->config.id;
    j["name"] = pimpl_->config.name;
    j["image"] = pimpl_->config.image;
    j["cmd"] = pimpl_->config.cmd;
    j["memory_limit"] = pimpl_->config.memory_limit;
    j["cpu_shares"] = pimpl_->config.cpu_shares;
    j["state"] = pimpl_->state;
    j["pid"] = pimpl_->pid;
    j["log_path"] = pimpl_->config.log_path;

    std::ofstream o(path);
    if (!o)
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "save failed");
    o << j.dump(2);
}

Container Container::load(const std::string &id) {
    auto path = data_dir() / "containers" / id / "config.json";
    std::ifstream i(path);
    if (!i)
        throw std::runtime_error("not found: " + id);
    json j;
    i >> j;

    ContainerConfig cfg;
    cfg.id = j.value("id", "");
    cfg.name = j.value("name", "");
    cfg.image = j.value("image", "");
    cfg.cmd = j.value("cmd", std::vector<std::string>{});
    cfg.memory_limit = j.value("memory_limit", 0);
    cfg.cpu_shares = j.value("cpu_shares", 0.0);
    cfg.log_path = j.value("log_path", "");

    Container c(std::move(cfg));
    c.pimpl_->state = j.value("state", "unknown");
    c.pimpl_->pid = j.value("pid", -1);
    return c;
}

fs::path Container::container_dir() const {
    return data_dir() / "containers" / pimpl_->config.id;
}

const std::vector<std::string> &Container::cmd() const {
    return pimpl_->config.cmd;
}

const std::string &Container::image() const { return pimpl_->config.image; }

const std::string &Container::log_path() const {
    return pimpl_->config.log_path;
}

const ContainerConfig &Container::config() const { return pimpl_->config; }