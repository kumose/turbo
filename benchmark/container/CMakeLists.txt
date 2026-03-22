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

kmcmake_cc_bm(
        NAME btree_benchmark
        MODULE container
        SOURCES btree_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME fixed_array_benchmark
        MODULE container
        SOURCES fixed_array_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME inlined_vector_benchmark
        MODULE container
        SOURCES inlined_vector_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME layout_benchmark
        MODULE container
        SOURCES layout_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME raw_hash_set_benchmark
        MODULE container
        SOURCES raw_hash_set_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME raw_hash_set_probe_benchmark
        MODULE container
        SOURCES raw_hash_set_probe_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)
kmcmake_cc_bm(
        NAME small_vector_benchmark
        MODULE container
        SOURCES small_vector_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)
