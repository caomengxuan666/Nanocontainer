#include "builder/image_builder.h"
#include "builder/dockerfile_parser.h"
#include "image/image_manager.h"
#include "overlayfs_strategy.h"
#include "container/container.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/mount.h>
#include <unistd.h>
#include <sys/wait.h>

namespace fs = std::filesystem;

ImageBuilder::ImageBuilder(std::string context_dir, std::string dockerfile_path)
    : context_dir_(std::move(context_dir))
    , dockerfile_path_(std::move(dockerfile_path)) {}

void ImageBuilder::build(const std::string& tag) {
    auto instructions = DockerfileParser::parse(context_dir_ + "/Dockerfile");

    std::string base_image = instructions[0].arg;
    std::string base_rootfs = ImageManager::instance().get_rootfs_path(base_image);

    std::string layer_id = "layer_" + std::to_string(std::hash<std::string>{}(tag));
    fs::path layer_dir = Container::data_dir() / "layers" / layer_id;
    fs::path merged = layer_dir / "merged";
    fs::path work = layer_dir / "work";
    fs::path upper = layer_dir / "upper";

    fs::create_directories(upper);
    fs::create_directories(work);
    fs::create_directories(merged);

    // 第一层：base image + overlayfs
    OverlayfsStrategy overlay_strategy(work);
    overlay_strategy.setup(base_rootfs, upper, merged);

    // 临时挂载管理
    auto cleanup = [&]() {
        umount2(merged.c_str(), MNT_DETACH);
    };

    for (size_t i = 1; i < instructions.size(); ++i) {
        const auto& inst = instructions[i];

        if (inst.type == "RUN") {
            ContainerConfig cfg;
            cfg.id = "build-" + layer_id;
            cfg.image = merged.string();  // 当前层作为根文件系统
            cfg.cmd = {"sh", "-c", inst.arg};
            cfg.network_mode = "bridge";

            Container container(std::move(cfg));
            container.start();

            int status;
            waitpid(container.pid(), &status, 0);
            if (status != 0) {
                std::cerr << "RUN failed: " << inst.arg << "\n";
                cleanup();
                throw std::runtime_error("build failed");
            }

        } else if (inst.type == "COPY") {
            std::istringstream iss(inst.arg);
            std::string src, dest;
            iss >> src >> dest;

            std::string host_src = context_dir_ + "/" + src;
            std::string container_dest = merged.string() + "/" + dest;

            if (fs::is_directory(host_src)) {
                fs::create_directories(container_dest);
            } else {
                fs::create_directories(fs::path(container_dest).parent_path());
                fs::copy(host_src, container_dest, fs::copy_options::overwrite_existing);
            }

        } else if (inst.type == "CMD") {
            // 记录 CMD，写入 /image.json
            std::ofstream cmd_file(merged.string() + "/image_cmd.txt");
            cmd_file << inst.arg;
            cmd_file.close();
        }
    }

    cleanup();

    // 提交镜像
    std::string image_path = ImageManager::instance().get_rootfs_path(tag);
    if (fs::exists(image_path)) {
        fs::remove_all(image_path);
    }
    fs::create_directories(image_path);
    fs::copy(upper, image_path, fs::copy_options::recursive);

    std::cout << "Successfully built image: " << tag << "\n";
}