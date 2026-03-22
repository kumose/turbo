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
        NAME no_destructor_benchmark
        MODULE base
        SOURCES no_destructor_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME spinlock_benchmark
        MODULE base
        SOURCES spinlock_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME strerror_benchmark
        MODULE base
        SOURCES strerror_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME thread_identity_benchmark
        MODULE base
        SOURCES thread_identity_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)