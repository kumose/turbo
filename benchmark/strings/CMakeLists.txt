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
        NAME ascii_benchmark
        MODULE strings
        SOURCES ascii_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME charconv_benchmark
        MODULE strings
        SOURCES charconv_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME charset_benchmark
        MODULE strings
        SOURCES charset_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME escaping_benchmark
        MODULE strings
        SOURCES escaping_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME memutil_benchmark
        MODULE strings
        SOURCES memutil_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME numbers_benchmark
        MODULE strings
        SOURCES numbers_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME ostringstream_benchmark
        MODULE strings
        SOURCES ostringstream_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME str_cat_benchmark
        MODULE strings
        SOURCES str_cat_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME str_join_benchmark
        MODULE strings
        SOURCES str_join_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME str_replace_benchmark
        MODULE strings
        SOURCES str_replace_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME str_split_benchmark
        MODULE strings
        SOURCES str_split_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)

kmcmake_cc_bm(
        NAME string_view_benchmark
        MODULE strings
        SOURCES string_view_benchmark.cc
        LINKS turbo::turbo benchmark::benchmark benchmark::benchmark_main ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${USER_CXX_FLAGS}
)