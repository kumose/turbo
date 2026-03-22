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
