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
