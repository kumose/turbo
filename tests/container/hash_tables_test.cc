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
