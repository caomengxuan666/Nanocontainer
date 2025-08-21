# NanoContainer

一个轻量级的容器实现，用于学习和理解 Linux 容器技术的底层机制。

## 🚀 简介

NanoContainer 是一个简化版的容器引擎，它实现了容器技术的核心功能，包括命名空间隔离、cgroups 资源限制、OverlayFS 文件系统和网络管理。该项目旨在提供一个易于理解的容器实现，帮助开发者深入了解 Docker 等容器技术的工作原理。

## 💾 安装指南

```bash
# 克隆仓库
git clone https://github.com/example/nanocontainer.git
cd nanocontainer

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make
```

## 🌟 特性

- **命名空间隔离**: 使用 Linux 命名空间实现进程、网络、文件系统等隔离
- **资源限制**: 通过 cgroups v2 控制容器的内存和 CPU 使用
- **文件系统**: 使用 OverlayFS 实现联合文件系统，支持镜像层和容器可写层
- **网络管理**: 支持多种网络模式（none、bridge）
- **镜像管理**: 支持拉取和管理容器镜像
- **命令行接口**: 提供类似 Docker 的命令行界面

## 🏗️ 构建

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
make
```

要求:
- CMake 3.26+
- C++17 编译器
- Linux 内核支持命名空间和 cgroups

## 🧪 快速开始

### 创建并运行容器

```bash
# 基本用法
sudo ./NanoContainer run alpine:latest /bin/sh

# 指定网络模式运行容器
sudo ./NanoContainer run --network bridge alpine:latest /bin/sh

# 挂载主机目录到容器
sudo ./NanoContainer run --mount /host/path:/container/path alpine:latest /bin/sh

# 运行容器并在退出后自动删除
sudo ./NanoContainer run --rm alpine:latest /bin/sh

# 构建容器镜像
sudo ./NanoContainer build -t myimage .
```

### 管理镜像

```bash
# 拉取镜像
sudo ./NanoContainer pull alpine:latest
```

### 管理容器

```bash
# 列出容器
sudo ./NanoContainer ps

# 查看容器日志
sudo ./NanoContainer log <container-id>

# 在运行的容器中执行命令
sudo ./NanoContainer exec <container-id> /bin/sh

# 停止容器
sudo ./NanoContainer stop <container-id>

# 删除容器
sudo ./NanoContainer rm <container-id>
```

## 🧩 支持命令

支持以下核心命令：

- `run`: 创建并运行容器
- `build`: 构建容器镜像
- `ps`: 列出正在运行的容器
- `exec`: 在运行中的容器中执行命令
- `log`: 查看容器日志
- `stop`: 停止容器
- `rm`: 删除容器
- `pull`: 拉取容器镜像
- `images`: 列出本地镜像

## 🧪 核心功能详解

### 命名空间隔离

NanoContainer 使用多种 Linux 命名空间实现隔离：
- PID 命名空间：为容器提供独立的进程空间
- UTS 命名空间：隔离主机名和域名
- IPC 命名空间：隔离进程间通信资源
- MNT 命名空间：隔离文件系统挂载点
- NET 命名空间：隔离网络设备、IP 地址等网络资源

### Cgroups 资源限制

通过 cgroups v2 实现资源限制：
- 内存限制：控制容器可以使用的内存量
- CPU 限制：控制容器可以使用的 CPU 时间

### OverlayFS 文件系统

使用 OverlayFS 实现容器文件系统：
- Lower 层：只读镜像层
- Upper 层：容器可写层
- Merged 层：合并视图

## 🔧 网络配置

默认使用 bridge 模式。如果需要手动管理网络接口，可以使用以下命令：

```bash
# 清理残留网络接口
sudo ip link delete veth_host 2>/dev/null || true
sudo ip link delete veth_container 2>/dev/null || true
sudo brctl delbr nanobridge 2>/dev/null || true

# 创建新的网桥
sudo brctl addbr nanobridge
sudo ip link set nanobridge up
```

支持的网络模式：
- `none`: 容器没有网络功能
- `bridge`: 默认模式，通过网桥提供网络访问

## 📚 学习资源

NanoContainer 是学习容器技术的绝佳项目：
1. 代码简洁，核心功能一目了然
2. 涵盖了容器技术的主要知识点
3. 模块化设计，易于扩展和修改
4. 实际可用，不只是演示代码
5. 提供详细的文档和示例

## ⚠️ 注意事项

- 需要 root 权限运行
- 仅支持 Linux 系统
- 仅供学习和研究使用，不建议在生产环境中使用

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进 NanoContainer！

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。