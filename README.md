turbo
=============================

[中文版](README_CN.md)


Turbo is a C++ basic library developed in compliance with the C++17 standard,
serving as the fundamental infrastructure for the Kumo product. Built and refined
over years of hands-on experience in search and server-side development by the
team, Turbo has been proven stable and reliable in tens of thousands of applications.

turbo is the fundamental of `kumo`, it serves with no dependencies, stability is the most
import, secondary is performance.

It will carefully to add new feature. for example, the utf8 conversion, turbo's performance
is lower than [vamos](https://github.com/kumose/vamose), but it is stable, some case, we
do not need that high performance in production.

## 🛠️ Build

This project uses [kmpkg](https://github.com/kumose/kmcmake) for dependency management and build integration.
`kmpkg` automatically handles third-party library downloads, dependency resolution, and compiler flag configuration, avoiding the need to manually maintain complex CMake settings.


### 0. Prepare the environment

- Linux (Ubuntu 20.04+ / CentOS 7+ Recommended)
- CMake >= 3.20
- GCC >= 9.4 / Clang >= 12
- Make sure `kmpkg` is installed correctly, documents see [installation guide](https://kumo-pub.github.io/docs/category/%E6%8C%81%E7%BB%AD%E9%9B%86%E6%88%90----kmpkg)

### 1.Configure the project (optional)

* For the complete dependencies, refer to [`kmpkg.json`](kmpkg.json)
* To update the dependency baseline, modify the `baseline` in `default-registry` of [`kmpkg-configuration.json`](kmpkg-configuration.json)
* the `baseline` can be obtained via `git log`.


### 2. Build the project

Run in the project root directory:

```bash
cmake --preset=defualt
cmake --build build -j$(nproc)
```
#### Using Manual Dependency Management

If you manage dependencies yourself, you can build the project
with standard CMake commands:

```shell
mkdir build
cd build
cmake ..
make -j$(nproc)
```

**Note**

- `--preset=default` requires that the corresponding CMake preset is defined in the project root directory.
- When managing dependencies manually, make sure CMake’s find_package can locate all required libraries.

### 3. Run Tests (Optional)

Run in the project root directory:

```shell
ctest --test-dir build
```

## LICENSE

turbo has some code integrate from below:

* [Abseil](https://github.com/abseil/abseil-cpp) licensed by[Apache 2](licenses/abseil.lic)
* [Brpc](https://github.com/apache/brpc) licensed by[Apache 2](licenses/brpc.lic)
* [Folly](https://github.com/facebook/folly) licensed by[Apache 2](licenses/folly.lic)
* [EASTL](https://github.com/electronicarts/EASTL) licensed by[BSD 3](licenses/eastl.lic)
* [CLI11](https://github.com/CLIUtils/CLI11) licensed by[NEW BSD 3](licenses/cli11.lic)

Kumo offical release as [Apache 2](LICENSE).