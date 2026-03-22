#
# Copyright 2023 The Kumo Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

kmcmake_cc_test(
        NAME thread_local_test
        MODULE threading 
        SOURCES thread_local_test.cc
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)




kmcmake_cc_test(
        NAME lazy_instance_test
        MODULE threading 
        SOURCES lazy_instance_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)



kmcmake_cc_test(
        NAME platform_thread_test
        MODULE threading 
        SOURCES platform_thread_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME simple_thread_test
        MODULE threading 
        SOURCES simple_thread_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME thread_collision_warner_test
        MODULE threading 
        SOURCES thread_collision_warner_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)


kmcmake_cc_test(
        NAME thread_id_name_manager_test
        MODULE threading 
        SOURCES thread_id_name_manager_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME internal_thread_local_test
        MODULE threading 
        SOURCES internal_thread_local_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

kmcmake_cc_test(
        NAME waitable_event_test
        MODULE threading 
        SOURCES waitable_event_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)
