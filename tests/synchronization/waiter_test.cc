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

#include <turbo/synchronization/internal/waiter.h>

#include <ctime>
#include <iostream>
#include <ostream>
#include <turbo/base/macros.h>
#include <turbo/random/random.h>
#include <turbo/synchronization/internal/create_thread_identity.h>
#include <turbo/synchronization/internal/futex_waiter.h>
#include <turbo/synchronization/internal/kernel_timeout.h>
#include <turbo/synchronization/internal/pthread_waiter.h>
#include <turbo/synchronization/internal/sem_waiter.h>
#include <turbo/synchronization/internal/stdcpp_waiter.h>
#include <turbo/synchronization/internal/thread_pool.h>
#include <turbo/synchronization/internal/win32_waiter.h>
#include <turbo/times/clock.h>
#include <turbo/times/time.h>
#include <gtest/gtest.h>

// Test go/btm support by randomizing the value of clock_gettime() for
// CLOCK_MONOTONIC. This works by overriding a weak symbol in glibc.
// We should be resistant to this randomization when !SupportsSteadyClock().
#if defined(__GOOGLE_GRTE_VERSION__) && \
    !defined(TURBO_HAVE_ADDRESS_SANITIZER) && \
    !defined(TURBO_HAVE_MEMORY_SANITIZER) && \
    !defined(TURBO_HAVE_THREAD_SANITIZER)
extern "C" int __clock_gettime(clockid_t c, struct timespec* ts);

