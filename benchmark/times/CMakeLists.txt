# Copyright (C) Kumo inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME bm_test_util
        SOURCES ${PROJECT_SOURCE_DIR}/tests/times/test_util.cc ${PROJECT_SOURCE_DIR}/tests/times/test_util.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_bm(
        NAME cctz_benchmark
        MODULE times
        SOURCES cctz_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME civil_time_benchmark
        MODULE times
        SOURCES civil_time_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME clock_benchmark
        MODULE times
        SOURCES clock_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)
kmcmake_cc_bm(
        NAME duration_benchmark
        MODULE times
        SOURCES duration_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME format_benchmark
        MODULE times
        SOURCES format_benchmark.cc
        LINKS
        turbo::turbo bm_test_util_static
        benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME time_benchmark
        MODULE times
        SOURCES time_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark bm_test_util_static benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)