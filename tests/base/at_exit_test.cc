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

#include <turbo/base/at_exit.h>
#include <tests/testing/test_util.h>
#include <gtest/gtest.h>


namespace {

    int g_test_counter_1 = 0;
    int g_test_counter_2 = 0;

    void IncrementTestCounter1(void *) {
        ++g_test_counter_1;
    }

    void IncrementTestCounter2(void *) {
        ++g_test_counter_2;
    }

    void ZeroTestCounters() {
        g_test_counter_1 = 0;
        g_test_counter_2 = 0;
    }

    void ExpectCounter1IsZero(void *unused) {
        EXPECT_EQ(0, g_test_counter_1);
    }

    void ExpectParamIsNull(void *param) {
        EXPECT_EQ(static_cast<void *>(nullptr), param);
    }

    void ExpectParamIsCounter(void *param) {
        EXPECT_EQ(&g_test_counter_1, param);
    }

}  // namespace

class AtExitTest : public testing::Test {
private:
    // Don't test the global AtExitManager, because asking it to process its
    // AtExit callbacks can ruin the global state that other tests may depend on.
    turbo::ShadowingAtExitManager exit_manager_;
};

TEST_F(AtExitTest, Basic) {
    ZeroTestCounters();
    turbo::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
    turbo::AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);
    turbo::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);

    EXPECT_EQ(0, g_test_counter_1);
    EXPECT_EQ(0, g_test_counter_2);
    turbo::AtExitManager::ProcessCallbacksNow();
    EXPECT_EQ(2, g_test_counter_1);
    EXPECT_EQ(1, g_test_counter_2);
}

TEST_F(AtExitTest, LIFOOrder) {
    ZeroTestCounters();
    turbo::AtExitManager::RegisterCallback(&IncrementTestCounter1, nullptr);
    turbo::AtExitManager::RegisterCallback(&ExpectCounter1IsZero, nullptr);
    turbo::AtExitManager::RegisterCallback(&IncrementTestCounter2, nullptr);

    EXPECT_EQ(0, g_test_counter_1);
    EXPECT_EQ(0, g_test_counter_2);
    turbo::AtExitManager::ProcessCallbacksNow();
    EXPECT_EQ(1, g_test_counter_1);
    EXPECT_EQ(1, g_test_counter_2);
}

TEST_F(AtExitTest, Param) {
    turbo::AtExitManager::RegisterCallback(&ExpectParamIsNull, nullptr);
    turbo::AtExitManager::RegisterCallback(&ExpectParamIsCounter,
                                                  &g_test_counter_1);
    turbo::AtExitManager::ProcessCallbacksNow();
}
