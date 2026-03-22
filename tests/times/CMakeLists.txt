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
        NAME time_test_util
        SOURCES test_util.cc test_util.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)


set(TIMES_TEST_SRC
        cctz_time_test
        civil_time_test
        clock_test
        duration_test
        flag_test
        format_test
        time_test
        time_zone_test
        time_zone_lookup_test
        time_zone_format_test
)
foreach (test ${TIMES_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE times
            SOURCES ${test}.cc
            LINKS
            turbo::turbo
            time_test_util_static
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()