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

set(FLAGS_TEST_SRC
        commandlineflag_test
        config_test
        marshalling_test
        parse_test
        path_util_test
        program_name_test
        reflection_test
        sequence_lock_test
        usage_test
        usage_config_test
        flags_validate_test
)
foreach (test ${FLAGS_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE flags
            SOURCES ${test}.cc
            LINKS
            turbo::turbo
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()
kmcmake_cc_test(
        NAME flag_test
        MODULE flags
        SOURCES flag_test.cc flag_test_defs.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)