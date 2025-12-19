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

#pragma once

#include <turbo/base/macros.h>
// See comment at top of thread_checker.h
#if (!defined(NDEBUG) || defined(DCHECK_ALWAYS_ON))
#define ENABLE_THREAD_RESTRICTIONS 1
#else
#define ENABLE_THREAD_RESTRICTIONS 0
#endif

class AcceleratedPresenter;

class BrowserProcessImpl;

class HistogramSynchronizer;

class MetricsService;

class NativeBackendKWallet;

class ScopedAllowWaitForLegacyWebViewApi;

class TestingAutomationProvider;


namespace turbo {

    class SequencedWorkerPool;

    class SimpleThread;

    class Thread;

    class ThreadTestHelper;

    // Certain behavior is disallowed on certain threads.  ThreadRestrictions helps
    // enforce these rules.  Examples of such rules:
    //
    // * Do not do blocking IO (makes the thread janky)
    // * Do not access Singleton/LazyInstance (may lead to shutdown crashes)
    //
    // Here's more about how the protection works:
    //
    // 1) If a thread should not be allowed to make IO calls, mark it:
    //      turbo::ThreadRestrictions::SetIOAllowed(false);
    //    By default, threads *are* allowed to make IO calls.
    //    In Chrome browser code, IO calls should be proxied to the File thread.
    //
    // 2) If a function makes a call that will go out to disk, check whether the
    //    current thread is allowed:
    //      turbo::ThreadRestrictions::AssertIOAllowed();
    //
    //
    // Style tip: where should you put AssertIOAllowed checks?  It's best
    // if you put them as close to the disk access as possible, at the
    // lowest level.  This rule is simple to follow and helps catch all
    // callers.  For example, if your function GoDoSomeBlockingDiskCall()
    // only calls other functions in Chrome and not fopen(), you should go
    // add the AssertIOAllowed checks in the helper functions.

    class TURBO_EXPORT ThreadRestrictions {
    public:
        // Constructing a ScopedAllowIO temporarily allows IO for the current
        // thread.  Doing this is almost certainly always incorrect.
        class TURBO_EXPORT ScopedAllowIO {
        public:
            ScopedAllowIO() { previous_value_ = SetIOAllowed(true); }

            ~ScopedAllowIO() { SetIOAllowed(previous_value_); }

        private:
            // Whether IO is allowed when the ScopedAllowIO was constructed.
            bool previous_value_;

            TURBO_DISALLOW_COPY_AND_ASSIGN(ScopedAllowIO);
        };

        // Constructing a ScopedAllowSingleton temporarily allows accessing for the
        // current thread.  Doing this is almost always incorrect.
        class TURBO_EXPORT ScopedAllowSingleton {
        public:
            ScopedAllowSingleton() { previous_value_ = SetSingletonAllowed(true); }

            ~ScopedAllowSingleton() { SetSingletonAllowed(previous_value_); }

        private:
            // Whether singleton use is allowed when the ScopedAllowSingleton was
            // constructed.
            bool previous_value_;

            TURBO_DISALLOW_COPY_AND_ASSIGN(ScopedAllowSingleton);
        };

#if ENABLE_THREAD_RESTRICTIONS

        // Set whether the current thread to make IO calls.
        // Threads start out in the *allowed* state.
        // Returns the previous value.
        static bool SetIOAllowed(bool allowed);

        // Check whether the current thread is allowed to make IO calls,
        // and DKCHECK if not.  See the block comment above the class for
        // a discussion of where to add these checks.
        static void AssertIOAllowed();

        // Set whether the current thread can use singletons.  Returns the previous
        // value.
        static bool SetSingletonAllowed(bool allowed);

        // Check whether the current thread is allowed to use singletons (Singleton /
        // LazyInstance).  DCHECKs if not.
        static void AssertSingletonAllowed();

        // Disable waiting on the current thread. Threads start out in the *allowed*
        // state. Returns the previous value.
        static void DisallowWaiting();

        // Check whether the current thread is allowed to wait, and DKCHECK if not.
        static void AssertWaitAllowed();

#else
        // Inline the empty definitions of these functions so that they can be
        // compiled out.
        static bool SetIOAllowed(bool) { return true; }
        static void AssertIOAllowed() {}
        static bool SetSingletonAllowed(bool) { return true; }
        static void AssertSingletonAllowed() {}
        static void DisallowWaiting() {}
        static void AssertWaitAllowed() {}
#endif

    private:
        // DO NOT ADD ANY OTHER FRIEND STATEMENTS, talk to jam or brettw first.
        // BEGIN ALLOWED USAGE.

        friend class ::HistogramSynchronizer;

        friend class ::ScopedAllowWaitForLegacyWebViewApi;

        friend class ::TestingAutomationProvider;

        friend class MessagePumpDefault;

        friend class SequencedWorkerPool;

        friend class SimpleThread;

        friend class Thread;

        friend class ThreadTestHelper;

        friend class PlatformThread;

        friend class ::AcceleratedPresenter;            // http://crbug.com/125391
        friend class ::BrowserProcessImpl;              // http://crbug.com/125207
        friend class ::MetricsService;                  // http://crbug.com/124954
        friend class ::NativeBackendKWallet;            // http://crbug.com/125331
        // END USAGE THAT NEEDS TO BE FIXED.

#if ENABLE_THREAD_RESTRICTIONS

        static bool SetWaitAllowed(bool allowed);

#else
        static bool SetWaitAllowed(bool) { return true; }
#endif

        // Constructing a ScopedAllowWait temporarily allows waiting on the current
        // thread.  Doing this is almost always incorrect, which is why we limit who
        // can use this through friend. If you find yourself needing to use this, find
        // another way. Talk to jam or brettw.
        class TURBO_EXPORT ScopedAllowWait {
        public:
            ScopedAllowWait() { previous_value_ = SetWaitAllowed(true); }

            ~ScopedAllowWait() { SetWaitAllowed(previous_value_); }

        private:
            // Whether singleton use is allowed when the ScopedAllowWait was
            // constructed.
            bool previous_value_;

            TURBO_DISALLOW_COPY_AND_ASSIGN(ScopedAllowWait);
        };

    private:
        TURBO_DISALLOW_IMPLICIT_CONSTRUCTORS(ThreadRestrictions);
    };

}  // namespace turbo
