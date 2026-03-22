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


kmcmake_cc_test(
        NAME memory_test
        MODULE memory
        SOURCES memory_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME aligned_memory_test
        MODULE memory
        SOURCES aligned_memory_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME object_pool_test
        MODULE memory
        SOURCES object_pool_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME ref_counted_test
        MODULE memory
        SOURCES ref_counted_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME resource_pool_test
        MODULE memory
        SOURCES resource_pool_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME scope_guard_test
        MODULE memory
        SOURCES scope_guard_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME singleton_test
        MODULE memory
        SOURCES singleton_test.cc
        DEPS
        test_instance_tracker_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)