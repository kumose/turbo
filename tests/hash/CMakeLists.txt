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

set(HASH_TEST_SRC
        city_test
        low_level_hash_test
)

foreach (test ${HASH_TEST_SRC})

    kmcmake_cc_test(
            NAME ${test}
            MODULE hash
            SOURCES ${test}.cc
            DEPS
            test_instance_tracker_static
            LINKS
            turbo::turbo
            test_instance_tracker_static
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME hash_test
        SOURCES hash_test.cc hash_test.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME hash_instantiated_test
        MODULE hash
        SOURCES hash_instantiated_test.cc
        DEPS
        hash_test_static
        LINKS
        turbo::turbo
        hash_test_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)