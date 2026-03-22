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
#include <turbo/threading/internal/thread_local.h>

#include <pthread.h>

#include <turbo/log/logging.h>

#if !defined(OS_ANDROID)

namespace turbo {
    namespace internal {

        // static
        void ThreadLocalPlatform::AllocateSlot(SlotType *slot) {
            int error = pthread_key_create(slot, nullptr);
            KCHECK_EQ(error, 0);
        }

        // static
        void ThreadLocalPlatform::FreeSlot(SlotType slot) {
            int error = pthread_key_delete(slot);
            DKCHECK_EQ(0, error);
        }

        // static
        void *ThreadLocalPlatform::GetValueFromSlot(SlotType slot) {
            return pthread_getspecific(slot);
        }

        // static
        void ThreadLocalPlatform::SetValueInSlot(SlotType slot, void *value) {
            int error = pthread_setspecific(slot, value);
            DKCHECK_EQ(error, 0);
        }

    }  // namespace internal
}  // namespace turbo

#endif  // !defined(OS_ANDROID)
