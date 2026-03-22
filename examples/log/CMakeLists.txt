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

kmcmake_cc_binary(
        NAME daily_log
        SOURCES daily_log.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_binary(
        NAME color_log
        SOURCES color_log.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_binary(
        NAME hourly_log
        SOURCES hourly_log.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
kmcmake_cc_binary(
        NAME rotating_log
        SOURCES rotating_log.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_binary(
        NAME no_prefix
        SOURCES no_prefix.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
kmcmake_cc_binary(
        NAME json_log
        SOURCES json_log.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
find_package(glog REQUIRED)
find_package(gflags REQUIRED)
kmcmake_cc_binary(
        NAME glog_proxy
        SOURCES glog_proxy.cc
        LINKS turbo::turbo
        glog::glog
        gflags::gflags
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)