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
