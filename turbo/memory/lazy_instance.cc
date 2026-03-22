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
