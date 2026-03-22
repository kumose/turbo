// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <turbo/algorithm/algorithm.h>

#include <algorithm>
#include <list>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <turbo/base/macros.h>

namespace {

    class LinearSearchTest : public testing::Test {
    protected:
        LinearSearchTest() : container_{1, 2, 3} {}

        static bool Is3(int n) { return n == 3; }

        static bool Is4(int n) { return n == 4; }

        std::vector<int> container_;
    };

    TEST_F(LinearSearchTest, linear_search) {
        EXPECT_TRUE(turbo::linear_search(container_.begin(), container_.end(), 3));
        EXPECT_FALSE(turbo::linear_search(container_.begin(), container_.end(), 4));
    }

    TEST_F(LinearSearchTest, linear_searchConst) {
        const std::vector<int> *const const_container = &container_;
        EXPECT_TRUE(
                turbo::linear_search(const_container->begin(), const_container->end(), 3));
        EXPECT_FALSE(
                turbo::linear_search(const_container->begin(), const_container->end(), 4));
    }

    void ExpectSortPermutation(std::vector<std::string> unsorted,
                               std::vector<int64_t> expected_indices,
                               size_t expected_cycle_count) {
        auto actual_indices = turbo::arg_sort(unsorted);
        EXPECT_THAT(actual_indices, ::testing::ContainerEq(expected_indices));

        auto sorted = unsorted;
        std::sort(sorted.begin(), sorted.end());

        auto permuted = unsorted;
        EXPECT_EQ(turbo::permute(expected_indices, &permuted), expected_cycle_count);

        EXPECT_THAT(permuted, ::testing::ContainerEq(sorted));
    }


    TEST(StlUtilTest, ArgSortPermute) {
        std::string f = "foxtrot", a = "alpha", b = "bravo", d = "delta", c = "charlie",
                e = "echo";

        ExpectSortPermutation({a, f}, {0, 1}, 2);
        ExpectSortPermutation({f, a}, {1, 0}, 1);
        ExpectSortPermutation({a, b, c}, {0, 1, 2}, 3);
        ExpectSortPermutation({a, c, b}, {0, 2, 1}, 2);
        ExpectSortPermutation({c, a, b}, {1, 2, 0}, 1);
        ExpectSortPermutation({a, b, c, d, e, f}, {0, 1, 2, 3, 4, 5}, 6);
        ExpectSortPermutation({f, e, d, c, b, a}, {5, 4, 3, 2, 1, 0}, 3);
        ExpectSortPermutation({d, f, e, c, b, a}, {5, 4, 3, 0, 2, 1}, 1);
        ExpectSortPermutation({b, a, c, d, f, e}, {1, 0, 2, 3, 5, 4}, 4);
        ExpectSortPermutation({c, b, a, d, e, f}, {2, 1, 0, 3, 4, 5}, 5);
        ExpectSortPermutation({b, c, a, f, d, e}, {2, 0, 1, 4, 5, 3}, 2);
        ExpectSortPermutation({b, c, d, e, a, f}, {4, 0, 1, 2, 3, 5}, 2);
    }


}  // namespace
