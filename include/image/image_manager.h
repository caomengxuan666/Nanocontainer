// include/image/image_manager.h
#pragma once
#include <string>

class ImageManager {
public:
    static ImageManager &instance();

    void pull(const std::string &image_ref);// e.g. "alpine:latest"
    bool exists(const std::string &image_ref) const;
    std::string get_rootfs_path(const std::string &image_ref) const;

private:
    std::string images_dir() const;
};