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

#include <turbo/threading/thread_restrictions.h>

#if ENABLE_THREAD_RESTRICTIONS

#include <turbo/memory/lazy_instance.h>
#include <turbo/log/logging.h>
#include <turbo/threading/internal/thread_local.h>

namespace turbo {

    namespace {

        LazyInstance<ThreadLocalBoolean>::Leaky
                g_io_disallowed = LAZY_INSTANCE_INITIALIZER;

        LazyInstance<ThreadLocalBoolean>::Leaky
                g_singleton_disallowed = LAZY_INSTANCE_INITIALIZER;

        LazyInstance<ThreadLocalBoolean>::Leaky
                g_wait_disallowed = LAZY_INSTANCE_INITIALIZER;

    }  // anonymous namespace

    // static
    bool ThreadRestrictions::SetIOAllowed(bool allowed) {
        bool previous_disallowed = g_io_disallowed.Get().Get();
        g_io_disallowed.Get().Set(!allowed);
        return !previous_disallowed;
    }

    // static
    void ThreadRestrictions::AssertIOAllowed() {
        if (g_io_disallowed.Get().Get()) {
            KLOG(FATAL) <<
                       "Function marked as IO-only was called from a thread that "
                       "disallows IO!  If this thread really should be allowed to "
                       "make IO calls, adjust the call to "
                       "turbo::ThreadRestrictions::SetIOAllowed() in this thread's "
                       "startup.";
        }
    }

    // static
    bool ThreadRestrictions::SetSingletonAllowed(bool allowed) {
        bool previous_disallowed = g_singleton_disallowed.Get().Get();
        g_singleton_disallowed.Get().Set(!allowed);
        return !previous_disallowed;
    }

    // static
    void ThreadRestrictions::AssertSingletonAllowed() {
        if (g_singleton_disallowed.Get().Get()) {
            KLOG(FATAL) << "LazyInstance/Singleton is not allowed to be used on this "
                       << "thread.  Most likely it's because this thread is not "
                       << "joinable, so AtExitManager may have deleted the object "
                       << "on shutdown, leading to a potential shutdown crash.";
        }
    }

    // static
    void ThreadRestrictions::DisallowWaiting() {
        g_wait_disallowed.Get().Set(true);
    }

    // static
    void ThreadRestrictions::AssertWaitAllowed() {
        if (g_wait_disallowed.Get().Get()) {
            KLOG(FATAL) << "Waiting is not allowed to be used on this thread to prevent"
                       << "jank and deadlock.";
        }
    }

    bool ThreadRestrictions::SetWaitAllowed(bool allowed) {
        bool previous_disallowed = g_wait_disallowed.Get().Get();
        g_wait_disallowed.Get().Set(!allowed);
        return !previous_disallowed;
    }

}  // namespace turbo

#endif  // ENABLE_THREAD_RESTRICTIONS