extern "C" int clock_gettime(clockid_t c, struct timespec* ts) {
  if (c == CLOCK_MONOTONIC &&
      !turbo::synchronization_internal::KernelTimeout::SupportsSteadyClock()) {
    thread_local turbo::BitGen gen;  // NOLINT
    ts->tv_sec = turbo::Uniform(gen, 0, 1'000'000'000);
    ts->tv_nsec = turbo::Uniform(gen, 0, 1'000'000'000);
    return 0;
  }
  return __clock_gettime(c, ts);
}
#endif

namespace {

    TEST(Waiter, PrintPlatformImplementation) {
        // Allows us to verify that the platform is using the expected implementation.
        std::cout << turbo::synchronization_internal::Waiter::kName << std::endl;
    }

    template<typename T>
    class WaiterTest : public ::testing::Test {
    public:
        // Waiter implementations assume that a ThreadIdentity has already been
        // created.
        WaiterTest() {
            turbo::synchronization_internal::GetOrCreateCurrentThreadIdentity();
        }
    };

    TYPED_TEST_SUITE_P(WaiterTest);

    turbo::Duration WithTolerance(turbo::Duration d) { return d * 0.95; }

    TYPED_TEST_P(WaiterTest, WaitNoTimeout) {
        turbo::synchronization_internal::ThreadPool tp(1);
        TypeParam waiter;
        tp.Schedule([&]() {
            // Include some `Poke()` calls to ensure they don't cause `waiter` to return
            // from `Wait()`.
            waiter.Poke();
            turbo::sleep_for(turbo::Duration::seconds(1));
            waiter.Poke();
            turbo::sleep_for(turbo::Duration::seconds(1));
            waiter.Post();
        });
        turbo::Time start = turbo::Time::current_time();
        EXPECT_TRUE(
                waiter.Wait(turbo::synchronization_internal::KernelTimeout::Never()));
        turbo::Duration waited = turbo::Time::current_time() - start;
        EXPECT_GE(waited, WithTolerance(turbo::Duration::seconds(2)));
    }

    TYPED_TEST_P(WaiterTest, WaitDurationWoken) {
        turbo::synchronization_internal::ThreadPool tp(1);
        TypeParam waiter;
        tp.Schedule([&]() {
            // Include some `Poke()` calls to ensure they don't cause `waiter` to return
            // from `Wait()`.
            waiter.Poke();
            turbo::sleep_for(turbo::Duration::milliseconds(500));
            waiter.Post();
        });
        turbo::Time start = turbo::Time::current_time();
        EXPECT_TRUE(waiter.Wait(
                turbo::synchronization_internal::KernelTimeout(turbo::Duration::seconds(10))));
        turbo::Duration waited = turbo::Time::current_time() - start;
        EXPECT_GE(waited, WithTolerance(turbo::Duration::milliseconds(500)));
        EXPECT_LT(waited, turbo::Duration::seconds(2));
    }

    TYPED_TEST_P(WaiterTest, WaitTimeWoken) {
        turbo::synchronization_internal::ThreadPool tp(1);
        TypeParam waiter;
        tp.Schedule([&]() {
            // Include some `Poke()` calls to ensure they don't cause `waiter` to return
            // from `Wait()`.
            waiter.Poke();
            turbo::sleep_for(turbo::Duration::milliseconds(500));
            waiter.Post();
        });
        turbo::Time start = turbo::Time::current_time();
        EXPECT_TRUE(waiter.Wait(turbo::synchronization_internal::KernelTimeout(
                start + turbo::Duration::seconds(10))));
        turbo::Duration waited = turbo::Time::current_time() - start;
        EXPECT_GE(waited, WithTolerance(turbo::Duration::milliseconds(500)));
        EXPECT_LT(waited, turbo::Duration::seconds(2));
    }

    TYPED_TEST_P(WaiterTest, WaitDurationReached) {
        TypeParam waiter;
        turbo::Time start = turbo::Time::current_time();
        EXPECT_FALSE(waiter.Wait(
                turbo::synchronization_internal::KernelTimeout(turbo::Duration::milliseconds(500))));
        turbo::Duration waited = turbo::Time::current_time() - start;
        EXPECT_GE(waited, WithTolerance(turbo::Duration::milliseconds(500)));
        EXPECT_LT(waited, turbo::Duration::seconds(1));
    }

    TYPED_TEST_P(WaiterTest, WaitTimeReached) {
        TypeParam waiter;
        turbo::Time start = turbo::Time::current_time();
        EXPECT_FALSE(waiter.Wait(turbo::synchronization_internal::KernelTimeout(
                start + turbo::Duration::milliseconds(500))));
        turbo::Duration waited = turbo::Time::current_time() - start;
        EXPECT_GE(waited, WithTolerance(turbo::Duration::milliseconds(500)));
        EXPECT_LT(waited, turbo::Duration::seconds(1));
    }

    REGISTER_TYPED_TEST_SUITE_P(WaiterTest,
                                WaitNoTimeout,
                                WaitDurationWoken,
                                WaitTimeWoken,
                                WaitDurationReached,
                                WaitTimeReached);

#ifdef TURBO_INTERNAL_HAVE_FUTEX_WAITER
    INSTANTIATE_TYPED_TEST_SUITE_P(Futex, WaiterTest,
                                   turbo::synchronization_internal::FutexWaiter);
#endif
#ifdef TURBO_INTERNAL_HAVE_PTHREAD_WAITER
    INSTANTIATE_TYPED_TEST_SUITE_P(Pthread, WaiterTest,
                                   turbo::synchronization_internal::PthreadWaiter);
#endif
#ifdef TURBO_INTERNAL_HAVE_SEM_WAITER
    INSTANTIATE_TYPED_TEST_SUITE_P(Sem, WaiterTest,
                                   turbo::synchronization_internal::SemWaiter);
#endif
#ifdef TURBO_INTERNAL_HAVE_WIN32_WAITER
    INSTANTIATE_TYPED_TEST_SUITE_P(Win32, WaiterTest,
                                   turbo::synchronization_internal::Win32Waiter);
#endif
#ifdef TURBO_INTERNAL_HAVE_STDCPP_WAITER
    INSTANTIATE_TYPED_TEST_SUITE_P(Stdcpp, WaiterTest,
                                   turbo::synchronization_internal::StdcppWaiter);
#endif

}  // namespace
