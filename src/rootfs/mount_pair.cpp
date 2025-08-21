#include "mount_pair.h"

void to_json(nlohmann::json &j, const MountPair &m) {
    j = nlohmann::json{{"host_path", m.host_path},
                       {"container_path", m.container_path}};
}

void from_json(const nlohmann::json &j, MountPair &m) {
    j.at("host_path").get_to(m.host_path);
    j.at("container_path").get_to(m.container_path);
}