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
