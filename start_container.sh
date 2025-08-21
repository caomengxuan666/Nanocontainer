#!/bin/bash
set -e  # 出错立即退出，避免无效操作

# ==================== 配置参数（可根据需要修改）====================
IMAGE_TAR_PATH="$HOME/download/alpine-minirootfs-3.20.3-x86_64.tar.gz"  # 镜像tar包路径
CONTAINER_ID="test1"                                                    # 容器ID
MEMORY_LIMIT="1073741824"                                               # 内存限制（1GB）
MOUNT_HOST_PATH="/tmp"                                                  # 宿主机挂载目录
MOUNT_CONTAINER_PATH="/container-data"                                  # 容器内挂载目录
NANOCONTAINER_DIR="/tmp/nanocontainer"                                  # 容器根目录
BUILD_DIR="$HOME/codespace/nanocontainer/build"                         # 编译目录（你的build路径）
# ==================================================================

echo "===== 1. 清理残留进程和旧环境 ====="
# 杀死所有相关进程
sudo pkill -f "NanoContainer" || true
sudo pkill -f "/bin/sh" || true  # 杀死容器内可能残留的sh进程

# 清理旧容器目录和CGroup
sudo rm -rf "${NANOCONTAINER_DIR}/containers/${CONTAINER_ID}" || true
sudo rm -rf "/sys/fs/cgroup/nanocontainer" || true

echo "===== 2. 准备Alpine镜像 ====="
# 创建镜像目录并解压（仅当镜像不存在时执行）
sudo mkdir -p "${NANOCONTAINER_DIR}/images/alpine"
if [ ! -f "${NANOCONTAINER_DIR}/images/alpine/bin/sh" ]; then
    echo "正在解压镜像..."
    sudo cp "${IMAGE_TAR_PATH}" "${NANOCONTAINER_DIR}/images/"
    sudo tar -xzf "${NANOCONTAINER_DIR}/images/alpine-minirootfs-3.20.3-x86_64.tar.gz" \
             -C "${NANOCONTAINER_DIR}/images/alpine"
    sudo rm -f "${NANOCONTAINER_DIR}/images/alpine-minirootfs-3.20.3-x86_64.tar.gz"
else
    echo "镜像已存在，跳过解压"
fi

echo "===== 3. 配置CGroup2 ====="
# 重建CGroup目录并启用memory控制
sudo mkdir -p "/sys/fs/cgroup/nanocontainer/${CONTAINER_ID}"
sudo bash -c "echo '+memory' > /sys/fs/cgroup/nanocontainer/cgroup.subtree_control"
sudo bash -c "echo ${MEMORY_LIMIT} > /sys/fs/cgroup/nanocontainer/${CONTAINER_ID}/memory.max"
# 重建CGroup并同时启用CPU和memory控制（关键！）
sudo mkdir -p /sys/fs/cgroup/nanocontainer/test1
sudo bash -c 'echo "+cpu +memory" > /sys/fs/cgroup/nanocontainer/cgroup.subtree_control'
