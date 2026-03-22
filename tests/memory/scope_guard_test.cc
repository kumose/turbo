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


#include <gtest/gtest.h>
#include <turbo/memory/scope_guard.h>

TEST(ScopedGuardTest, sanity) {
    bool flag = false;
    {
        auto guard = turbo::MakeScopeGuard([&flag] {
            flag = true;
        });
    }
    ASSERT_TRUE(flag);

    flag = false;
    {
        TURBO_SCOPE_EXIT {
            flag = true;
        };
    }
    ASSERT_TRUE(flag);

    {
        TURBO_SCOPE_EXIT {
            flag = true;
        };

        TURBO_SCOPE_EXIT {
            flag = false;
        };
    }
    ASSERT_TRUE(flag);

    flag = false;
    {
        auto guard = turbo::MakeScopeGuard([&flag] {
            flag = true;
        });
        guard.dismiss();
    }
    ASSERT_FALSE(flag);
}
