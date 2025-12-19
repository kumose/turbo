// Copyright (C) 2024 Kumo inc.
// Author: Jeff.li lijippy@163.com
// All rights reserved.
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//


#include <turbo/container/hash_tables.h>
#include <gtest/gtest.h>

namespace {

    class HashPairTest : public testing::Test {
    };

#define INSERT_PAIR_TEST(Type, value1, value2) \
  { \
    Type pair(value1, value2); \
    turbo::hash_map<Type, int> map; \
    map[pair] = 1; \
  }

// Verify that a hash_map can be constructed for pairs of integers of various
// sizes.
    TEST_F(HashPairTest, IntegerPairs) {
        typedef std::pair<int16_t, int16_t> Int16Int16Pair;
        typedef std::pair<int16_t, int32_t> Int16Int32Pair;
        typedef std::pair<int16_t, int64_t> Int16Int64Pair;

        INSERT_PAIR_TEST(Int16Int16Pair, 4, 6);
        INSERT_PAIR_TEST(Int16Int32Pair, 9, (1 << 29) + 378128932);
        INSERT_PAIR_TEST(Int16Int64Pair, 10,
                         (int64_t(1) << 60) + 78931732321LL);

        typedef std::pair<int32_t, int16_t> Int32Int16Pair;
        typedef std::pair<int32_t, int32_t> Int32Int32Pair;
        typedef std::pair<int32_t, int64_t> Int32Int64Pair;

        INSERT_PAIR_TEST(Int32Int16Pair, 4, 6);
        INSERT_PAIR_TEST(Int32Int32Pair, 9, (1 << 29) + 378128932);
        INSERT_PAIR_TEST(Int32Int64Pair, 10,
                         (1LL << 60) + 78931732321LL);

        typedef std::pair<int64_t, int16_t> Int64Int16Pair;
        typedef std::pair<int64_t, int32_t> Int64Int32Pair;
        typedef std::pair<int64_t, int64_t> Int64Int64Pair;

        INSERT_PAIR_TEST(Int64Int16Pair, 4, 6);
        INSERT_PAIR_TEST(Int64Int32Pair, 9, (1 << 29) + 378128932);
        INSERT_PAIR_TEST(Int64Int64Pair, 10,
                         (1LL << 60) + 78931732321LL);
    }

}  // namespace
