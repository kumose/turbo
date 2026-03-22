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
