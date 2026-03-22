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

#pragma once

namespace turbo {

    namespace detail {

        template<typename T>
        class ThreadLocalHelper {
        public:
            inline static T *get() {
                if (TURBO_LIKELY(value != nullptr)) {
                    return value;
                }
                value = new(std::nothrow) T;
                if (value != nullptr) {
                    turbo::thread_atexit(delete_object<T>, value);
                }
                return value;
            }

            static TURBO_THREAD_LOCAL T *value;
        };

        template<typename T> TURBO_THREAD_LOCAL T *ThreadLocalHelper<T>::value = nullptr;

    }  // namespace detail

    template<typename T>
    inline T *get_thread_local() {
        return detail::ThreadLocalHelper<T>::get();
    }

}  // namespace turbo
