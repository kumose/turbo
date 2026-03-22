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

#if defined(OS_WIN)
#include <windows.h>
#include <process.h>
#endif

#include <turbo/threading/simple_thread.h>
#include <turbo/threading/internal/thread_local_storage.h>
#include <gtest/gtest.h>

#if defined(OS_WIN)
// Ignore warnings about ptr->int conversions that we use when
// storing ints into ThreadLocalStorage.
#pragma warning(disable : 4311 4312)
#endif

namespace turbo {

    namespace {

        const int kInitialTlsValue = 0x5555;
        const int kFinalTlsValue = 0x7777;
// How many times must a destructor be called before we really are done.
        const int kNumberDestructorCallRepetitions = 3;

        static ThreadLocalStorage::StaticSlot tls_slot = TLS_INITIALIZER;

        class ThreadLocalStorageRunner : public DelegateSimpleThread::Delegate {
        public:
            explicit ThreadLocalStorageRunner(int *tls_value_ptr)
                    : tls_value_ptr_(tls_value_ptr) {}

            virtual ~ThreadLocalStorageRunner() {}

            virtual void Run() override {
                *tls_value_ptr_ = kInitialTlsValue;
                tls_slot.Set(tls_value_ptr_);

                int *ptr = static_cast<int *>(tls_slot.Get());
                EXPECT_EQ(ptr, tls_value_ptr_);
                EXPECT_EQ(*ptr, kInitialTlsValue);
                *tls_value_ptr_ = 0;

                ptr = static_cast<int *>(tls_slot.Get());
                EXPECT_EQ(ptr, tls_value_ptr_);
                EXPECT_EQ(*ptr, 0);

                *ptr = kFinalTlsValue + kNumberDestructorCallRepetitions;
            }

        private:
            int *tls_value_ptr_;
            TURBO_DISALLOW_COPY_AND_ASSIGN(ThreadLocalStorageRunner);
        };


        void ThreadLocalStorageCleanup(void *value) {
            int *ptr = reinterpret_cast<int *>(value);
            // Destructors should never be called with a nullptr.
            ASSERT_NE(reinterpret_cast<int *>(nullptr), ptr);
            if (*ptr == kFinalTlsValue)
                return;  // We've been called enough times.
            ASSERT_LT(kFinalTlsValue, *ptr);
            ASSERT_GE(kFinalTlsValue + kNumberDestructorCallRepetitions, *ptr);
            --*ptr;  // Move closer to our target.
            // Tell tls that we're not done with this thread, and still need destruction.
            tls_slot.Set(value);
        }

    }  // namespace

    TEST(ThreadLocalStorageTest, Basics) {
        ThreadLocalStorage::Slot slot;
        slot.Set(reinterpret_cast<void *>(123));
        int value = reinterpret_cast<intptr_t>(slot.Get());
        EXPECT_EQ(value, 123);
    }

#if defined(THREAD_SANITIZER)
    // Do not run the test under ThreadSanitizer. Because this test iterates its
    // own TSD destructor for the maximum possible number of times, TSan can't jump
    // in after the last destructor invocation, therefore the destructor remains
    // unsynchronized with the following users of the same TSD slot. This results
    // in race reports between the destructor and functions in other tests.
#define MAYBE_TLSDestructors DISABLED_TLSDestructors
#else
#define MAYBE_TLSDestructors TLSDestructors
#endif
    TEST(ThreadLocalStorageTest, MAYBE_TLSDestructors) {
        // Create a TLS index with a destructor.  Create a set of
        // threads that set the TLS, while the destructor cleans it up.
        // After the threads finish, verify that the value is cleaned up.
        const int kNumThreads = 5;
        int values[kNumThreads];
        ThreadLocalStorageRunner *thread_delegates[kNumThreads];
        DelegateSimpleThread *threads[kNumThreads];

        tls_slot.Initialize(ThreadLocalStorageCleanup);

        // Spawn the threads.
        for (int index = 0; index < kNumThreads; index++) {
            values[index] = kInitialTlsValue;
            thread_delegates[index] = new ThreadLocalStorageRunner(&values[index]);
            threads[index] = new DelegateSimpleThread(thread_delegates[index],
                                                      "tls thread");
            threads[index]->Start();
        }

        // Wait for the threads to finish.
        for (int index = 0; index < kNumThreads; index++) {
            threads[index]->Join();
            delete threads[index];
            delete thread_delegates[index];

            // Verify that the destructor was called and that we reset.
            EXPECT_EQ(values[index], kFinalTlsValue);
        }
        tls_slot.Free();  // Stop doing callbacks to cleanup threads.
    }

}  // namespace turbo
