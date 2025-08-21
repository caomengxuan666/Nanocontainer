# NanoContainer

一个轻量级的容器实现，用于学习和理解 Linux 容器技术的底层机制。

## 🚀 简介

NanoContainer 是一个简化版的容器引擎，它实现了容器技术的核心功能，包括命名空间隔离、cgroups 资源限制、OverlayFS 文件系统和网络管理。该项目旨在提供一个易于理解的容器实现，帮助开发者深入了解 Docker 等容器技术的工作原理。

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

## ▶️ 使用方法

### 启动容器

```bash
# 运行一个容器
sudo ./NanoContainer run alpine:latest /bin/sh

# 运行容器并指定网络模式
sudo ./NanoContainer run --network bridge alpine:latest /bin/sh

# 运行容器并挂载目录
sudo ./NanoContainer run --mount /host/path:/container/path alpine:latest /bin/sh

# 运行容器并在退出后自动删除
sudo ./NanoContainer run --rm alpine:latest /bin/sh
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

## 🛠️ 技术架构

NanoContainer 遵循模块化设计，各组件职责明确：

- **容器管理** (`container/`): 负责容器的生命周期管理
- **镜像管理** (`image/`): 处理容器镜像的拉取和存储
- **文件系统** (`rootfs/`): 使用 OverlayFS 管理容器文件系统
- **运行时** (`runtime/`): 处理命名空间和 cgroups 等底层技术
- **网络** (`network/`): 实现容器网络功能
- **命令行** (`cmd/`): 实现各种用户命令
- **守护进程** (`daemon/`): 提供后台服务支持

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

### 网络管理

支持多种网络模式：
- None 模式：无网络访问
- Bridge 模式：通过网桥连接网络，支持外部访问

## 📚 学习价值

NanoContainer 是学习容器技术的绝佳项目：
1. 代码简洁，核心功能一目了然
2. 涵盖了容器技术的主要知识点
3. 模块化设计，易于扩展和修改
4. 实际可用，不只是演示代码

## ⚠️ 注意事项

- 需要 root 权限运行
- 仅支持 Linux 系统
- 仅供学习和研究使用，不建议在生产环境中使用

## 🤝 贡献

欢迎提交 Issue 和 Pull Request 来改进 NanoContainer！

## 📄 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。