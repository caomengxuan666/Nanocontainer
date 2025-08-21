#pragma once
#include "container_config.h"
#include <filesystem>
#include <memory>
#include <string>
namespace fs = std::filesystem;

class ContainerImpl;// Pimpl

class Container {
    std::unique_ptr<ContainerImpl> pimpl_;

public:
    explicit Container(ContainerConfig config);
    ~Container();

    Container(const Container &) = delete;
    Container &operator=(const Container &) = delete;

    Container(Container &&) noexcept;
    Container &operator=(Container &&) noexcept;

    void start();
    void stop();
    [[nodiscard]] std::string state() const;
    [[nodiscard]] pid_t pid() const;
    [[nodiscard]] const std::string &id() const;
    [[nodiscard]] const std::string &name() const;

    void save() const;
    static Container load(const std::string &id);

    static fs::path data_dir();
    [[nodiscard]] fs::path container_dir() const;

    // 新增：提供对 cmd 的只读访问
    [[nodiscard]] const std::vector<std::string> &cmd() const;

    // 如果需要，也可以加 image()
    [[nodiscard]] const std::string &image() const;

    [[nodiscard]] const std::string &log_path() const;

    // 也可以只用 config()
    [[nodiscard]] const ContainerConfig &config() const;
};