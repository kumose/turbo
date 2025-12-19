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

#include <atomic>
#include <mutex>

namespace turbo {


    template <typename T> class GetLeakySingleton {
    public:
        static std::atomic<T*> g_leaky_singleton_untyped;
        static std::once_flag g_create_leaky_singleton_once;
        static void create_leaky_singleton();
    };
    template <typename T>
    std::atomic<T*> GetLeakySingleton<T>::g_leaky_singleton_untyped = 0;

    template <typename T>
    std::once_flag GetLeakySingleton<T>::g_create_leaky_singleton_once;

    template <typename T>
    void GetLeakySingleton<T>::create_leaky_singleton() {
        T* obj = new T;
        g_leaky_singleton_untyped.store(obj, std::memory_order_relaxed);
    }

    // To get a never-deleted singleton of a type T, just call get_leaky_singleton<T>().
    // Most daemon threads or objects that need to be always-on can be created by
    // this function.
    // This function can be called safely before main() w/o initialization issues of
    // global variables.
    template <typename T>
    inline T* get_leaky_singleton() {
        T* value = GetLeakySingleton<T>::g_leaky_singleton_untyped.load(std::memory_order_acquire);
        if (value) {
            return value;
        }
        std::call_once(GetLeakySingleton<T>::g_create_leaky_singleton_once,
                     GetLeakySingleton<T>::create_leaky_singleton);
        return GetLeakySingleton<T>::g_leaky_singleton_untyped.load();
    }

    // True(non-nullptr) if the singleton is created.
    // The returned object (if not nullptr) can be used directly.
    template <typename T>
    inline T* has_leaky_singleton() {
        return GetLeakySingleton<T>::g_leaky_singleton_untyped.load(std::memory_order_acquire);
    }
}  // namespace turbo
