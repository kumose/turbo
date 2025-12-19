turbo * c++ 基础库
====

<div align="center">
<img src=docs/images/kumo_logo.svg width=240 height=200 />
</div>

Turbo 是基于c++17标准的c++基础库，是Kumo产品的基础设施。Turbo是团队在经过数年在搜索、服务端
开发积累的一套基础库，应用于数万个应用的稳定基础。

# 构建

* gcc/g++ >= 9.3
* cmake >=3.24.3
Turbo没有第三方依赖，直接下载编译即可。

```shell
git clone https://github.com/kumose/turbo.git
cd turbo
cmake --preset=kmpkg
cmake --build build
```

# 开发编译

```shell
kmpkg install gtest
kmpkg install benchmark
git clone https://github.com/kumose/turbo.git
cd turbo
mkdir build && cd build
cmake --preset=kmpkg -DCARBIN_BUILD_TEST=ON
cmake --build build
cd build && make test 
```
# Try Try Try

* Read [overview](https://turbo-docs.readthedocs.io/en/latest/en/overview.html) to know the goals of turbo and its advantages. 
* Read [getting start](https://turbo-docs.readthedocs.io/en/latest/) for building turbo and work with [examples](examples).
* Api reference is [here](https://turbo-docs.readthedocs.io/en/latest/en/api/base.html).
* Carbin is a tool to manage c++ project, it's [here](https://carbin.readthedocs.io/en/latest/).
* Modules:
  * platform compact cross platform
  * base basic types and functions
  * fiber & flow fiber and task control flow -- **good performance**
    * Fiber
    * FiberMutex
    * FiberCond
    * FiberBarrier
    * TaskFlow
  * simd simd instructions abstraction to batch processing -- **good performance**
  * flags cmd line tools help to parse cmd line
  * strings strings and std::string_view processing
  * concurrent threading and lock options
    * Barrier
    * CallOnce
    * SpinLock
    * ThreadLocal
  * times time and date processing -- **good performance**
    * Time
    * Duration
    * CivilTime
    * TimeZone
  * format  string format and table format -- **good performance**
    * format string
    * println
    * table format
  * logging logging and log to file -- **good performance**
  * files c++17 filesystem and convenient file api
    * SequentialReadFile
    * SequentialWriteFile
    * RandomAccessFile
    * RandomWriteFile
  * hash hash framework and hash functions -- **good performance**
    * city hash and bytes
    * murmur hash
    * xxhash
  * crypto
    * md5
    * sha1
    * sha256
    * sha512
    * crc32
  * Unicode multi engine unicode support scalar and AVX2 -- **good performance**
    * utf8
    * utf16
    * utf32
  * profiling profiling with write-most variables                 -- **good performance**
    * Counter
    * Histogram
    * Gauge
  * random random number generator                                -- **good performance**
    * Random
    * RandomEngine
    * RandomDevice
  * container                                                    -- **good performance**
    * small_vector stack allocated vector
    * flat_hash_map goode performance hash map 
    * flat_hash_set goode performance 
    * flat_tree_map goode performance 

# Acknowledgement

* [Turbo](github.com/abseil/abseil-cpp)
* [Folly](github.com/facebook/folly)
* [Onnxruntime](github.com/microsoft/onnxruntime)
* [brpc](github.com/apache/brpc)
* [tvm](github.com/apache/tvm)