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

#include <turbo/memory/singleton.h>
#include <turbo/threading/platform_thread.h>

namespace turbo {
    namespace internal {

        intptr_t WaitForInstance(std::atomic<intptr_t> *instance) {
            // Handle the race. Another thread beat us and either:
            // - Has the object in BeingCreated state
            // - Already has the object created...
            // We know value != nullptr.  It could be kBeingCreatedMarker, or a valid ptr.
            // Unless your constructor can be very time consuming, it is very unlikely
            // to hit this race.  When it does, we just spin and yield the thread until
            // the object has been created.
            intptr_t value;
            while (true) {
                // The load has acquire memory ordering as the thread which reads the
                // instance pointer must acquire visibility over the associated data.
                // The pairing Release_Store operation is in Singleton::get().
                value = instance->load();
                if (value != kBeingCreatedMarker)
                    break;
                PlatformThread::YieldCurrentThread();
            }
            return value;
        }

    }  // namespace internal
}  // namespace turbo
