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
// Contains utilities for making UBSan happy.

#pragma once

#include <cstring>
#include <memory>
#include <type_traits>

#include <turbo/base/macros.h>

namespace turbo::base_internal {

    constexpr uint8_t kNonNullFiller = 0;

}  // namespace turbo::base_internal

namespace turbo {

    /// \brief Returns maybe_null if not null or a non-null pointer to an arbitrary memory
    /// that shouldn't be dereferenced.
    ///
    /// Memset/Memcpy are undefined when a nullptr is passed as an argument use this utility
    /// method to wrap locations where this could happen.
    ///
    /// Note: Flatbuffers has UBSan warnings if a zero length vector is passed.
    /// https://github.com/google/flatbuffers/pull/5355 is trying to resolve
    /// them.
    template<typename T>
    inline T *make_non_null(T *maybe_null = nullptr) {
        if (TURBO_LIKELY(maybe_null != nullptr)) {
            return maybe_null;
        }

        return const_cast<T *>(reinterpret_cast<const T *>(&base_internal::kNonNullFiller));
    }

    template<typename T>
    inline std::enable_if_t<std::is_trivially_copyable_v<T>, T> safe_load_as(const uint8_t *unaligned) {
        std::remove_const_t<T> ret;
        std::memcpy(&ret, unaligned, sizeof(T));
        return ret;
    }

    template<typename T>
    inline std::enable_if_t<std::is_trivially_copyable_v<T>, T> safe_load(const T *unaligned) {
        std::remove_const_t<T> ret;
        std::memcpy(&ret, unaligned, sizeof(T));
        return ret;
    }

    template<typename U, typename T>
    inline std::enable_if_t<std::is_trivially_copyable_v<T> &&
                            std::is_trivially_copyable_v<U> && sizeof(T) == sizeof(U), U>
    safe_copy(T value) {
        std::remove_const_t<U> ret;
        std::memcpy(&ret, &value, sizeof(T));
        return ret;
    }

    template<typename T>
    inline std::enable_if_t<std::is_trivially_copyable_v<T>, void> safe_store(void *unaligned,
                                                                              T value) {
        std::memcpy(unaligned, &value, sizeof(T));
    }

}  // namespace turbo
