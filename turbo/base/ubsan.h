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
