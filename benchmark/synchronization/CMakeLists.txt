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
        NAME blocking_counter_benchmark
        MODULE synchronization
        SOURCES blocking_counter_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME graphcycles_benchmark
        MODULE synchronization
        SOURCES graphcycles_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME mutex_benchmark
        MODULE synchronization
        SOURCES mutex_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)