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
        NAME scoped_mock_log
        SOURCES scoped_mock_log.cc scoped_mock_log.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME test_actions
        SOURCES test_actions.cc test_actions.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME test_matchers
        SOURCES test_matchers.cc test_matchers.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME test_helpers
        SOURCES test_helpers.cc test_helpers.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

set(LOG_TEST_SRC
        check_test
        die_if_null_test
        flags_test
        fnmatch_test
        globals_test
        log_basic_test
        log_entry_test
        log_format_test
        log_macro_hygiene_test
        log_modifier_methods_test
        log_sink_test
        log_streamer_test
        scoped_mock_log_test
        stderr_log_sink_test
        stripping_test
        structured_test
        turbo_check_test
        turbo_log_basic_test
        vlog_is_on_test
)

foreach (test ${LOG_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE log
            SOURCES ${test}.cc
            LINKS
            turbo::turbo
            scoped_mock_log_static
            test_actions_static
            test_matchers_static
            test_helpers_static
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()
