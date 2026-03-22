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
        NAME hash_generator_testing
        SOURCES hash_generator_testing.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)


set(CONTAINER_SRC_LIST
        common_policy_traits_test
        fixed_array_test
        hash_function_defaults_test
        hash_policy_testing_test
        hash_policy_traits_test
        hashtablez_sampler_test
        layout_test
        node_hash_map_test
        node_slot_policy_test
        raw_hash_set_allocator_test
        raw_hash_set_test
        sample_element_size_test
        unordered_map_test
        unordered_set_test
        intrusive_list_test
        span_test
        cache_test
)

foreach (SRC ${CONTAINER_SRC_LIST})
    kmcmake_cc_test(
            NAME ${SRC}
            MODULE container
            SOURCES ${SRC}.cc
            LINKS
            turbo::turbo
            GTest::gtest
            GTest::gmock
            hash_generator_testing_static
            GTest::gmock_main
            ${KMCMAKE_DEPS_LINK}
            CXXOPTS ${KMCMAKE_CXX_OPTIONS}
    )
endforeach ()

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME btree_test_lib
        SOURCES btree_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test_library(
        NAMESPACE turbo
        NAME test_instance_tracker
        SOURCES test_instance_tracker.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)


kmcmake_cc_test(
        NAME node_hash_set_test
        MODULE container
        SOURCES node_hash_set_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        hash_generator_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME inlined_vector_test
        MODULE container
        SOURCES inlined_vector_test.cc
        DEPS
        exception_safety_testing_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        test_instance_tracker_static
        exception_safety_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME fixed_array_exception_safety_test
        MODULE container
        SOURCES fixed_array_exception_safety_test.cc
        DEPS
        exception_safety_testing_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        test_instance_tracker_static
        exception_safety_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME inlined_vector_exception_safety_test
        MODULE container
        SOURCES inlined_vector_exception_safety_test.cc
        DEPS
        exception_safety_testing_static
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        test_instance_tracker_static
        exception_safety_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME compressed_tuple_test
        MODULE container
        SOURCES compressed_tuple_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        test_instance_tracker_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME container_memory_test
        MODULE container
        SOURCES container_memory_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        test_instance_tracker_static
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)


kmcmake_cc_test(
        NAME flat_hash_map_test
        MODULE container
        SOURCES flat_hash_map_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        hash_generator_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)
kmcmake_cc_test(
        NAME test_instance_tracker_test
        MODULE container
        SOURCES test_instance_tracker_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        test_instance_tracker_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS}
)

kmcmake_cc_test(
        NAME flat_hash_set_test
        MODULE container
        SOURCES flat_hash_set_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        hash_generator_testing_static
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} "-DUNORDERED_SET_CXX17"
)

kmcmake_cc_test(
        NAME small_vector_test
        MODULE container
        SOURCES small_vector_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} "-DUNORDERED_SET_CXX17"
)

kmcmake_cc_test(
        NAME linked_list_test
        MODULE container
        SOURCES linked_list_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} "-DUNORDERED_SET_CXX17"
)

kmcmake_cc_test(
        NAME bounded_queue_test
        MODULE container
        SOURCES bounded_queue_test.cc
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${KMCMAKE_DEPS_LINK}
        CXXOPTS ${KMCMAKE_CXX_OPTIONS} "-DUNORDERED_SET_CXX17"
)

kmcmake_cc_test(
        NAME flat_map_test
        MODULE container
        SOURCES flat_map_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME hash_tables_test
        MODULE container
        SOURCES hash_tables_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME mpsc_queue_test
        MODULE container
        SOURCES mpsc_queue_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
