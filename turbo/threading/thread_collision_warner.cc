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
#include <turbo/threading/thread_collision_warner.h>
#include <turbo/log/logging.h>
#include <turbo/threading/platform_thread.h>

namespace turbo {

    void DCheckAsserter::warn() {
        KLOG(FATAL) << "Thread Collision";
    }

    static int32_t CurrentThread() {
        const PlatformThreadId current_thread_id = PlatformThread::CurrentId();
        // We need to get the thread id into an atomic data type. This might be a
        // truncating conversion, but any loss-of-information just increases the
        // chance of a fault negative, not a false positive.
        const auto atomic_thread_id =
                static_cast<int32_t>(current_thread_id);

        return atomic_thread_id;
    }

    void ThreadCollisionWarner::EnterSelf() {
        // If the active thread is 0 then I'll write the current thread ID
        // if two or more threads arrive here only one will succeed to
        // write on valid_thread_id_ the current thread ID.
        auto current_thread_id = CurrentThread();
        int previous_value = 0;
        !valid_thread_id_.compare_exchange_weak(previous_value,current_thread_id);
        if (previous_value != 0 && previous_value != current_thread_id) {
            // gotcha! a thread is trying to use the same class and that is
            // not current thread.
            asserter_->warn();
        }

        counter_.fetch_add(1);
    }

    void ThreadCollisionWarner::Enter() {
        auto current_thread_id = CurrentThread();
        int old = 0;
        if (!valid_thread_id_.compare_exchange_weak(old,current_thread_id) && old!= 0) {
            // gotcha! another thread is trying to use the same class.
            asserter_->warn();
        }

        counter_.fetch_add(1);
    }

    void ThreadCollisionWarner::Leave() {
        if (counter_.fetch_sub(1) == 0) {
            valid_thread_id_.store(0);
        }

    }

}  // namespace turbo
