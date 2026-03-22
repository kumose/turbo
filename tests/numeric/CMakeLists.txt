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
        NAME bits_test
        MODULE numeric
        SOURCES bits_test.cc
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
        NAME int128_stream_test
        MODULE numeric
        SOURCES int128_stream_test.cc
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
        NAME int128_test
        MODULE numeric
        SOURCES int128_test.cc
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
        NAME float16_test
        MODULE numeric
        SOURCES float16_test.cc
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