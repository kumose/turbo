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

#include <turbo/memory/lazy_instance.h>
#include <turbo/base/at_exit.h>
#include <turbo/threading/platform_thread.h>

namespace turbo {
    namespace internal {

        // TODO(joth): This function could be shared with Singleton, in place of its
        // WaitForInstance() call.
        bool NeedsLazyInstance(std::atomic<intptr_t> *state) {
            // Try to create the instance, if we're the first, will go from 0 to
            // kLazyInstanceStateCreating, otherwise we've already been beaten here.
            // The memory access has no memory ordering as state 0 and
            // kLazyInstanceStateCreating have no associated data (memory barriers are
            // all about ordering of memory accesses to *associated* data).
            // Caller must create instance
            intptr_t z = 0;
            if (state->compare_exchange_weak(z, kLazyInstanceStateCreating, std::memory_order_acquire)) {
                return true;
            }

            // It's either in the process of being created, or already created. Spin.
            // The load has acquire memory ordering as a thread which sees
            // state_ == STATE_CREATED needs to acquire visibility over
            // the associated data (buf_). Pairing Release_Store is in
            // CompleteLazyInstance().
            while (state->load(std::memory_order_acquire) == kLazyInstanceStateCreating) {
                PlatformThread::YieldCurrentThread();
            }
            // Someone else created the instance.
            return false;
        }

        void CompleteLazyInstance(std::atomic<intptr_t> *state,
                                  intptr_t new_instance,
                                  void *lazy_instance,
                                  void (*dtor)(void *)) {
            // Instance is created, go from CREATING to CREATED.
            // Releases visibility over private_buf_ to readers. Pairing Acquire_Load's
            // are in NeedsInstance() and Pointer().
            state->store(new_instance);

            // Make sure that the lazily instantiated object will get destroyed at exit.
            if (dtor)
                AtExitManager::RegisterCallback(dtor, lazy_instance);
        }

    }  // namespace internal
}  // namespace turbo
