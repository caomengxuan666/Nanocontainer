// src/image/image_manager.cpp
#include "image_manager.h"
#include "container/container.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

ImageManager &ImageManager::instance() {
    static ImageManager instance;
    return instance;
}

std::string ImageManager::images_dir() const {
    return Container::data_dir() / "images";
}

void ImageManager::pull(const std::string &image_ref) {
    std::string image = "alpine";
    std::string tag = "latest";

    if (image_ref.find(':') != std::string::npos) {
        auto pos = image_ref.find(':');
        image = image_ref.substr(0, pos);
        tag = image_ref.substr(pos + 1);
    }

    std::string target_dir = images_dir() + "/" + image;
    if (exists(image_ref)) {
        std::cout << image_ref << " already exists.\n";
        return;
    }

    fs::create_directories(target_dir);
    // TODO 用libcurl下载
    std::string url = "https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/"
                      "x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz";
    std::string cmd = "cd " + target_dir + " && wget -q " + url +
                      " && tar -xzf alpine-minirootfs-3.20.3-x86_64.tar.gz && rm "
                      "alpine-minirootfs-3.20.3-x86_64.tar.gz";

    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        throw std::runtime_error("pull failed");
    }

    std::cout << "Pulled " << image_ref << " successfully.\n";
}

bool ImageManager::exists(const std::string &image_ref) const {
    std::string path = get_rootfs_path(image_ref);
    return fs::exists(path + "/bin/sh");
}

std::string ImageManager::get_rootfs_path(const std::string &image_ref) const {
    std::string image = image_ref;
    if (image.find(':') != std::string::npos) {
        image = image.substr(0, image.find(':'));
    }
    return images_dir() + "/" + image;
}