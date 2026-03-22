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
        NAME atomic_hook_test
        MODULE base
        SOURCES atomic_hook_test.cc atomic_hook_test_helper.h atomic_hook_test_helper.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS
        ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME bit_cast_test
        MODULE base
        SOURCES bit_cast_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME call_once_test
        MODULE base
        SOURCES call_once_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME cmake_thread_test
        MODULE base
        SOURCES cmake_thread_test.cc
        LINKS
        turbo::turbo
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
set(BASE_SRC_LIST
        config_test
        endian_test
        errno_saver_test
        fast_type_id_test
        invoke_test
        log_severity_test
        low_level_alloc_test
        no_destructor_test
        nullability_test
        optimization_test
        prefetch_test
        raw_logging_test
        scoped_set_env_test
        spinlock_test_common
        strerror_test
        sysinfo_test
        thread_identity_test
        throw_delegate_test
        unique_small_name_test
)

foreach (SRC ${BASE_SRC_LIST})
    kmcmake_cc_test(
            NAME ${SRC}
            MODULE base
            SOURCES ${SRC}.cc
            LINKS
            turbo::turbo
            GTest::gtest
            GTest::gmock
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME exception_safety_testing
        SOURCES exception_safety_testing.cc
        DEPS  GTest::gtest GTest::gmock
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME exception_safety_testing_test
        MODULE base
        SOURCES exception_safety_testing_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        exception_safety_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME inline_variable_test_a
        SOURCES inline_variable_test_a.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME inline_variable_test_b
        SOURCES inline_variable_test_b.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME inline_variable_test
        MODULE base
        SOURCES inline_variable_test.cc
        LINKS
        turbo::turbo
        inline_variable_test_a_static
        inline_variable_test_b_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        exception_safety_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
kmcmake_cc_test(
        NAME checked_cast_test
        MODULE base
        SOURCES checked_cast_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME class_name_test
        MODULE base
        SOURCES class_name_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME cacheline_test
        MODULE base
        SOURCES cacheline_test.cc
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME endpoint_test
        MODULE base
        SOURCES endpoint_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME at_exit_test
        MODULE base
        SOURCES at_exit_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME bits_test
        MODULE base
        SOURCES bits_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME fd_guard_test
        MODULE base
        SOURCES fd_guard_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME popen_test
        MODULE base
        SOURCES popen_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME scoped_clear_errno_test
        MODULE base
        SOURCES scoped_clear_errno_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME scoped_generic_test
        MODULE base
        SOURCES scoped_generic_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME synchronous_event_test
        MODULE base
        SOURCES synchronous_event_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
