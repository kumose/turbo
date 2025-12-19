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
