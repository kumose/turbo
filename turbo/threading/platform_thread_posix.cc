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

#include <turbo/threading/platform_thread.h>

#include <errno.h>
#include <sched.h>
#include <turbo/log/logging.h>
#include <turbo/base/internal/safe_strerror_posix.h>
#include <turbo/threading/waitable_event.h>
#include <turbo/threading/thread_id_name_manager.h>
#include <turbo/threading/thread_restrictions.h>

#if defined(OS_MACOSX)
#include <sys/resource.h>
#include <algorithm>
#endif

#if defined(OS_LINUX)

#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <unistd.h>

#endif

namespace turbo {

    void InitThreading();

    void InitOnThread();

    void TerminateOnThread();

    size_t GetDefaultThreadStackSize(const pthread_attr_t &attributes);

    namespace {

        struct ThreadParams {
            ThreadParams()
                    : delegate(nullptr),
                      joinable(false),
                      priority(kThreadPriority_Normal),
                      handle(nullptr),
                      handle_set(false, false) {
            }

            PlatformThread::Delegate *delegate;
            bool joinable;
            ThreadPriority priority;
            PlatformThreadHandle *handle;
            WaitableEvent handle_set;
        };

        void *ThreadFunc(void *params) {
            turbo::InitOnThread();
            ThreadParams *thread_params = static_cast<ThreadParams *>(params);

            PlatformThread::Delegate *delegate = thread_params->delegate;
            if (!thread_params->joinable)
                turbo::ThreadRestrictions::SetSingletonAllowed(false);

            if (thread_params->priority != kThreadPriority_Normal) {
                PlatformThread::SetThreadPriority(PlatformThread::CurrentHandle(),
                                                  thread_params->priority);
            }

            // Stash the id in the handle so the calling thread has a complete
            // handle, and unblock the parent thread.
            *(thread_params->handle) = PlatformThreadHandle(pthread_self(),
                                                            PlatformThread::CurrentId());
            thread_params->handle_set.Signal();

            ThreadIdNameManager::GetInstance()->RegisterThread(
                    PlatformThread::CurrentHandle().platform_handle(),
                    PlatformThread::CurrentId());

            delegate->ThreadMain();

            ThreadIdNameManager::GetInstance()->RemoveName(
                    PlatformThread::CurrentHandle().platform_handle(),
                    PlatformThread::CurrentId());

            turbo::TerminateOnThread();
            return nullptr;
        }

        bool CreateThread(size_t stack_size, bool joinable,
                          PlatformThread::Delegate *delegate,
                          PlatformThreadHandle *thread_handle,
                          ThreadPriority priority) {
            turbo::InitThreading();

            bool success = false;
            pthread_attr_t attributes;
            pthread_attr_init(&attributes);

            // Pthreads are joinable by default, so only specify the detached
            // attribute if the thread should be non-joinable.
            if (!joinable) {
                pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);
            }

            // Get a better default if available.
            if (stack_size == 0)
                stack_size = turbo::GetDefaultThreadStackSize(attributes);

            if (stack_size > 0)
                pthread_attr_setstacksize(&attributes, stack_size);

            ThreadParams params;
            params.delegate = delegate;
            params.joinable = joinable;
            params.priority = priority;
            params.handle = thread_handle;

            pthread_t handle;
            int err = pthread_create(&handle,
                                     &attributes,
                                     ThreadFunc,
                                     &params);
            success = !err;
            if (!success) {
                // Value of |handle| is undefined if pthread_create fails.
                handle = 0;
                errno = err;
                PKLOG(ERROR) << "pthread_create";
            }

            pthread_attr_destroy(&attributes);

            // Don't let this call complete until the thread id
            // is set in the handle.
            if (success)
                params.handle_set.Wait();
            KCHECK_EQ(handle, thread_handle->platform_handle());

            return success;
        }

    }  // namespace

    // static
    PlatformThreadId PlatformThread::CurrentId() {
        // Pthreads doesn't have the concept of a thread ID, so we have to reach down
        // into the kernel.
#if defined(OS_MACOSX)
        return pthread_mach_thread_np(pthread_self());
#elif defined(OS_LINUX)
        return syscall(__NR_gettid);
#elif defined(OS_ANDROID)
        return gettid();
#elif defined(OS_SOLARIS) || defined(OS_QNX)
        return pthread_self();
#elif defined(OS_NACL) && defined(__GLIBC__)
        return pthread_self();
#elif defined(OS_NACL) && !defined(__GLIBC__)
        // Pointers are 32-bits in NaCl.
        return reinterpret_cast<int32_t>(pthread_self());
#elif defined(OS_POSIX)
        return reinterpret_cast<int64_t>(pthread_self());
#endif
    }

    // static
    PlatformThreadRef PlatformThread::CurrentRef() {
        return PlatformThreadRef(pthread_self());
    }

    // static
    PlatformThreadHandle PlatformThread::CurrentHandle() {
        return PlatformThreadHandle(pthread_self(), CurrentId());
    }

    // static
    void PlatformThread::YieldCurrentThread() {
        sched_yield();
    }

    // static
    const char *PlatformThread::GetName() {
        return ThreadIdNameManager::GetInstance()->GetName(CurrentId());
    }

    // static
    bool PlatformThread::Create(size_t stack_size, Delegate *delegate,
                                PlatformThreadHandle *thread_handle) {
        turbo::ThreadRestrictions::ScopedAllowWait allow_wait;
        return CreateThread(stack_size, true /* joinable thread */,
                            delegate, thread_handle, kThreadPriority_Normal);
    }

    // static
    bool PlatformThread::CreateWithPriority(size_t stack_size, Delegate *delegate,
                                            PlatformThreadHandle *thread_handle,
                                            ThreadPriority priority) {
        turbo::ThreadRestrictions::ScopedAllowWait allow_wait;
        return CreateThread(stack_size, true,  // joinable thread
                            delegate, thread_handle, priority);
    }

// static
    bool PlatformThread::CreateNonJoinable(size_t stack_size, Delegate *delegate) {
        PlatformThreadHandle unused;

        turbo::ThreadRestrictions::ScopedAllowWait allow_wait;
        bool result = CreateThread(stack_size, false /* non-joinable thread */,
                                   delegate, &unused, kThreadPriority_Normal);
        return result;
    }

// static
    void PlatformThread::Join(PlatformThreadHandle thread_handle) {
        // Joining another thread may block the current thread for a long time, since
        // the thread referred to by |thread_handle| may still be running long-lived /
        // blocking tasks.
        turbo::ThreadRestrictions::AssertIOAllowed();
        KCHECK_EQ(0, pthread_join(thread_handle.handle_, nullptr));
    }

}  // namespace turbo
