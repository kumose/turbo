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

set(DEBUGGING_TEST_SRC
       demangle_test
        demangle_rust_test
        failure_signal_handler_test
        leak_check_fail_test
        leak_check_test
        stack_consumption_test
        stacktrace_test
        symbolize_test
)
foreach (test ${DEBUGGING_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE debugging
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