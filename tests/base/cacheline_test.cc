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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <gtest/gtest.h>
#include <turbo/base/macros.h>

namespace {

class CachelineTest : public testing::Test {
};

struct TURBO_CACHELINE_ALIGNED Bar {
    int y;
};

struct Foo {
    char dummy1[0];
    int z;
    TURBO_CACHELINE_ALIGNED int x[0];
    int y;
    int m;
    Bar bar;
};

TEST_F(CachelineTest, cacheline_alignment) {
    ASSERT_EQ(64u, offsetof(Foo, x));
    ASSERT_EQ(64u, offsetof(Foo, y));
    ASSERT_EQ(68u, offsetof(Foo, m));
    ASSERT_EQ(128u, offsetof(Foo, bar));
    ASSERT_EQ(64u, sizeof(Bar));
    ASSERT_EQ(192u, sizeof(Foo));
}

}
