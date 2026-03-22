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
        NAME validator_ex
        SOURCES validator.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_binary(
        NAME flag_file
        SOURCES flag_file.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_binary(
        NAME simple
        SOURCES simple.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
kmcmake_cc_binary(
        NAME custom_parse
        SOURCES custom_parse.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)


kmcmake_cc_binary(
        NAME positional_arity
        SOURCES positional_arity.cc
        LINKS turbo::turbo
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
