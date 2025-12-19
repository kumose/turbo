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
