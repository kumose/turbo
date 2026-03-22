### Turbo 基础库
[中文版](README_CN.md)

Turbo 是一套遵循 C++17 标准开发的 C++ 基础库，作为 Kumo 产品的核心基础设施。该库由团队结合多年搜索与服务端开发的实战经验打造并持续打磨，已在数万个应用中得到验证，具备稳定、可靠的特性。

Turbo 是 `kumo` 产品的核心基础组件，无外部依赖，首要设计目标是稳定性，其次是性能。

本库对新增功能的接入持审慎态度。例如在 UTF-8 转换功能上，Turbo 的性能虽低于 [vamos](https://github.com/kumose/vamose)，但稳定性更优；在生产环境的部分场景中，我们并不需要极致的性能表现。

## 🛠️ 构建方式

本项目采用 [kmpkg](https://github.com/kumose/kmcmake) 进行依赖管理与构建集成。`kmpkg` 可自动完成第三方库下载、依赖解析及编译器参数配置，无需手动维护复杂的 CMake 配置。

### 0. 环境准备
- 系统：Linux（推荐 Ubuntu 20.04+ / CentOS 7+）
- 构建工具：CMake ≥ 3.20
- 编译器：GCC ≥ 9.4 / Clang ≥ 12
- 确保 `kmpkg` 已正确安装，安装文档参考 [安装指南](https://kumo-pub.github.io/docs/category/%E6%8C%81%E7%BB%AD%E9%9B%86%E6%88%90----kmpkg)

### 1. 项目配置（可选）
- 完整依赖清单请参考 [`kmpkg.json`](kmpkg.json)
- 如需更新依赖基线，可修改 [`kmpkg-configuration.json`](kmpkg-configuration.json) 中 `default-registry` 下的 `baseline` 字段
- `baseline` 可通过 `git log` 命令获取

### 2. 项目构建
在项目根目录执行以下命令：
```bash
cmake --preset=default  # 注：原英文拼写 defualt 为笔误，已修正为 default
cmake --build build -j$(nproc)
```

#### 手动管理依赖的构建方式
若需自行管理依赖，可通过标准 CMake 命令构建：
```shell
mkdir build
cd build
cmake ..
make -j$(nproc)
```

**注意事项**
- `--preset=default` 要求项目根目录已定义对应的 CMake 预设配置
- 手动管理依赖时，需确保 CMake 的 find_package 能定位到所有必需的库

### 3. 运行测试（可选）
在项目根目录执行：
```shell
ctest --test-dir build
```

## 许可证说明

Turbo 中整合了以下开源项目的部分代码：
* [Abseil](https://github.com/abseil/abseil-cpp) - 基于 [Apache 2 协议](licenses/abseil.lic)
* [Brpc](https://github.com/apache/brpc) - 基于 [Apache 2 协议](licenses/brpc.lic)
* [Folly](https://github.com/facebook/folly) - 基于 [Apache 2 协议](licenses/folly.lic)
* [EASTL](https://github.com/electronicarts/EASTL) - 基于 [BSD 3 协议](licenses/eastl.lic)
* [CLI11](https://github.com/CLIUtils/CLI11) - 基于 [新版 BSD 3 协议](licenses/cli11.lic)

Kumo 官方发布版本采用 [Apache 2 协议](LICENSE) 开源。

---

### 总结
1. Turbo 是 Kumo 产品的核心 C++17 基础库，优先保障稳定性，其次兼顾性能，已在数万应用中验证；
2. 项目支持 kmpkg 自动构建（推荐）和手动依赖管理两种构建方式，需满足特定的编译环境要求；
3. 代码整合了多个开源库（Apache 2/BSD 3 协议），官方发布版本采用 Apache 2 协议。