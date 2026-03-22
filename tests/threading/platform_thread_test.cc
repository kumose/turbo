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

#include <turbo/base/macros.h>
#include <turbo/threading/platform_thread.h>

#include <gtest/gtest.h>

namespace turbo {

// Trivial tests that thread runs and doesn't crash on create and join ---------

    class TrivialThread : public PlatformThread::Delegate {
    public:
        TrivialThread() : did_run_(false) {}

        virtual void ThreadMain() override {
            did_run_ = true;
        }

        bool did_run() const { return did_run_; }

    private:
        bool did_run_;

        TURBO_DISALLOW_COPY_AND_ASSIGN(TrivialThread);
    };

    TEST(PlatformThreadTest, Trivial) {
        TrivialThread thread;
        PlatformThreadHandle handle;

        ASSERT_FALSE(thread.did_run());
        ASSERT_TRUE(PlatformThread::Create(0, &thread, &handle));
        PlatformThread::Join(handle);
        ASSERT_TRUE(thread.did_run());
    }

    TEST(PlatformThreadTest, TrivialTimesTen) {
        TrivialThread thread[10];
        PlatformThreadHandle handle[TURBO_ARRAYSIZE(thread)];

        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            ASSERT_FALSE(thread[n].did_run());
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            ASSERT_TRUE(PlatformThread::Create(0, &thread[n], &handle[n]));
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            PlatformThread::Join(handle[n]);
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            ASSERT_TRUE(thread[n].did_run());
    }

    // Tests of basic thread functions ---------------------------------------------

    class FunctionTestThread : public TrivialThread {
    public:
        FunctionTestThread() : thread_id_(0) {}

        virtual void ThreadMain() override {
            thread_id_ = PlatformThread::CurrentId();
            PlatformThread::YieldCurrentThread();
            turbo::sleep_for(turbo::Duration::milliseconds(50));

            // Make sure that the thread ID is the same across calls.
            EXPECT_EQ(thread_id_, PlatformThread::CurrentId());

            TrivialThread::ThreadMain();
        }

        PlatformThreadId thread_id() const { return thread_id_; }

    private:
        PlatformThreadId thread_id_;

        TURBO_DISALLOW_COPY_AND_ASSIGN(FunctionTestThread);
    };

    TEST(PlatformThreadTest, Function) {
        PlatformThreadId main_thread_id = PlatformThread::CurrentId();

        FunctionTestThread thread;
        PlatformThreadHandle handle;

        ASSERT_FALSE(thread.did_run());
        ASSERT_TRUE(PlatformThread::Create(0, &thread, &handle));
        PlatformThread::Join(handle);
        ASSERT_TRUE(thread.did_run());
        EXPECT_NE(thread.thread_id(), main_thread_id);

        // Make sure that the thread ID is the same across calls.
        EXPECT_EQ(main_thread_id, PlatformThread::CurrentId());
    }

    TEST(PlatformThreadTest, FunctionTimesTen) {
        PlatformThreadId main_thread_id = PlatformThread::CurrentId();

        FunctionTestThread thread[10];
        PlatformThreadHandle handle[TURBO_ARRAYSIZE(thread)];

        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            ASSERT_FALSE(thread[n].did_run());
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            ASSERT_TRUE(PlatformThread::Create(0, &thread[n], &handle[n]));
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++)
            PlatformThread::Join(handle[n]);
        for (size_t n = 0; n < TURBO_ARRAYSIZE(thread); n++) {
            ASSERT_TRUE(thread[n].did_run());
            EXPECT_NE(thread[n].thread_id(), main_thread_id);

            // Make sure no two threads get the same ID.
            for (size_t i = 0; i < n; ++i) {
                EXPECT_NE(thread[i].thread_id(), thread[n].thread_id());
            }
        }

        // Make sure that the thread ID is the same across calls.
        EXPECT_EQ(main_thread_id, PlatformThread::CurrentId());
    }

}  // namespace turbo
