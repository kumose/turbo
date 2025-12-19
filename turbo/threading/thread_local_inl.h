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
