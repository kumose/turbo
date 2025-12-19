#
# Copyright (C) 2024 EA group inc.
# Author: Jeff.li lijippy@163.com
# All rights reserved.
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#


carbin_cc_test_library(
        NAMESPACE turbo
        NAME status_matchers
        SOURCES status_matchers.h status_matchers.cc status_matchers_api.h
        LINKS
        turbo::turbo
        GTest::gtest
        GTest::gmock
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)

carbin_cc_test(
        NAME status_matchers_test
        MODULE utility
        SOURCES status_matchers_test.cc
        LINKS
        turbo::turbo
        status_matchers_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${CARBIN_DEPS_LINK}
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)

carbin_cc_test(
        NAME status_test
        MODULE utility
        SOURCES status_test.cc
        LINKS
        turbo::turbo
        status_matchers_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${CARBIN_DEPS_LINK}
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)

carbin_cc_test(
        NAME result_test
        MODULE utility
        SOURCES result_test.cc
        LINKS
        turbo::turbo
        status_matchers_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${CARBIN_DEPS_LINK}
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)

carbin_cc_test(
        NAME tdigest_test
        MODULE utility
        SOURCES tdigest_test.cc
        LINKS
        turbo::turbo
        status_matchers_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${CARBIN_DEPS_LINK}
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)
carbin_cc_test(
        NAME range_test
        MODULE utility
        SOURCES range_test.cc
        LINKS
        turbo::turbo
        status_matchers_static
        GTest::gtest
        GTest::gmock
        GTest::gmock_main
        ${CARBIN_DEPS_LINK}
        CXXOPTS ${CARBIN_CXX_OPTIONS}
)

carbin_cc_test(
        NAME guid_test
        MODULE utility
        SOURCES guid_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

carbin_cc_test(
        NAME errno_test
        MODULE utility
        SOURCES errno_test.cc
        CXXOPTS
        -fno-access-control
        LINKS
        GTest::gtest
        GTest::gtest_main
        turbo::turbo_static
        ${KMCMAKE_DEPS_LINK}
)

