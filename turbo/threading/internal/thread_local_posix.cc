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
